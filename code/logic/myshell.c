/**
 * -----------------------------------------------------------------------------
 * Project: Fossil Logic
 *
 * This file is part of the Fossil Logic project, which aims to develop
 * high-performance, cross-platform applications and libraries. The code
 * contained herein is licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License. You may obtain
 * a copy of the License at:
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS, WITHOUT
 * WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied. See the
 * License for the specific language governing permissions and limitations
 * under the License.
 *
 * Author: Michael Gene Brockus (Dreamer)
 * Date: 04/05/2014
 *
 * Copyright (C) 2014-2025 Fossil Logic. All rights reserved.
 * -----------------------------------------------------------------------------
 */
#include "fossil/crabdb/myshell.h"

// ===========================================================
// Internal Structures (implementation details)
// ===========================================================

struct fossil_bluecrab_myshell_t {
    char *path;                                  // Path to DB file/directory
    fossil_bluecrab_myshell_open_options_t opts; // Open options
    bool opened;                                 // Open state

    // Core storage
    FILE *file;                                  // Primary DB file handle
    size_t file_size;                            // Current file size
    uint64_t last_commit_hash;                   // Head commit hash
    char *current_branch;                        // Checked out branch name

    // In-memory tables/indexes
    void *record_index;                          // Key -> record mapping (hash table/trie)
    void *commit_index;                          // Commit hash -> commit metadata
    void *branch_index;                          // Branch name -> head hash

    // Locks / concurrency
    bool write_locked;
    void *lock_table;                            // Named advisory locks
    int active_connections;                      // Open connection handles

    // Runtime state
    time_t opened_at;
    uint64_t txn_counter;
    uint64_t op_counter;

    // Streaming & hooks
    void *open_streams;                          // Active stream handles
    void *event_callbacks;                       // List of registered event callbacks
};


struct fossil_bluecrab_myshell_stmt_t {
    char *sql;                                   // Original SQL/FSON query
    int step_count;                              // Number of rows iterated
    bool prepared;                               // Prepared flag

    // Parsed representation
    char *table_name;
    char **columns;
    int column_count;

    // Execution state
    void *cursor;                                // Iterator over matching records
    void *last_row;                              // Pointer to last returned row
    bool eof;                                    // End-of-results indicator
};


struct fossil_bluecrab_myshell_txn_t {
    fossil_bluecrab_myshell_t *db;
    bool active;

    // Staged changes (before commit)
    void *staged_records;                        // Map: key -> record (insert/update)
    void *deleted_keys;                          // Set of keys marked for deletion
    char *txn_message;                           // Commit message buffer
    time_t started_at;                           // When txn began

    uint64_t parent_commit;                      // Base commit hash
    uint64_t working_hash;                       // Hash of working tree snapshot

    // Savepoints
    void *savepoints;                            // Stack of savepoint states
};


struct fossil_bluecrab_myshell_record_t {
    fossil_bluecrab_myshell_hash64_t hash;       // Stable 64-bit ID
    char *key;                                   // Primary key
    char *fson_text;                             // Serialized FSON payload
    time_t created_at;
    time_t modified_at;
    char *owner;

    // Derived metadata
    size_t fson_size;                            // Cached serialized size
    uint64_t version;                            // Monotonic record version
    bool tombstone;                              // Deleted marker

    // Indexing hooks
    void *secondary_indexes;                     // Map of index_name -> value
};


struct fossil_bluecrab_myshell_commit_t {
    fossil_bluecrab_myshell_hash64_t  hash;      // Commit hash
    fossil_bluecrab_myshell_hash64_t  parent_hash;
    char *author;
    char *message;
    char *timestamp_iso;

    // Change set metadata
    size_t record_count;                         // # of records changed
    fossil_bluecrab_myshell_record_t **records;  // Array of record pointers
    char **deleted_keys;                         // Keys marked as deleted
    size_t deleted_count;

    // Branch association
    char *branch;                                // Branch commit belongs to

    // Linking
    fossil_bluecrab_myshell_commit_t *parent;    // Pointer to parent commit (in-memory cache)
    fossil_bluecrab_myshell_commit_t **children; // Children commits (branch tips/merges)
    size_t child_count;
};

/* -------------------------------------------------------------------------
 * Error string mapping
 * ------------------------------------------------------------------------- */
const char *
fossil_bluecrab_myshell_errstr(fossil_bluecrab_myshell_error_t err) {
    switch (err) {
    case FOSSIL_MYSHELL_ERROR_SUCCESS: return "Success";
    case FOSSIL_MYSHELL_ERROR_INVALID_FILE: return "Invalid file";
    case FOSSIL_MYSHELL_ERROR_FILE_NOT_FOUND: return "File not found";
    case FOSSIL_MYSHELL_ERROR_IO: return "I/O error";
    case FOSSIL_MYSHELL_ERROR_INVALID_QUERY: return "Invalid query";
    case FOSSIL_MYSHELL_ERROR_CONCURRENCY: return "Concurrency error";
    case FOSSIL_MYSHELL_ERROR_NOT_FOUND: return "Not found";
    case FOSSIL_MYSHELL_ERROR_PERMISSION_DENIED: return "Permission denied";
    case FOSSIL_MYSHELL_ERROR_CORRUPTED: return "Data corrupted";
    case FOSSIL_MYSHELL_ERROR_OUT_OF_MEMORY: return "Out of memory";
    case FOSSIL_MYSHELL_ERROR_UNSUPPORTED: return "Unsupported operation";
    case FOSSIL_MYSHELL_ERROR_LOCKED: return "Resource locked";
    case FOSSIL_MYSHELL_ERROR_TIMEOUT: return "Operation timed out";
    case FOSSIL_MYSHELL_ERROR_ALREADY_EXISTS: return "Already exists";
    case FOSSIL_MYSHELL_ERROR_BACKUP_FAILED: return "Backup failed";
    case FOSSIL_MYSHELL_ERROR_PARSE_FAILED: return "Parse failed";
    case FOSSIL_MYSHELL_ERROR_RESTORE_FAILED: return "Restore failed";
    case FOSSIL_MYSHELL_ERROR_LOCK_FAILED: return "Lock failed";
    case FOSSIL_MYSHELL_ERROR_UNKNOWN:
    default: return "Unknown error";
    }
}

/* -------------------------------------------------------------------------
 * Utility: safe strdup (C11 portable)
 * ------------------------------------------------------------------------- */
static char *myshell_strdup(const char *s) {
    if (!s) return NULL;
    size_t n = strlen(s) + 1;
    char *r = (char *)malloc(n);
    if (r) memcpy(r, s, n);
    return r;
}

/* -------------------------------------------------------------------------
 * Open/create/close
 * ------------------------------------------------------------------------- */
fossil_bluecrab_myshell_t *
fossil_bluecrab_myshell_open(const char *path,
                             const fossil_bluecrab_myshell_open_options_t *opts,
                             fossil_bluecrab_myshell_error_t *out_err) {
    if (!path) {
        if (out_err) *out_err = FOSSIL_MYSHELL_ERROR_INVALID_FILE;
        return NULL;
    }

    fossil_bluecrab_myshell_t *db = calloc(1, sizeof(*db));
    if (!db) {
        if (out_err) *out_err = FOSSIL_MYSHELL_ERROR_OUT_OF_MEMORY;
        return NULL;
    }

    db->path = myshell_strdup(path);
    if (opts) db->opts = *opts;
    else {
        memset(&db->opts, 0, sizeof(db->opts));
        db->opts.page_size = 4096;
        db->opts.max_connections = 4;
    }
    db->opened = true;

    if (out_err) *out_err = FOSSIL_MYSHELL_ERROR_SUCCESS;
    return db;
}

fossil_bluecrab_myshell_t *
fossil_bluecrab_myshell_create(const char *path,
                               const fossil_bluecrab_myshell_open_options_t *opts,
                               fossil_bluecrab_myshell_error_t *out_err) {
    if (!path) {
        if (out_err) *out_err = FOSSIL_MYSHELL_ERROR_INVALID_FILE;
        return NULL;
    }

    // For the stub we behave same as open (no on-disk initialization).
    fossil_bluecrab_myshell_t *db = fossil_bluecrab_myshell_open(path, opts, out_err);
    if (!db) {
        return NULL;
    }
    // Real impl: create directory/files, initialize chain root, write metadata.
    if (out_err) *out_err = FOSSIL_MYSHELL_ERROR_SUCCESS;
    return db;
}

fossil_bluecrab_myshell_error_t
fossil_bluecrab_myshell_close(fossil_bluecrab_myshell_t *db) {
    if (!db) return FOSSIL_MYSHELL_ERROR_INVALID_FILE;

    free(db->path);
    // Free other internals if present
    free(db);
    return FOSSIL_MYSHELL_ERROR_SUCCESS;
}

/* -------------------------------------------------------------------------
 * Basic SQL-like API (stubs)
 * ------------------------------------------------------------------------- */

fossil_bluecrab_myshell_error_t
fossil_bluecrab_myshell_exec_sql(fossil_bluecrab_myshell_t *db,
                                 const char *sql,
                                 fossil_bluecrab_myshell_error_t *out_err) {
    (void)db;
    (void)sql;
    if (out_err) *out_err = FOSSIL_MYSHELL_ERROR_UNSUPPORTED;
    return FOSSIL_MYSHELL_ERROR_UNSUPPORTED;
}

fossil_bluecrab_myshell_stmt_t *
fossil_bluecrab_myshell_prepare(fossil_bluecrab_myshell_t *db,
                                const char *sql,
                                fossil_bluecrab_myshell_error_t *out_err) {
    if (!db || !sql) {
        if (out_err) *out_err = FOSSIL_MYSHELL_ERROR_INVALID_QUERY;
        return NULL;
    }
    fossil_bluecrab_myshell_stmt_t *stmt = calloc(1, sizeof(*stmt));
    if (!stmt) {
        if (out_err) *out_err = FOSSIL_MYSHELL_ERROR_OUT_OF_MEMORY;
        return NULL;
    }
    stmt->sql = myshell_strdup(sql);
    stmt->step_count = 0;
    if (out_err) *out_err = FOSSIL_MYSHELL_ERROR_SUCCESS;
    return stmt;
}

fossil_bluecrab_myshell_error_t
fossil_bluecrab_myshell_stmt_bind_null(fossil_bluecrab_myshell_stmt_t *stmt, int idx) {
    (void)stmt; (void)idx;
    return FOSSIL_MYSHELL_ERROR_UNSUPPORTED;
}

fossil_bluecrab_myshell_error_t
fossil_bluecrab_myshell_stmt_bind_i64(fossil_bluecrab_myshell_stmt_t *stmt, int idx, int64_t v) {
    (void)stmt; (void)idx; (void)v;
    return FOSSIL_MYSHELL_ERROR_UNSUPPORTED;
}

fossil_bluecrab_myshell_error_t
fossil_bluecrab_myshell_stmt_bind_u64(fossil_bluecrab_myshell_stmt_t *stmt, int idx, uint64_t v)
{
    (void)stmt; (void)idx; (void)v;
    return FOSSIL_MYSHELL_ERROR_UNSUPPORTED;
}

fossil_bluecrab_myshell_error_t
fossil_bluecrab_myshell_stmt_bind_double(fossil_bluecrab_myshell_stmt_t *stmt, int idx, double v) {
    (void)stmt; (void)idx; (void)v;
    return FOSSIL_MYSHELL_ERROR_UNSUPPORTED;
}

fossil_bluecrab_myshell_error_t
fossil_bluecrab_myshell_stmt_bind_text(fossil_bluecrab_myshell_stmt_t *stmt, int idx, const char *text) {
    (void)stmt; (void)idx; (void)text;
    return FOSSIL_MYSHELL_ERROR_UNSUPPORTED;
}

fossil_bluecrab_myshell_error_t
fossil_bluecrab_myshell_stmt_step(fossil_bluecrab_myshell_stmt_t *stmt) {
    if (!stmt) return FOSSIL_MYSHELL_ERROR_INVALID_QUERY;
    // Minimal behavior: return EOF (NOT_FOUND) after one step in stub.
    if (stmt->step_count == 0) {
        stmt->step_count++;
        return FOSSIL_MYSHELL_ERROR_SUCCESS; // pretend a row exists once
    } else {
        return FOSSIL_MYSHELL_ERROR_NOT_FOUND; // EOF-like
    }
}

const char *
fossil_bluecrab_myshell_stmt_column_text(fossil_bluecrab_myshell_stmt_t *stmt, int col) {
    (void)stmt; (void)col;
    return NULL;
}

int64_t
fossil_bluecrab_myshell_stmt_column_i64(fossil_bluecrab_myshell_stmt_t *stmt, int col) {
    (void)stmt; (void)col;
    return 0;
}

double
fossil_bluecrab_myshell_stmt_column_double(fossil_bluecrab_myshell_stmt_t *stmt, int col) {
    (void)stmt; (void)col;
    return 0.0;
}

fossil_bluecrab_myshell_error_t
fossil_bluecrab_myshell_stmt_reset(fossil_bluecrab_myshell_stmt_t *stmt) {
    if (!stmt) return FOSSIL_MYSHELL_ERROR_INVALID_QUERY;
    stmt->step_count = 0;
    return FOSSIL_MYSHELL_ERROR_SUCCESS;
}

fossil_bluecrab_myshell_error_t
fossil_bluecrab_myshell_stmt_finalize(fossil_bluecrab_myshell_stmt_t *stmt) {
    if (!stmt) return FOSSIL_MYSHELL_ERROR_INVALID_QUERY;
    free(stmt->sql);
    free(stmt);
    return FOSSIL_MYSHELL_ERROR_SUCCESS;
}

/* -------------------------------------------------------------------------
 * Transactions (stubs)
 * ------------------------------------------------------------------------- */
fossil_bluecrab_myshell_txn_t *
fossil_bluecrab_myshell_txn_begin(fossil_bluecrab_myshell_t *db,
                                  fossil_bluecrab_myshell_error_t *out_err) {
    if (!db) {
        if (out_err) *out_err = FOSSIL_MYSHELL_ERROR_INVALID_FILE;
        return NULL;
    }
    fossil_bluecrab_myshell_txn_t *txn = calloc(1, sizeof(*txn));
    if (!txn) {
        if (out_err) *out_err = FOSSIL_MYSHELL_ERROR_OUT_OF_MEMORY;
        return NULL;
    }
    txn->db = db;
    txn->active = true;
    if (out_err) *out_err = FOSSIL_MYSHELL_ERROR_SUCCESS;
    return txn;
}

fossil_bluecrab_myshell_error_t
fossil_bluecrab_myshell_txn_commit(fossil_bluecrab_myshell_txn_t *txn,
                                   const char *message,
                                   fossil_bluecrab_myshell_hash64_t *out_commit_hash) {
    if (!txn) return FOSSIL_MYSHELL_ERROR_INVALID_QUERY;
    if (!txn->active) return FOSSIL_MYSHELL_ERROR_INVALID_QUERY;

    // Real impl: write commit object into chain, compute hash.
    txn->active = false;
    if (out_commit_hash) *out_commit_hash = 0xDEADBEEFu; // placeholder
    (void)message;
    return FOSSIL_MYSHELL_ERROR_SUCCESS;
}

fossil_bluecrab_myshell_error_t
fossil_bluecrab_myshell_txn_rollback(fossil_bluecrab_myshell_txn_t *txn) {
    if (!txn) return FOSSIL_MYSHELL_ERROR_INVALID_QUERY;
    txn->active = false;
    free(txn);
    return FOSSIL_MYSHELL_ERROR_SUCCESS;
}

/* -------------------------------------------------------------------------
 * Branch / commit shim functions (stubs)
 * ------------------------------------------------------------------------- */
fossil_bluecrab_myshell_error_t
fossil_bluecrab_myshell_branch_create(fossil_bluecrab_myshell_t *db,
                                      const char *branch_name,
                                      const fossil_bluecrab_myshell_hash64_t *base_commit,
                                      fossil_bluecrab_myshell_error_t *out_err) {
    (void)db; (void)branch_name; (void)base_commit;
    if (out_err) *out_err = FOSSIL_MYSHELL_ERROR_UNSUPPORTED;
    return FOSSIL_MYSHELL_ERROR_UNSUPPORTED;
}

char **
fossil_bluecrab_myshell_branch_list(fossil_bluecrab_myshell_t *db, size_t *out_count) {
    (void)db;
    if (out_count) *out_count = 0;
    return NULL;
}

fossil_bluecrab_myshell_error_t
fossil_bluecrab_myshell_checkout(fossil_bluecrab_myshell_t *db,
                                 const char *branch_or_hash,
                                 fossil_bluecrab_myshell_error_t *out_err) {
    (void)db; (void)branch_or_hash;
    if (out_err) *out_err = FOSSIL_MYSHELL_ERROR_UNSUPPORTED;
    return FOSSIL_MYSHELL_ERROR_UNSUPPORTED;
}

fossil_bluecrab_myshell_error_t
fossil_bluecrab_myshell_merge(fossil_bluecrab_myshell_t *db,
                              const char *source_branch_or_hash,
                              const char *target_branch_or_hash,
                              fossil_bluecrab_myshell_merge_result_t *out_result) {
    (void)db; (void)source_branch_or_hash; (void)target_branch_or_hash;
    if (out_result) {
        out_result->conflicts = false;
        out_result->conflict_report = NULL;
    }
    return FOSSIL_MYSHELL_ERROR_UNSUPPORTED;
}

/* -------------------------------------------------------------------------
 * Record-level / FSON helpers (stubs)
 * ------------------------------------------------------------------------- */
fossil_bluecrab_myshell_error_t
fossil_bluecrab_myshell_put_record(fossil_bluecrab_myshell_t *db,
                                   const fossil_bluecrab_myshell_record_t *rec,
                                   fossil_bluecrab_myshell_hash64_t *out_hash) {
    (void)db; (void)rec;
    if (out_hash) *out_hash = 0;
    return FOSSIL_MYSHELL_ERROR_UNSUPPORTED;
}

fossil_bluecrab_myshell_record_t *
fossil_bluecrab_myshell_get_record(fossil_bluecrab_myshell_t *db,
                                   const char *key,
                                   fossil_bluecrab_myshell_error_t *out_err) {
    (void)db; (void)key;
    if (out_err) *out_err = FOSSIL_MYSHELL_ERROR_NOT_FOUND;
    return NULL;
}

fossil_bluecrab_myshell_error_t
fossil_bluecrab_myshell_delete_record(fossil_bluecrab_myshell_t *db,
                                      const char *key,
                                      fossil_bluecrab_myshell_error_t *out_err) {
    (void)db; (void)key;
    if (out_err) *out_err = FOSSIL_MYSHELL_ERROR_UNSUPPORTED;
    return FOSSIL_MYSHELL_ERROR_UNSUPPORTED;
}

fossil_bluecrab_myshell_error_t
fossil_bluecrab_myshell_iter_records(fossil_bluecrab_myshell_t *db,
                                     const char *prefix_filter,
                                     fossil_bluecrab_myshell_record_iter_cb cb,
                                     void *user) {
    (void)db; (void)prefix_filter; (void)cb; (void)user;
    return FOSSIL_MYSHELL_ERROR_UNSUPPORTED;
}

/* Parse/serialize/free FSON values (stubs) */
fossil_bluecrab_myshell_fson_value_t *
fossil_bluecrab_myshell_fson_parse_value(const char *text,
                                         fossil_bluecrab_myshell_error_t *out_err) {
    (void)text;
    if (out_err) *out_err = FOSSIL_MYSHELL_ERROR_UNSUPPORTED;
    return NULL;
}

char *
fossil_bluecrab_myshell_fson_serialize_value(const fossil_bluecrab_myshell_fson_value_t *val,
                                             fossil_bluecrab_myshell_error_t *out_err) {
    (void)val;
    if (out_err) *out_err = FOSSIL_MYSHELL_ERROR_UNSUPPORTED;
    return NULL;
}

void
fossil_bluecrab_myshell_fson_free_value(fossil_bluecrab_myshell_fson_value_t *val) {
    if (!val) return;
    // Free inner allocations depending on type — stub does nothing.
    free(val);
}

char *
fossil_bluecrab_myshell_fson_to_sql_insert(const char *table_name,
                                           const char *fson_object_text,
                                           fossil_bluecrab_myshell_error_t *out_err) {
    (void)table_name; (void)fson_object_text;
    if (out_err) *out_err = FOSSIL_MYSHELL_ERROR_UNSUPPORTED;
    return NULL;
}

/* -------------------------------------------------------------------------
 * Hash utilities
 * ------------------------------------------------------------------------- */

/* Reuse the mix64 algorithm from your header as a local function here */
static inline uint64_t local_mix64(uint64_t x) {
    x ^= x >> 30;
    x *= 0xbf58476d1ce4e5b9ULL;
    x ^= x >> 27;
    x *= 0x94d049bb133111ebULL;
    x ^= x >> 31;
    return x;
}

fossil_bluecrab_myshell_hash64_t
fossil_bluecrab_myshell_compute_record_hash(const char *key,
                                            const char *fson_text,
                                            const char *timestamp_iso,
                                            uint64_t seed) {
    uint64_t h = seed ? seed : 0x9e3779b97f4a7c15ULL;
    if (key) {
        for (const unsigned char *p = (const unsigned char *)key; *p; ++p) {
            h = local_mix64(h ^ (uint64_t)(*p));
        }
    }
    if (fson_text) {
        for (const unsigned char *p = (const unsigned char *)fson_text; *p; ++p) {
            h = local_mix64(h ^ (uint64_t)(*p));
        }
    }
    if (timestamp_iso) {
        for (const unsigned char *p = (const unsigned char *)timestamp_iso; *p; ++p) {
            h = local_mix64(h ^ (uint64_t)(*p));
        }
    }
    return (fossil_bluecrab_myshell_hash64_t)h;
}

/* -------------------------------------------------------------------------
 * Indexing / search (stubs)
 * ------------------------------------------------------------------------- */
fossil_bluecrab_myshell_error_t
fossil_bluecrab_myshell_index_create(fossil_bluecrab_myshell_t *db,
                                     const char *index_name,
                                     const char *fson_path) {
    (void)db; (void)index_name; (void)fson_path;
    return FOSSIL_MYSHELL_ERROR_UNSUPPORTED;
}

char **
fossil_bluecrab_myshell_index_query(fossil_bluecrab_myshell_t *db,
                                    const char *index_name,
                                    const char *match_value,
                                    size_t *out_count) {
    (void)db; (void)index_name; (void)match_value;
    if (out_count) *out_count = 0;
    return NULL;
}

/* -------------------------------------------------------------------------
 * Backups / restore / import / export (stubs)
 * ------------------------------------------------------------------------- */

fossil_bluecrab_myshell_error_t
fossil_bluecrab_myshell_backup(fossil_bluecrab_myshell_t *db,
                               const char *backup_path) {
    (void)db; (void)backup_path;
    return FOSSIL_MYSHELL_ERROR_UNSUPPORTED;
}

fossil_bluecrab_myshell_error_t
fossil_bluecrab_myshell_restore(const char *backup_path,
                                const char *target_path) {
    (void)backup_path; (void)target_path;
    return FOSSIL_MYSHELL_ERROR_UNSUPPORTED;
}

fossil_bluecrab_myshell_error_t
fossil_bluecrab_myshell_import_presidents_csv(fossil_bluecrab_myshell_t *db,
                                              const char *csv_path,
                                              const char *table_name,
                                              size_t *out_rows_imported) {
    (void)db; (void)csv_path; (void)table_name;
    if (out_rows_imported) *out_rows_imported = 0;
    return FOSSIL_MYSHELL_ERROR_UNSUPPORTED;
}

fossil_bluecrab_myshell_error_t
fossil_bluecrab_myshell_export_presidents_csv(fossil_bluecrab_myshell_t *db,
                                              const char *table_name,
                                              const char *csv_path) {
    (void)db; (void)table_name; (void)csv_path;
    return FOSSIL_MYSHELL_ERROR_UNSUPPORTED;
}

fossil_bluecrab_myshell_error_t
fossil_bluecrab_myshell_create_president_schema(fossil_bluecrab_myshell_t *db,
                                                const char *table_name) {
    (void)db; (void)table_name;
    return FOSSIL_MYSHELL_ERROR_UNSUPPORTED;
}

/* -------------------------------------------------------------------------
 * Diagnostics / compact (stubs)
 * ------------------------------------------------------------------------- */
fossil_bluecrab_myshell_error_t
fossil_bluecrab_myshell_integrity_check(fossil_bluecrab_myshell_t *db,
                                        char **out_report) {
    (void)db;
    if (out_report) {
        *out_report = myshell_strdup("Integrity check not implemented in stub.");
    }
    return FOSSIL_MYSHELL_ERROR_UNSUPPORTED;
}

fossil_bluecrab_myshell_error_t
fossil_bluecrab_myshell_compact(fossil_bluecrab_myshell_t *db,
                                fossil_bluecrab_myshell_error_t *out_err) {
    (void)db;
    if (out_err) *out_err = FOSSIL_MYSHELL_ERROR_UNSUPPORTED;
    return FOSSIL_MYSHELL_ERROR_UNSUPPORTED;
}

/* -------------------------------------------------------------------------
 * Blob store / load / free
 * ------------------------------------------------------------------------- */
fossil_bluecrab_myshell_hash64_t
fossil_bluecrab_myshell_store_blob(fossil_bluecrab_myshell_t *db,
                                   const void *data, size_t n,
                                   fossil_bluecrab_myshell_error_t *out_err) {
    (void)db; (void)data; (void)n;
    if (out_err) *out_err = FOSSIL_MYSHELL_ERROR_UNSUPPORTED;
    return 0;
}

void *
fossil_bluecrab_myshell_load_blob(fossil_bluecrab_myshell_t *db,
                                  fossil_bluecrab_myshell_hash64_t blob_hash,
                                  size_t *out_size,
                                  fossil_bluecrab_myshell_error_t *out_err) {
    (void)db; (void)blob_hash;
    if (out_size) *out_size = 0;
    if (out_err) *out_err = FOSSIL_MYSHELL_ERROR_UNSUPPORTED;
    return NULL;
}

void
fossil_bluecrab_myshell_record_free(fossil_bluecrab_myshell_record_t *rec) {
    if (!rec) return;
    free(rec->key);
    free(rec->fson_text);
    free(rec->owner);
    free(rec);
}

/* -------------------------------------------------------------------------
 * Time / duration helpers (simple stub implementations)
 * ------------------------------------------------------------------------- */
time_t
fossil_bluecrab_myshell_parse_iso8601(const char *iso, fossil_bluecrab_myshell_error_t *out_err) {
    if (!iso) {
        if (out_err) *out_err = FOSSIL_MYSHELL_ERROR_INVALID_QUERY;
        return (time_t)-1;
    }
    // Very simple fallback: try parse as seconds since epoch string
    char *endptr = NULL;
    long long seconds = strtoll(iso, &endptr, 10);
    if (endptr && *endptr == '\0') {
        if (out_err) *out_err = FOSSIL_MYSHELL_ERROR_SUCCESS;
        return (time_t)seconds;
    }
    if (out_err) *out_err = FOSSIL_MYSHELL_ERROR_UNSUPPORTED;
    return (time_t)-1;
}

char *
fossil_bluecrab_myshell_format_iso8601(time_t t) {
    struct tm tm;
    if (gmtime_r(&t, &tm) == NULL) return NULL;
    char buf[64];
    if (strftime(buf, sizeof(buf), "%Y-%m-%dT%H:%M:%SZ", &tm) == 0) return NULL;
    return myshell_strdup(buf);
}

int64_t
fossil_bluecrab_myshell_parse_duration_seconds(const char *duration_str,
                                               fossil_bluecrab_myshell_error_t *out_err) {
    if (!duration_str) {
        if (out_err) *out_err = FOSSIL_MYSHELL_ERROR_INVALID_QUERY;
        return -1;
    }
    // Very simple parser: supports suffixes s, m, h, d
    char *endptr = NULL;
    long long val = strtoll(duration_str, &endptr, 10);
    if (!endptr) {
        if (out_err) *out_err = FOSSIL_MYSHELL_ERROR_UNSUPPORTED;
        return -1;
    }
    int64_t mult = 1;
    if (*endptr == 's' || *endptr == '\0') mult = 1;
    else if (*endptr == 'm') mult = 60;
    else if (*endptr == 'h') mult = 3600;
    else if (*endptr == 'd') mult = 3600 * 24;
    else {
        if (out_err) *out_err = FOSSIL_MYSHELL_ERROR_UNSUPPORTED;
        return -1;
    }
    if (out_err) *out_err = FOSSIL_MYSHELL_ERROR_SUCCESS;
    return val * mult;
}

/* -------------------------------------------------------------------------
 * Bootstrapping / schema / builtin functions (stubs)
 * ------------------------------------------------------------------------- */
fossil_bluecrab_myshell_error_t
fossil_bluecrab_myshell_init_builtin_functions(fossil_bluecrab_myshell_t *db) {
    (void)db;
    return FOSSIL_MYSHELL_ERROR_UNSUPPORTED;
}

fossil_bluecrab_myshell_error_t
fossil_bluecrab_myshell_load_datafile(fossil_bluecrab_myshell_t *db,
                                      const char *datafile_path,
                                      fossil_bluecrab_myshell_error_t *out_err) {
    (void)db; (void)datafile_path;
    if (out_err) *out_err = FOSSIL_MYSHELL_ERROR_UNSUPPORTED;
    return FOSSIL_MYSHELL_ERROR_UNSUPPORTED;
}

/* -------------------------------------------------------------------------
 * Debug / introspection (stubs)
 * ------------------------------------------------------------------------- */
char *
fossil_bluecrab_myshell_dump_head(const fossil_bluecrab_myshell_t *db) {
    if (!db) return NULL;
    // Return a small JSON-ish description — caller frees
    const char *tmpl = "{ \"path\": \"%s\", \"opened\": %d }";
    size_t need = snprintf(NULL, 0, tmpl, db->path ? db->path : "(null)", db->opened) + 1;
    char *s = malloc(need);
    if (!s) return NULL;
    snprintf(s, need, tmpl, db->path ? db->path : "(null)", db->opened);
    return s;
}

char *
fossil_bluecrab_myshell_stats(const fossil_bluecrab_myshell_t *db) {
    if (!db) return NULL;
    return myshell_strdup("Stats not implemented in stub.");
}

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
#ifndef FOSSIL_CRABDB_MYSHELL_H
#define FOSSIL_CRABDB_MYSHELL_H

#include <inttypes.h>
#include <stdbool.h>
#include <strings.h>
#include <string.h>
#include <stddef.h>
#include <stdlib.h>
#include <stdint.h>
#include <sys/stat.h>   // for file size
#include <errno.h>
#include <stdio.h>
#include <ctype.h>
#include <time.h>

#ifdef __cplusplus
extern "C" {
#endif

// *****************************************************************************
// Enumerations for Data Types and Attributes
// *****************************************************************************

// ===========================================================
// MyShell Error Codes
// ===========================================================
typedef enum {
    FOSSIL_MYSHELL_ERROR_SUCCESS = 0,
    FOSSIL_MYSHELL_ERROR_INVALID_FILE,
    FOSSIL_MYSHELL_ERROR_FILE_NOT_FOUND,
    FOSSIL_MYSHELL_ERROR_IO,
    FOSSIL_MYSHELL_ERROR_INVALID_QUERY,
    FOSSIL_MYSHELL_ERROR_CONCURRENCY,
    FOSSIL_MYSHELL_ERROR_NOT_FOUND,
    FOSSIL_MYSHELL_ERROR_PERMISSION_DENIED,
    FOSSIL_MYSHELL_ERROR_CORRUPTED,
    FOSSIL_MYSHELL_ERROR_OUT_OF_MEMORY,
    FOSSIL_MYSHELL_ERROR_UNSUPPORTED,
    FOSSIL_MYSHELL_ERROR_LOCKED,
    FOSSIL_MYSHELL_ERROR_TIMEOUT,
    FOSSIL_MYSHELL_ERROR_ALREADY_EXISTS,
    FOSSIL_MYSHELL_ERROR_BACKUP_FAILED,
    FOSSIL_MYSHELL_ERROR_PARSE_FAILED,
    FOSSIL_MYSHELL_ERROR_RESTORE_FAILED,
    FOSSIL_MYSHELL_ERROR_LOCK_FAILED,
    FOSSIL_MYSHELL_ERROR_UNKNOWN
} fossil_bluecrab_myshell_error_t;

// ============================================================================
// FSON v2 compatible value representation (local to MyShell)
// ============================================================================
typedef enum {
    MYSHELL_FSON_TYPE_NULL = 0,
    MYSHELL_FSON_TYPE_BOOL,

    // Scalars
    MYSHELL_FSON_TYPE_I8,
    MYSHELL_FSON_TYPE_I16,
    MYSHELL_FSON_TYPE_I32,
    MYSHELL_FSON_TYPE_I64,
    MYSHELL_FSON_TYPE_U8,
    MYSHELL_FSON_TYPE_U16,
    MYSHELL_FSON_TYPE_U32,
    MYSHELL_FSON_TYPE_U64,
    MYSHELL_FSON_TYPE_F32,
    MYSHELL_FSON_TYPE_F64,

    // Literals
    MYSHELL_FSON_TYPE_OCT,
    MYSHELL_FSON_TYPE_HEX,
    MYSHELL_FSON_TYPE_BIN,

    // Strings
    MYSHELL_FSON_TYPE_CHAR,
    MYSHELL_FSON_TYPE_CSTR,

    // Composite
    MYSHELL_FSON_TYPE_ARRAY,
    MYSHELL_FSON_TYPE_OBJECT,

    // v2 Additions
    MYSHELL_FSON_TYPE_ENUM,
    MYSHELL_FSON_TYPE_DATETIME,
    MYSHELL_FSON_TYPE_DURATION
} fossil_bluecrab_myshell_fson_type_t;

// -------------------------------
// Forward declarations / common
// -------------------------------
typedef struct fossil_bluecrab_myshell_t fossil_bluecrab_myshell_t;
typedef struct fossil_bluecrab_myshell_txn_t fossil_bluecrab_myshell_txn_t;
typedef struct fossil_bluecrab_myshell_stmt_t fossil_bluecrab_myshell_stmt_t;
typedef struct fossil_bluecrab_myshell_commit_t fossil_bluecrab_myshell_commit_t;
typedef struct fossil_bluecrab_myshell_record_t fossil_bluecrab_myshell_record_t;
typedef uint64_t fossil_bluecrab_myshell_hash64_t;

typedef struct {
    fossil_bluecrab_myshell_fson_type_t type;
    union {
        bool b;
        int8_t   i8;
        int16_t  i16;
        int32_t  i32;
        int64_t  i64;
        uint8_t  u8;
        uint16_t u16;
        uint32_t u32;
        uint64_t u64;
        float    f32;
        double   f64;
        char    *oct;
        char    *hex;
        char    *bin;
        char     c;
        char    *cstr;
        char    *array;
        char    *object;
        char    *enum_symbol;
        char    *datetime;
        char    *duration;
    } as;
} fossil_bluecrab_myshell_fson_value_t;


// -------------------------------
// Core database handle + options
// -------------------------------
typedef struct {
    const char *path;           // path to DB file / directory
    uint64_t    flags;          // open flags (bitmask, e.g. readonly, create)
    uint64_t    hash_seed;      // seed for hashing
    size_t      page_size;      // for internal storage tuning
    int         max_connections;
} fossil_bluecrab_myshell_open_options_t;

/** Open an existing DB. */
fossil_bluecrab_myshell_t *
fossil_bluecrab_myshell_open(const char *path,
                             const fossil_bluecrab_myshell_open_options_t *opts,
                             fossil_bluecrab_myshell_error_t *out_err);

/** Create a new DB (initializes git-like chain root, indexes, schema). */
fossil_bluecrab_myshell_t *
fossil_bluecrab_myshell_create(const char *path,
                               const fossil_bluecrab_myshell_open_options_t *opts,
                               fossil_bluecrab_myshell_error_t *out_err);

/** Close/free handle (flush + close files). */
fossil_bluecrab_myshell_error_t
fossil_bluecrab_myshell_close(fossil_bluecrab_myshell_t *db);

/** Map error code to human readable string (thread-safe static buffer not guaranteed). */
const char *
fossil_bluecrab_myshell_errstr(fossil_bluecrab_myshell_error_t err);


// -------------------------------
// Basic SQL-like API
// -------------------------------

/** Execute a SQL string directly. (SQL is a subset / dialect mapped to FSON-backed tables.) */
fossil_bluecrab_myshell_error_t
fossil_bluecrab_myshell_exec_sql(fossil_bluecrab_myshell_t *db,
                                 const char *sql,
                                 fossil_bluecrab_myshell_error_t *out_err);

/** Prepare/compile a statement. */
fossil_bluecrab_myshell_stmt_t *
fossil_bluecrab_myshell_prepare(fossil_bluecrab_myshell_t *db,
                                const char *sql,
                                fossil_bluecrab_myshell_error_t *out_err);

/** Bind helpers (index 1..N) */
fossil_bluecrab_myshell_error_t
fossil_bluecrab_myshell_stmt_bind_null(fossil_bluecrab_myshell_stmt_t *stmt, int idx);
fossil_bluecrab_myshell_error_t
fossil_bluecrab_myshell_stmt_bind_i64(fossil_bluecrab_myshell_stmt_t *stmt, int idx, int64_t v);
fossil_bluecrab_myshell_error_t
fossil_bluecrab_myshell_stmt_bind_u64(fossil_bluecrab_myshell_stmt_t *stmt, int idx, uint64_t v);
fossil_bluecrab_myshell_error_t
fossil_bluecrab_myshell_stmt_bind_double(fossil_bluecrab_myshell_stmt_t *stmt, int idx, double v);
fossil_bluecrab_myshell_error_t
fossil_bluecrab_myshell_stmt_bind_text(fossil_bluecrab_myshell_stmt_t *stmt, int idx, const char *text);

/** Step/iterate a statement; returns FOSSIL_MYSHELL_ERROR_SUCCESS on success, or error/EOF. */
fossil_bluecrab_myshell_error_t
fossil_bluecrab_myshell_stmt_step(fossil_bluecrab_myshell_stmt_t *stmt);

/** Retrieve column as text / i64 / double */
const char *
fossil_bluecrab_myshell_stmt_column_text(fossil_bluecrab_myshell_stmt_t *stmt, int col);
int64_t
fossil_bluecrab_myshell_stmt_column_i64(fossil_bluecrab_myshell_stmt_t *stmt, int col);
double
fossil_bluecrab_myshell_stmt_column_double(fossil_bluecrab_myshell_stmt_t *stmt, int col);

/** Reset & finalize statement. */
fossil_bluecrab_myshell_error_t
fossil_bluecrab_myshell_stmt_reset(fossil_bluecrab_myshell_stmt_t *stmt);
fossil_bluecrab_myshell_error_t
fossil_bluecrab_myshell_stmt_finalize(fossil_bluecrab_myshell_stmt_t *stmt);


// -------------------------------
// Transactions
// -------------------------------

/** Begin/commit/rollback. Transactions map to git-like working tree changes until commit. */
fossil_bluecrab_myshell_txn_t *
fossil_bluecrab_myshell_txn_begin(fossil_bluecrab_myshell_t *db,
                                  fossil_bluecrab_myshell_error_t *out_err);

fossil_bluecrab_myshell_error_t
fossil_bluecrab_myshell_txn_commit(fossil_bluecrab_myshell_txn_t *txn,
                                   const char *message,           // commit message
                                   fossil_bluecrab_myshell_hash64_t *out_commit_hash);

fossil_bluecrab_myshell_error_t
fossil_bluecrab_myshell_txn_rollback(fossil_bluecrab_myshell_txn_t *txn);

// -------------------------------
// Git-like chain & commits
// -------------------------------

/** Commit metadata structure. */
struct fossil_bluecrab_myshell_commit_t {
    fossil_bluecrab_myshell_hash64_t  hash;
    fossil_bluecrab_myshell_hash64_t  parent_hash;
    char                            *author;
    char                            *message;
    char                            *timestamp_iso; /* e.g. "2025-09-30T12:00:00Z" */
    // reserved: pointers to serialized change-set, indexes etc
};

/** Create a branch reference. */
fossil_bluecrab_myshell_error_t
fossil_bluecrab_myshell_branch_create(fossil_bluecrab_myshell_t *db,
                                      const char *branch_name,
                                      const fossil_bluecrab_myshell_hash64_t *base_commit,
                                      fossil_bluecrab_myshell_error_t *out_err);

/** List branches (caller frees array). */
char **
fossil_bluecrab_myshell_branch_list(fossil_bluecrab_myshell_t *db, size_t *out_count);

/** Checkout branch / commit (detached allowed). */
fossil_bluecrab_myshell_error_t
fossil_bluecrab_myshell_checkout(fossil_bluecrab_myshell_t *db,
                                 const char *branch_or_hash,
                                 fossil_bluecrab_myshell_error_t *out_err);

/** Merge two commits (fast-forward or three-way). Returns conflict detail if any. */
typedef struct {
    bool        conflicts;
    char       *conflict_report; // textual report; must be freed by caller
} fossil_bluecrab_myshell_merge_result_t;

fossil_bluecrab_myshell_error_t
fossil_bluecrab_myshell_merge(fossil_bluecrab_myshell_t *db,
                              const char *source_branch_or_hash,
                              const char *target_branch_or_hash,
                              fossil_bluecrab_myshell_merge_result_t *out_result);


// -------------------------------
// Record-level / FSON helpers
// -------------------------------

/** Record structure (opaque pointer returned to caller). */
struct fossil_bluecrab_myshell_record_t {
    fossil_bluecrab_myshell_hash64_t hash;    // stable 64-bit identity
    char   *key;                            // primary key
    char   *fson_text;                      // serialized FSON (v2) object
    time_t  created_at;
    time_t  modified_at;
    char   *owner;                          // owner principal
};

/** Create or update a record in the current branch / working tree. */
fossil_bluecrab_myshell_error_t
fossil_bluecrab_myshell_put_record(fossil_bluecrab_myshell_t *db,
                                   const fossil_bluecrab_myshell_record_t *rec,
                                   fossil_bluecrab_myshell_hash64_t *out_hash);

/** Read a record by key (returns malloc'd record; caller frees). */
fossil_bluecrab_myshell_record_t *
fossil_bluecrab_myshell_get_record(fossil_bluecrab_myshell_t *db,
                                   const char *key,
                                   fossil_bluecrab_myshell_error_t *out_err);

/** Delete a record by key (marks tombstone in chain). */
fossil_bluecrab_myshell_error_t
fossil_bluecrab_myshell_delete_record(fossil_bluecrab_myshell_t *db,
                                      const char *key,
                                      fossil_bluecrab_myshell_error_t *out_err);

/** Iterate records with callback; return false from callback to abort early. */
typedef bool (*fossil_bluecrab_myshell_record_iter_cb)(
    const fossil_bluecrab_myshell_record_t *rec, void *user);
fossil_bluecrab_myshell_error_t
fossil_bluecrab_myshell_iter_records(fossil_bluecrab_myshell_t *db,
                                     const char *prefix_filter,   // optional null
                                     fossil_bluecrab_myshell_record_iter_cb cb,
                                     void *user);


// -------------------------------
// FSON helpers (parse/serialize/convert)
// -------------------------------

/** Parse a FSON text fragment into a runtime value. Returns malloc'd value. */
fossil_bluecrab_myshell_fson_value_t *
fossil_bluecrab_myshell_fson_parse_value(const char *text,
                                         fossil_bluecrab_myshell_error_t *out_err);

/** Serialize a FSON value into a heap-allocated string (caller frees). */
char *
fossil_bluecrab_myshell_fson_serialize_value(const fossil_bluecrab_myshell_fson_value_t *val,
                                             fossil_bluecrab_myshell_error_t *out_err);

/** Deep-free a parsed FSON value. */
void
fossil_bluecrab_myshell_fson_free_value(fossil_bluecrab_myshell_fson_value_t *val);

/** Convert FSON object to SQL INSERT statement (helper for SQL-backed tables). */
char *
fossil_bluecrab_myshell_fson_to_sql_insert(const char *table_name,
                                           const char *fson_object_text,
                                           fossil_bluecrab_myshell_error_t *out_err);


// -------------------------------
// Hashing utilities (64-bit)
// -------------------------------

/** Portable 64-bit mixing/hash function (deterministic). */
static inline uint64_t fossil_bluecrab_myshell_mix64(uint64_t x) {
    x ^= x >> 30;
    x *= 0xbf58476d1ce4e5b9ULL;
    x ^= x >> 27;
    x *= 0x94d049bb133111ebULL;
    x ^= x >> 31;
    return x;
}

/** Compute stable 64-bit hash for a record (based on key + fson_text + timestamp). */
fossil_bluecrab_myshell_hash64_t
fossil_bluecrab_myshell_compute_record_hash(const char *key,
                                            const char *fson_text,
                                            const char *timestamp_iso,
                                            uint64_t seed);


// -------------------------------
// Indexing / search
// -------------------------------

/** Create a simple index on a top-level FSON field (string/number). */
fossil_bluecrab_myshell_error_t
fossil_bluecrab_myshell_index_create(fossil_bluecrab_myshell_t *db,
                                     const char *index_name,
                                     const char *fson_path); /* e.g. "name", "address.city" */

/** Query index: returns array of keys (caller frees). */
char **
fossil_bluecrab_myshell_index_query(fossil_bluecrab_myshell_t *db,
                                    const char *index_name,
                                    const char *match_value,
                                    size_t *out_count);

// -------------------------------
// Backups, snapshots, restore
// -------------------------------

/** Full backup to file. */
fossil_bluecrab_myshell_error_t
fossil_bluecrab_myshell_backup(fossil_bluecrab_myshell_t *db,
                               const char *backup_path);

/** Restore DB from backup; must be closed before restore. */
fossil_bluecrab_myshell_error_t
fossil_bluecrab_myshell_restore(const char *backup_path,
                                const char *target_path);

// -------------------------------
// Concurrency / locking / permissions
// -------------------------------

/** Acquire advisory lock for critical operation (blocking or try). */
fossil_bluecrab_myshell_error_t
fossil_bluecrab_myshell_lock_acquire(fossil_bluecrab_myshell_t *db,
                                     const char *lock_name,
                                     bool blocking);

/** Release advisory lock. */
fossil_bluecrab_myshell_error_t
fossil_bluecrab_myshell_lock_release(fossil_bluecrab_myshell_t *db,
                                     const char *lock_name);

/** Set file/record-level permissions (simple ACL model). */
typedef enum { ACL_PERM_READ = 1, ACL_PERM_WRITE = 2, ACL_PERM_ADMIN = 4 } fossil_bluecrab_myshell_acl_perm_t;
fossil_bluecrab_myshell_error_t
fossil_bluecrab_myshell_acl_set(fossil_bluecrab_myshell_t *db,
                                const char *resource,         // "record:presidents/obama"
                                const char *principal,        // "user:alice" or "group:admins"
                                int perms);


// -------------------------------
// Utilities & import/export (example: president data file)
// -------------------------------

/** Import a CSV of presidents (returns # rows imported). Caller can supply mapping or use default). */
fossil_bluecrab_myshell_error_t
fossil_bluecrab_myshell_import_presidents_csv(fossil_bluecrab_myshell_t *db,
                                              const char *csv_path,
                                              const char *table_name,
                                              size_t *out_rows_imported);

/** Export a presidents table to CSV. */
fossil_bluecrab_myshell_error_t
fossil_bluecrab_myshell_export_presidents_csv(fossil_bluecrab_myshell_t *db,
                                              const char *table_name,
                                              const char *csv_path);

/** Convenience: create a schema suitable for storing president records (name, term_start, term_end, party, bio as FSON). */
fossil_bluecrab_myshell_error_t
fossil_bluecrab_myshell_create_president_schema(fossil_bluecrab_myshell_t *db,
                                                const char *table_name);


// -------------------------------
// Diagnostics, compacting, VACUUM
// -------------------------------

/** Run integrity checks (consistency between chain, storage and indexes). */
fossil_bluecrab_myshell_error_t
fossil_bluecrab_myshell_integrity_check(fossil_bluecrab_myshell_t *db,
                                        char **out_report); /* malloc'd report; caller frees */

/** Compact DB storage (repack, drop tombstones, optimize indexes). */
fossil_bluecrab_myshell_error_t
fossil_bluecrab_myshell_compact(fossil_bluecrab_myshell_t *db,
                                fossil_bluecrab_myshell_error_t *out_err);


// -------------------------------
// Low-level I/O & utilities
// -------------------------------

/** Write raw blob (used by low-level store internals). Returns a stable blob-hash. */
fossil_bluecrab_myshell_hash64_t
fossil_bluecrab_myshell_store_blob(fossil_bluecrab_myshell_t *db,
                                   const void *data, size_t n,
                                   fossil_bluecrab_myshell_error_t *out_err);

/** Read raw blob by hash (caller owns buffer). */
void *
fossil_bluecrab_myshell_load_blob(fossil_bluecrab_myshell_t *db,
                                  fossil_bluecrab_myshell_hash64_t blob_hash,
                                  size_t *out_size,
                                  fossil_bluecrab_myshell_error_t *out_err);

/** Free memory for record returned by get_record(). */
void
fossil_bluecrab_myshell_record_free(fossil_bluecrab_myshell_record_t *rec);


// -------------------------------
// Helpers for timestamp / duration parsing
// -------------------------------
time_t
fossil_bluecrab_myshell_parse_iso8601(const char *iso, fossil_bluecrab_myshell_error_t *out_err);

char *
fossil_bluecrab_myshell_format_iso8601(time_t t);


/** Parse duration strings like "30s", "1h", "5d" -> seconds. */
int64_t
fossil_bluecrab_myshell_parse_duration_seconds(const char *duration_str,
                                               fossil_bluecrab_myshell_error_t *out_err);

// -------------------------------
// Bootstrapping utilities (schema / builtins)
// -------------------------------
fossil_bluecrab_myshell_error_t
fossil_bluecrab_myshell_init_builtin_functions(fossil_bluecrab_myshell_t *db);

/** Load initial dataset file (example: presidents.fson). Useful during create(). */
fossil_bluecrab_myshell_error_t
fossil_bluecrab_myshell_load_datafile(fossil_bluecrab_myshell_t *db,
                                      const char *datafile_path,
                                      fossil_bluecrab_myshell_error_t *out_err);

// -------------------------------
// Commit history iteration
// -------------------------------

/** Iterate commits in a branch (newest → oldest). */
typedef bool (*fossil_bluecrab_myshell_commit_iter_cb)(
    const fossil_bluecrab_myshell_commit_t *commit, void *user);

fossil_bluecrab_myshell_error_t
fossil_bluecrab_myshell_iter_commits(fossil_bluecrab_myshell_t *db,
                                     const char *branch_name,
                                     fossil_bluecrab_myshell_commit_iter_cb cb,
                                     void *user);

// -------------------------------
// Replication / sync
// -------------------------------

/** Push local commits to a remote (URL or path). */
fossil_bluecrab_myshell_error_t
fossil_bluecrab_myshell_sync_push(fossil_bluecrab_myshell_t *db,
                                  const char *remote_url);

/** Pull commits from a remote into local. */
fossil_bluecrab_myshell_error_t
fossil_bluecrab_myshell_sync_pull(fossil_bluecrab_myshell_t *db,
                                  const char *remote_url);

// -------------------------------
// Schema inspection
// -------------------------------

/** List table names. Caller frees array. */
char **
fossil_bluecrab_myshell_list_tables(fossil_bluecrab_myshell_t *db,
                                    size_t *out_count);

/** List index names. Caller frees array. */
char **
fossil_bluecrab_myshell_list_indexes(fossil_bluecrab_myshell_t *db,
                                     size_t *out_count);


// -------------------------------
// Streaming API for large data
// -------------------------------

/** Streaming handle (opaque). */
typedef struct fossil_bluecrab_myshell_stream_t fossil_bluecrab_myshell_stream_t;

/** Open a stream for a large blob or FSON field. Mode = "r" or "w". */
fossil_bluecrab_myshell_stream_t *
fossil_bluecrab_myshell_stream_open(fossil_bluecrab_myshell_t *db,
                                    const char *key,
                                    const char *mode,
                                    fossil_bluecrab_myshell_error_t *out_err);

/** Read from stream. */
size_t
fossil_bluecrab_myshell_stream_read(fossil_bluecrab_myshell_stream_t *s,
                                    void *buf,
                                    size_t n,
                                    fossil_bluecrab_myshell_error_t *out_err);

/** Write to stream. */
size_t
fossil_bluecrab_myshell_stream_write(fossil_bluecrab_myshell_stream_t *s,
                                     const void *buf,
                                     size_t n,
                                     fossil_bluecrab_myshell_error_t *out_err);

/** Close and free stream. */
fossil_bluecrab_myshell_error_t
fossil_bluecrab_myshell_stream_close(fossil_bluecrab_myshell_stream_t *s);


// -------------------------------
// Savepoints / checkpoints
// -------------------------------

/** Lightweight savepoints (nested transactions). */
fossil_bluecrab_myshell_error_t
fossil_bluecrab_myshell_savepoint(fossil_bluecrab_myshell_txn_t *txn,
                                  const char *name);

fossil_bluecrab_myshell_error_t
fossil_bluecrab_myshell_release_savepoint(fossil_bluecrab_myshell_txn_t *txn,
                                          const char *name);

fossil_bluecrab_myshell_error_t
fossil_bluecrab_myshell_rollback_to_savepoint(fossil_bluecrab_myshell_txn_t *txn,
                                              const char *name);


// -------------------------------
// Event hooks / callbacks
// -------------------------------

/** Event callback signature (event string, e.g. "commit", "merge", "record.put"). */
typedef void (*fossil_bluecrab_myshell_event_cb)(const char *event, void *user);

/** Install an event callback. */
void
fossil_bluecrab_myshell_set_event_callback(fossil_bluecrab_myshell_t *db,
                                           fossil_bluecrab_myshell_event_cb cb,
                                           void *user);

// -------------------------------
// Debug / introspection
// -------------------------------
/** Dump small representation of current HEAD commit to caller-owned buffer (string). */
char *
fossil_bluecrab_myshell_dump_head(const fossil_bluecrab_myshell_t *db);

/** Print internal stats to text (malloc'd). */
char *
fossil_bluecrab_myshell_stats(const fossil_bluecrab_myshell_t *db);

#ifdef __cplusplus
}
#include <string>

namespace fossil {

    namespace bluecrab {

        /**
         * @brief C++ wrapper class for MyShell database operations.
         * 
         * Provides static methods for CRUD, database management, backup/restore,
         * validation, and iteration, mapping to the C API.
         */
        class MyShell {
        public:
            /**
             * @brief Creates a new record in the database.
             * @param file_name Name of the database file.
             * @param key Key of the record.
             * @param value Value of the record.
             * @return Error code (0 on success).
             */
            static fossil_bluecrab_myshell_error_t create_record(const std::string& file_name, const std::string& key, const std::string& value) {
                return fossil_bluecrab_myshell_create_record(file_name.c_str(), key.c_str(), value.c_str());
            }

            /**
             * @brief Reads a record from the database.
             * @param file_name Name of the database file.
             * @param key Key of the record to read.
             * @param value Output parameter to store the value.
             * @return Error code (0 on success).
             */
            static fossil_bluecrab_myshell_error_t read_record(const std::string& file_name, const std::string& key, std::string& value) {
                char buffer[4096] = {0};
                fossil_bluecrab_myshell_error_t err = fossil_bluecrab_myshell_read_record(file_name.c_str(), key.c_str(), buffer, sizeof(buffer));
                if (err == FOSSIL_MYSHELL_ERROR_SUCCESS) {
                    value = buffer;
                }
                return err;
            }

            /**
             * @brief Updates the value of a record in the database.
             * @param file_name Name of the database file.
             * @param key Key of the record to update.
             * @param new_value New value to set.
             * @return Error code (0 on success).
             */
            static fossil_bluecrab_myshell_error_t update_record(const std::string& file_name, const std::string& key, const std::string& new_value) {
                return fossil_bluecrab_myshell_update_record(file_name.c_str(), key.c_str(), new_value.c_str());
            }

            /**
             * @brief Deletes a record from the database.
             * @param file_name Name of the database file.
             * @param key Key of the record to delete.
             * @return Error code (0 on success).
             */
            static fossil_bluecrab_myshell_error_t delete_record(const std::string& file_name, const std::string& key) {
                return fossil_bluecrab_myshell_delete_record(file_name.c_str(), key.c_str());
            }

            /**
             * @brief Creates a new database file.
             * @param file_name Name of the database file.
             * @return Error code (0 on success).
             */
            static fossil_bluecrab_myshell_error_t create_database(const std::string& file_name) {
                return fossil_bluecrab_myshell_create_database(file_name.c_str());
            }

            /**
             * @brief Opens an existing database file.
             * @param file_name Name of the database file.
             * @return Error code (0 on success).
             */
            static fossil_bluecrab_myshell_error_t open_database(const std::string& file_name) {
                return fossil_bluecrab_myshell_open_database(file_name.c_str());
            }

            /**
             * @brief Deletes a database file.
             * @param file_name Name of the database file.
             * @return Error code (0 on success).
             */
            static fossil_bluecrab_myshell_error_t delete_database(const std::string& file_name) {
                return fossil_bluecrab_myshell_delete_database(file_name.c_str());
            }

            /**
             * @brief Backs up a database file.
             * @param source_file Name of the source database file.
             * @param backup_file Name of the backup file.
             * @return Error code (0 on success).
             */
            static fossil_bluecrab_myshell_error_t backup_database(const std::string& source_file, const std::string& backup_file) {
                return fossil_bluecrab_myshell_backup_database(source_file.c_str(), backup_file.c_str());
            }

            /**
             * @brief Restores a database file from a backup.
             * @param backup_file Name of the backup file.
             * @param destination_file Name of the destination database file.
             * @return Error code (0 on success).
             */
            static fossil_bluecrab_myshell_error_t restore_database(const std::string& backup_file, const std::string& destination_file) {
                return fossil_bluecrab_myshell_restore_database(backup_file.c_str(), destination_file.c_str());
            }

            /**
             * @brief Verifies the integrity of a database file.
             * @param file_name Name of the database file.
             * @return Error code (0 on success, FOSSIL_MYSHELL_ERROR_CORRUPTED if failed).
             */
            static fossil_bluecrab_myshell_error_t verify_database(const std::string& file_name) {
                return fossil_bluecrab_myshell_verify_database(file_name.c_str());
            }

            /**
             * @brief Validates the file extension of a database file.
             * @param file_name Name of the database file.
             * @return True if the extension is valid, false otherwise.
             */
            static bool validate_extension(const std::string& file_name) {
                return fossil_bluecrab_myshell_validate_extension(file_name.c_str());
            }

            /**
             * @brief Validates a data string.
             * @param data Data string to validate.
             * @return True if the data is valid, false otherwise.
             */
            static bool validate_data(const std::string& data) {
                return fossil_bluecrab_myshell_validate_data(data.c_str());
            }

            /**
             * @brief Closes an opened database file.
             * @param file_name Name of the database file.
             * @return Error code (0 on success).
             */
            static fossil_bluecrab_myshell_error_t close_database(const std::string& file_name) {
                return fossil_bluecrab_myshell_close_database(file_name.c_str());
            }

            /**
             * @brief Checks if a database file is currently open.
             * @param file_name Name of the database file.
             * @return True if open, false otherwise.
             */
            static bool is_open(const std::string& file_name) {
                return fossil_bluecrab_myshell_is_open(file_name.c_str());
            }

            /**
             * @brief Gets the first key in the database.
             * @param file_name Name of the database file.
             * @param key Output parameter to store the key.
             * @return Error code (0 on success).
             */
            static fossil_bluecrab_myshell_error_t first_key(const std::string& file_name, std::string& key) {
                char buffer[4096] = {0};
                fossil_bluecrab_myshell_error_t err = fossil_bluecrab_myshell_first_key(file_name.c_str(), buffer, sizeof(buffer));
                if (err == FOSSIL_MYSHELL_ERROR_SUCCESS) {
                    key = buffer;
                }
                return err;
            }

            /**
             * @brief Gets the next key in the database (iteration).
             * @param file_name Name of the database file.
             * @param prev_key Previous key from iteration.
             * @param key Output parameter to store the next key.
             * @return Error code (0 on success, or EOF if no more keys).
             */
            static fossil_bluecrab_myshell_error_t next_key(const std::string& file_name, const std::string& prev_key, std::string& key) {
                char buffer[4096] = {0};
                fossil_bluecrab_myshell_error_t err = fossil_bluecrab_myshell_next_key(file_name.c_str(), prev_key.c_str(), buffer, sizeof(buffer));
                if (err == FOSSIL_MYSHELL_ERROR_SUCCESS) {
                    key = buffer;
                }
                return err;
            }

            /**
             * @brief Gets the total number of records in the database.
             * @param file_name Name of the database file.
             * @param count Output parameter to store the record count.
             * @return Error code (0 on success).
             */
            static fossil_bluecrab_myshell_error_t count_records(const std::string& file_name, size_t& count) {
                return fossil_bluecrab_myshell_count_records(file_name.c_str(), &count);
            }

            /**
             * @brief Gets the size of the database file in bytes.
             * @param file_name Name of the database file.
             * @param size_bytes Output parameter to store the file size.
             * @return Error code (0 on success).
             */
            static fossil_bluecrab_myshell_error_t get_file_size(const std::string& file_name, size_t& size_bytes) {
                return fossil_bluecrab_myshell_get_file_size(file_name.c_str(), &size_bytes);
            }

            /**
             * @brief Converts an error code to a human-readable string.
             * @param error_code The error code.
             * @return String describing the error.
             */
            static std::string error_string(fossil_bluecrab_myshell_error_t error_code) {
                return fossil_bluecrab_myshell_error_string(error_code);
            }
        };

    } // namespace bluecrab

} // namespace fossil

#endif

#endif /* FOSSIL_CRABDB_FRAMEWORK_H */

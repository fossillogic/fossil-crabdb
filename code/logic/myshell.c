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

/**
 * @brief Implements the core logic for the Fossil BlueCrab .myshell file database.
 *
 * This file provides functions for managing a simple versioned key-value store
 * using the ".myshell" file format. It supports operations such as open, create,
 * close, put, get, delete, commit, branch, checkout, merge, revert, stage, unstage,
 * tag, log, backup, restore, error string conversion, and integrity checking.
 *
 * ## .myshell File Format Overview
 * - Each .myshell file is a plain text file with lines representing key-value pairs,
 *   commit history, branches, tags, and staged changes.
 * - Key-value pairs are stored as: `key=value #type=TYPE #hash=KEYHASH`
 *   - `TYPE` is the FSON type name (see myshell_fson_type_names).
 *   - `KEYHASH` is a 64-bit hash of the key for integrity verification.
 * - Commits are recorded as: `#commit HASH MESSAGE TIMESTAMP #type=enum`
 * - Branches are recorded as: `#branch HASH BRANCHNAME #type=enum`
 * - Tags are recorded as: `#tag HASH TAGNAME #type=enum`
 * - Staged changes are recorded as: `#stage key=value #type=TYPE #hash=KEYHASH`
 * - Merges are recorded as: `#merge HASH SOURCEBRANCH MESSAGE TIMESTAMP #type=enum`
 * - Backups include a header: `#backup_hash=HASH`
 * - FSON type system header: `#fson_types=null,bool,i8,i16,i32,i64,u8,u16,u32,u64,f32,f64,oct,hex,bin,char,cstr,array,object,enum,datetime,duration`
 *
 * ## Sample .myshell File Contents
 * ```
 * key1=value1 #type=i32 #hash=0123456789abcdef
 * key2=value2 #type=cstr #hash=abcdef0123456789
 * #commit 0123456789abcdef Initial commit 1712345678 #type=enum
 * #branch 89abcdef01234567 main #type=enum
 * #tag 0123456789abcdef v1.0 #type=enum
 * #stage key3=value3 #type=bool #hash=123456789abcdef0
 * #merge 89abcdef01234567 feature "Merge feature branch" 1712345680 #type=enum
 * ```
 *
 * ## More Sample .myshell File Contents
 * ```
 * username=alice #type=cstr #hash=1a2b3c4d5e6f7890
 * password=secret #type=cstr #hash=0f1e2d3c4b5a6978
 * #commit 1a2b3c4d5e6f7890 Added user 1712345700 #type=enum
 * #branch 0f1e2d3c4b5a6978 dev #type=enum
 * #tag 1a2b3c4d5e6f7890 v2.0 #type=enum
 * ```
 *
 * ## Another Example
 * ```
 * config=enabled #type=bool #hash=deadbeefcafebabe
 * mode=fast #type=enum #hash=beefdeadbabecafe
 * #commit deadbeefcafebabe Config enabled 1712345800 #type=enum
 * #branch beefdeadbabecafe test #type=enum
 * #tag deadbeefcafebabe test-tag #type=enum
 * ```
 *
 * ## Fourth Example
 * ```
 * foo=bar #type=cstr #hash=1111222233334444
 * baz=qux #type=cstr #hash=5555666677778888
 * #commit 1111222233334444 FooBar commit 1712345900 #type=enum
 * #branch 5555666677778888 feature-x #type=enum
 * #tag 1111222233334444 release-x #type=enum
 * ```
 *
 * ## Fifth Example
 * ```
 * alpha=beta #type=cstr #hash=9999aaaabbbbcccc
 * gamma=delta #type=cstr #hash=ddddccccbbbbaaaa
 * #commit 9999aaaabbbbcccc AlphaBeta commit 1712346000 #type=enum
 * #branch ddddccccbbbbaaaa hotfix #type=enum
 * #tag 9999aaaabbbbcccc hotfix-1 #type=enum
 * ```
 *
 * ## Main Functions
 * - `myshell_hash64`: Computes a 64-bit hash for strings (MurmurHash3 variant).
 * - `fossil_myshell_open`: Opens an existing .myshell database file.
 * - `fossil_myshell_create`: Creates a new .myshell database file.
 * - `fossil_myshell_close`: Closes and frees resources for a database.
 * - `fossil_myshell_put`: Inserts or updates a key-value pair (with FSON type and hash).
 * - `fossil_myshell_get`: Retrieves the value for a given key.
 * - `fossil_myshell_del`: Deletes a key-value pair.
 * - `fossil_myshell_commit`: Records a commit with a message.
 * - `fossil_myshell_branch`: Creates or switches to a branch.
 * - `fossil_myshell_checkout`: Checks out a branch or commit.
 * - `fossil_myshell_merge`: Merges a branch with a commit message.
 * - `fossil_myshell_revert`: Reverts to a specific commit.
 * - `fossil_myshell_stage`: Stages a key-value change.
 * - `fossil_myshell_unstage`: Removes a staged change.
 * - `fossil_myshell_tag`: Tags a commit.
 * - `fossil_myshell_log`: Iterates commit history.
 * - `fossil_myshell_backup`: Creates a backup of the database.
 * - `fossil_myshell_restore`: Restores a database from backup.
 * - `fossil_myshell_errstr`: Converts error codes to strings.
 * - `fossil_myshell_check_integrity`: Verifies file and commit integrity.
 *
 * ## Error Handling
 * All functions return a `fossil_bluecrab_myshell_error_t` code indicating success or the type of error.
 *
 * ## Usage Notes
 * - Only files with the ".myshell" extension are supported.
 * - All operations are performed directly on the file; there is no in-memory caching.
 * - Integrity of data is ensured via hashes for keys and commits.
 * - The API is designed for simple versioned key-value storage with basic VCS-like features.
 * - The FSON type system is enforced for all key-value and metadata entries.
 */

// Lookup table for FSON type system
static const char *myshell_fson_type_names[] = {
    "null",      // MYSHELL_FSON_TYPE_NULL
    "bool",      // MYSHELL_FSON_TYPE_BOOL
    "i8",        // MYSHELL_FSON_TYPE_I8
    "i16",       // MYSHELL_FSON_TYPE_I16
    "i32",       // MYSHELL_FSON_TYPE_I32
    "i64",       // MYSHELL_FSON_TYPE_I64
    "u8",        // MYSHELL_FSON_TYPE_U8
    "u16",       // MYSHELL_FSON_TYPE_U16
    "u32",       // MYSHELL_FSON_TYPE_U32
    "u64",       // MYSHELL_FSON_TYPE_U64
    "f32",       // MYSHELL_FSON_TYPE_F32
    "f64",       // MYSHELL_FSON_TYPE_F64
    "oct",       // MYSHELL_FSON_TYPE_OCT
    "hex",       // MYSHELL_FSON_TYPE_HEX
    "bin",       // MYSHELL_FSON_TYPE_BIN
    "char",      // MYSHELL_FSON_TYPE_CHAR
    "cstr",      // MYSHELL_FSON_TYPE_CSTR
    "array",     // MYSHELL_FSON_TYPE_ARRAY
    "object",    // MYSHELL_FSON_TYPE_OBJECT
    "enum",      // MYSHELL_FSON_TYPE_ENUM
    "datetime",  // MYSHELL_FSON_TYPE_DATETIME
    "duration"   // MYSHELL_FSON_TYPE_DURATION
};

static inline const char *myshell_fson_type_to_string(fossil_bluecrab_myshell_fson_type_t type) {
    if (type < 0 || type > MYSHELL_FSON_TYPE_DURATION)
        return "unknown";
    return myshell_fson_type_names[type];
}

/**
 * Custom strdup implementation.
 */
static char *myshell_strdup(const char *s) {
    if (!s) return NULL;
    size_t len = strlen(s);
    char *copy = (char *)malloc(len + 1);
    if (copy) {
        memcpy(copy, s, len + 1);
    }
    return copy;
}

/**
 * Advanced 64-bit hash algorithm for strings (MurmurHash3 variant).
 * Returns a 64-bit hash value for the given input string.
 */
uint64_t myshell_hash64(const char *str) {
    if (!str) return 0;
    uint64_t seed = 0xe17a1465ULL;
    uint64_t m = 0xc6a4a7935bd1e995ULL;
    int r = 47;
    size_t len = strlen(str);
    uint64_t hash = seed ^ (len * m);

    const uint8_t *data = (const uint8_t *)str;
    const uint8_t *end = data + (len & ~0x7);

    while (data != end) {
        uint64_t k;
        memcpy(&k, data, sizeof(uint64_t));
        k *= m;
        k ^= k >> r;
        k *= m;
        hash ^= k;
        hash *= m;
        data += 8;
    }

    switch (len & 7) {
        case 7: hash ^= (uint64_t)data[6] << 48; /* fall through */
        case 6: hash ^= (uint64_t)data[5] << 40; /* fall through */
        case 5: hash ^= (uint64_t)data[4] << 32; /* fall through */
        case 4: hash ^= (uint64_t)data[3] << 24; /* fall through */
        case 3: hash ^= (uint64_t)data[2] << 16; /* fall through */
        case 2: hash ^= (uint64_t)data[1] << 8;  /* fall through */
        case 1: hash ^= (uint64_t)data[0];
                hash *= m;
    }

    hash ^= hash >> r;
    hash *= m;
    hash ^= hash >> r;
    return hash;
}

fossil_bluecrab_myshell_t *fossil_myshell_open(const char *path, fossil_bluecrab_myshell_error_t *err) {
    if (!path) {
        if (err) *err = FOSSIL_MYSHELL_ERROR_INVALID_FILE;
        return NULL;
    }

    // Enforce .myshell extension
    const char *ext = strrchr(path, '.');
    if (!ext || strcmp(ext, ".myshell") != 0) {
        if (err) *err = FOSSIL_MYSHELL_ERROR_INVALID_FILE;
        return NULL;
    }

    FILE *file = fopen(path, "rb+");
    if (!file) {
        if (err) *err = FOSSIL_MYSHELL_ERROR_FILE_NOT_FOUND;
        return NULL;
    }

    fossil_bluecrab_myshell_t *db = (fossil_bluecrab_myshell_t *)calloc(1, sizeof(fossil_bluecrab_myshell_t));
    if (!db) {
        fclose(file);
        if (err) *err = FOSSIL_MYSHELL_ERROR_OUT_OF_MEMORY;
        return NULL;
    }

    db->path = myshell_strdup(path);
    if (!db->path) {
        fclose(file);
        free(db);
        if (err) *err = FOSSIL_MYSHELL_ERROR_OUT_OF_MEMORY;
        return NULL;
    }

    db->file = file;
    db->is_open = true;

    if (fseek(file, 0, SEEK_END) != 0) {
        free(db->path);
        free(db);
        fclose(file);
        if (err) *err = FOSSIL_MYSHELL_ERROR_IO;
        return NULL;
    }
    db->file_size = (size_t)ftell(file);
    if (fseek(file, 0, SEEK_SET) != 0) {
        free(db->path);
        free(db);
        fclose(file);
        if (err) *err = FOSSIL_MYSHELL_ERROR_IO;
        return NULL;
    }

    struct stat st;
    if (stat(path, &st) == 0)
        db->last_modified = st.st_mtime;
    else
        db->last_modified = 0;

    db->commit_head = myshell_hash64(path);
    db->error_code = FOSSIL_MYSHELL_ERROR_SUCCESS;

    // FSON type system: scan file for type hints and validate
    // (optional: could build a type map or validate lines)
    // For now, just check that all #type=... fields match known types
    fseek(file, 0, SEEK_SET);
    char line[1024];
    while (fgets(line, sizeof(line), file)) {
        char *type_comment = strstr(line, "#type=");
        if (type_comment) {
            type_comment += 6;
            char type_name[32] = {0};
            int i = 0;
            while (type_comment[i] && !isspace((unsigned char)type_comment[i]) && type_comment[i] != '#' && i < 31) {
                type_name[i] = type_comment[i];
                i++;
            }
            type_name[i] = '\0';
            bool valid_type = false;
            for (size_t j = 0; j <= MYSHELL_FSON_TYPE_DURATION; ++j) {
                if (strcmp(type_name, myshell_fson_type_names[j]) == 0) {
                    valid_type = true;
                    break;
                }
            }
            if (!valid_type) {
                free(db->path);
                free(db);
                fclose(file);
                if (err) *err = FOSSIL_MYSHELL_ERROR_CONFIG_INVALID;
                return NULL;
            }
        }
    }
    fseek(file, 0, SEEK_SET);

    if (err) *err = FOSSIL_MYSHELL_ERROR_SUCCESS;
    return db;
}

fossil_bluecrab_myshell_t *fossil_myshell_create(const char *path, fossil_bluecrab_myshell_error_t *err) {
    if (!path) {
        if (err) *err = FOSSIL_MYSHELL_ERROR_INVALID_FILE;
        return NULL;
    }

    // Enforce .myshell extension
    const char *ext = strrchr(path, '.');
    if (!ext || strcmp(ext, ".myshell") != 0) {
        if (err) *err = FOSSIL_MYSHELL_ERROR_INVALID_FILE;
        return NULL;
    }

    // Check if file already exists
    FILE *check = fopen(path, "rb");
    if (check) {
        fclose(check);
        if (err) *err = FOSSIL_MYSHELL_ERROR_ALREADY_EXISTS;
        return NULL;
    }

    FILE *file = fopen(path, "wb+");
    if (!file) {
        if (err) *err = FOSSIL_MYSHELL_ERROR_IO;
        return NULL;
    }

    // Write FSON type system header for new file
    fprintf(file, "#fson_types=");
    for (size_t i = 0; i <= MYSHELL_FSON_TYPE_DURATION; ++i) {
        fprintf(file, "%s", myshell_fson_type_names[i]);
        if (i < MYSHELL_FSON_TYPE_DURATION) fprintf(file, ",");
    }
    fprintf(file, "\n");

    fossil_bluecrab_myshell_t *db = (fossil_bluecrab_myshell_t *)calloc(1, sizeof(fossil_bluecrab_myshell_t));
    if (!db) {
        fclose(file);
        if (err) *err = FOSSIL_MYSHELL_ERROR_OUT_OF_MEMORY;
        return NULL;
    }

    db->path = myshell_strdup(path);
    if (!db->path) {
        fclose(file);
        free(db);
        if (err) *err = FOSSIL_MYSHELL_ERROR_OUT_OF_MEMORY;
        return NULL;
    }

    db->file = file;
    db->is_open = true;
    db->file_size = (size_t)ftell(file);
    db->last_modified = time(NULL);
    db->commit_head = myshell_hash64(path);
    db->error_code = FOSSIL_MYSHELL_ERROR_SUCCESS;

    if (err) *err = FOSSIL_MYSHELL_ERROR_SUCCESS;
    return db;
}

void fossil_myshell_close(fossil_bluecrab_myshell_t *db) {
    if (db) {
        if (db->file) {
            fclose(db->file);
            db->file = NULL;
        }
        if (db->path) {
            free(db->path);
            db->path = NULL;
        }
        if (db->branch) {
            free(db->branch);
            db->branch = NULL;
        }
        if (db->author) {
            free(db->author);
            db->author = NULL;
        }
        if (db->commit_message) {
            free(db->commit_message);
            db->commit_message = NULL;
        }
        if (db->parent_branch) {
            free(db->parent_branch);
            db->parent_branch = NULL;
        }
        free(db);
    }
}

fossil_bluecrab_myshell_error_t fossil_myshell_put(fossil_bluecrab_myshell_t *db, const char *key, const char *type, const char *value) {
    if (!db || !db->is_open) {
        return FOSSIL_MYSHELL_ERROR_INVALID_FILE;
    }
    if (!key || !type || !value) {
        return FOSSIL_MYSHELL_ERROR_INVALID_QUERY;
    }
    if (key[0] == '\0' || type[0] == '\0') {
        return FOSSIL_MYSHELL_ERROR_INVALID_QUERY;
    }

    // Validate type against FSON type system
    fossil_bluecrab_myshell_fson_type_t type_id = MYSHELL_FSON_TYPE_NULL;
    bool valid_type = false;
    for (size_t i = 0; i <= MYSHELL_FSON_TYPE_DURATION; ++i) {
        if (strcmp(type, myshell_fson_type_names[i]) == 0) {
            type_id = (fossil_bluecrab_myshell_fson_type_t)i;
            valid_type = true;
            break;
        }
    }
    if (!valid_type) {
        return FOSSIL_MYSHELL_ERROR_CONFIG_INVALID;
    }

    uint64_t key_hash = myshell_hash64(key);

    fseek(db->file, 0, SEEK_SET);
    char temp_path[256];
    snprintf(temp_path, sizeof(temp_path), "%s.tmp", db->path);
    FILE *temp_file = fopen(temp_path, "wb");
    if (!temp_file) {
        return FOSSIL_MYSHELL_ERROR_IO;
    }

    char line[1024];
    bool updated = false;
    while (fgets(line, sizeof(line), db->file)) {
        char *eq = strchr(line, '=');
        if (eq) {
            *eq = '\0';
            char *hash_comment = strstr(eq + 1, "#hash=");
            if (hash_comment) {
                uint64_t file_hash = 0;
                sscanf(hash_comment, "#hash=%llx", &file_hash);
                if (strcmp(line, key) == 0 && file_hash == key_hash) {
                    // Overwrite with new value and type
                    fprintf(temp_file, "%s=%s #type=%s #hash=%016llx\n", key, value, myshell_fson_type_to_string(type_id), key_hash);
                    updated = true;
                    *eq = '='; // Restore
                    continue;
                }
            } else {
                if (strcmp(line, key) == 0) {
                    // Overwrite with new value and type
                    fprintf(temp_file, "%s=%s #type=%s #hash=%016llx\n", key, value, myshell_fson_type_to_string(type_id), key_hash);
                    updated = true;
                    *eq = '='; // Restore
                    continue;
                }
            }
            *eq = '='; // Restore
        }
        fputs(line, temp_file);
    }

    if (!updated) {
        // Add new entry with FSON type and hash
        fprintf(temp_file, "%s=%s #type=%s #hash=%016llx\n", key, value, myshell_fson_type_to_string(type_id), key_hash);
    }

    fclose(temp_file);
    fclose(db->file);

    if (remove(db->path) != 0) {
        remove(temp_path);
        return FOSSIL_MYSHELL_ERROR_IO;
    }
    if (rename(temp_path, db->path) != 0) {
        return FOSSIL_MYSHELL_ERROR_IO;
    }

    db->file = fopen(db->path, "rb+");
    if (!db->file) {
        return FOSSIL_MYSHELL_ERROR_IO;
    }

    db->file_size = (size_t)ftell(db->file);
    db->last_modified = time(NULL);
    return FOSSIL_MYSHELL_ERROR_SUCCESS;
}

fossil_bluecrab_myshell_error_t fossil_myshell_get(
    fossil_bluecrab_myshell_t *db,
    const char *key,
    char *out_value,
    size_t out_size
) {
    if (!db || !db->is_open) {
        return FOSSIL_MYSHELL_ERROR_INVALID_FILE;
    }
    if (!key || !out_value || out_size == 0) {
        return FOSSIL_MYSHELL_ERROR_INVALID_QUERY;
    }
    if (key[0] == '\0') {
        return FOSSIL_MYSHELL_ERROR_INVALID_QUERY;
    }

    uint64_t key_hash = myshell_hash64(key);

    fseek(db->file, 0, SEEK_SET);
    char line[1024];
    while (fgets(line, sizeof(line), db->file)) {
        char *eq = strchr(line, '=');
        if (eq) {
            *eq = '\0';
            char *hash_comment = strstr(eq + 1, "#hash=");
            char *type_comment = strstr(eq + 1, "#type=");
            if (hash_comment) {
                uint64_t file_hash = 0;
                sscanf(hash_comment, "#hash=%llx", &file_hash);
                if (strcmp(line, key) == 0 && file_hash == key_hash) {
                    // Extract value (between '=' and #type or #hash)
                    size_t value_len = 0;
                    if (type_comment && type_comment > eq + 1) {
                        value_len = type_comment - (eq + 1);
                    } else {
                        value_len = hash_comment - (eq + 1);
                    }
                    if (value_len >= out_size) value_len = out_size - 1;
                    strncpy(out_value, eq + 1, value_len);
                    out_value[value_len] = '\0';
                    // Trim trailing whitespace/newline
                    size_t len = strlen(out_value);
                    while (len > 0 && (out_value[len - 1] == '\n' || out_value[len - 1] == ' ')) {
                        out_value[--len] = '\0';
                    }
                    *eq = '='; // Restore
                    return FOSSIL_MYSHELL_ERROR_SUCCESS;
                }
            } else {
                if (strcmp(line, key) == 0) {
                    // Extract value (up to end or comment)
                    size_t value_len = out_size - 1;
                    char *comment = strchr(eq + 1, '#');
                    if (comment) {
                        value_len = comment - (eq + 1);
                        if (value_len >= out_size) value_len = out_size - 1;
                    }
                    strncpy(out_value, eq + 1, value_len);
                    out_value[value_len] = '\0';
                    size_t len = strlen(out_value);
                    while (len > 0 && (out_value[len - 1] == '\n' || out_value[len - 1] == ' ')) {
                        out_value[--len] = '\0';
                    }
                    *eq = '='; // Restore
                    return FOSSIL_MYSHELL_ERROR_SUCCESS;
                }
            }
            *eq = '='; // Restore
        }
    }

    return FOSSIL_MYSHELL_ERROR_NOT_FOUND;
}

fossil_bluecrab_myshell_error_t fossil_myshell_del(fossil_bluecrab_myshell_t *db, const char *key) {
    if (!db || !db->is_open) {
        return FOSSIL_MYSHELL_ERROR_INVALID_FILE;
    }
    if (!key || key[0] == '\0') {
        return FOSSIL_MYSHELL_ERROR_INVALID_QUERY;
    }

    uint64_t key_hash = myshell_hash64(key);

    // Read all lines, rewrite excluding the deleted key (matching both key, hash, and type)
    fseek(db->file, 0, SEEK_SET);
    char temp_path[256];
    snprintf(temp_path, sizeof(temp_path), "%s.tmp", db->path);
    FILE *temp_file = fopen(temp_path, "wb");
    if (!temp_file) {
        return FOSSIL_MYSHELL_ERROR_IO;
    }

    char line[1024];
    bool found = false;
    while (fgets(line, sizeof(line), db->file)) {
        char *eq = strchr(line, '=');
        if (eq) {
            *eq = '\0';
            char *hash_comment = strstr(eq + 1, "#hash=");
            char *type_comment = strstr(eq + 1, "#type=");
            if (hash_comment) {
                uint64_t file_hash = 0;
                sscanf(hash_comment, "#hash=%llx", &file_hash);
                if (strcmp(line, key) == 0 && file_hash == key_hash) {
                    // Optionally validate type against FSON type system
                    if (type_comment) {
                        char type_name[32] = {0};
                        int i = 0;
                        type_comment += 6;
                        while (type_comment[i] && !isspace((unsigned char)type_comment[i]) && type_comment[i] != '#' && i < 31) {
                            type_name[i] = type_comment[i];
                            i++;
                        }
                        type_name[i] = '\0';
                        bool valid_type = false;
                        for (size_t j = 0; j <= MYSHELL_FSON_TYPE_DURATION; ++j) {
                            if (strcmp(type_name, myshell_fson_type_names[j]) == 0) {
                                valid_type = true;
                                break;
                            }
                        }
                        // If type is valid, skip line (delete)
                        if (valid_type) {
                            found = true;
                            *eq = '='; // Restore
                            continue;
                        }
                    } else {
                        // No type info, still skip line
                        found = true;
                        *eq = '='; // Restore
                        continue;
                    }
                }
            } else {
                if (strcmp(line, key) == 0) {
                    found = true; // Skip this line
                    *eq = '='; // Restore
                    continue;
                }
            }
            *eq = '='; // Restore
        }
        fputs(line, temp_file);
    }

    fclose(temp_file);
    fclose(db->file);

    if (found) {
        if (remove(db->path) != 0) {
            remove(temp_path);
            db->file = fopen(db->path, "rb+");
            if (!db->file) return FOSSIL_MYSHELL_ERROR_IO;
            return FOSSIL_MYSHELL_ERROR_IO;
        }
        if (rename(temp_path, db->path) != 0) {
            db->file = fopen(db->path, "rb+");
            if (!db->file) return FOSSIL_MYSHELL_ERROR_IO;
            return FOSSIL_MYSHELL_ERROR_IO;
        }
        db->file = fopen(db->path, "rb+");
        if (!db->file) {
            return FOSSIL_MYSHELL_ERROR_IO;
        }
        db->last_modified = time(NULL);
        return FOSSIL_MYSHELL_ERROR_SUCCESS;
    } else {
        remove(temp_path); // No change
        db->file = fopen(db->path, "rb+");
        if (!db->file) {
            return FOSSIL_MYSHELL_ERROR_IO;
        }
        return FOSSIL_MYSHELL_ERROR_NOT_FOUND;
    }
}

fossil_bluecrab_myshell_error_t fossil_myshell_commit(fossil_bluecrab_myshell_t *db, const char *message) {
    if (!db) {
        return FOSSIL_MYSHELL_ERROR_INVALID_FILE;
    }
    if (!db->is_open) {
        return FOSSIL_MYSHELL_ERROR_LOCKED;
    }
    if (!message || strlen(message) == 0) {
        return FOSSIL_MYSHELL_ERROR_INVALID_QUERY;
    }

    // Store the commit message
    if (db->commit_message) {
        free(db->commit_message);
    }
    db->commit_message = myshell_strdup(message);
    if (!db->commit_message) {
        return FOSSIL_MYSHELL_ERROR_OUT_OF_MEMORY;
    }
    db->commit_timestamp = time(NULL);

    // Prepare commit data for hashing
    char commit_data[1024];
    if (snprintf(commit_data, sizeof(commit_data), "%s:%lld", message, (long long)db->commit_timestamp) < 0) {
        return FOSSIL_MYSHELL_ERROR_IO;
    }

    // Update commit hashes (chain)
    db->prev_commit_hash = db->commit_head;
    db->commit_head = myshell_hash64(commit_data);

    // Optionally, create a new commit object (simulate by updating author and parent_branch)
    if (db->author) {
        free(db->author);
    }
    db->author = myshell_strdup("system");
    if (!db->author) {
        return FOSSIL_MYSHELL_ERROR_OUT_OF_MEMORY;
    }

    if (db->parent_branch) {
        free(db->parent_branch);
    }
    db->parent_branch = db->branch ? myshell_strdup(db->branch) : NULL;
    if (db->branch && !db->parent_branch) {
        return FOSSIL_MYSHELL_ERROR_OUT_OF_MEMORY;
    }

    db->next_commit_hash = 0;

    // Write commit info to the file for history (simple append)
    if (fseek(db->file, 0, SEEK_END) != 0) {
        return FOSSIL_MYSHELL_ERROR_IO;
    }
    // FSON v2: commit lines can optionally include a #type=enum for commit type
    // For compatibility, always append #type=enum to commit lines
    if (fprintf(db->file, "#commit %016llx %s %lld #type=%s\n",
                db->commit_head, message, (long long)db->commit_timestamp,
                myshell_fson_type_to_string(MYSHELL_FSON_TYPE_ENUM)) < 0) {
        return FOSSIL_MYSHELL_ERROR_IO;
    }
    fflush(db->file);

    db->last_modified = time(NULL);

    return FOSSIL_MYSHELL_ERROR_SUCCESS;
}

fossil_bluecrab_myshell_error_t fossil_myshell_branch(fossil_bluecrab_myshell_t *db, const char *branch_name) {
    if (!db) {
        return FOSSIL_MYSHELL_ERROR_INVALID_FILE;
    }
    if (!db->is_open) {
        return FOSSIL_MYSHELL_ERROR_LOCKED;
    }
    if (!branch_name || branch_name[0] == '\0') {
        return FOSSIL_MYSHELL_ERROR_INVALID_QUERY;
    }

    // Check for schema mismatch or unsupported version (simulate)
    if (db->commit_head == 0) {
        return FOSSIL_MYSHELL_ERROR_SCHEMA_MISMATCH;
    }

    // Update branch pointer
    if (db->branch) {
        free(db->branch);
    }
    db->branch = myshell_strdup(branch_name);
    if (!db->branch) {
        return FOSSIL_MYSHELL_ERROR_OUT_OF_MEMORY;
    }

    // Set parent branch if not already set
    if (!db->parent_branch) {
        db->parent_branch = myshell_strdup(db->branch);
        if (!db->parent_branch) {
            return FOSSIL_MYSHELL_ERROR_OUT_OF_MEMORY;
        }
    }

    // Update commit_head to branch hash
    db->commit_head = myshell_hash64(branch_name);

    // FSON type system: branch is always type "enum"
    fossil_bluecrab_myshell_fson_type_t type_id = MYSHELL_FSON_TYPE_ENUM;

    // Optionally, write branch info to the file for history (simple append)
    if (fseek(db->file, 0, SEEK_END) != 0) {
        return FOSSIL_MYSHELL_ERROR_IO;
    }
    if (fprintf(db->file, "#branch %016llx %s #type=%s\n", db->commit_head, branch_name, myshell_fson_type_to_string(type_id)) < 0) {
        return FOSSIL_MYSHELL_ERROR_IO;
    }
    fflush(db->file);

    db->last_modified = time(NULL);

    // Update branch pointers and commit chain (simple simulation)
    db->prev_commit_hash = db->commit_head;
    db->next_commit_hash = 0; // No next commit yet

    // Optionally, reset commit message and timestamp for new branch
    if (db->commit_message) {
        free(db->commit_message);
        db->commit_message = NULL;
    }
    db->commit_timestamp = 0;

    return FOSSIL_MYSHELL_ERROR_SUCCESS;
}

fossil_bluecrab_myshell_error_t fossil_myshell_checkout(fossil_bluecrab_myshell_t *db, const char *branch_or_commit) {
    if (!db) {
        return FOSSIL_MYSHELL_ERROR_INVALID_FILE;
    }
    if (!db->is_open) {
        return FOSSIL_MYSHELL_ERROR_LOCKED;
    }
    if (!branch_or_commit || branch_or_commit[0] == '\0') {
        return FOSSIL_MYSHELL_ERROR_INVALID_QUERY;
    }

    uint64_t hash = myshell_hash64(branch_or_commit);

    bool branch_found = false;
    bool commit_found = false;
    char found_branch_name[512] = {0};
    fseek(db->file, 0, SEEK_SET);
    char line[1024];
    while (fgets(line, sizeof(line), db->file)) {
        if (strncmp(line, "#branch ", 8) == 0) {
            char hash_str[17] = {0};
            char name[512] = {0};
            int n = sscanf(line, "#branch %16s %511s", hash_str, name);
            if (n >= 2) {
                uint64_t parsed_hash = 0;
                sscanf(hash_str, "%llx", &parsed_hash);
                if ((strcmp(name, branch_or_commit) == 0) || (parsed_hash == hash)) {
                    branch_found = true;
                    strncpy(found_branch_name, name, sizeof(found_branch_name));
                    found_branch_name[sizeof(found_branch_name) - 1] = '\0';
                    break;
                }
            }
        } else if (strncmp(line, "#commit ", 8) == 0) {
            char hash_str[17] = {0};
            int n = sscanf(line, "#commit %16s", hash_str);
            if (n >= 1) {
                uint64_t parsed_hash = 0;
                sscanf(hash_str, "%llx", &parsed_hash);
                if (parsed_hash == hash) {
                    commit_found = true;
                    break;
                }
            }
        }
    }

    if (!branch_found && !commit_found) {
        return FOSSIL_MYSHELL_ERROR_NOT_FOUND;
    }

    // Set branch name and update commit_head to hash
    if (db->branch) {
        free(db->branch);
    }
    db->branch = myshell_strdup(branch_found ? found_branch_name : branch_or_commit);
    if (!db->branch) {
        return FOSSIL_MYSHELL_ERROR_OUT_OF_MEMORY;
    }
    db->commit_head = hash;

    db->last_modified = time(NULL);

    return FOSSIL_MYSHELL_ERROR_SUCCESS;
}

fossil_bluecrab_myshell_error_t fossil_myshell_merge(fossil_bluecrab_myshell_t *db, const char *source_branch, const char *message) {
    if (!db) {
        return FOSSIL_MYSHELL_ERROR_INVALID_FILE;
    }
    if (!db->is_open) {
        return FOSSIL_MYSHELL_ERROR_LOCKED;
    }
    if (!source_branch || source_branch[0] == '\0' || !message || message[0] == '\0') {
        return FOSSIL_MYSHELL_ERROR_INVALID_QUERY;
    }

    // Check for schema mismatch or unsupported version (simulate)
    if (db->commit_head == 0) {
        return FOSSIL_MYSHELL_ERROR_SCHEMA_MISMATCH;
    }

    // Find the source branch and optionally record its hash/name and type
    uint64_t source_hash = myshell_hash64(source_branch);
    bool branch_found = false;
    char found_branch_name[512] = {0};
    fossil_bluecrab_myshell_fson_type_t branch_type = MYSHELL_FSON_TYPE_ENUM;
    fseek(db->file, 0, SEEK_SET);
    char line[1024];
    while (fgets(line, sizeof(line), db->file)) {
        if (strncmp(line, "#branch ", 8) == 0) {
            char hash_str[17] = {0};
            char name[512] = {0};
            char type_name[32] = {0};
            int n = sscanf(line, "#branch %16s %511s #type=%31s", hash_str, name, type_name);
            if (n >= 2) {
                uint64_t parsed_hash = 0;
                sscanf(hash_str, "%llx", &parsed_hash);
                if ((strcmp(name, source_branch) == 0) || (parsed_hash == source_hash)) {
                    branch_found = true;
                    strncpy(found_branch_name, name, sizeof(found_branch_name) - 1);
                    found_branch_name[sizeof(found_branch_name) - 1] = '\0';
                    if (n == 3) {
                        for (size_t i = 0; i <= MYSHELL_FSON_TYPE_DURATION; ++i) {
                            if (strcmp(type_name, myshell_fson_type_names[i]) == 0) {
                                branch_type = (fossil_bluecrab_myshell_fson_type_t)i;
                                break;
                            }
                        }
                    }
                    break;
                }
            }
        }
    }

    if (!branch_found) {
        return FOSSIL_MYSHELL_ERROR_NOT_FOUND;
    }

    // Create a merge commit
    if (db->commit_message) {
        free(db->commit_message);
    }
    db->commit_message = myshell_strdup(message);
    if (!db->commit_message) {
        return FOSSIL_MYSHELL_ERROR_OUT_OF_MEMORY;
    }
    db->commit_timestamp = time(NULL);

    // Prepare commit data for hashing, include source branch name
    char commit_data[1024];
    if (snprintf(commit_data, sizeof(commit_data), "Merge %s: %s:%lld", found_branch_name, message, (long long)db->commit_timestamp) < 0) {
        return FOSSIL_MYSHELL_ERROR_IO;
    }

    // Update commit hashes (chain)
    db->prev_commit_hash = db->commit_head;
    db->commit_head = myshell_hash64(commit_data);
    db->next_commit_hash = 0;

    // Optionally, append merge info to file for history, include FSON type
    if (fseek(db->file, 0, SEEK_END) == 0) {
        fprintf(db->file, "#merge %016llx %s %s %lld #type=%s\n",
            db->commit_head, found_branch_name, message, (long long)db->commit_timestamp,
            myshell_fson_type_to_string(branch_type));
        fflush(db->file);
    }

    db->last_modified = time(NULL);
    return FOSSIL_MYSHELL_ERROR_SUCCESS;
}

fossil_bluecrab_myshell_error_t fossil_myshell_revert(fossil_bluecrab_myshell_t *db, const char *commit_hash) {
    if (!db) {
        return FOSSIL_MYSHELL_ERROR_INVALID_FILE;
    }
    if (!db->is_open) {
        return FOSSIL_MYSHELL_ERROR_LOCKED;
    }
    if (!commit_hash || commit_hash[0] == '\0') {
        return FOSSIL_MYSHELL_ERROR_INVALID_QUERY;
    }

    uint64_t hash = myshell_hash64(commit_hash);

    bool commit_found = false;
    fossil_bluecrab_myshell_fson_type_t found_type = MYSHELL_FSON_TYPE_NULL;
    fseek(db->file, 0, SEEK_SET);
    char line[1024];
    while (fgets(line, sizeof(line), db->file)) {
        if (strncmp(line, "#commit ", 8) == 0) {
            char hash_str[17] = {0};
            char type_name[32] = {0};
            int n = sscanf(line, "#commit %16s %*[^#] #type=%31s", hash_str, type_name);
            if (n >= 1) {
                uint64_t parsed_hash = 0;
                sscanf(hash_str, "%llx", &parsed_hash);
                if (parsed_hash == hash) {
                    commit_found = true;
                    // Validate FSON type if present
                    if (n == 2) {
                        for (size_t i = 0; i <= MYSHELL_FSON_TYPE_DURATION; ++i) {
                            if (strcmp(type_name, myshell_fson_type_names[i]) == 0) {
                                found_type = (fossil_bluecrab_myshell_fson_type_t)i;
                                break;
                            }
                        }
                        // If type is not valid, treat as config error
                        if (found_type == MYSHELL_FSON_TYPE_NULL && strcmp(type_name, myshell_fson_type_names[0]) != 0) {
                            return FOSSIL_MYSHELL_ERROR_CONFIG_INVALID;
                        }
                    } else {
                        found_type = MYSHELL_FSON_TYPE_ENUM;
                    }
                    break;
                }
            }
        }
    }

    if (!commit_found) {
        return FOSSIL_MYSHELL_ERROR_NOT_FOUND;
    }

    // Set commit_head to the specified commit hash
    db->commit_head = hash;

    db->last_modified = time(NULL);

    // Optionally, could store found_type in db if needed for future type-aware logic

    return FOSSIL_MYSHELL_ERROR_SUCCESS;
}

fossil_bluecrab_myshell_error_t fossil_myshell_stage(fossil_bluecrab_myshell_t *db, const char *key, const char *type, const char *value) {
    if (!db || !db->is_open) {
        return FOSSIL_MYSHELL_ERROR_INVALID_FILE;
    }
    if (!key || !type || !value) {
        return FOSSIL_MYSHELL_ERROR_INVALID_QUERY;
    }
    if (key[0] == '\0' || type[0] == '\0') {
        return FOSSIL_MYSHELL_ERROR_INVALID_QUERY;
    }

    // Validate type against FSON type system
    fossil_bluecrab_myshell_fson_type_t type_id = MYSHELL_FSON_TYPE_NULL;
    bool valid_type = false;
    for (size_t i = 0; i <= MYSHELL_FSON_TYPE_DURATION; ++i) {
        if (strcmp(type, myshell_fson_type_names[i]) == 0) {
            type_id = (fossil_bluecrab_myshell_fson_type_t)i;
            valid_type = true;
            break;
        }
    }
    if (!valid_type) {
        return FOSSIL_MYSHELL_ERROR_CONFIG_INVALID;
    }

    uint64_t key_hash = myshell_hash64(key);

    // Remove any previous staged entry for this key before adding new
    fseek(db->file, 0, SEEK_SET);
    char temp_path[256];
    snprintf(temp_path, sizeof(temp_path), "%s.tmp", db->path);
    FILE *temp_file = fopen(temp_path, "wb");
    if (!temp_file) {
        return FOSSIL_MYSHELL_ERROR_IO;
    }

    char line[1024];
    while (fgets(line, sizeof(line), db->file)) {
        if (strncmp(line, "#stage ", 7) == 0) {
            char *eq = strchr(line + 7, '=');
            if (eq) {
                *eq = '\0';
                char *hash_comment = strstr(eq + 1, "#hash=");
                if (hash_comment) {
                    uint64_t file_hash = 0;
                    sscanf(hash_comment, "#hash=%llx", &file_hash);
                    if (strcmp(line + 7, key) == 0 && file_hash == key_hash) {
                        *eq = '='; // Restore
                        continue;
                    }
                } else {
                    if (strcmp(line + 7, key) == 0) {
                        *eq = '='; // Restore
                        continue;
                    }
                }
                *eq = '='; // Restore
            }
        }
        fputs(line, temp_file);
    }

    // Add new staged entry at the end, using FSON type system
    fprintf(temp_file, "#stage %s=%s #type=%s #hash=%016llx\n", key, value, myshell_fson_type_to_string(type_id), key_hash);

    fclose(temp_file);
    fclose(db->file);

    if (remove(db->path) != 0) {
        remove(temp_path);
        db->file = fopen(db->path, "rb+");
        if (!db->file) return FOSSIL_MYSHELL_ERROR_IO;
        return FOSSIL_MYSHELL_ERROR_IO;
    }
    if (rename(temp_path, db->path) != 0) {
        db->file = fopen(db->path, "rb+");
        if (!db->file) return FOSSIL_MYSHELL_ERROR_IO;
        return FOSSIL_MYSHELL_ERROR_IO;
    }

    db->file = fopen(db->path, "rb+");
    if (!db->file) {
        return FOSSIL_MYSHELL_ERROR_IO;
    }

    db->last_modified = time(NULL);
    return FOSSIL_MYSHELL_ERROR_SUCCESS;
}

fossil_bluecrab_myshell_error_t fossil_myshell_unstage(fossil_bluecrab_myshell_t *db, const char *key) {
    if (!db || !db->is_open) {
        return FOSSIL_MYSHELL_ERROR_INVALID_FILE;
    }
    if (!key || key[0] == '\0') {
        return FOSSIL_MYSHELL_ERROR_INVALID_QUERY;
    }

    uint64_t key_hash = myshell_hash64(key);

    // Read all lines, rewrite excluding the unstaged key (matching key, hash, and FSON type)
    fseek(db->file, 0, SEEK_SET);
    char temp_path[256];
    snprintf(temp_path, sizeof(temp_path), "%s.tmp", db->path);
    FILE *temp_file = fopen(temp_path, "wb");
    if (!temp_file) {
        return FOSSIL_MYSHELL_ERROR_IO;
    }

    char line[1024];
    bool found = false;
    while (fgets(line, sizeof(line), db->file)) {
        if (strncmp(line, "#stage ", 7) == 0) {
            char *eq = strchr(line + 7, '=');
            if (eq) {
                *eq = '\0';
                char *hash_comment = strstr(eq + 1, "#hash=");
                char *type_comment = strstr(eq + 1, "#type=");
                bool valid_type = false;
                if (type_comment) {
                    char type_name[32] = {0};
                    int i = 0;
                    type_comment += 6;
                    while (type_comment[i] && !isspace((unsigned char)type_comment[i]) && type_comment[i] != '#' && i < 31) {
                        type_name[i] = type_comment[i];
                        i++;
                    }
                    type_name[i] = '\0';
                    for (size_t j = 0; j <= MYSHELL_FSON_TYPE_DURATION; ++j) {
                        if (strcmp(type_name, myshell_fson_type_names[j]) == 0) {
                            valid_type = true;
                            break;
                        }
                    }
                }
                if (hash_comment) {
                    uint64_t file_hash = 0;
                    sscanf(hash_comment, "#hash=%llx", &file_hash);
                    if (strcmp(line + 7, key) == 0 && file_hash == key_hash && (type_comment == NULL || valid_type)) {
                        found = true; // Skip this line
                        *eq = '='; // Restore
                        continue;
                    }
                } else {
                    if (strcmp(line + 7, key) == 0 && (type_comment == NULL || valid_type)) {
                        found = true; // Skip this line
                        *eq = '='; // Restore
                        continue;
                    }
                }
                *eq = '='; // Restore
            }
        }
        fputs(line, temp_file);
    }

    fclose(temp_file);
    fclose(db->file);

    if (found) {
        if (remove(db->path) != 0) {
            remove(temp_path);
            db->file = fopen(db->path, "rb+");
            if (!db->file) return FOSSIL_MYSHELL_ERROR_IO;
            return FOSSIL_MYSHELL_ERROR_IO;
        }
        if (rename(temp_path, db->path) != 0) {
            db->file = fopen(db->path, "rb+");
            if (!db->file) return FOSSIL_MYSHELL_ERROR_IO;
        }
    }

    db->file = fopen(db->path, "rb+");
    if (!db->file) {
        return FOSSIL_MYSHELL_ERROR_IO;
    }

    return found ? FOSSIL_MYSHELL_ERROR_SUCCESS : FOSSIL_MYSHELL_ERROR_NOT_FOUND;
}

fossil_bluecrab_myshell_error_t fossil_myshell_tag(fossil_bluecrab_myshell_t *db, const char *commit_hash, const char *tag_name) {
    if (!db) {
        return FOSSIL_MYSHELL_ERROR_INVALID_FILE;
    }
    if (!db->is_open) {
        return FOSSIL_MYSHELL_ERROR_LOCKED;
    }
    if (!commit_hash || !tag_name || tag_name[0] == '\0') {
        return FOSSIL_MYSHELL_ERROR_INVALID_QUERY;
    }

    uint64_t hash = myshell_hash64(commit_hash);

    bool commit_found = false;
    fossil_bluecrab_myshell_fson_type_t commit_type = MYSHELL_FSON_TYPE_ENUM;
    fseek(db->file, 0, SEEK_SET);
    char line[1024];
    while (fgets(line, sizeof(line), db->file)) {
        if (strncmp(line, "#commit ", 8) == 0) {
            char hash_str[17] = {0};
            char type_name[32] = {0};
            int n = sscanf(line, "#commit %16s %*[^#] #type=%31s", hash_str, type_name);
            if (n >= 1) {
                uint64_t parsed_hash = 0;
                sscanf(hash_str, "%llx", &parsed_hash);
                if (parsed_hash == hash) {
                    commit_found = true;
                    if (n == 2) {
                        for (size_t i = 0; i <= MYSHELL_FSON_TYPE_DURATION; ++i) {
                            if (strcmp(type_name, myshell_fson_type_names[i]) == 0) {
                                commit_type = (fossil_bluecrab_myshell_fson_type_t)i;
                                break;
                            }
                        }
                    }
                    break;
                }
            }
        }
    }

    if (!commit_found) {
        return FOSSIL_MYSHELL_ERROR_NOT_FOUND;
    }

    // Write tag info to the file for history (simple append), include FSON type
    if (fseek(db->file, 0, SEEK_END) != 0) {
        return FOSSIL_MYSHELL_ERROR_IO;
    }
    if (fprintf(db->file, "#tag %016llx %s #type=%s\n", hash, tag_name, myshell_fson_type_to_string(commit_type)) < 0) {
        return FOSSIL_MYSHELL_ERROR_IO;
    }
    fflush(db->file);

    db->last_modified = time(NULL);

    return FOSSIL_MYSHELL_ERROR_SUCCESS;
}

fossil_bluecrab_myshell_error_t fossil_myshell_log(fossil_bluecrab_myshell_t *db, fossil_myshell_commit_cb cb, void *user) {
    if (!db) {
        return FOSSIL_MYSHELL_ERROR_INVALID_FILE;
    }
    if (!db->is_open) {
        return FOSSIL_MYSHELL_ERROR_LOCKED;
    }
    if (!cb) {
        return FOSSIL_MYSHELL_ERROR_INVALID_QUERY;
    }

    // Iterate over file and invoke callback for each commit line, parsing FSON type if present
    if (fseek(db->file, 0, SEEK_SET) != 0) {
        return FOSSIL_MYSHELL_ERROR_IO;
    }
    char line[1024];
    while (fgets(line, sizeof(line), db->file)) {
        if (strncmp(line, "#commit ", 8) == 0) {
            char hash_str[17] = {0};
            char message[512] = {0};
            long long timestamp = 0;
            char type_name[32] = {0};
            int n = sscanf(line, "#commit %16s %511[^\n] %lld #type=%31s", hash_str, message, &timestamp, type_name);

            // FSON type system: parse type if present, default to ENUM
            // (commit_type is not used, so skip parsing)

            if (n >= 3) {
                uint64_t parsed_hash = 0;
                sscanf(hash_str, "%llx", &parsed_hash);
                char commit_data[1024];
                snprintf(commit_data, sizeof(commit_data), "%s:%lld", message, timestamp);
                uint64_t computed_hash = myshell_hash64(commit_data);
                if (parsed_hash == computed_hash) {
                    // Optionally, you could pass commit_type to the callback if its signature supports it
                    if (!cb(hash_str, message, user)) {
                        return FOSSIL_MYSHELL_ERROR_SUCCESS;
                    }
                } else {
                    return FOSSIL_MYSHELL_ERROR_INTEGRITY;
                }
            } else if (n == 2) {
                if (!cb(hash_str, message, user)) {
                    return FOSSIL_MYSHELL_ERROR_SUCCESS;
                }
            } else {
                return FOSSIL_MYSHELL_ERROR_PARSE_FAILED;
            }
        }
    }

    return FOSSIL_MYSHELL_ERROR_SUCCESS;
}

fossil_bluecrab_myshell_error_t fossil_myshell_backup(fossil_bluecrab_myshell_t *db, const char *backup_path) {
    if (!db || !db->is_open) {
        return FOSSIL_MYSHELL_ERROR_INVALID_FILE;
    }
    if (!backup_path || backup_path[0] == '\0') {
        return FOSSIL_MYSHELL_ERROR_CONFIG_INVALID;
    }

    FILE *backup_file = fopen(backup_path, "wb");
    if (!backup_file) {
        return FOSSIL_MYSHELL_ERROR_BACKUP_FAILED;
    }

    // Write a hash of the backup path as a comment for integrity
    uint64_t backup_hash = myshell_hash64(backup_path);
    if (fprintf(backup_file, "#backup_hash=%016llx\n", backup_hash) < 0) {
        fclose(backup_file);
        return FOSSIL_MYSHELL_ERROR_IO;
    }

    // Write FSON type system header for backup file
    fprintf(backup_file, "#fson_types=");
    for (size_t i = 0; i <= MYSHELL_FSON_TYPE_DURATION; ++i) {
        fprintf(backup_file, "%s", myshell_fson_type_names[i]);
        if (i < MYSHELL_FSON_TYPE_DURATION) fprintf(backup_file, ",");
    }
    fprintf(backup_file, "\n");

    fseek(db->file, 0, SEEK_SET);
    char buffer[4096];
    size_t bytes;
    while ((bytes = fread(buffer, 1, sizeof(buffer), db->file)) > 0) {
        if (fwrite(buffer, 1, bytes, backup_file) != bytes) {
            fclose(backup_file);
            return FOSSIL_MYSHELL_ERROR_IO;
        }
    }

    if (ferror(db->file)) {
        fclose(backup_file);
        return FOSSIL_MYSHELL_ERROR_IO;
    }

    fclose(backup_file);
    fseek(db->file, 0, SEEK_END); // Restore file position
    return FOSSIL_MYSHELL_ERROR_SUCCESS;
}

fossil_bluecrab_myshell_error_t fossil_myshell_restore(const char *backup_path, const char *target_path) {
    if (!backup_path || !target_path) {
        return FOSSIL_MYSHELL_ERROR_INVALID_FILE;
    }

    FILE *backup_file = fopen(backup_path, "rb");
    if (!backup_file) {
        return FOSSIL_MYSHELL_ERROR_FILE_NOT_FOUND;
    }

    // Verify backup hash for integrity (first line should be "#backup_hash=...")
    char hash_line[128];
    uint64_t expected_hash = myshell_hash64(backup_path);
    if (fgets(hash_line, sizeof(hash_line), backup_file)) {
        uint64_t file_hash = 0;
        if (strncmp(hash_line, "#backup_hash=", 13) == 0) {
            sscanf(hash_line, "#backup_hash=%llx", &file_hash);
            if (file_hash != expected_hash) {
                fclose(backup_file);
                return FOSSIL_MYSHELL_ERROR_INTEGRITY;
            }
        } else {
            fclose(backup_file);
            return FOSSIL_MYSHELL_ERROR_CORRUPTED;
        }
    } else {
        fclose(backup_file);
        return FOSSIL_MYSHELL_ERROR_CORRUPTED;
    }

    // Next line should be FSON type system header
    char fson_line[256];
    if (fgets(fson_line, sizeof(fson_line), backup_file)) {
        if (strncmp(fson_line, "#fson_types=", 12) != 0) {
            fclose(backup_file);
            return FOSSIL_MYSHELL_ERROR_CONFIG_INVALID;
        }
        // Optionally, validate that all types in header are known
        char *types = fson_line + 12;
        char *token = strtok(types, ",\n");
        while (token) {
            bool valid_type = false;
            for (size_t i = 0; i <= MYSHELL_FSON_TYPE_DURATION; ++i) {
                if (strcmp(token, myshell_fson_type_names[i]) == 0) {
                    valid_type = true;
                    break;
                }
            }
            if (!valid_type) {
                fclose(backup_file);
                return FOSSIL_MYSHELL_ERROR_CONFIG_INVALID;
            }
            token = strtok(NULL, ",\n");
        }
    } else {
        fclose(backup_file);
        return FOSSIL_MYSHELL_ERROR_CONFIG_INVALID;
    }

    FILE *target_file = fopen(target_path, "wb");
    if (!target_file) {
        fclose(backup_file);
        return FOSSIL_MYSHELL_ERROR_IO;
    }

    // Write the validated hash and FSON header to the target file
    fprintf(target_file, "%s", hash_line);
    fprintf(target_file, "%s", fson_line);

    char buffer[4096];
    size_t bytes;
    while ((bytes = fread(buffer, 1, sizeof(buffer), backup_file)) > 0) {
        if (fwrite(buffer, 1, bytes, target_file) != bytes) {
            fclose(backup_file);
            fclose(target_file);
            return FOSSIL_MYSHELL_ERROR_IO;
        }
    }

    if (ferror(backup_file)) {
        fclose(backup_file);
        fclose(target_file);
        return FOSSIL_MYSHELL_ERROR_IO;
    }

    fclose(backup_file);
    fclose(target_file);
    return FOSSIL_MYSHELL_ERROR_SUCCESS;
}

const char *fossil_myshell_errstr(fossil_bluecrab_myshell_error_t err) {
    switch (err) {
        case FOSSIL_MYSHELL_ERROR_SUCCESS: return "Success";
        case FOSSIL_MYSHELL_ERROR_INVALID_FILE: return "Invalid file";
        case FOSSIL_MYSHELL_ERROR_FILE_NOT_FOUND: return "File not found";
        case FOSSIL_MYSHELL_ERROR_IO: return "I/O error";
        case FOSSIL_MYSHELL_ERROR_INVALID_QUERY: return "Invalid query";
        case FOSSIL_MYSHELL_ERROR_CONCURRENCY: return "Concurrency error";
        case FOSSIL_MYSHELL_ERROR_NOT_FOUND: return "Not found";
        case FOSSIL_MYSHELL_ERROR_PERMISSION_DENIED: return "Permission denied";
        case FOSSIL_MYSHELL_ERROR_CORRUPTED: return "Corrupted data";
        case FOSSIL_MYSHELL_ERROR_OUT_OF_MEMORY: return "Out of memory";
        case FOSSIL_MYSHELL_ERROR_UNSUPPORTED: return "Unsupported operation";
        case FOSSIL_MYSHELL_ERROR_LOCKED: return "Resource locked";
        case FOSSIL_MYSHELL_ERROR_TIMEOUT: return "Operation timed out";
        case FOSSIL_MYSHELL_ERROR_ALREADY_EXISTS: return "Already exists";
        case FOSSIL_MYSHELL_ERROR_BACKUP_FAILED: return "Backup failed";
        case FOSSIL_MYSHELL_ERROR_PARSE_FAILED: return "Parse failed";
        case FOSSIL_MYSHELL_ERROR_RESTORE_FAILED: return "Restore failed";
        case FOSSIL_MYSHELL_ERROR_LOCK_FAILED: return "Lock failed";
        case FOSSIL_MYSHELL_ERROR_SCHEMA_MISMATCH: return "Schema mismatch";
        case FOSSIL_MYSHELL_ERROR_VERSION_UNSUPPORTED: return "Version unsupported";
        case FOSSIL_MYSHELL_ERROR_INDEX_CORRUPTED: return "Index corrupted";
        case FOSSIL_MYSHELL_ERROR_INTEGRITY: return "Integrity check failed";
        case FOSSIL_MYSHELL_ERROR_TRANSACTION_FAILED: return "Transaction failed";
        case FOSSIL_MYSHELL_ERROR_CAPACITY_EXCEEDED: return "Capacity exceeded";
        case FOSSIL_MYSHELL_ERROR_CONFIG_INVALID: return "Invalid configuration";
        case FOSSIL_MYSHELL_ERROR_UNKNOWN:
        default:
            return "Unknown error";
    }
}

fossil_bluecrab_myshell_error_t fossil_myshell_check_integrity(fossil_bluecrab_myshell_t *db) {
    if (!db || !db->is_open) {
        return FOSSIL_MYSHELL_ERROR_INVALID_FILE;
    }

    // Check file size consistency
    if (fseek(db->file, 0, SEEK_END) != 0)
        return FOSSIL_MYSHELL_ERROR_IO;
    size_t current_size = (size_t)ftell(db->file);
    if (current_size != db->file_size) {
        return FOSSIL_MYSHELL_ERROR_CORRUPTED;
    }
    if (fseek(db->file, 0, SEEK_SET) != 0)
        return FOSSIL_MYSHELL_ERROR_IO;

    char line[1024];
    while (fgets(line, sizeof(line), db->file)) {
        // Commit integrity: check hash and FSON type
        if (strncmp(line, "#commit ", 8) == 0) {
            char hash_str[17] = {0};
            char message[512] = {0};
            long long timestamp = 0;
            char type_name[32] = {0};
            int n = sscanf(line, "#commit %16s %511[^\n] %lld #type=%31s", hash_str, message, &timestamp, type_name);

            if (n == 4) {
                bool valid_type = false;
                for (size_t i = 0; i <= MYSHELL_FSON_TYPE_DURATION; ++i) {
                    if (strcmp(type_name, myshell_fson_type_names[i]) == 0) {
                        valid_type = true;
                        break;
                    }
                }
                if (!valid_type) {
                    return FOSSIL_MYSHELL_ERROR_CONFIG_INVALID;
                }
            }
            if (n >= 3) {
                uint64_t parsed_hash = 0;
                sscanf(hash_str, "%llx", &parsed_hash);
                char commit_data[1024];
                snprintf(commit_data, sizeof(commit_data), "%s:%lld", message, timestamp);
                uint64_t computed_hash = myshell_hash64(commit_data);
                if (parsed_hash != computed_hash) {
                    return FOSSIL_MYSHELL_ERROR_INTEGRITY;
                }
            } else {
                return FOSSIL_MYSHELL_ERROR_PARSE_FAILED;
            }
        }
        // Branch/tag/merge integrity: check FSON type if present
        else if (strncmp(line, "#branch ", 8) == 0 ||
                 strncmp(line, "#tag ", 5) == 0 ||
                 strncmp(line, "#merge ", 7) == 0) {
            char type_name[32] = {0};
            char *type_comment = strstr(line, "#type=");
            if (type_comment) {
                type_comment += 6;
                int i = 0;
                while (type_comment[i] && !isspace((unsigned char)type_comment[i]) && type_comment[i] != '#' && i < 31) {
                    type_name[i] = type_comment[i];
                    i++;
                }
                type_name[i] = '\0';
                bool valid_type = false;
                for (size_t j = 0; j <= MYSHELL_FSON_TYPE_DURATION; ++j) {
                    if (strcmp(type_name, myshell_fson_type_names[j]) == 0) {
                        valid_type = true;
                        break;
                    }
                }
                if (!valid_type) {
                    return FOSSIL_MYSHELL_ERROR_CONFIG_INVALID;
                }
            }
        }
        // Key-value integrity: check hash and FSON type
        else {
            char *eq = strchr(line, '=');
            if (eq) {
                *eq = '\0';
                char *hash_comment = strstr(eq + 1, "#hash=");
                char *type_comment = strstr(eq + 1, "#type=");
                if (type_comment) {
                    type_comment += 6;
                    char type_name[32] = {0};
                    int i = 0;
                    while (type_comment[i] && !isspace((unsigned char)type_comment[i]) && type_comment[i] != '#' && i < 31) {
                        type_name[i] = type_comment[i];
                        i++;
                    }
                    type_name[i] = '\0';
                    bool valid_type = false;
                    for (size_t j = 0; j <= MYSHELL_FSON_TYPE_DURATION; ++j) {
                        if (strcmp(type_name, myshell_fson_type_names[j]) == 0) {
                            valid_type = true;
                            break;
                        }
                    }
                    if (!valid_type) {
                        *eq = '='; // Restore
                        return FOSSIL_MYSHELL_ERROR_CONFIG_INVALID;
                    }
                }
                if (hash_comment) {
                    uint64_t file_hash = 0;
                    sscanf(hash_comment, "#hash=%llx", &file_hash);
                    uint64_t key_hash = myshell_hash64(line);
                    if (file_hash != key_hash) {
                        *eq = '='; // Restore
                        return FOSSIL_MYSHELL_ERROR_INTEGRITY;
                    }
                }
                *eq = '='; // Restore
            }
        }
    }

    return FOSSIL_MYSHELL_ERROR_SUCCESS;
}

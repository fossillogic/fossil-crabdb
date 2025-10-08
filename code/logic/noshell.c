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
#include "fossil/crabdb/noshell.h"

/**
 * @brief Implements the core logic for the Fossil BlueCrab .noshell file database.
 *
 * This file provides functions for managing a simple versioned key-value store
 * using the ".noshell" file format. It supports operations such as open, create,
 * close, insert, find, update, remove, backup, restore, error string conversion,
 * and integrity checking.
 *
 * ## FSON Type System
 * - The FSON type system is defined by the lookup table `noshell_fson_type_names[]`:
 *   ```
 *   static const char *noshell_fson_type_names[] = {
 *       "null",      // NOSHELL_FSON_TYPE_NULL
 *       "bool",      // NOSHELL_FSON_TYPE_BOOL
 *       "i8",        // NOSHELL_FSON_TYPE_I8
 *       "i16",       // NOSHELL_FSON_TYPE_I16
 *       "i32",       // NOSHELL_FSON_TYPE_I32
 *       "i64",       // NOSHELL_FSON_TYPE_I64
 *       "u8",        // NOSHELL_FSON_TYPE_U8
 *       "u16",       // NOSHELL_FSON_TYPE_U16
 *       "u32",       // NOSHELL_FSON_TYPE_U32
 *       "u64",       // NOSHELL_FSON_TYPE_U64
 *       "f32",       // NOSHELL_FSON_TYPE_F32
 *       "f64",       // NOSHELL_FSON_TYPE_F64
 *       "oct",       // NOSHELL_FSON_TYPE_OCT
 *       "hex",       // NOSHELL_FSON_TYPE_HEX
 *       "bin",       // NOSHELL_FSON_TYPE_BIN
 *       "char",      // NOSHELL_FSON_TYPE_CHAR
 *       "cstr",      // NOSHELL_FSON_TYPE_CSTR
 *       "array",     // NOSHELL_FSON_TYPE_ARRAY
 *       "object",    // NOSHELL_FSON_TYPE_OBJECT
 *       "enum",      // NOSHELL_FSON_TYPE_ENUM
 *       "datetime",  // NOSHELL_FSON_TYPE_DATETIME
 *       "duration"   // NOSHELL_FSON_TYPE_DURATION
 *   };
 *   ```
 * - The function `noshell_fson_type_to_string` converts a type enum to its string name.
 *
 * ## .noshell File Format Overview
 * - Each .noshell file is a plain text file with lines representing key-value pairs
 *   and metadata.
 * - Key-value pairs are stored as: `key=value #type=TYPE #hash=KEYHASH`
 *   - `TYPE` is the FSON type name (see noshell_fson_type_names).
 *   - `KEYHASH` is a 64-bit hash of the key for integrity verification.
 * - Document IDs are recorded as: `#id=HASH`
 * - FSON type system header: `#fson_types=null,bool,i8,i16,i32,i64,u8,u16,u32,u64,f32,f64,oct,hex,bin,char,cstr,array,object,enum,datetime,duration`
 *
 * ## Sample .noshell File Contents
 * ```
 * key1=value1 #type=i32 #hash=0123456789abcdef #id=abcdef0123456789
 * key2=value2 #type=cstr #hash=abcdef0123456789 #id=0123456789abcdef
 * ```
 *
 * ## More Sample .noshell File Contents
 * ```
 * username=alice #type=cstr #hash=1a2b3c4d5e6f7890 #id=1a2b3c4d5e6f7890
 * password=secret #type=cstr #hash=0f1e2d3c4b5a6978 #id=0f1e2d3c4b5a6978
 * ```
 *
 * ## Another Example
 * ```
 * config=enabled #type=bool #hash=deadbeefcafebabe #id=deadbeefcafebabe
 * mode=fast #type=enum #hash=beefdeadbabecafe #id=beefdeadbabecafe
 * ```
 *
 * ## Fourth Example
 * ```
 * foo=bar #type=cstr #hash=1111222233334444 #id=1111222233334444
 * baz=qux #type=cstr #hash=5555666677778888 #id=5555666677778888
 * ```
 *
 * ## Fifth Example
 * ```
 * alpha=beta #type=cstr #hash=9999aaaabbbbcccc #id=9999aaaabbbbcccc
 * gamma=delta #type=cstr #hash=ddddccccbbbbaaaa #id=ddddccccbbbbaaaa
 * ```
 *
 * ## Main Functions
 * - `noshell_hash64`: Computes a 64-bit hash for strings (MurmurHash3 variant).
 * - `fossil_bluecrab_noshell_open_database`: Opens an existing .noshell database file.
 * - `fossil_bluecrab_noshell_create_database`: Creates a new .noshell database file.
 * - `fossil_bluecrab_noshell_delete_database`: Deletes a database file.
 * - `fossil_bluecrab_noshell_insert`: Inserts a document.
 * - `fossil_bluecrab_noshell_insert_with_id`: Inserts a document and returns its ID.
 * - `fossil_bluecrab_noshell_find`: Finds a document by query.
 * - `fossil_bluecrab_noshell_update`: Updates a document.
 * - `fossil_bluecrab_noshell_remove`: Removes a document.
 * - `fossil_bluecrab_noshell_backup_database`: Creates a backup of the database.
 * - `fossil_bluecrab_noshell_restore_database`: Restores a database from backup.
 * - `fossil_bluecrab_noshell_verify_database`: Verifies file and document integrity.
 * - `fossil_bluecrab_noshell_first_document`: Gets the first document ID.
 * - `fossil_bluecrab_noshell_next_document`: Gets the next document ID.
 * - `fossil_bluecrab_noshell_count_documents`: Counts the number of documents.
 * - `fossil_bluecrab_noshell_get_file_size`: Gets the database file size.
 * - `fossil_bluecrab_noshell_validate_extension`: Validates file extension.
 * - `fossil_bluecrab_noshell_validate_document`: Validates document format.
 *
 * ## Error Handling
 * All functions return a `fossil_bluecrab_noshell_error_t` code indicating success or the type of error.
 *
 * ## Usage Notes
 * - Only files with the ".noshell" extension are supported.
 * - All operations are performed directly on the file; there is no in-memory caching.
 * - Integrity of data is ensured via hashes for keys and documents.
 * - The API is designed for simple key-value storage with basic integrity features.
 * - The FSON type system is enforced for all key-value and metadata entries.
 */

// Lookup table for FSON type system
static const char *noshell_fson_type_names[] = {
    "null",      // NOSHELL_FSON_TYPE_NULL
    "bool",      // NOSHELL_FSON_TYPE_BOOL
    "i8",        // NOSHELL_FSON_TYPE_I8
    "i16",       // NOSHELL_FSON_TYPE_I16
    "i32",       // NOSHELL_FSON_TYPE_I32
    "i64",       // NOSHELL_FSON_TYPE_I64
    "u8",        // NOSHELL_FSON_TYPE_U8
    "u16",       // NOSHELL_FSON_TYPE_U16
    "u32",       // NOSHELL_FSON_TYPE_U32
    "u64",       // NOSHELL_FSON_TYPE_U64
    "f32",       // NOSHELL_FSON_TYPE_F32
    "f64",       // NOSHELL_FSON_TYPE_F64
    "oct",       // NOSHELL_FSON_TYPE_OCT
    "hex",       // NOSHELL_FSON_TYPE_HEX
    "bin",       // NOSHELL_FSON_TYPE_BIN
    "char",      // NOSHELL_FSON_TYPE_CHAR
    "cstr",      // NOSHELL_FSON_TYPE_CSTR
    "array",     // NOSHELL_FSON_TYPE_ARRAY
    "object",    // NOSHELL_FSON_TYPE_OBJECT
    "enum",      // NOSHELL_FSON_TYPE_ENUM
    "datetime",  // NOSHELL_FSON_TYPE_DATETIME
    "duration"   // NOSHELL_FSON_TYPE_DURATION
};

static inline const char *noshell_fson_type_to_string(fossil_bluecrab_noshell_fson_type_t type) {
    if (type < 0 || type > NOSHELL_FSON_TYPE_DURATION)
        return "unknown";
    return noshell_fson_type_names[type];
}

/**
 * Custom strdup implementation.
 */
static char *noshell_strdup(const char *s) {
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
uint64_t noshell_hash64(const char *str) {
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

// ===========================================================
// Document CRUD Operations
// ===========================================================

fossil_bluecrab_noshell_error_t fossil_bluecrab_noshell_insert(
    const char *file_name,
    const char *document,
    const char *param_list,
    const char *type // type as string parameter
) {
    if (!file_name || !document)
        return FOSSIL_NOSHELL_ERROR_INVALID_FILE;

    if (!fossil_bluecrab_noshell_validate_extension(file_name))
        return FOSSIL_NOSHELL_ERROR_INVALID_FILE;

    // Basic FSON validation: must start with '{' or '['
    const char *doc_ptr = document;
    while (isspace((unsigned char)*doc_ptr)) doc_ptr++;
    if (*doc_ptr != '{' && *doc_ptr != '[')
        return FOSSIL_NOSHELL_ERROR_INVALID_TYPE;

    FILE *fp = fopen(file_name, "a");
    if (!fp)
        return FOSSIL_NOSHELL_ERROR_IO;

    // Optionally append param_list if provided, always append #type=TYPE
    if (param_list && strlen(param_list) > 0) {
        fprintf(fp, "%s %s #type=%s\n", document, param_list, type);
    } else {
        fprintf(fp, "%s #type=%s\n", document, type);
    }

    fclose(fp);
    return FOSSIL_NOSHELL_ERROR_SUCCESS;
}

fossil_bluecrab_noshell_error_t fossil_bluecrab_noshell_insert_with_id(
    const char *file_name,
    const char *document,
    const char *param_list,
    const char *type, // type as string parameter
    char *out_id,
    size_t id_size
) {
    if (!file_name || !document || !out_id || id_size < 17)
        return FOSSIL_NOSHELL_ERROR_INVALID_FILE;

    if (!fossil_bluecrab_noshell_validate_extension(file_name))
        return FOSSIL_NOSHELL_ERROR_INVALID_FILE;

    // FSON format: must start with '{' or '['
    const char *doc_ptr = document;
    while (isspace((unsigned char)*doc_ptr)) doc_ptr++;
    if (*doc_ptr != '{' && *doc_ptr != '[')
        return FOSSIL_NOSHELL_ERROR_INVALID_TYPE;

    // Generate document ID using hash64 of document string (FSON object)
    uint64_t doc_id = noshell_hash64(document);
    snprintf(out_id, id_size, "%016llx", (unsigned long long)doc_id);

    FILE *fp = fopen(file_name, "a");
    if (!fp)
        return FOSSIL_NOSHELL_ERROR_IO;

    // Write document in FSON format, append param_list, #type and #id
    if (param_list && strlen(param_list) > 0) {
        fprintf(fp, "%s %s #type=%s #id=%s\n", document, param_list, type, out_id);
    } else {
        fprintf(fp, "%s #type=%s #id=%s\n", document, type, out_id);
    }

    fclose(fp);
    return FOSSIL_NOSHELL_ERROR_SUCCESS;
}

fossil_bluecrab_noshell_error_t fossil_bluecrab_noshell_find(
    const char *file_name,
    const char *query,
    char *result,
    size_t buffer_size,
    const char *type_id // optional type id parameter
) {
    if (!file_name || !query || !result || buffer_size == 0)
        return FOSSIL_NOSHELL_ERROR_INVALID_FILE;

    if (!fossil_bluecrab_noshell_validate_extension(file_name))
        return FOSSIL_NOSHELL_ERROR_INVALID_FILE;

    FILE *fp = fopen(file_name, "r");
    if (!fp)
        return FOSSIL_NOSHELL_ERROR_IO;

    char line[1024];
    while (fgets(line, sizeof(line), fp)) {
        // Skip non-FSON lines (must start with '{' or '[' after whitespace)
        char *p = line;
        while (isspace((unsigned char)*p)) p++;
        if (*p != '{' && *p != '[')
            continue;
        if (strstr(line, query)) {
            if (type_id && strlen(type_id) > 0) {
                // Check for type match in line
                char type_tag[32];
                snprintf(type_tag, sizeof(type_tag), "#type=%s", type_id);
                if (!strstr(line, type_tag)) continue;
            }
            strncpy(result, line, buffer_size - 1);
            result[buffer_size - 1] = '\0';
            fclose(fp);
            return FOSSIL_NOSHELL_ERROR_SUCCESS;
        }
    }

    fclose(fp);
    return FOSSIL_NOSHELL_ERROR_NOT_FOUND;
}

fossil_bluecrab_noshell_error_t fossil_bluecrab_noshell_find_cb(
    const char *file_name,
    bool (*cb)(const char *document, void *userdata),
    void *userdata
) {
    if (!file_name || !cb)
        return FOSSIL_NOSHELL_ERROR_INVALID_FILE;

    if (!fossil_bluecrab_noshell_validate_extension(file_name))
        return FOSSIL_NOSHELL_ERROR_INVALID_FILE;

    FILE *fp = fopen(file_name, "r");
    if (!fp)
        return FOSSIL_NOSHELL_ERROR_IO;

    char line[1024];
    fossil_bluecrab_noshell_error_t result = FOSSIL_NOSHELL_ERROR_NOT_FOUND;
    while (fgets(line, sizeof(line), fp)) {
        // Only consider FSON-formatted lines (start with '{' or '[' after whitespace)
        char *p = line;
        while (isspace((unsigned char)*p)) p++;
        if (*p != '{' && *p != '[')
            continue;
        if (cb(line, userdata)) {
            result = FOSSIL_NOSHELL_ERROR_SUCCESS;
            break;
        }
    }

    fclose(fp);
    return result;
}

fossil_bluecrab_noshell_error_t fossil_bluecrab_noshell_update(
    const char *file_name,
    const char *query,
    const char *new_document,
    const char *param_list,
    const char *type_id // optional type id parameter
) {
    if (!file_name || !query || !new_document)
        return FOSSIL_NOSHELL_ERROR_INVALID_FILE;

    if (!fossil_bluecrab_noshell_validate_extension(file_name))
        return FOSSIL_NOSHELL_ERROR_INVALID_FILE;

    // Basic FSON validation: must start with '{' or '['
    const char *doc_ptr = new_document;
    while (isspace((unsigned char)*doc_ptr)) doc_ptr++;
    if (*doc_ptr != '{' && *doc_ptr != '[')
        return FOSSIL_NOSHELL_ERROR_INVALID_TYPE;

    FILE *fp = fopen(file_name, "r");
    if (!fp)
        return FOSSIL_NOSHELL_ERROR_IO;

    // Read all lines into memory
    char **lines = NULL;
    size_t count = 0, cap = 0;
    char buf[1024];
    bool updated = false;

    while (fgets(buf, sizeof(buf), fp)) {
        // Only update FSON-formatted lines that match the query and (if provided) type_id
        char *p = buf;
        while (isspace((unsigned char)*p)) p++;
        if ((*p == '{' || *p == '[') && strstr(buf, query)) {
            if (type_id && strlen(type_id) > 0) {
                char type_tag[32];
                snprintf(type_tag, sizeof(type_tag), "#type=%s", type_id);
                if (!strstr(buf, type_tag)) {
                    // Not matching type, keep original line
                    if (count == cap) {
                        cap = cap ? cap * 2 : 16;
                        lines = (char **)realloc(lines, cap * sizeof(char *));
                        if (!lines) {
                            fclose(fp);
                            return FOSSIL_NOSHELL_ERROR_OUT_OF_MEMORY;
                        }
                    }
                    lines[count++] = noshell_strdup(buf);
                    continue;
                }
            }
            // Replace line with new_document (+ param_list if provided) and #type if type_id is given
            char new_line[1024];
            if (param_list && strlen(param_list) > 0) {
                if (type_id && strlen(type_id) > 0)
                    snprintf(new_line, sizeof(new_line), "%s %s #type=%s\n", new_document, param_list, type_id);
                else
                    snprintf(new_line, sizeof(new_line), "%s %s\n", new_document, param_list);
            } else {
                if (type_id && strlen(type_id) > 0)
                    snprintf(new_line, sizeof(new_line), "%s #type=%s\n", new_document, type_id);
                else
                    snprintf(new_line, sizeof(new_line), "%s\n", new_document);
            }

            if (count == cap) {
                cap = cap ? cap * 2 : 16;
                lines = (char **)realloc(lines, cap * sizeof(char *));
                if (!lines) {
                    fclose(fp);
                    return FOSSIL_NOSHELL_ERROR_OUT_OF_MEMORY;
                }
            }
            lines[count++] = noshell_strdup(new_line);
            updated = true;
        } else {
            if (count == cap) {
                cap = cap ? cap * 2 : 16;
                lines = (char **)realloc(lines, cap * sizeof(char *));
                if (!lines) {
                    fclose(fp);
                    return FOSSIL_NOSHELL_ERROR_OUT_OF_MEMORY;
                }
            }
            lines[count++] = noshell_strdup(buf);
        }
    }
    fclose(fp);

    if (!updated) {
        // No matching document found
        for (size_t i = 0; i < count; ++i) free(lines[i]);
        free(lines);
        return FOSSIL_NOSHELL_ERROR_NOT_FOUND;
    }

    // Write all lines back to file
    fp = fopen(file_name, "w");
    if (!fp) {
        for (size_t i = 0; i < count; ++i) free(lines[i]);
        free(lines);
        return FOSSIL_NOSHELL_ERROR_IO;
    }
    for (size_t i = 0; i < count; ++i) {
        fputs(lines[i], fp);
        free(lines[i]);
    }
    free(lines);
    fclose(fp);

    return FOSSIL_NOSHELL_ERROR_SUCCESS;
}

fossil_bluecrab_noshell_error_t fossil_bluecrab_noshell_remove(const char *file_name, const char *query) {
    if (!file_name || !query)
        return FOSSIL_NOSHELL_ERROR_INVALID_FILE;

    if (!fossil_bluecrab_noshell_validate_extension(file_name))
        return FOSSIL_NOSHELL_ERROR_INVALID_FILE;

    FILE *fp = fopen(file_name, "r");
    if (!fp)
        return FOSSIL_NOSHELL_ERROR_IO;

    char **lines = NULL;
    size_t count = 0, cap = 0;
    char buf[1024];
    bool removed = false;

    while (fgets(buf, sizeof(buf), fp)) {
        // Only remove FSON-formatted lines (start with '{' or '[' after whitespace) that match the query
        char *p = buf;
        while (isspace((unsigned char)*p)) p++;
        if ((*p == '{' || *p == '[') && strstr(buf, query)) {
            removed = true;
            continue; // Skip this line (remove)
        }
        if (count == cap) {
            cap = cap ? cap * 2 : 16;
            lines = (char **)realloc(lines, cap * sizeof(char *));
            if (!lines) {
                fclose(fp);
                return FOSSIL_NOSHELL_ERROR_OUT_OF_MEMORY;
            }
        }
        lines[count++] = noshell_strdup(buf);
    }
    fclose(fp);

    if (!removed) {
        for (size_t i = 0; i < count; ++i) free(lines[i]);
        free(lines);
        return FOSSIL_NOSHELL_ERROR_NOT_FOUND;
    }

    fp = fopen(file_name, "w");
    if (!fp) {
        for (size_t i = 0; i < count; ++i) free(lines[i]);
        free(lines);
        return FOSSIL_NOSHELL_ERROR_IO;
    }
    for (size_t i = 0; i < count; ++i) {
        fputs(lines[i], fp);
        free(lines[i]);
    }
    free(lines);
    fclose(fp);

    return FOSSIL_NOSHELL_ERROR_SUCCESS;
}

// ===========================================================
// Database Management
// ===========================================================

fossil_bluecrab_noshell_error_t fossil_bluecrab_noshell_create_database(const char *file_name) {
    if (!file_name)
        return FOSSIL_NOSHELL_ERROR_INVALID_FILE;

    if (!fossil_bluecrab_noshell_validate_extension(file_name))
        return FOSSIL_NOSHELL_ERROR_INVALID_FILE;

    FILE *fp = fopen(file_name, "w");
    if (!fp)
        return FOSSIL_NOSHELL_ERROR_IO;

    // Write FSON type system header
    fprintf(fp, "#fson_types=null,bool,i8,i16,i32,i64,u8,u16,u32,u64,f32,f64,oct,hex,bin,char,cstr,array,object,enum,datetime,duration\n");
    // Write an empty FSON object as the initial content
    fprintf(fp, "{ }\n");
    fclose(fp);

    return FOSSIL_NOSHELL_ERROR_SUCCESS;
}

fossil_bluecrab_noshell_error_t fossil_bluecrab_noshell_open_database(const char *file_name) {
    if (!file_name)
        return FOSSIL_NOSHELL_ERROR_INVALID_FILE;

    if (!fossil_bluecrab_noshell_validate_extension(file_name))
        return FOSSIL_NOSHELL_ERROR_INVALID_FILE;

    FILE *fp = fopen(file_name, "r");
    if (!fp)
        return FOSSIL_NOSHELL_ERROR_FILE_NOT_FOUND;

    // Check for FSON header
    char buf[256];
    if (!fgets(buf, sizeof(buf), fp)) {
        fclose(fp);
        return FOSSIL_NOSHELL_ERROR_CORRUPTED;
    }
    if (strncmp(buf, "#fson_types=", 12) != 0) {
        fclose(fp);
        return FOSSIL_NOSHELL_ERROR_SCHEMA_MISMATCH;
    }

    // Check that the next non-header line is a valid FSON object or array
    int found = 0;
    while (fgets(buf, sizeof(buf), fp)) {
        // Skip header/comments
        if (buf[0] == '#')
            continue;
        char *p = buf;
        while (isspace((unsigned char)*p)) p++;
        if (*p == '{' || *p == '[') {
            found = 1;
            break;
        }
    }
    fclose(fp);

    if (!found)
        return FOSSIL_NOSHELL_ERROR_CORRUPTED;

    return FOSSIL_NOSHELL_ERROR_SUCCESS;
}

fossil_bluecrab_noshell_error_t fossil_bluecrab_noshell_delete_database(const char *file_name) {
    if (!file_name)
        return FOSSIL_NOSHELL_ERROR_INVALID_FILE;

    if (!fossil_bluecrab_noshell_validate_extension(file_name))
        return FOSSIL_NOSHELL_ERROR_INVALID_FILE;

    // Optionally check for FSON header before deletion
    FILE *fp = fopen(file_name, "r");
    if (!fp)
        return FOSSIL_NOSHELL_ERROR_FILE_NOT_FOUND;
    char buf[256];
    if (!fgets(buf, sizeof(buf), fp) || strncmp(buf, "#fson_types=", 12) != 0) {
        fclose(fp);
        return FOSSIL_NOSHELL_ERROR_SCHEMA_MISMATCH;
    }
    fclose(fp);

    if (remove(file_name) == 0)
        return FOSSIL_NOSHELL_ERROR_SUCCESS;
    else
        return FOSSIL_NOSHELL_ERROR_IO;
}

fossil_bluecrab_noshell_error_t fossil_bluecrab_noshell_lock_database(const char *file_name) {
    if (!file_name)
        return FOSSIL_NOSHELL_ERROR_INVALID_FILE;

    if (!fossil_bluecrab_noshell_validate_extension(file_name))
        return FOSSIL_NOSHELL_ERROR_INVALID_FILE;

    // Create a lock file: file_name + ".lock"
    char lock_file[1024];
    snprintf(lock_file, sizeof(lock_file), "%s.lock", file_name);

    FILE *fp = fopen(lock_file, "wx");
    if (!fp)
        return FOSSIL_NOSHELL_ERROR_LOCK_FAILED;

    // Write FSON lock metadata
    fprintf(fp, "#fson_lock=true\n");
    fclose(fp);
    return FOSSIL_NOSHELL_ERROR_SUCCESS;
}

fossil_bluecrab_noshell_error_t fossil_bluecrab_noshell_unlock_database(const char *file_name) {
    if (!file_name)
        return FOSSIL_NOSHELL_ERROR_INVALID_FILE;

    if (!fossil_bluecrab_noshell_validate_extension(file_name))
        return FOSSIL_NOSHELL_ERROR_INVALID_FILE;

    char lock_file[1024];
    snprintf(lock_file, sizeof(lock_file), "%s.lock", file_name);

    // FSON: lock metadata is stored in a separate file
    if (remove(lock_file) == 0)
        return FOSSIL_NOSHELL_ERROR_SUCCESS;
    else
        return FOSSIL_NOSHELL_ERROR_LOCK_FAILED;
}

bool fossil_bluecrab_noshell_is_locked(const char *file_name) {
    if (!file_name)
        return false;

    if (!fossil_bluecrab_noshell_validate_extension(file_name))
        return false;

    char lock_file[1024];
    snprintf(lock_file, sizeof(lock_file), "%s.lock", file_name);

    FILE *fp = fopen(lock_file, "r");
    if (fp) {
        fclose(fp);
        return true;
    }
    return false;
}

// ===========================================================
// Backup, Restore, and Verification
// ===========================================================

fossil_bluecrab_noshell_error_t fossil_bluecrab_noshell_backup_database(const char *source_file, const char *backup_file) {
    if (!source_file || !backup_file)
        return FOSSIL_NOSHELL_ERROR_INVALID_FILE;

    if (!fossil_bluecrab_noshell_validate_extension(source_file) ||
        !fossil_bluecrab_noshell_validate_extension(backup_file))
        return FOSSIL_NOSHELL_ERROR_INVALID_FILE;

    FILE *src = fopen(source_file, "r");
    if (!src)
        return FOSSIL_NOSHELL_ERROR_IO;

    FILE *dst = fopen(backup_file, "w");
    if (!dst) {
        fclose(src);
        return FOSSIL_NOSHELL_ERROR_IO;
    }

    char line[4096];
    while (fgets(line, sizeof(line), src)) {
        // Only backup header lines and FSON-formatted documents (start with '{' or '[' after whitespace)
        char *p = line;
        while (isspace((unsigned char)*p)) p++;
        if (line[0] == '#' || *p == '{' || *p == '[') {
            if (fputs(line, dst) == EOF) {
                fclose(src);
                fclose(dst);
                return FOSSIL_NOSHELL_ERROR_BACKUP_FAILED;
            }
        }
    }

    fclose(src);
    fclose(dst);
    return FOSSIL_NOSHELL_ERROR_SUCCESS;
}

fossil_bluecrab_noshell_error_t fossil_bluecrab_noshell_restore_database(const char *backup_file, const char *destination_file) {
    if (!backup_file || !destination_file)
        return FOSSIL_NOSHELL_ERROR_INVALID_FILE;

    if (!fossil_bluecrab_noshell_validate_extension(backup_file) ||
        !fossil_bluecrab_noshell_validate_extension(destination_file))
        return FOSSIL_NOSHELL_ERROR_INVALID_FILE;

    FILE *src = fopen(backup_file, "r");
    if (!src)
        return FOSSIL_NOSHELL_ERROR_IO;

    FILE *dst = fopen(destination_file, "w");
    if (!dst) {
        fclose(src);
        return FOSSIL_NOSHELL_ERROR_IO;
    }

    char line[4096];
    while (fgets(line, sizeof(line), src)) {
        // Only restore header lines and FSON-formatted documents (start with '{' or '[' after whitespace)
        char *p = line;
        while (isspace((unsigned char)*p)) p++;
        if (line[0] == '#' || *p == '{' || *p == '[') {
            if (fputs(line, dst) == EOF) {
                fclose(src);
                fclose(dst);
                return FOSSIL_NOSHELL_ERROR_RESTORE_FAILED;
            }
        }
    }

    fclose(src);
    fclose(dst);
    return FOSSIL_NOSHELL_ERROR_SUCCESS;
}

fossil_bluecrab_noshell_error_t fossil_bluecrab_noshell_verify_database(const char *file_name) {
    if (!file_name)
        return FOSSIL_NOSHELL_ERROR_INVALID_FILE;

    if (!fossil_bluecrab_noshell_validate_extension(file_name))
        return FOSSIL_NOSHELL_ERROR_INVALID_FILE;

    FILE *fp = fopen(file_name, "r");
    if (!fp)
        return FOSSIL_NOSHELL_ERROR_IO;

    char line[1024];
    while (fgets(line, sizeof(line), fp)) {
        // Skip header lines
        if (line[0] == '#')
            continue;

        // Only check FSON-formatted lines (start with '{' or '[' after whitespace)
        char *p = line;
        while (isspace((unsigned char)*p)) p++;
        if (*p != '{' && *p != '[')
            continue;

        // Find "#hash=" in line
        char *hash_pos = strstr(line, "#hash=");
        if (hash_pos) {
            // Find key part (first field before ':')
            char *key_start = p;
            char *colon = strchr(key_start, ':');
            if (!colon) continue;
            size_t key_len = colon - key_start;
            char key[256];
            if (key_len >= sizeof(key)) continue;
            strncpy(key, key_start, key_len);
            key[key_len] = '\0';

            // Compute hash
            uint64_t expected_hash = noshell_hash64(key);

            // Extract hash from line
            char hash_str[17] = {0};
            strncpy(hash_str, hash_pos + 6, 16);
            uint64_t actual_hash = strtoull(hash_str, NULL, 16);

            if (expected_hash != actual_hash) {
                fclose(fp);
                return FOSSIL_NOSHELL_ERROR_CORRUPTED;
            }
        }
    }
    fclose(fp);
    return FOSSIL_NOSHELL_ERROR_SUCCESS;
}

// ===========================================================
// Iteration Helpers
// ===========================================================

fossil_bluecrab_noshell_error_t fossil_bluecrab_noshell_first_document(const char *file_name, char *id_buffer, size_t buffer_size) {
    if (!file_name || !id_buffer || buffer_size < 17)
        return FOSSIL_NOSHELL_ERROR_INVALID_FILE;

    if (!fossil_bluecrab_noshell_validate_extension(file_name))
        return FOSSIL_NOSHELL_ERROR_INVALID_FILE;

    FILE *fp = fopen(file_name, "r");
    if (!fp)
        return FOSSIL_NOSHELL_ERROR_IO;

    char line[1024];
    while (fgets(line, sizeof(line), fp)) {
        // Skip header lines
        if (line[0] == '#')
            continue;
        // Only consider FSON-formatted lines (start with '{' or '[' after whitespace)
        char *p = line;
        while (isspace((unsigned char)*p)) p++;
        if ((*p == '{' || *p == '[')) {
            char *id_pos = strstr(line, "#id=");
            if (id_pos) {
                strncpy(id_buffer, id_pos + 4, 16);
                id_buffer[16] = '\0';
                fclose(fp);
                return FOSSIL_NOSHELL_ERROR_SUCCESS;
            }
        }
    }
    fclose(fp);
    return FOSSIL_NOSHELL_ERROR_NOT_FOUND;
}

fossil_bluecrab_noshell_error_t fossil_bluecrab_noshell_next_document(
    const char *file_name,
    const char *prev_id,
    char *id_buffer,
    size_t buffer_size
) {
    if (!file_name || !prev_id || !id_buffer || buffer_size < 17)
        return FOSSIL_NOSHELL_ERROR_INVALID_FILE;

    if (!fossil_bluecrab_noshell_validate_extension(file_name))
        return FOSSIL_NOSHELL_ERROR_INVALID_FILE;

    FILE *fp = fopen(file_name, "r");
    if (!fp)
        return FOSSIL_NOSHELL_ERROR_IO;

    char line[1024];
    bool found_prev = false;
    while (fgets(line, sizeof(line), fp)) {
        // Skip header lines
        if (line[0] == '#')
            continue;
        // Only consider FSON-formatted lines (start with '{' or '[' after whitespace)
        char *p = line;
        while (isspace((unsigned char)*p)) p++;
        if (*p == '{' || *p == '[') {
            char *id_pos = strstr(line, "#id=");
            if (id_pos) {
                char curr_id[17] = {0};
                strncpy(curr_id, id_pos + 4, 16);
                curr_id[16] = '\0';
                if (found_prev) {
                    strncpy(id_buffer, curr_id, 16);
                    id_buffer[16] = '\0';
                    fclose(fp);
                    return FOSSIL_NOSHELL_ERROR_SUCCESS;
                }
                if (strncmp(curr_id, prev_id, 16) == 0) {
                    found_prev = true;
                }
            }
        }
    }
    fclose(fp);
    return FOSSIL_NOSHELL_ERROR_NOT_FOUND;
}

// ===========================================================
// Metadata Helpers
// ===========================================================

fossil_bluecrab_noshell_error_t fossil_bluecrab_noshell_count_documents(const char *file_name, size_t *count) {
    if (!file_name || !count)
        return FOSSIL_NOSHELL_ERROR_INVALID_FILE;

    if (!fossil_bluecrab_noshell_validate_extension(file_name))
        return FOSSIL_NOSHELL_ERROR_INVALID_FILE;

    FILE *fp = fopen(file_name, "r");
    if (!fp)
        return FOSSIL_NOSHELL_ERROR_IO;

    size_t doc_count = 0;
    char line[1024];
    while (fgets(line, sizeof(line), fp)) {
        // Skip header lines
        if (line[0] == '#')
            continue;
        // Only count FSON-formatted lines (start with '{' or '[' after whitespace) and containing "#id="
        char *p = line;
        while (isspace((unsigned char)*p)) p++;
        if ((*p == '{' || *p == '[') && strstr(line, "#id="))
            doc_count++;
    }
    fclose(fp);

    *count = doc_count;
    return FOSSIL_NOSHELL_ERROR_SUCCESS;
}

fossil_bluecrab_noshell_error_t fossil_bluecrab_noshell_get_file_size(const char *file_name, size_t *size_bytes) {
    if (!file_name || !size_bytes)
        return FOSSIL_NOSHELL_ERROR_INVALID_FILE;

    if (!fossil_bluecrab_noshell_validate_extension(file_name))
        return FOSSIL_NOSHELL_ERROR_INVALID_FILE;

    FILE *fp = fopen(file_name, "rb");
    if (!fp)
        return FOSSIL_NOSHELL_ERROR_IO;

    // Validate FSON header
    char header[256];
    if (!fgets(header, sizeof(header), fp) || strncmp(header, "#fson_types=", 12) != 0) {
        fclose(fp);
        return FOSSIL_NOSHELL_ERROR_SCHEMA_MISMATCH;
    }

    // Get file size
    if (fseek(fp, 0, SEEK_END) != 0) {
        fclose(fp);
        return FOSSIL_NOSHELL_ERROR_IO;
    }
    long sz = ftell(fp);
    fclose(fp);

    if (sz < 0)
        return FOSSIL_NOSHELL_ERROR_IO;

    *size_bytes = (size_t)sz;
    return FOSSIL_NOSHELL_ERROR_SUCCESS;
}

// ===========================================================
// Validation Helpers
// ===========================================================

bool fossil_bluecrab_noshell_validate_extension(const char *file_name) {
    if (!file_name) return false;
    const char *ext = strrchr(file_name, '.');
    return (ext && strcmp(ext, ".noshell") == 0);
}

bool fossil_bluecrab_noshell_validate_document(const char *document) {
    if (!document) return false;
    // Skip leading whitespace
    while (isspace((unsigned char)*document)) document++;
    // Must start with '{' or '['
    if (*document != '{' && *document != '[') return false;

    // Check for at least one explicit type (e.g., ": cstr:", ": i32:", etc.)
    if (!strstr(document, ": cstr:") &&
        !strstr(document, ": i8:") &&
        !strstr(document, ": i16:") &&
        !strstr(document, ": i32:") &&
        !strstr(document, ": i64:") &&
        !strstr(document, ": u8:") &&
        !strstr(document, ": u16:") &&
        !strstr(document, ": u32:") &&
        !strstr(document, ": u64:") &&
        !strstr(document, ": f32:") &&
        !strstr(document, ": f64:") &&
        !strstr(document, ": bool:") &&
        !strstr(document, ": null:") &&
        !strstr(document, ": oct:") &&
        !strstr(document, ": hex:") &&
        !strstr(document, ": bin:") &&
        !strstr(document, ": char:") &&
        !strstr(document, ": array:") &&
        !strstr(document, ": object:") &&
        !strstr(document, ": enum:") &&
        !strstr(document, ": datetime:") &&
        !strstr(document, ": duration:"))
        return false;

    // Basic structure check: must end with '}' or ']'
    size_t len = strlen(document);
    if (len == 0) return false;
    char last = document[len - 1];
    if (last != '}' && last != ']') return false;

    return true;
}

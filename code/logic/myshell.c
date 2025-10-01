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

// Helper macro for safe snprintf of key=value pairs
#define SAFE_SNPRINTF_KV(buf, bufsize, key, value) \
    snprintf((buf), (bufsize), "%.*s=%.*s", \
             (int)((bufsize)/2 - 1), (key), \
             (int)((bufsize)/2 - 1), (value))

#define MAX_OPEN_DBS 32

static const char* open_dbs[MAX_OPEN_DBS] = {0};


// ============================================================================
// Internal: Track open databases
// ============================================================================

// Helper: mark database as closed
static bool mark_db_closed(const char *file_name) {
    for (int i = 0; i < MAX_OPEN_DBS; i++) {
        if (open_dbs[i] && strcmp(open_dbs[i], file_name) == 0) {
            free((void*)open_dbs[i]);
            open_dbs[i] = NULL;
            return true;
        }
    }
    return false;
}

// ============================================================================
// Hash algorithm
// ============================================================================

/**
 * @brief Enhanced 64-bit hash for MyShell records.
 * 
 * Combines FNV-1a with bit mixing for better avalanche properties.
 */
static uint64_t fossil_bluecrab_myshell_hash(const char *str) {
    uint64_t hash = 14695981039346656037ULL; // FNV-1a 64-bit offset basis
    unsigned char c;

    // FNV-1a accumulation
    while ((c = (unsigned char)*str++)) {
        hash ^= c;
        hash *= 1099511628211ULL; // FNV-1a 64-bit prime
    }

    // Final mixing step (64-bit mix inspired by MurmurHash3 finalizer)
    hash ^= hash >> 33;
    hash *= 0xff51afd7ed558ccdULL;
    hash ^= hash >> 33;
    hash *= 0xc4ceb9fe1a85ec53ULL;
    hash ^= hash >> 33;

    return hash;
}

static bool fossil_bluecrab_myshell_split_record(char *line, char **key, char **value, unsigned long *stored_hash) {
    // Format: key=value|hash
    char *hash_sep = strrchr(line, '|');
    if (!hash_sep) return false;
    *hash_sep = '\0';
    *stored_hash = strtoul(hash_sep + 1, NULL, 10);

    char *eq = strchr(line, '=');
    if (!eq) return false;

    *eq = '\0';
    *key = line;
    *value = eq + 1;

    return true;
}

static char *fossil_myshell_bluecrab_strdup(const char *s) {
    if (!s) return NULL;
    size_t len = strlen(s);
    char *dup = (char*)malloc(len + 1);
    if (dup) {
        memcpy(dup, s, len + 1);
    }
    return dup;
}

// ===========================================================
// FSON v2 support 
// ===========================================================

// portable case-insensitive compare
static int myshell_stricmp(const char *a, const char *b) {
    if (!a || !b) return (a == b) ? 0 : (a ? 1 : -1);

    while (*a && *b) {
        unsigned char ca = (unsigned char)tolower((unsigned char)*a);
        unsigned char cb = (unsigned char)tolower((unsigned char)*b);
        if (ca != cb) return (int)ca - (int)cb;
        ++a;
        ++b;
    }
    return (int)(unsigned char)*a - (int)(unsigned char)*b;
}

static const char* myshell_fson_type_to_string(fossil_bluecrab_myshell_fson_type_t type) {
    switch (type) {
        case MYSHELL_FSON_TYPE_NULL:     return "null";
        case MYSHELL_FSON_TYPE_BOOL:     return "bool";
        case MYSHELL_FSON_TYPE_I8:       return "i8";
        case MYSHELL_FSON_TYPE_I16:      return "i16";
        case MYSHELL_FSON_TYPE_I32:      return "i32";
        case MYSHELL_FSON_TYPE_I64:      return "i64";
        case MYSHELL_FSON_TYPE_U8:       return "u8";
        case MYSHELL_FSON_TYPE_U16:      return "u16";
        case MYSHELL_FSON_TYPE_U32:      return "u32";
        case MYSHELL_FSON_TYPE_U64:      return "u64";
        case MYSHELL_FSON_TYPE_F32:      return "f32";
        case MYSHELL_FSON_TYPE_F64:      return "f64";
        case MYSHELL_FSON_TYPE_OCT:      return "oct";
        case MYSHELL_FSON_TYPE_HEX:      return "hex";
        case MYSHELL_FSON_TYPE_BIN:      return "bin";
        case MYSHELL_FSON_TYPE_CHAR:     return "char";
        case MYSHELL_FSON_TYPE_CSTR:     return "cstr";
        case MYSHELL_FSON_TYPE_ARRAY:    return "array";
        case MYSHELL_FSON_TYPE_OBJECT:   return "object";
        case MYSHELL_FSON_TYPE_ENUM:     return "enum";
        case MYSHELL_FSON_TYPE_DATETIME: return "datetime";
        case MYSHELL_FSON_TYPE_DURATION: return "duration";
        default: return "unknown";
    }
}

static fossil_bluecrab_myshell_fson_type_t myshell_fson_type_from_string(const char *s) {
    if (!s) return MYSHELL_FSON_TYPE_NULL;

    if (myshell_stricmp(s, "null")     == 0) return MYSHELL_FSON_TYPE_NULL;
    if (myshell_stricmp(s, "bool")     == 0) return MYSHELL_FSON_TYPE_BOOL;
    if (myshell_stricmp(s, "i8")       == 0) return MYSHELL_FSON_TYPE_I8;
    if (myshell_stricmp(s, "i16")      == 0) return MYSHELL_FSON_TYPE_I16;
    if (myshell_stricmp(s, "i32")      == 0) return MYSHELL_FSON_TYPE_I32;
    if (myshell_stricmp(s, "i64")      == 0) return MYSHELL_FSON_TYPE_I64;
    if (myshell_stricmp(s, "u8")       == 0) return MYSHELL_FSON_TYPE_U8;
    if (myshell_stricmp(s, "u16")      == 0) return MYSHELL_FSON_TYPE_U16;
    if (myshell_stricmp(s, "u32")      == 0) return MYSHELL_FSON_TYPE_U32;
    if (myshell_stricmp(s, "u64")      == 0) return MYSHELL_FSON_TYPE_U64;
    if (myshell_stricmp(s, "f32")      == 0) return MYSHELL_FSON_TYPE_F32;
    if (myshell_stricmp(s, "f64")      == 0) return MYSHELL_FSON_TYPE_F64;
    if (myshell_stricmp(s, "oct")      == 0) return MYSHELL_FSON_TYPE_OCT;
    if (myshell_stricmp(s, "hex")      == 0) return MYSHELL_FSON_TYPE_HEX;
    if (myshell_stricmp(s, "bin")      == 0) return MYSHELL_FSON_TYPE_BIN;
    if (myshell_stricmp(s, "char")     == 0) return MYSHELL_FSON_TYPE_CHAR;
    if (myshell_stricmp(s, "cstr")     == 0) return MYSHELL_FSON_TYPE_CSTR;
    if (myshell_stricmp(s, "array")    == 0) return MYSHELL_FSON_TYPE_ARRAY;
    if (myshell_stricmp(s, "object")   == 0) return MYSHELL_FSON_TYPE_OBJECT;
    if (myshell_stricmp(s, "enum")     == 0) return MYSHELL_FSON_TYPE_ENUM;
    if (myshell_stricmp(s, "datetime") == 0) return MYSHELL_FSON_TYPE_DATETIME;
    if (myshell_stricmp(s, "duration") == 0) return MYSHELL_FSON_TYPE_DURATION;

    return MYSHELL_FSON_TYPE_NULL; // fallback
}

static void myshell_fson_value_to_string(
    const fossil_bluecrab_myshell_fson_value_t *val,
    char *out, size_t out_size)
{
    const char *tname = myshell_fson_type_to_string(val->type);

    switch (val->type) {
        case MYSHELL_FSON_TYPE_NULL:
            snprintf(out, out_size, "%s:null", tname);
            break;
        case MYSHELL_FSON_TYPE_BOOL:
            snprintf(out, out_size, "%s:%s", tname, val->as.b ? "true" : "false");
            break;

        case MYSHELL_FSON_TYPE_I8:  snprintf(out, out_size, "%s:%" PRId8,  tname, val->as.i8); break;
        case MYSHELL_FSON_TYPE_I16: snprintf(out, out_size, "%s:%" PRId16, tname, val->as.i16); break;
        case MYSHELL_FSON_TYPE_I32: snprintf(out, out_size, "%s:%" PRId32, tname, val->as.i32); break;
        case MYSHELL_FSON_TYPE_I64: snprintf(out, out_size, "%s:%" PRId64, tname, val->as.i64); break;

        case MYSHELL_FSON_TYPE_U8:  snprintf(out, out_size, "%s:%" PRIu8,  tname, val->as.u8); break;
        case MYSHELL_FSON_TYPE_U16: snprintf(out, out_size, "%s:%" PRIu16, tname, val->as.u16); break;
        case MYSHELL_FSON_TYPE_U32: snprintf(out, out_size, "%s:%" PRIu32, tname, val->as.u32); break;
        case MYSHELL_FSON_TYPE_U64: snprintf(out, out_size, "%s:%" PRIu64, tname, val->as.u64); break;

        case MYSHELL_FSON_TYPE_F32: snprintf(out, out_size, "%s:%g",   tname, val->as.f32); break;
        case MYSHELL_FSON_TYPE_F64: snprintf(out, out_size, "%s:%lf",  tname, val->as.f64); break;

        case MYSHELL_FSON_TYPE_CHAR:
            snprintf(out, out_size, "%s:%c", tname, val->as.c);
            break;

        // heap-allocated/string-likes
        case MYSHELL_FSON_TYPE_CSTR:
        case MYSHELL_FSON_TYPE_OCT:
        case MYSHELL_FSON_TYPE_HEX:
        case MYSHELL_FSON_TYPE_BIN:
        case MYSHELL_FSON_TYPE_ARRAY:
        case MYSHELL_FSON_TYPE_OBJECT:
        case MYSHELL_FSON_TYPE_ENUM:
        case MYSHELL_FSON_TYPE_DATETIME:
        case MYSHELL_FSON_TYPE_DURATION:
            snprintf(out, out_size, "%s:%s", tname, val->as.cstr ? val->as.cstr : "");
            break;

        default:
            snprintf(out, out_size, "unknown:null");
            break;
    }
}

// parse "type:value" into union
static bool myshell_fson_value_from_string(const char *encoded, fossil_bluecrab_myshell_fson_value_t *out) {
    if (!encoded || !out) return false;

    const char *colon = strchr(encoded, ':');
    if (!colon) return false;

    size_t type_len = (size_t)(colon - encoded);
    char type_buf[32];
    if (type_len >= sizeof(type_buf)) return false;

    memcpy(type_buf, encoded, type_len);
    type_buf[type_len] = '\0';

    const char *value = colon + 1;
    fossil_bluecrab_myshell_fson_type_t t = myshell_fson_type_from_string(type_buf);
    out->type = t;

    switch (t) {
        case MYSHELL_FSON_TYPE_NULL:
            break;
        case MYSHELL_FSON_TYPE_BOOL:
            out->as.b = (strcasecmp(value, "true") == 0);
            break;
        case MYSHELL_FSON_TYPE_I8:  out->as.i8  = (int8_t) strtol(value, NULL, 10); break;
        case MYSHELL_FSON_TYPE_I16: out->as.i16 = (int16_t)strtol(value, NULL, 10); break;
        case MYSHELL_FSON_TYPE_I32: out->as.i32 = (int32_t)strtol(value, NULL, 10); break;
        case MYSHELL_FSON_TYPE_I64: out->as.i64 = (int64_t)strtoll(value, NULL, 10); break;
        case MYSHELL_FSON_TYPE_U8:  out->as.u8  = (uint8_t) strtoul(value, NULL, 10); break;
        case MYSHELL_FSON_TYPE_U16: out->as.u16 = (uint16_t)strtoul(value, NULL, 10); break;
        case MYSHELL_FSON_TYPE_U32: out->as.u32 = (uint32_t)strtoul(value, NULL, 10); break;
        case MYSHELL_FSON_TYPE_U64: out->as.u64 = (uint64_t)strtoull(value, NULL, 10); break;
        case MYSHELL_FSON_TYPE_F32: out->as.f32 = strtof(value, NULL); break;
        case MYSHELL_FSON_TYPE_F64: out->as.f64 = strtod(value, NULL); break;
        case MYSHELL_FSON_TYPE_CHAR: out->as.c  = value[0]; break;

        case MYSHELL_FSON_TYPE_CSTR:
        case MYSHELL_FSON_TYPE_OCT:
        case MYSHELL_FSON_TYPE_HEX:
        case MYSHELL_FSON_TYPE_BIN:
        case MYSHELL_FSON_TYPE_ARRAY:
        case MYSHELL_FSON_TYPE_OBJECT:
        case MYSHELL_FSON_TYPE_ENUM:
        case MYSHELL_FSON_TYPE_DATETIME:
        case MYSHELL_FSON_TYPE_DURATION:
            out->as.cstr = fossil_myshell_bluecrab_strdup(value);
            break;

        default:
            return false;
    }
    return true;
}

// ===========================================================
// CRUD Operations
// ===========================================================

fossil_bluecrab_myshell_error_t fossil_bluecrab_myshell_create_record(const char *file_name, const char *key, const char *value) {
    if (!fossil_bluecrab_myshell_validate_extension(file_name))
        return FOSSIL_MYSHELL_ERROR_INVALID_FILE;

    // Use FSON encoding for value
    fossil_bluecrab_myshell_fson_value_t fson_val;
    if (!myshell_fson_value_from_string(value, &fson_val))
        return FOSSIL_MYSHELL_ERROR_PARSE_FAILED;

    char encoded[512];
    myshell_fson_value_to_string(&fson_val, encoded, sizeof(encoded));

    // Check for duplicate key
    FILE *file = fopen(file_name, "r");
    if (file) {
        char line[512];
        while (fgets(line, sizeof(line), file)) {
            char *line_key, *line_value;
            unsigned long stored_hash;
            if (!fossil_bluecrab_myshell_split_record(line, &line_key, &line_value, &stored_hash))
                continue;
            if (myshell_stricmp(line_key, key) == 0) {
                fclose(file);
                if (fson_val.type >= MYSHELL_FSON_TYPE_CSTR && fson_val.type <= MYSHELL_FSON_TYPE_DURATION && fson_val.as.cstr)
                    free(fson_val.as.cstr);
                return FOSSIL_MYSHELL_ERROR_ALREADY_EXISTS;
            }
        }
        fclose(file);
    }

    // Append new record
    file = fopen(file_name, "a");
    if (!file) {
        if (fson_val.type >= MYSHELL_FSON_TYPE_CSTR && fson_val.type <= MYSHELL_FSON_TYPE_DURATION && fson_val.as.cstr)
            free(fson_val.as.cstr);
        return FOSSIL_MYSHELL_ERROR_IO;
    }

    char record[600];
    snprintf(record, sizeof(record), "%s=%s", key, encoded);
    unsigned long hash = fossil_bluecrab_myshell_hash(record);

    fprintf(file, "%s|%lu\n", record, hash);
    fclose(file);

    // Free heap if needed
    if (fson_val.type >= MYSHELL_FSON_TYPE_CSTR && fson_val.type <= MYSHELL_FSON_TYPE_DURATION && fson_val.as.cstr)
        free(fson_val.as.cstr);

    return FOSSIL_MYSHELL_ERROR_SUCCESS;
}

fossil_bluecrab_myshell_error_t fossil_bluecrab_myshell_read_record(const char *file_name, const char *key, char *value, size_t buffer_size) {
    if (!fossil_bluecrab_myshell_validate_extension(file_name))
        return FOSSIL_MYSHELL_ERROR_INVALID_FILE;

    FILE *file = fopen(file_name, "r");
    if (!file) return FOSSIL_MYSHELL_ERROR_IO;

    char line[512];
    while (fgets(line, sizeof(line), file)) {
        char *line_key, *line_value;
        unsigned long stored_hash;

        if (!fossil_bluecrab_myshell_split_record(line, &line_key, &line_value, &stored_hash))
            continue;

        char temp[600];
        snprintf(temp, sizeof(temp), "%s=%s", line_key, line_value);
        unsigned long calc_hash = fossil_bluecrab_myshell_hash(temp);

        if (calc_hash != stored_hash) {
            fclose(file);
            return FOSSIL_MYSHELL_ERROR_CORRUPTED;
        }

        if (myshell_stricmp(line_key, key) == 0) {
            // Decode FSON value to string
            fossil_bluecrab_myshell_fson_value_t fson_val;
            if (!myshell_fson_value_from_string(line_value, &fson_val)) {
                fclose(file);
                return FOSSIL_MYSHELL_ERROR_PARSE_FAILED;
            }
            char out[512];
            myshell_fson_value_to_string(&fson_val, out, sizeof(out));
            strncpy(value, out, buffer_size);
            value[buffer_size - 1] = '\0';

            if (fson_val.type >= MYSHELL_FSON_TYPE_CSTR && fson_val.type <= MYSHELL_FSON_TYPE_DURATION && fson_val.as.cstr)
                free(fson_val.as.cstr);

            fclose(file);
            return FOSSIL_MYSHELL_ERROR_SUCCESS;
        }
    }

    fclose(file);
    return FOSSIL_MYSHELL_ERROR_NOT_FOUND;
}

fossil_bluecrab_myshell_error_t fossil_bluecrab_myshell_update_record(const char *file_name, const char *key, const char *new_value) {
    if (!fossil_bluecrab_myshell_validate_extension(file_name))
        return FOSSIL_MYSHELL_ERROR_INVALID_FILE;

    // Use FSON encoding for new_value
    fossil_bluecrab_myshell_fson_value_t fson_val;
    if (!myshell_fson_value_from_string(new_value, &fson_val))
        return FOSSIL_MYSHELL_ERROR_PARSE_FAILED;

    char encoded[512];
    myshell_fson_value_to_string(&fson_val, encoded, sizeof(encoded));

    FILE *file = fopen(file_name, "r");
    if (!file) return FOSSIL_MYSHELL_ERROR_IO;

    FILE *temp = fopen("temp.myshell", "w");
    if (!temp) {
        fclose(file);
        if (fson_val.type >= MYSHELL_FSON_TYPE_CSTR && fson_val.type <= MYSHELL_FSON_TYPE_DURATION && fson_val.as.cstr)
            free(fson_val.as.cstr);
        return FOSSIL_MYSHELL_ERROR_IO;
    }

    char line[512];
    bool updated = false;

    while (fgets(line, sizeof(line), file)) {
        char *line_key, *line_value;
        unsigned long stored_hash;

        if (!fossil_bluecrab_myshell_split_record(line, &line_key, &line_value, &stored_hash)) {
            fputs(line, temp);
            continue;
        }

        if (myshell_stricmp(line_key, key) == 0) {
            char record[600];
            snprintf(record, sizeof(record), "%s=%s", key, encoded);
            unsigned long new_hash = fossil_bluecrab_myshell_hash(record);
            fprintf(temp, "%s|%lu\n", record, new_hash);
            updated = true;
        } else {
            fprintf(temp, "%s=%s|%lu\n", line_key, line_value, stored_hash);
        }
    }

    fclose(file);
    fclose(temp);

    if (fson_val.type >= MYSHELL_FSON_TYPE_CSTR && fson_val.type <= MYSHELL_FSON_TYPE_DURATION && fson_val.as.cstr)
        free(fson_val.as.cstr);

    if (updated) {
        remove(file_name);
        rename("temp.myshell", file_name);
        return FOSSIL_MYSHELL_ERROR_SUCCESS;
    }

    remove("temp.myshell");
    return FOSSIL_MYSHELL_ERROR_NOT_FOUND;
}

fossil_bluecrab_myshell_error_t fossil_bluecrab_myshell_delete_record(const char *file_name, const char *key) {
    if (!fossil_bluecrab_myshell_validate_extension(file_name))
        return FOSSIL_MYSHELL_ERROR_INVALID_FILE;

    FILE *file = fopen(file_name, "r");
    if (!file) return FOSSIL_MYSHELL_ERROR_IO;

    FILE *temp = fopen("temp.myshell", "w");
    if (!temp) {
        fclose(file);
        return FOSSIL_MYSHELL_ERROR_IO;
    }

    char line[512];
    bool deleted = false;

    while (fgets(line, sizeof(line), file)) {
        char *line_key, *line_value;
        unsigned long stored_hash;

        if (!fossil_bluecrab_myshell_split_record(line, &line_key, &line_value, &stored_hash)) {
            fputs(line, temp);
            continue;
        }

        if (myshell_stricmp(line_key, key) == 0) {
            deleted = true;
            continue;
        }

        fprintf(temp, "%s=%s|%lu\n", line_key, line_value, stored_hash);
    }

    fclose(file);
    fclose(temp);

    if (deleted) {
        remove(file_name);
        rename("temp.myshell", file_name);
        return FOSSIL_MYSHELL_ERROR_SUCCESS;
    }

    remove("temp.myshell");
    return FOSSIL_MYSHELL_ERROR_NOT_FOUND;
}

// ===========================================================
// Database Management
// ===========================================================

fossil_bluecrab_myshell_error_t fossil_bluecrab_myshell_create_database(const char *file_name) {
    if (!fossil_bluecrab_myshell_validate_extension(file_name))
        return FOSSIL_MYSHELL_ERROR_INVALID_FILE;

    FILE *file = fopen(file_name, "w");
    if (!file) return FOSSIL_MYSHELL_ERROR_IO;

    fclose(file);
    return FOSSIL_MYSHELL_ERROR_SUCCESS;
}

fossil_bluecrab_myshell_error_t fossil_bluecrab_myshell_open_database(const char *file_name) {
    if (!fossil_bluecrab_myshell_validate_extension(file_name))
        return FOSSIL_MYSHELL_ERROR_INVALID_FILE;

    // Check if already open
    for (int i = 0; i < MAX_OPEN_DBS; i++) {
        if (open_dbs[i] && strcmp(open_dbs[i], file_name) == 0) {
            return FOSSIL_MYSHELL_ERROR_ALREADY_EXISTS;
        }
    }

    FILE *file = fopen(file_name, "r");
    if (!file) return FOSSIL_MYSHELL_ERROR_FILE_NOT_FOUND;
    fclose(file);

    // Track as open
    for (int i = 0; i < MAX_OPEN_DBS; i++) {
        if (!open_dbs[i]) {
            open_dbs[i] = fossil_myshell_bluecrab_strdup(file_name);
            return FOSSIL_MYSHELL_ERROR_SUCCESS;
        }
    }
    return FOSSIL_MYSHELL_ERROR_CONCURRENCY;
}

fossil_bluecrab_myshell_error_t fossil_bluecrab_myshell_delete_database(const char *file_name) {
    if (!fossil_bluecrab_myshell_validate_extension(file_name))
        return FOSSIL_MYSHELL_ERROR_INVALID_FILE;

    if (remove(file_name) == 0) {
        mark_db_closed(file_name);
        return FOSSIL_MYSHELL_ERROR_SUCCESS;
    }
    return FOSSIL_MYSHELL_ERROR_IO;
}

fossil_bluecrab_myshell_error_t fossil_bluecrab_myshell_close_database(const char *file_name) {
    if (!file_name) return FOSSIL_MYSHELL_ERROR_INVALID_FILE;

    if (!mark_db_closed(file_name)) {
        return FOSSIL_MYSHELL_ERROR_NOT_FOUND;
    }
    return FOSSIL_MYSHELL_ERROR_SUCCESS;
}

// ===========================================================
// Backup and Restore
// ===========================================================

fossil_bluecrab_myshell_error_t fossil_bluecrab_myshell_backup_database(const char *src_file, const char *dst_file) {
    if (!fossil_bluecrab_myshell_validate_extension(src_file) || !fossil_bluecrab_myshell_validate_extension(dst_file))
        return FOSSIL_MYSHELL_ERROR_INVALID_FILE;

    FILE *in = fopen(src_file, "r");
    if (!in) return FOSSIL_MYSHELL_ERROR_FILE_NOT_FOUND;

    FILE *out = fopen(dst_file, "w");
    if (!out) {
        fclose(in);
        return FOSSIL_MYSHELL_ERROR_IO;
    }

    char line[512];
    while (fgets(line, sizeof(line), in)) {
        char *line_key, *line_value;
        unsigned long stored_hash;

        if (!fossil_bluecrab_myshell_split_record(line, &line_key, &line_value, &stored_hash)) {
            fclose(in);
            fclose(out);
            remove(dst_file);
            return FOSSIL_MYSHELL_ERROR_CORRUPTED;
        }

        char record[512];
        SAFE_SNPRINTF_KV(record, sizeof(record), line_key, line_value);
        unsigned long calc_hash = fossil_bluecrab_myshell_hash(record);

        if (calc_hash != stored_hash) {
            fclose(in);
            fclose(out);
            remove(dst_file);
            return FOSSIL_MYSHELL_ERROR_CORRUPTED;
        }

        fprintf(out, "%s|%lu\n", record, stored_hash);
    }

    fclose(in);
    fclose(out);
    return FOSSIL_MYSHELL_ERROR_SUCCESS;
}

fossil_bluecrab_myshell_error_t fossil_bluecrab_myshell_restore_database(const char *backup_file, const char *dst_file) {
    if (!fossil_bluecrab_myshell_validate_extension(backup_file) || !fossil_bluecrab_myshell_validate_extension(dst_file))
        return FOSSIL_MYSHELL_ERROR_INVALID_FILE;

    FILE *in = fopen(backup_file, "r");
    if (!in) return FOSSIL_MYSHELL_ERROR_FILE_NOT_FOUND;

    FILE *out = fopen(dst_file, "w");
    if (!out) {
        fclose(in);
        return FOSSIL_MYSHELL_ERROR_IO;
    }

    char line[512];
    while (fgets(line, sizeof(line), in)) {
        char *line_key, *line_value;
        unsigned long stored_hash;

        if (!fossil_bluecrab_myshell_split_record(line, &line_key, &line_value, &stored_hash)) {
            fclose(in);
            fclose(out);
            remove(dst_file);
            return FOSSIL_MYSHELL_ERROR_CORRUPTED;
        }

        char record[512];
        SAFE_SNPRINTF_KV(record, sizeof(record), line_key, line_value);
        unsigned long calc_hash = fossil_bluecrab_myshell_hash(record);

        if (calc_hash != stored_hash) {
            fclose(in);
            fclose(out);
            remove(dst_file);
            return FOSSIL_MYSHELL_ERROR_CORRUPTED;
        }

        fprintf(out, "%s|%lu\n", record, stored_hash);
    }

    fclose(in);
    fclose(out);
    return FOSSIL_MYSHELL_ERROR_SUCCESS;
}

// ===========================================================
// Query and Data Validation
// ===========================================================

bool fossil_bluecrab_myshell_is_open(const char *file_name) {
    if (!file_name) return false;
    for (int i = 0; i < MAX_OPEN_DBS; i++) {
        if (open_dbs[i] && strcmp(open_dbs[i], file_name) == 0) {
            return true;
        }
    }
    return false;
}

fossil_bluecrab_myshell_error_t fossil_bluecrab_myshell_verify_database(const char *file_name) {
    if (!fossil_bluecrab_myshell_validate_extension(file_name))
        return FOSSIL_MYSHELL_ERROR_INVALID_FILE;

    FILE *file = fopen(file_name, "r");
    if (!file) return FOSSIL_MYSHELL_ERROR_FILE_NOT_FOUND;

    char line[512];
    while (fgets(line, sizeof(line), file)) {
        char *line_key, *line_value;
        unsigned long stored_hash;

        if (!fossil_bluecrab_myshell_split_record(line, &line_key, &line_value, &stored_hash)) {
            fclose(file);
            return FOSSIL_MYSHELL_ERROR_CORRUPTED;
        }

        char record[512];
        SAFE_SNPRINTF_KV(record, sizeof(record), line_key, line_value);
        unsigned long calc_hash = fossil_bluecrab_myshell_hash(record);

        if (calc_hash != stored_hash) {
            fclose(file);
            return FOSSIL_MYSHELL_ERROR_CORRUPTED;
        }
    }

    fclose(file);
    return FOSSIL_MYSHELL_ERROR_SUCCESS;
}

bool fossil_bluecrab_myshell_validate_extension(const char *file_name) {
    const char *ext = strrchr(file_name, '.');
    return ext && strcmp(ext, ".myshell") == 0;
}

bool fossil_bluecrab_myshell_validate_data(const char *data) {
    return data != NULL && strlen(data) > 0;
}

// ============================================================================
// Iteration Helpers
// ============================================================================
// These helpers must skip blank lines and lines that do not contain '='.
// They must also skip lines that fail hash validation.

fossil_bluecrab_myshell_error_t fossil_bluecrab_myshell_first_key(const char *file_name, char *key_buffer, size_t buffer_size) {
    if (!fossil_bluecrab_myshell_validate_extension(file_name))
        return FOSSIL_MYSHELL_ERROR_INVALID_FILE;

    FILE *fp = fopen(file_name, "r");
    if (!fp) return FOSSIL_MYSHELL_ERROR_FILE_NOT_FOUND;

    char line[512];
    fossil_bluecrab_myshell_error_t result = FOSSIL_MYSHELL_ERROR_NOT_FOUND;
    while (fgets(line, sizeof(line), fp)) {
        // Skip blank lines
        if (line[0] == '\n' || line[0] == '\r' || line[0] == '\0')
            continue;
        // Skip lines without '='
        if (!strchr(line, '='))
            continue;

        char *line_key, *line_value;
        unsigned long stored_hash;
        if (!fossil_bluecrab_myshell_split_record(line, &line_key, &line_value, &stored_hash))
            continue;

        char record[512];
        SAFE_SNPRINTF_KV(record, sizeof(record), line_key, line_value);
        if (fossil_bluecrab_myshell_hash(record) != stored_hash)
            continue;

        strncpy(key_buffer, line_key, buffer_size);
        key_buffer[buffer_size - 1] = '\0';
        result = FOSSIL_MYSHELL_ERROR_SUCCESS;
        break;
    }
    fclose(fp);
    return result;
}

fossil_bluecrab_myshell_error_t fossil_bluecrab_myshell_next_key(const char *file_name, const char *prev_key, char *key_buffer, size_t buffer_size) {
    if (!fossil_bluecrab_myshell_validate_extension(file_name))
        return FOSSIL_MYSHELL_ERROR_INVALID_FILE;

    FILE *fp = fopen(file_name, "r");
    if (!fp) return FOSSIL_MYSHELL_ERROR_FILE_NOT_FOUND;

    char line[512];
    bool found = false;
    fossil_bluecrab_myshell_error_t result = FOSSIL_MYSHELL_ERROR_NOT_FOUND;

    while (fgets(line, sizeof(line), fp)) {
        // Skip blank lines
        if (line[0] == '\n' || line[0] == '\r' || line[0] == '\0')
            continue;
        // Skip lines without '='
        if (!strchr(line, '='))
            continue;

        char *line_key, *line_value;
        unsigned long stored_hash;

        if (!fossil_bluecrab_myshell_split_record(line, &line_key, &line_value, &stored_hash))
            continue;

        char record[512];
        SAFE_SNPRINTF_KV(record, sizeof(record), line_key, line_value);
        if (fossil_bluecrab_myshell_hash(record) != stored_hash)
            continue;

        if (found) {
            strncpy(key_buffer, line_key, buffer_size);
            key_buffer[buffer_size - 1] = '\0';
            result = FOSSIL_MYSHELL_ERROR_SUCCESS;
            break;
        }

        if (strcmp(line_key, prev_key) == 0)
            found = true;
    }

    fclose(fp);
    return result;
}

// ============================================================================
// Metadata Helpers
// ============================================================================
fossil_bluecrab_myshell_error_t fossil_bluecrab_myshell_count_records(const char *file_name, size_t *count) {
    if (!file_name || !count) return FOSSIL_MYSHELL_ERROR_INVALID_FILE;

    FILE *fp = fopen(file_name, "r");
    if (!fp) return FOSSIL_MYSHELL_ERROR_FILE_NOT_FOUND;

    size_t counter = 0;
    char line[1024];
    while (fgets(line, sizeof(line), fp)) {
        if (strchr(line, '=')) counter++;
    }
    fclose(fp);

    *count = counter;
    return FOSSIL_MYSHELL_ERROR_SUCCESS;
}

fossil_bluecrab_myshell_error_t fossil_bluecrab_myshell_get_file_size(const char *file_name, size_t *size_bytes) {
    if (!file_name || !size_bytes) return FOSSIL_MYSHELL_ERROR_INVALID_FILE;

    struct stat st;
    if (stat(file_name, &st) != 0) {
        return FOSSIL_MYSHELL_ERROR_FILE_NOT_FOUND;
    }

    *size_bytes = (size_t)st.st_size;
    return FOSSIL_MYSHELL_ERROR_SUCCESS;
}

// ============================================================================
// Error to String
// ============================================================================
const char* fossil_bluecrab_myshell_error_string(fossil_bluecrab_myshell_error_t error_code) {
    switch (error_code) {
        case FOSSIL_MYSHELL_ERROR_SUCCESS:        return "Success";
        case FOSSIL_MYSHELL_ERROR_INVALID_FILE:   return "Invalid file";
        case FOSSIL_MYSHELL_ERROR_FILE_NOT_FOUND: return "File not found";
        case FOSSIL_MYSHELL_ERROR_IO:             return "I/O error";
        case FOSSIL_MYSHELL_ERROR_INVALID_QUERY:  return "Invalid query";
        case FOSSIL_MYSHELL_ERROR_CONCURRENCY:    return "Concurrency error";
        case FOSSIL_MYSHELL_ERROR_NOT_FOUND:      return "Record not found";
        case FOSSIL_MYSHELL_ERROR_ALREADY_EXISTS: return "Already exists";
        case FOSSIL_MYSHELL_ERROR_BACKUP_FAILED:  return "Backup failed";
        case FOSSIL_MYSHELL_ERROR_RESTORE_FAILED: return "Restore failed";
        case FOSSIL_MYSHELL_ERROR_UNKNOWN:        return "Unknown error";
        default:                                  return "Unrecognized error code";
    }
}

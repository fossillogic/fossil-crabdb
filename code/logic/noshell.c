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

#define MAX_OPEN_DBS 32
#define MAX_LOCKED_DBS 32

/**
 * @brief Custom strdup implementation.
 * 
 * @param src       The source string to duplicate.
 * @return          Pointer to newly allocated string, or NULL on failure.
 */
char* fossil_bluecrab_noshell_strdup(const char *src) {
    if (!src) return NULL;

    size_t len = 0;
    while (src[len] != '\0') len++;   // compute length manually

    char *dup = (char*)malloc(len + 1); // +1 for null terminator
    if (!dup) return NULL;

    for (size_t i = 0; i < len; i++) {
        dup[i] = src[i];
    }
    dup[len] = '\0';

    return dup;
}

// ============================================================================
// Internal State
// ============================================================================
static const char* open_dbs[MAX_OPEN_DBS] = {0};
static const char* locked_dbs[MAX_LOCKED_DBS] = {0};

// ============================================================================
// Internal: 64-bit Hash
// ============================================================================
static uint64_t fossil_bluecrab_noshell_hash64(const char *str) {
    uint64_t hash = 14695981039346656037ULL; // FNV-1a 64-bit offset basis
    unsigned char c;
    while ((c = (unsigned char)*str++)) {
        hash ^= c;
        hash *= 1099511628211ULL;
    }
    // Final mixing
    hash ^= hash >> 33;
    hash *= 0xff51afd7ed558ccdULL;
    hash ^= hash >> 33;
    hash *= 0xc4ceb9fe1a85ec53ULL;
    hash ^= hash >> 33;
    return hash;
}

// ============================================================================
// Internal: Open / Lock Helpers
// ============================================================================
static bool mark_db_open(const char *file_name) {
    for (int i=0;i<MAX_OPEN_DBS;i++) {
        if (open_dbs[i] && strcmp(open_dbs[i], file_name)==0) return false;
    }
    for (int i=0;i<MAX_OPEN_DBS;i++) {
        if (!open_dbs[i]) { open_dbs[i] = fossil_bluecrab_noshell_strdup(file_name); return true; }
    }
    return false;
}

static bool mark_db_closed(const char *file_name) {
    for (int i=0;i<MAX_OPEN_DBS;i++) {
        if (open_dbs[i] && strcmp(open_dbs[i], file_name)==0) {
            free((void*)open_dbs[i]);
            open_dbs[i]=NULL;
            return true;
        }
    }
    return false;
}

static bool mark_db_locked(const char *file_name) {
    for (int i=0;i<MAX_LOCKED_DBS;i++) if (locked_dbs[i] && strcmp(locked_dbs[i], file_name)==0) return false;
    for (int i=0;i<MAX_LOCKED_DBS;i++) if (!locked_dbs[i]) { locked_dbs[i]=fossil_bluecrab_noshell_strdup(file_name); return true; }
    return false;
}

static bool mark_db_unlocked(const char *file_name) {
    for (int i=0;i<MAX_LOCKED_DBS;i++) {
        if (locked_dbs[i] && strcmp(locked_dbs[i], file_name)==0) {
            free((void*)locked_dbs[i]);
            locked_dbs[i]=NULL;
            return true;
        }
    }
    return false;
}

bool fossil_bluecrab_noshell_is_locked(const char *file_name) {
    for (int i=0;i<MAX_LOCKED_DBS;i++) if (locked_dbs[i] && strcmp(locked_dbs[i], file_name)==0) return true;
    return false;
}

// ===========================================================
// FSON v2 support 
// ===========================================================

// portable case-insensitive compare
static int noshell_stricmp(const char *a, const char *b) {
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

static const char* noshell_fson_type_to_string(fossil_bluecrab_noshell_fson_type_t type) {
    switch (type) {
        case NOSHELL_FSON_TYPE_NULL:     return "null";
        case NOSHELL_FSON_TYPE_BOOL:     return "bool";
        case NOSHELL_FSON_TYPE_I8:       return "i8";
        case NOSHELL_FSON_TYPE_I16:      return "i16";
        case NOSHELL_FSON_TYPE_I32:      return "i32";
        case NOSHELL_FSON_TYPE_I64:      return "i64";
        case NOSHELL_FSON_TYPE_U8:       return "u8";
        case NOSHELL_FSON_TYPE_U16:      return "u16";
        case NOSHELL_FSON_TYPE_U32:      return "u32";
        case NOSHELL_FSON_TYPE_U64:      return "u64";
        case NOSHELL_FSON_TYPE_F32:      return "f32";
        case NOSHELL_FSON_TYPE_F64:      return "f64";
        case NOSHELL_FSON_TYPE_OCT:      return "oct";
        case NOSHELL_FSON_TYPE_HEX:      return "hex";
        case NOSHELL_FSON_TYPE_BIN:      return "bin";
        case NOSHELL_FSON_TYPE_CHAR:     return "char";
        case NOSHELL_FSON_TYPE_CSTR:     return "cstr";
        case NOSHELL_FSON_TYPE_ARRAY:    return "array";
        case NOSHELL_FSON_TYPE_OBJECT:   return "object";
        case NOSHELL_FSON_TYPE_ENUM:     return "enum";
        case NOSHELL_FSON_TYPE_DATETIME: return "datetime";
        case NOSHELL_FSON_TYPE_DURATION: return "duration";
        default: return "unknown";
    }
}

static fossil_bluecrab_noshell_fson_type_t noshell_fson_type_from_string(const char *s) {
    if (!s) return NOSHELL_FSON_TYPE_NULL;

    if (noshell_stricmp(s, "null")     == 0) return NOSHELL_FSON_TYPE_NULL;
    if (noshell_stricmp(s, "bool")     == 0) return NOSHELL_FSON_TYPE_BOOL;
    if (noshell_stricmp(s, "i8")       == 0) return NOSHELL_FSON_TYPE_I8;
    if (noshell_stricmp(s, "i16")      == 0) return NOSHELL_FSON_TYPE_I16;
    if (noshell_stricmp(s, "i32")      == 0) return NOSHELL_FSON_TYPE_I32;
    if (noshell_stricmp(s, "i64")      == 0) return NOSHELL_FSON_TYPE_I64;
    if (noshell_stricmp(s, "u8")       == 0) return NOSHELL_FSON_TYPE_U8;
    if (noshell_stricmp(s, "u16")      == 0) return NOSHELL_FSON_TYPE_U16;
    if (noshell_stricmp(s, "u32")      == 0) return NOSHELL_FSON_TYPE_U32;
    if (noshell_stricmp(s, "u64")      == 0) return NOSHELL_FSON_TYPE_U64;
    if (noshell_stricmp(s, "f32")      == 0) return NOSHELL_FSON_TYPE_F32;
    if (noshell_stricmp(s, "f64")      == 0) return NOSHELL_FSON_TYPE_F64;
    if (noshell_stricmp(s, "oct")      == 0) return NOSHELL_FSON_TYPE_OCT;
    if (noshell_stricmp(s, "hex")      == 0) return NOSHELL_FSON_TYPE_HEX;
    if (noshell_stricmp(s, "bin")      == 0) return NOSHELL_FSON_TYPE_BIN;
    if (noshell_stricmp(s, "char")     == 0) return NOSHELL_FSON_TYPE_CHAR;
    if (noshell_stricmp(s, "cstr")     == 0) return NOSHELL_FSON_TYPE_CSTR;
    if (noshell_stricmp(s, "array")    == 0) return NOSHELL_FSON_TYPE_ARRAY;
    if (noshell_stricmp(s, "object")   == 0) return NOSHELL_FSON_TYPE_OBJECT;
    if (noshell_stricmp(s, "enum")     == 0) return NOSHELL_FSON_TYPE_ENUM;
    if (noshell_stricmp(s, "datetime") == 0) return NOSHELL_FSON_TYPE_DATETIME;
    if (noshell_stricmp(s, "duration") == 0) return NOSHELL_FSON_TYPE_DURATION;

    return NOSHELL_FSON_TYPE_NULL; // fallback
}

static void noshell_fson_value_to_string(
    const fossil_bluecrab_noshell_fson_value_t *val,
    char *out, size_t out_size)
{
    const char *tname = noshell_fson_type_to_string(val->type);

    switch (val->type) {
        case NOSHELL_FSON_TYPE_NULL:
            snprintf(out, out_size, "%s:null", tname);
            break;
        case NOSHELL_FSON_TYPE_BOOL:
            snprintf(out, out_size, "%s:%s", tname, val->as.b ? "true" : "false");
            break;

        case NOSHELL_FSON_TYPE_I8:  snprintf(out, out_size, "%s:%" PRId8,  tname, val->as.i8); break;
        case NOSHELL_FSON_TYPE_I16: snprintf(out, out_size, "%s:%" PRId16, tname, val->as.i16); break;
        case NOSHELL_FSON_TYPE_I32: snprintf(out, out_size, "%s:%" PRId32, tname, val->as.i32); break;
        case NOSHELL_FSON_TYPE_I64: snprintf(out, out_size, "%s:%" PRId64, tname, val->as.i64); break;

        case NOSHELL_FSON_TYPE_U8:  snprintf(out, out_size, "%s:%" PRIu8,  tname, val->as.u8); break;
        case NOSHELL_FSON_TYPE_U16: snprintf(out, out_size, "%s:%" PRIu16, tname, val->as.u16); break;
        case NOSHELL_FSON_TYPE_U32: snprintf(out, out_size, "%s:%" PRIu32, tname, val->as.u32); break;
        case NOSHELL_FSON_TYPE_U64: snprintf(out, out_size, "%s:%" PRIu64, tname, val->as.u64); break;

        case NOSHELL_FSON_TYPE_F32: snprintf(out, out_size, "%s:%g",   tname, val->as.f32); break;
        case NOSHELL_FSON_TYPE_F64: snprintf(out, out_size, "%s:%lf",  tname, val->as.f64); break;

        case NOSHELL_FSON_TYPE_CHAR:
            snprintf(out, out_size, "%s:%c", tname, val->as.c);
            break;

        // heap-allocated/string-likes
        case NOSHELL_FSON_TYPE_CSTR:
            snprintf(out, out_size, "%s:%s", tname, val->as.cstr ? val->as.cstr : "");
            break;
        case NOSHELL_FSON_TYPE_OCT:
            snprintf(out, out_size, "%s:%s", tname, val->as.oct ? val->as.oct : "");
            break;
        case NOSHELL_FSON_TYPE_HEX:
            snprintf(out, out_size, "%s:%s", tname, val->as.hex ? val->as.hex : "");
            break;
        case NOSHELL_FSON_TYPE_BIN:
            snprintf(out, out_size, "%s:%s", tname, val->as.bin ? val->as.bin : "");
            break;
        case NOSHELL_FSON_TYPE_ARRAY:
            snprintf(out, out_size, "%s:%s", tname, val->as.array ? val->as.array : "");
            break;
        case NOSHELL_FSON_TYPE_OBJECT:
            snprintf(out, out_size, "%s:%s", tname, val->as.object ? val->as.object : "");
            break;
        case NOSHELL_FSON_TYPE_ENUM:
            snprintf(out, out_size, "%s:%s", tname, val->as.enum_symbol ? val->as.enum_symbol : "");
            break;
        case NOSHELL_FSON_TYPE_DATETIME:
            snprintf(out, out_size, "%s:%s", tname, val->as.datetime ? val->as.datetime : "");
            break;
        case NOSHELL_FSON_TYPE_DURATION:
            snprintf(out, out_size, "%s:%s", tname, val->as.duration ? val->as.duration : "");
            break;

        default:
            snprintf(out, out_size, "unknown:null");
            break;
    }
}

// parse "type:value" into union
static bool noshell_fson_value_from_string(const char *encoded, fossil_bluecrab_noshell_fson_value_t *out) {
    if (!encoded || !out) return false;

    const char *colon = strchr(encoded, ':');
    if (!colon) return false;

    size_t type_len = (size_t)(colon - encoded);
    char type_buf[32];
    if (type_len >= sizeof(type_buf)) return false;

    memcpy(type_buf, encoded, type_len);
    type_buf[type_len] = '\0';

    const char *value = colon + 1;
    fossil_bluecrab_noshell_fson_type_t t = noshell_fson_type_from_string(type_buf);
    out->type = t;

    switch (t) {
        case NOSHELL_FSON_TYPE_NULL:
            break;
        case NOSHELL_FSON_TYPE_BOOL:
            out->as.b = (noshell_stricmp(value, "true") == 0);
            break;
        case NOSHELL_FSON_TYPE_I8:  out->as.i8  = (int8_t) strtol(value, NULL, 10); break;
        case NOSHELL_FSON_TYPE_I16: out->as.i16 = (int16_t)strtol(value, NULL, 10); break;
        case NOSHELL_FSON_TYPE_I32: out->as.i32 = (int32_t)strtol(value, NULL, 10); break;
        case NOSHELL_FSON_TYPE_I64: out->as.i64 = (int64_t)strtoll(value, NULL, 10); break;
        case NOSHELL_FSON_TYPE_U8:  out->as.u8  = (uint8_t) strtoul(value, NULL, 10); break;
        case NOSHELL_FSON_TYPE_U16: out->as.u16 = (uint16_t)strtoul(value, NULL, 10); break;
        case NOSHELL_FSON_TYPE_U32: out->as.u32 = (uint32_t)strtoul(value, NULL, 10); break;
        case NOSHELL_FSON_TYPE_U64: out->as.u64 = (uint64_t)strtoull(value, NULL, 10); break;
        case NOSHELL_FSON_TYPE_F32: out->as.f32 = strtof(value, NULL); break;
        case NOSHELL_FSON_TYPE_F64: out->as.f64 = strtod(value, NULL); break;
        case NOSHELL_FSON_TYPE_CHAR: out->as.c  = value[0]; break;

        case NOSHELL_FSON_TYPE_CSTR:
            out->as.cstr = fossil_bluecrab_noshell_strdup(value);
            break;
        case NOSHELL_FSON_TYPE_OCT:
            out->as.oct = fossil_bluecrab_noshell_strdup(value);
            break;
        case NOSHELL_FSON_TYPE_HEX:
            out->as.hex = fossil_bluecrab_noshell_strdup(value);
            break;
        case NOSHELL_FSON_TYPE_BIN:
            out->as.bin = fossil_bluecrab_noshell_strdup(value);
            break;
        case NOSHELL_FSON_TYPE_ARRAY:
            out->as.array = fossil_bluecrab_noshell_strdup(value);
            break;
        case NOSHELL_FSON_TYPE_OBJECT:
            out->as.object = fossil_bluecrab_noshell_strdup(value);
            break;
        case NOSHELL_FSON_TYPE_ENUM:
            out->as.enum_symbol = fossil_bluecrab_noshell_strdup(value);
            break;
        case NOSHELL_FSON_TYPE_DATETIME:
            out->as.datetime = fossil_bluecrab_noshell_strdup(value);
            break;
        case NOSHELL_FSON_TYPE_DURATION:
            out->as.duration = fossil_bluecrab_noshell_strdup(value);
            break;

        default:
            return false;
    }
    return true;
}

// ============================================================================
// Validation
// ============================================================================
bool fossil_bluecrab_noshell_validate_extension(const char *file_name) {
    return file_name && strstr(file_name, ".noshell") != NULL;
}

bool fossil_bluecrab_noshell_validate_document(const char *document) {
    return document && strlen(document)>0;
}

// ============================================================================
// Database Management
// ============================================================================
fossil_bluecrab_noshell_error_t fossil_bluecrab_noshell_create_database(const char *file_name) {
    if (!fossil_bluecrab_noshell_validate_extension(file_name)) return FOSSIL_NOSHELL_ERROR_INVALID_FILE;
    FILE *fp = fopen(file_name,"w");
    if (!fp) return FOSSIL_NOSHELL_ERROR_IO;
    fclose(fp);
    mark_db_open(file_name);
    return FOSSIL_NOSHELL_ERROR_SUCCESS;
}

fossil_bluecrab_noshell_error_t fossil_bluecrab_noshell_open_database(const char *file_name) {
    FILE *fp = fopen(file_name,"r");
    if (!fp) return FOSSIL_NOSHELL_ERROR_FILE_NOT_FOUND;
    fclose(fp);
    // Use noshell_stricmp for case-insensitive comparison in mark_db_open
    for (int i=0;i<MAX_OPEN_DBS;i++) {
        if (open_dbs[i] && noshell_stricmp(open_dbs[i], file_name)==0) return FOSSIL_NOSHELL_ERROR_SUCCESS;
    }
    mark_db_open(file_name);
    return FOSSIL_NOSHELL_ERROR_SUCCESS;
}

fossil_bluecrab_noshell_error_t fossil_bluecrab_noshell_delete_database(const char *file_name) {
    if (remove(file_name)==0) {
        // Use noshell_stricmp for case-insensitive comparison in mark_db_closed
        for (int i=0;i<MAX_OPEN_DBS;i++) {
            if (open_dbs[i] && noshell_stricmp(open_dbs[i], file_name)==0) {
                free((void*)open_dbs[i]);
                open_dbs[i]=NULL;
                break;
            }
        }
        return FOSSIL_NOSHELL_ERROR_SUCCESS;
    }
    return FOSSIL_NOSHELL_ERROR_IO;
}

// ============================================================================
// Locking
// ============================================================================
fossil_bluecrab_noshell_error_t fossil_bluecrab_noshell_lock_database(const char *file_name) {
    if (!mark_db_locked(file_name)) return FOSSIL_NOSHELL_ERROR_CONCURRENCY;
    return FOSSIL_NOSHELL_ERROR_SUCCESS;
}

fossil_bluecrab_noshell_error_t fossil_bluecrab_noshell_unlock_database(const char *file_name) {
    if (!mark_db_unlocked(file_name)) return FOSSIL_NOSHELL_ERROR_NOT_FOUND;
    return FOSSIL_NOSHELL_ERROR_SUCCESS;
}

// ============================================================================
// CRUD
// ============================================================================
fossil_bluecrab_noshell_error_t fossil_bluecrab_noshell_insert(const char *file_name, const char *document) {
    if (!fossil_bluecrab_noshell_validate_document(document)) return FOSSIL_NOSHELL_ERROR_INVALID_QUERY;
    FILE *fp=fopen(file_name,"a");
    if (!fp) return FOSSIL_NOSHELL_ERROR_IO;
    uint64_t hash=fossil_bluecrab_noshell_hash64(document);
    fprintf(fp,"%s|%llu\n",document,(unsigned long long)hash);
    fclose(fp);
    return FOSSIL_NOSHELL_ERROR_SUCCESS;
}

fossil_bluecrab_noshell_error_t fossil_bluecrab_noshell_insert_with_id(const char *file_name, const char *document, char *out_id, size_t id_size) {
    if (!fossil_bluecrab_noshell_validate_document(document)) return FOSSIL_NOSHELL_ERROR_INVALID_QUERY;
    uint64_t hash=fossil_bluecrab_noshell_hash64(document);
    snprintf(out_id,id_size,"%llu",(unsigned long long)hash);
    return fossil_bluecrab_noshell_insert(file_name, document);
}

// Simple line-based find (first match, case-insensitive)
fossil_bluecrab_noshell_error_t fossil_bluecrab_noshell_find(const char *file_name, const char *query, char *result, size_t buffer_size) {
    FILE *fp=fopen(file_name,"r");
    if (!fp) return FOSSIL_NOSHELL_ERROR_FILE_NOT_FOUND;
    char line[1024];
    while(fgets(line,sizeof(line),fp)) {
        char *pipe=strchr(line,'|');
        if (pipe) *pipe='\0';
        if (noshell_stricmp(line,query)==0 || (query && strstr(line,query))) {
            strncpy(result,line,buffer_size-1);
            result[buffer_size-1]='\0';
            fclose(fp);
            return FOSSIL_NOSHELL_ERROR_SUCCESS;
        }
    }
    fclose(fp);
    return FOSSIL_NOSHELL_ERROR_NOT_FOUND;
}

// Callback-based find (all matches, first match returned, case-insensitive)
fossil_bluecrab_noshell_error_t fossil_bluecrab_noshell_find_cb(const char *file_name, bool (*cb)(const char *, void *), void *userdata) {
    FILE *fp=fopen(file_name,"r");
    if (!fp) return FOSSIL_NOSHELL_ERROR_FILE_NOT_FOUND;
    char line[1024];
    while(fgets(line,sizeof(line),fp)) {
        char *pipe=strchr(line,'|');
        if (pipe) *pipe='\0';
        if (cb && cb(line, userdata)) {
            fclose(fp);
            return FOSSIL_NOSHELL_ERROR_SUCCESS;
        }
    }
    fclose(fp);
    return FOSSIL_NOSHELL_ERROR_NOT_FOUND;
}

// Update / Remove (simple first-match replace/delete, case-insensitive)
fossil_bluecrab_noshell_error_t fossil_bluecrab_noshell_update(const char *file_name, const char *query, const char *new_document) {
    FILE *fp=fopen(file_name,"r");
    if (!fp) return FOSSIL_NOSHELL_ERROR_FILE_NOT_FOUND;
    FILE *tmp=fopen("tmp.noshell","w");
    if (!tmp) { fclose(fp); return FOSSIL_NOSHELL_ERROR_IO; }
    char line[1024]; bool updated=false;
    while(fgets(line,sizeof(line),fp)) {
        char *pipe=strchr(line,'|'); if (pipe) *pipe='\0';
        if (!updated && (noshell_stricmp(line,query)==0 || (query && strstr(line,query)))) {
            fprintf(tmp,"%s|%llu\n",new_document,(unsigned long long)fossil_bluecrab_noshell_hash64(new_document)); updated=true;
        }
        else fprintf(tmp,"%s|%s\n",line,pipe?pipe+1:"");
    }
    fclose(fp); fclose(tmp);
    remove(file_name); rename("tmp.noshell",file_name);
    return updated?FOSSIL_NOSHELL_ERROR_SUCCESS:FOSSIL_NOSHELL_ERROR_NOT_FOUND;
}

fossil_bluecrab_noshell_error_t fossil_bluecrab_noshell_remove(const char *file_name, const char *query) {
    FILE *fp=fopen(file_name,"r");
    if (!fp) return FOSSIL_NOSHELL_ERROR_FILE_NOT_FOUND;
    FILE *tmp=fopen("tmp.noshell","w");
    if (!tmp) { fclose(fp); return FOSSIL_NOSHELL_ERROR_IO; }
    char line[1024]; bool removed=false;
    while(fgets(line,sizeof(line),fp)) {
        char *pipe=strchr(line,'|'); if (pipe) *pipe='\0';
        if (!removed && (noshell_stricmp(line,query)==0 || (query && strstr(line,query)))) { removed=true; continue; }
        fprintf(tmp,"%s|%s\n",line,pipe?pipe+1:"");
    }
    fclose(fp); fclose(tmp);
    remove(file_name); rename("tmp.noshell",file_name);
    return removed?FOSSIL_NOSHELL_ERROR_SUCCESS:FOSSIL_NOSHELL_ERROR_NOT_FOUND;
}

// ============================================================================
// Backup / Restore / Verify
// ============================================================================
fossil_bluecrab_noshell_error_t fossil_bluecrab_noshell_backup_database(const char *source_file, const char *backup_file) {
    // Use noshell_stricmp for case-insensitive file name comparison
    if (!source_file || !backup_file) return FOSSIL_NOSHELL_ERROR_INVALID_FILE;
    if (noshell_stricmp(source_file, backup_file) == 0) return FOSSIL_NOSHELL_ERROR_INVALID_QUERY;

    FILE *src = fopen(source_file, "rb");
    if (!src) return FOSSIL_NOSHELL_ERROR_FILE_NOT_FOUND;
    FILE *dst = fopen(backup_file, "wb");
    if (!dst) { fclose(src); return FOSSIL_NOSHELL_ERROR_IO; }
    char buf[1024]; size_t n;
    while ((n = fread(buf, 1, sizeof(buf), src)) > 0) fwrite(buf, 1, n, dst);
    fclose(src); fclose(dst);
    return FOSSIL_NOSHELL_ERROR_SUCCESS;
}

fossil_bluecrab_noshell_error_t fossil_bluecrab_noshell_restore_database(const char *backup_file, const char *destination_file) {
    // Use noshell_stricmp for case-insensitive file name comparison
    if (!backup_file || !destination_file) return FOSSIL_NOSHELL_ERROR_INVALID_FILE;
    if (noshell_stricmp(backup_file, destination_file) == 0) return FOSSIL_NOSHELL_ERROR_INVALID_QUERY;

    FILE *src = fopen(backup_file, "rb");
    if (!src) return FOSSIL_NOSHELL_ERROR_FILE_NOT_FOUND;
    FILE *dst = fopen(destination_file, "wb");
    if (!dst) { fclose(src); return FOSSIL_NOSHELL_ERROR_IO; }
    char buf[1024]; size_t n;
    while ((n = fread(buf, 1, sizeof(buf), src)) > 0) fwrite(buf, 1, n, dst);
    fclose(src); fclose(dst);
    return FOSSIL_NOSHELL_ERROR_SUCCESS;
}

fossil_bluecrab_noshell_error_t fossil_bluecrab_noshell_verify_database(const char *file_name) {
    FILE *fp = fopen(file_name, "r");
    if (!fp) return FOSSIL_NOSHELL_ERROR_FILE_NOT_FOUND;
    char line[1024];
    while (fgets(line, sizeof(line), fp)) {
        char *pipe = strchr(line, '|');
        if (!pipe) { fclose(fp); return FOSSIL_NOSHELL_ERROR_UNKNOWN; }
        *pipe = '\0';
        uint64_t stored_hash = strtoull(pipe + 1, NULL, 10);
        // Use noshell_stricmp for case-insensitive document hash verification
        if (fossil_bluecrab_noshell_hash64(line) != stored_hash) {
            fclose(fp);
            return FOSSIL_NOSHELL_ERROR_INVALID_QUERY;
        }
    }
    fclose(fp);
    return FOSSIL_NOSHELL_ERROR_SUCCESS;
}

// ============================================================================
// Iteration / Metadata
// ============================================================================

fossil_bluecrab_noshell_error_t fossil_bluecrab_noshell_first_document(const char *file_name, char *id_buffer, size_t buffer_size) {
    FILE *fp = fopen(file_name, "r");
    if (!fp) return FOSSIL_NOSHELL_ERROR_FILE_NOT_FOUND;
    char line[1024];
    if (!fgets(line, sizeof(line), fp)) {
        fclose(fp);
        return FOSSIL_NOSHELL_ERROR_NOT_FOUND;
    }
    char *pipe = strchr(line, '|');
    if (pipe) *pipe = '\0';
    strncpy(id_buffer, line, buffer_size - 1);
    id_buffer[buffer_size - 1] = '\0';
    fclose(fp);
    return FOSSIL_NOSHELL_ERROR_SUCCESS;
}

fossil_bluecrab_noshell_error_t fossil_bluecrab_noshell_next_document(const char *file_name, const char *prev_id, char *id_buffer, size_t buffer_size) {
    FILE *fp = fopen(file_name, "r");
    if (!fp) return FOSSIL_NOSHELL_ERROR_FILE_NOT_FOUND;
    char line[1024];
    bool found = false;
    while (fgets(line, sizeof(line), fp)) {
        char *pipe = strchr(line, '|');
        if (pipe) *pipe = '\0';
        if (found) {
            strncpy(id_buffer, line, buffer_size - 1);
            id_buffer[buffer_size - 1] = '\0';
            fclose(fp);
            return FOSSIL_NOSHELL_ERROR_SUCCESS;
        }
        if (noshell_stricmp(line, prev_id) == 0) found = true;
    }
    fclose(fp);
    return FOSSIL_NOSHELL_ERROR_NOT_FOUND;
}

fossil_bluecrab_noshell_error_t fossil_bluecrab_noshell_count_documents(const char *file_name, size_t *count) {
    FILE *fp = fopen(file_name, "r");
    if (!fp) return FOSSIL_NOSHELL_ERROR_FILE_NOT_FOUND;
    char line[1024];
    size_t c = 0;
    while (fgets(line, sizeof(line), fp)) {
        if (strchr(line, '|')) c++;
    }
    fclose(fp);
    *count = c;
    return FOSSIL_NOSHELL_ERROR_SUCCESS;
}

fossil_bluecrab_noshell_error_t fossil_bluecrab_noshell_get_file_size(const char *file_name, size_t *size_bytes) {
    struct stat st;
    if (stat(file_name, &st) != 0) return FOSSIL_NOSHELL_ERROR_FILE_NOT_FOUND;
    *size_bytes = (size_t)st.st_size;
    return FOSSIL_NOSHELL_ERROR_SUCCESS;
}

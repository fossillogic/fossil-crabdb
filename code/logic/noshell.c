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
    FILE *fp = fopen(file_name, "w");
    if (!fp) {
        if (errno == EACCES) return FOSSIL_NOSHELL_ERROR_PERMISSION_DENIED;
        if (errno == ENOMEM) return FOSSIL_NOSHELL_ERROR_OUT_OF_MEMORY;
        return FOSSIL_NOSHELL_ERROR_IO;
    }
    fclose(fp);
    mark_db_open(file_name);
    return FOSSIL_NOSHELL_ERROR_SUCCESS;
}

fossil_bluecrab_noshell_error_t fossil_bluecrab_noshell_open_database(const char *file_name) {
    FILE *fp = fopen(file_name, "r");
    if (!fp) {
        if (errno == EACCES) return FOSSIL_NOSHELL_ERROR_PERMISSION_DENIED;
        if (errno == ENOENT) return FOSSIL_NOSHELL_ERROR_FILE_NOT_FOUND;
        return FOSSIL_NOSHELL_ERROR_IO;
    }
    fclose(fp);
    for (int i = 0; i < MAX_OPEN_DBS; i++) {
        if (open_dbs[i] && noshell_stricmp(open_dbs[i], file_name) == 0) return FOSSIL_NOSHELL_ERROR_ALREADY_EXISTS;
    }
    mark_db_open(file_name);
    return FOSSIL_NOSHELL_ERROR_SUCCESS;
}

fossil_bluecrab_noshell_error_t fossil_bluecrab_noshell_delete_database(const char *file_name) {
    if (remove(file_name) == 0) {
        for (int i = 0; i < MAX_OPEN_DBS; i++) {
            if (open_dbs[i] && noshell_stricmp(open_dbs[i], file_name) == 0) {
                free((void*)open_dbs[i]);
                open_dbs[i] = NULL;
                break;
            }
        }
        return FOSSIL_NOSHELL_ERROR_SUCCESS;
    }
    if (errno == EACCES) return FOSSIL_NOSHELL_ERROR_PERMISSION_DENIED;
    if (errno == ENOENT) return FOSSIL_NOSHELL_ERROR_FILE_NOT_FOUND;
    return FOSSIL_NOSHELL_ERROR_IO;
}

// ============================================================================
// Locking
// ============================================================================
fossil_bluecrab_noshell_error_t fossil_bluecrab_noshell_lock_database(const char *file_name) {
    if (fossil_bluecrab_noshell_is_locked(file_name)) return FOSSIL_NOSHELL_ERROR_LOCKED;
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
    if (!fp) {
        if (errno == EACCES) return FOSSIL_NOSHELL_ERROR_PERMISSION_DENIED;
        if (errno == ENOENT) return FOSSIL_NOSHELL_ERROR_FILE_NOT_FOUND;
        return FOSSIL_NOSHELL_ERROR_IO;
    }
    uint64_t hash=fossil_bluecrab_noshell_hash64(document);
    if (fprintf(fp,"%s|%llu\n",document,(unsigned long long)hash) < 0) {
        fclose(fp);
        return FOSSIL_NOSHELL_ERROR_IO;
    }
    fclose(fp);
    return FOSSIL_NOSHELL_ERROR_SUCCESS;
}

fossil_bluecrab_noshell_error_t fossil_bluecrab_noshell_insert_with_id(const char *file_name, const char *document, char *out_id, size_t id_size) {
    if (!fossil_bluecrab_noshell_validate_document(document)) return FOSSIL_NOSHELL_ERROR_INVALID_QUERY;
    uint64_t hash=fossil_bluecrab_noshell_hash64(document);
    snprintf(out_id,id_size,"%llu",(unsigned long long)hash);
    return fossil_bluecrab_noshell_insert(file_name, document);
}

fossil_bluecrab_noshell_error_t fossil_bluecrab_noshell_find(const char *file_name, const char *query, char *result, size_t buffer_size) {
    FILE *fp=fopen(file_name,"r");
    if (!fp) {
        if (errno == EACCES) return FOSSIL_NOSHELL_ERROR_PERMISSION_DENIED;
        if (errno == ENOENT) return FOSSIL_NOSHELL_ERROR_FILE_NOT_FOUND;
        return FOSSIL_NOSHELL_ERROR_IO;
    }
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

fossil_bluecrab_noshell_error_t fossil_bluecrab_noshell_find_cb(const char *file_name, bool (*cb)(const char *, void *), void *userdata) {
    FILE *fp=fopen(file_name,"r");
    if (!fp) {
        if (errno == EACCES) return FOSSIL_NOSHELL_ERROR_PERMISSION_DENIED;
        if (errno == ENOENT) return FOSSIL_NOSHELL_ERROR_FILE_NOT_FOUND;
        return FOSSIL_NOSHELL_ERROR_IO;
    }
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

fossil_bluecrab_noshell_error_t fossil_bluecrab_noshell_update(const char *file_name, const char *query, const char *new_document) {
    FILE *fp=fopen(file_name,"r");
    if (!fp) {
        if (errno == EACCES) return FOSSIL_NOSHELL_ERROR_PERMISSION_DENIED;
        if (errno == ENOENT) return FOSSIL_NOSHELL_ERROR_FILE_NOT_FOUND;
        return FOSSIL_NOSHELL_ERROR_IO;
    }
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
    if (remove(file_name) != 0) return FOSSIL_NOSHELL_ERROR_IO;
    if (rename("tmp.noshell",file_name) != 0) return FOSSIL_NOSHELL_ERROR_IO;
    return updated?FOSSIL_NOSHELL_ERROR_SUCCESS:FOSSIL_NOSHELL_ERROR_NOT_FOUND;
}

fossil_bluecrab_noshell_error_t fossil_bluecrab_noshell_remove(const char *file_name, const char *query) {
    FILE *fp=fopen(file_name,"r");
    if (!fp) {
        if (errno == EACCES) return FOSSIL_NOSHELL_ERROR_PERMISSION_DENIED;
        if (errno == ENOENT) return FOSSIL_NOSHELL_ERROR_FILE_NOT_FOUND;
        return FOSSIL_NOSHELL_ERROR_IO;
    }
    FILE *tmp=fopen("tmp.noshell","w");
    if (!tmp) { fclose(fp); return FOSSIL_NOSHELL_ERROR_IO; }
    char line[1024]; bool removed=false;
    while(fgets(line,sizeof(line),fp)) {
        char *pipe=strchr(line,'|'); if (pipe) *pipe='\0';
        if (!removed && (noshell_stricmp(line,query)==0 || (query && strstr(line,query)))) { removed=true; continue; }
        fprintf(tmp,"%s|%s\n",line,pipe?pipe+1:"");
    }
    fclose(fp); fclose(tmp);
    if (remove(file_name) != 0) return FOSSIL_NOSHELL_ERROR_IO;
    if (rename("tmp.noshell",file_name) != 0) return FOSSIL_NOSHELL_ERROR_IO;
    return removed?FOSSIL_NOSHELL_ERROR_SUCCESS:FOSSIL_NOSHELL_ERROR_NOT_FOUND;
}

// ============================================================================
// Backup / Restore / Verify
// ============================================================================
fossil_bluecrab_noshell_error_t fossil_bluecrab_noshell_backup_database(const char *source_file, const char *backup_file) {
    if (!source_file || !backup_file) return FOSSIL_NOSHELL_ERROR_INVALID_FILE;
    if (noshell_stricmp(source_file, backup_file) == 0) return FOSSIL_NOSHELL_ERROR_INVALID_QUERY;

    FILE *src = fopen(source_file, "rb");
    if (!src) {
        if (errno == EACCES) return FOSSIL_NOSHELL_ERROR_PERMISSION_DENIED;
        if (errno == ENOENT) return FOSSIL_NOSHELL_ERROR_FILE_NOT_FOUND;
        return FOSSIL_NOSHELL_ERROR_BACKUP_FAILED;
    }
    FILE *dst = fopen(backup_file, "wb");
    if (!dst) {
        fclose(src);
        if (errno == EACCES) return FOSSIL_NOSHELL_ERROR_PERMISSION_DENIED;
        if (errno == ENOMEM) return FOSSIL_NOSHELL_ERROR_OUT_OF_MEMORY;
        return FOSSIL_NOSHELL_ERROR_BACKUP_FAILED;
    }
    char buf[1024]; size_t n;
    while ((n = fread(buf, 1, sizeof(buf), src)) > 0) {
        if (fwrite(buf, 1, n, dst) != n) {
            fclose(src); fclose(dst);
            return FOSSIL_NOSHELL_ERROR_BACKUP_FAILED;
        }
    }
    fclose(src); fclose(dst);
    return FOSSIL_NOSHELL_ERROR_SUCCESS;
}

fossil_bluecrab_noshell_error_t fossil_bluecrab_noshell_restore_database(const char *backup_file, const char *destination_file) {
    if (!backup_file || !destination_file) return FOSSIL_NOSHELL_ERROR_INVALID_FILE;
    if (noshell_stricmp(backup_file, destination_file) == 0) return FOSSIL_NOSHELL_ERROR_INVALID_QUERY;

    FILE *src = fopen(backup_file, "rb");
    if (!src) {
        if (errno == EACCES) return FOSSIL_NOSHELL_ERROR_PERMISSION_DENIED;
        if (errno == ENOENT) return FOSSIL_NOSHELL_ERROR_FILE_NOT_FOUND;
        return FOSSIL_NOSHELL_ERROR_RESTORE_FAILED;
    }
    FILE *dst = fopen(destination_file, "wb");
    if (!dst) {
        fclose(src);
        if (errno == EACCES) return FOSSIL_NOSHELL_ERROR_PERMISSION_DENIED;
        if (errno == ENOMEM) return FOSSIL_NOSHELL_ERROR_OUT_OF_MEMORY;
        return FOSSIL_NOSHELL_ERROR_RESTORE_FAILED;
    }
    char buf[1024]; size_t n;
    while ((n = fread(buf, 1, sizeof(buf), src)) > 0) {
        if (fwrite(buf, 1, n, dst) != n) {
            fclose(src); fclose(dst);
            return FOSSIL_NOSHELL_ERROR_RESTORE_FAILED;
        }
    }
    fclose(src); fclose(dst);
    return FOSSIL_NOSHELL_ERROR_SUCCESS;
}

fossil_bluecrab_noshell_error_t fossil_bluecrab_noshell_verify_database(const char *file_name) {
    FILE *fp = fopen(file_name, "r");
    if (!fp) {
        if (errno == EACCES) return FOSSIL_NOSHELL_ERROR_PERMISSION_DENIED;
        if (errno == ENOENT) return FOSSIL_NOSHELL_ERROR_FILE_NOT_FOUND;
        return FOSSIL_NOSHELL_ERROR_IO;
    }
    char line[1024];
    while (fgets(line, sizeof(line), fp)) {
        char *pipe = strchr(line, '|');
        if (!pipe) { fclose(fp); return FOSSIL_NOSHELL_ERROR_PARSE_FAILED; }
        *pipe = '\0';
        uint64_t stored_hash = strtoull(pipe + 1, NULL, 10);
        if (fossil_bluecrab_noshell_hash64(line) != stored_hash) {
            fclose(fp);
            return FOSSIL_NOSHELL_ERROR_CORRUPTED;
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
    if (!fp) {
        if (errno == EACCES) return FOSSIL_NOSHELL_ERROR_PERMISSION_DENIED;
        if (errno == ENOENT) return FOSSIL_NOSHELL_ERROR_FILE_NOT_FOUND;
        return FOSSIL_NOSHELL_ERROR_IO;
    }
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
    if (!fp) {
        if (errno == EACCES) return FOSSIL_NOSHELL_ERROR_PERMISSION_DENIED;
        if (errno == ENOENT) return FOSSIL_NOSHELL_ERROR_FILE_NOT_FOUND;
        return FOSSIL_NOSHELL_ERROR_IO;
    }
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
    if (!fp) {
        if (errno == EACCES) return FOSSIL_NOSHELL_ERROR_PERMISSION_DENIED;
        if (errno == ENOENT) return FOSSIL_NOSHELL_ERROR_FILE_NOT_FOUND;
        return FOSSIL_NOSHELL_ERROR_IO;
    }
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
    if (stat(file_name, &st) != 0) {
        if (errno == EACCES) return FOSSIL_NOSHELL_ERROR_PERMISSION_DENIED;
        if (errno == ENOENT) return FOSSIL_NOSHELL_ERROR_FILE_NOT_FOUND;
        return FOSSIL_NOSHELL_ERROR_IO;
    }
    *size_bytes = (size_t)st.st_size;
    return FOSSIL_NOSHELL_ERROR_SUCCESS;
}

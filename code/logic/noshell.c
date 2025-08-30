/*
 * -----------------------------------------------------------------------------
 * Project: Fossil Logic
 *
 * This file is part of the Fossil Logic project, which aims to develop high-
 * performance, cross-platform applications and libraries. The code contained
 * herein is subject to the terms and conditions defined in the project license.
 *
 * Author: Michael Gene Brockus (Dreamer)
 *
 * Copyright (C) 2024 Fossil Logic. All rights reserved.
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

// ============================================================================
// Validation
// ============================================================================
bool fossil_bluecrab_noshell_validate_extension(const char *file_name) {
    return file_name && strstr(file_name, ".ndb") != NULL;
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
    mark_db_open(file_name);
    return FOSSIL_NOSHELL_ERROR_SUCCESS;
}

fossil_bluecrab_noshell_error_t fossil_bluecrab_noshell_delete_database(const char *file_name) {
    if (remove(file_name)==0) { mark_db_closed(file_name); return FOSSIL_NOSHELL_ERROR_SUCCESS; }
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

// Simple line-based find (first match)
fossil_bluecrab_noshell_error_t fossil_bluecrab_noshell_find(const char *file_name, const char *query, char *result, size_t buffer_size) {
    FILE *fp=fopen(file_name,"r");
    if (!fp) return FOSSIL_NOSHELL_ERROR_FILE_NOT_FOUND;
    char line[1024];
    while(fgets(line,sizeof(line),fp)) {
        char *pipe=strchr(line,'|');
        if (pipe) *pipe='\0';
        if (strstr(line,query)) {
            strncpy(result,line,buffer_size-1);
            result[buffer_size-1]='\0';
            fclose(fp);
            return FOSSIL_NOSHELL_ERROR_SUCCESS;
        }
    }
    fclose(fp);
    return FOSSIL_NOSHELL_ERROR_NOT_FOUND;
}

// Callback-based find (all matches, first match returned)
fossil_bluecrab_noshell_error_t fossil_bluecrab_noshell_find_cb(const char *file_name, bool (*cb)(const char *, void *), void *userdata) {
    FILE *fp=fopen(file_name,"r");
    if (!fp) return FOSSIL_NOSHELL_ERROR_FILE_NOT_FOUND;
    char line[1024];
    while(fgets(line,sizeof(line),fp)) {
        char *pipe=strchr(line,'|');
        if (pipe) *pipe='\0';
        if (cb(line, userdata)) {
            fclose(fp);
            return FOSSIL_NOSHELL_ERROR_SUCCESS;
        }
    }
    fclose(fp);
    return FOSSIL_NOSHELL_ERROR_NOT_FOUND;
}

// Update / Remove (simple first-match replace/delete)
fossil_bluecrab_noshell_error_t fossil_bluecrab_noshell_update(const char *file_name, const char *query, const char *new_document) {
    FILE *fp=fopen(file_name,"r");
    if (!fp) return FOSSIL_NOSHELL_ERROR_FILE_NOT_FOUND;
    FILE *tmp=fopen("tmp.fdb","w");
    if (!tmp) { fclose(fp); return FOSSIL_NOSHELL_ERROR_IO; }
    char line[1024]; bool updated=false;
    while(fgets(line,sizeof(line),fp)) {
        char *pipe=strchr(line,'|'); if (pipe) *pipe='\0';
        if (!updated && strstr(line,query)) { fprintf(tmp,"%s|%llu\n",new_document,(unsigned long long)fossil_bluecrab_noshell_hash64(new_document)); updated=true; }
        else fprintf(tmp,"%s|%s\n",line,pipe?pipe+1:"");
    }
    fclose(fp); fclose(tmp);
    remove(file_name); rename("tmp.fdb",file_name);
    return updated?FOSSIL_NOSHELL_ERROR_SUCCESS:FOSSIL_NOSHELL_ERROR_NOT_FOUND;
}

fossil_bluecrab_noshell_error_t fossil_bluecrab_noshell_remove(const char *file_name, const char *query) {
    FILE *fp=fopen(file_name,"r");
    if (!fp) return FOSSIL_NOSHELL_ERROR_FILE_NOT_FOUND;
    FILE *tmp=fopen("tmp.fdb","w");
    if (!tmp) { fclose(fp); return FOSSIL_NOSHELL_ERROR_IO; }
    char line[1024]; bool removed=false;
    while(fgets(line,sizeof(line),fp)) {
        char *pipe=strchr(line,'|'); if (pipe) *pipe='\0';
        if (!removed && strstr(line,query)) { removed=true; continue; }
        fprintf(tmp,"%s|%s\n",line,pipe?pipe+1:"");
    }
    fclose(fp); fclose(tmp);
    remove(file_name); rename("tmp.fdb",file_name);
    return removed?FOSSIL_NOSHELL_ERROR_SUCCESS:FOSSIL_NOSHELL_ERROR_NOT_FOUND;
}

// ============================================================================
// Backup / Restore / Verify
// ============================================================================
fossil_bluecrab_noshell_error_t fossil_bluecrab_noshell_backup_database(const char *source_file, const char *backup_file) {
    FILE *src=fopen(source_file,"rb");
    if (!src) return FOSSIL_NOSHELL_ERROR_FILE_NOT_FOUND;
    FILE *dst=fopen(backup_file,"wb");
    if (!dst) { fclose(src); return FOSSIL_NOSHELL_ERROR_IO; }
    char buf[1024]; size_t n;
    while((n=fread(buf,1,sizeof(buf),src))>0) fwrite(buf,1,n,dst);
    fclose(src); fclose(dst);
    return FOSSIL_NOSHELL_ERROR_SUCCESS;
}

fossil_bluecrab_noshell_error_t fossil_bluecrab_noshell_restore_database(const char *backup_file, const char *destination_file) {
    FILE *src=fopen(backup_file,"rb");
    if (!src) return FOSSIL_NOSHELL_ERROR_FILE_NOT_FOUND;
    FILE *dst=fopen(destination_file,"wb");
    if (!dst) { fclose(src); return FOSSIL_NOSHELL_ERROR_IO; }
    char buf[1024]; size_t n;
    while((n=fread(buf,1,sizeof(buf),src))>0) fwrite(buf,1,n,dst);
    fclose(src); fclose(dst);
    return FOSSIL_NOSHELL_ERROR_SUCCESS;
}

fossil_bluecrab_noshell_error_t fossil_bluecrab_noshell_verify_database(const char *file_name) {
    FILE *fp=fopen(file_name,"r");
    if (!fp) return FOSSIL_NOSHELL_ERROR_FILE_NOT_FOUND;
    char line[1024];
    while(fgets(line,sizeof(line),fp)) {
        char *pipe=strchr(line,'|');
        if (!pipe) { fclose(fp); return FOSSIL_NOSHELL_ERROR_UNKNOWN; }
        *pipe='\0';
        uint64_t stored_hash=strtoull(pipe+1,NULL,10);
        if (fossil_bluecrab_noshell_hash64(line)!=stored_hash) { fclose(fp); return FOSSIL_NOSHELL_ERROR_INVALID_QUERY; }
    }
    fclose(fp);
    return FOSSIL_NOSHELL_ERROR_SUCCESS;
}

// ============================================================================
// Iteration / Metadata
// ============================================================================
fossil_bluecrab_noshell_error_t fossil_bluecrab_noshell_first_document(const char *file_name, char *id_buffer, size_t buffer_size) {
    FILE *fp=fopen(file_name,"r"); if (!fp) return FOSSIL_NOSHELL_ERROR_FILE_NOT_FOUND;
    char line[1024]; if (!fgets(line,sizeof(line),fp)) { fclose(fp); return FOSSIL_NOSHELL_ERROR_NOT_FOUND; }
    char *pipe=strchr(line,'|'); if (pipe) *pipe='\0';
    strncpy(id_buffer,line,buffer_size-1); id_buffer[buffer_size-1]='\0';
    fclose(fp); return FOSSIL_NOSHELL_ERROR_SUCCESS;
}

fossil_bluecrab_noshell_error_t fossil_bluecrab_noshell_next_document(const char *file_name, const char *prev_id, char *id_buffer, size_t buffer_size) {
    FILE *fp=fopen(file_name,"r"); if (!fp) return FOSSIL_NOSHELL_ERROR_FILE_NOT_FOUND;
    char line[1024]; bool found=false;
    while(fgets(line,sizeof(line),fp)) {
        char *pipe=strchr(line,'|'); if (pipe) *pipe='\0';
        if (found) { strncpy(id_buffer,line,buffer_size-1); id_buffer[buffer_size-1]='\0'; fclose(fp); return FOSSIL_NOSHELL_ERROR_SUCCESS; }
        if (strcmp(line,prev_id)==0) found=true;
    }
    fclose(fp); return FOSSIL_NOSHELL_ERROR_NOT_FOUND;
}

fossil_bluecrab_noshell_error_t fossil_bluecrab_noshell_count_documents(const char *file_name, size_t *count) {
    FILE *fp=fopen(file_name,"r"); if (!fp) return FOSSIL_NOSHELL_ERROR_FILE_NOT_FOUND;
    char line[1024]; size_t c=0;
    while(fgets(line,sizeof(line),fp)) { if (strchr(line,'|')) c++; }
    fclose(fp); *count=c; return FOSSIL_NOSHELL_ERROR_SUCCESS;
}

fossil_bluecrab_noshell_error_t fossil_bluecrab_noshell_get_file_size(const char *file_name, size_t *size_bytes) {
    struct stat st; if (stat(file_name,&st)!=0) return FOSSIL_NOSHELL_ERROR_FILE_NOT_FOUND;
    *size_bytes=(size_t)st.st_size; return FOSSIL_NOSHELL_ERROR_SUCCESS;
}

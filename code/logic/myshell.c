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
#include "fossil/crabdb/myshell.h"

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
static uint64_t fossil_myshell_hash(const char *str) {
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

static bool fossil_myshell_split_record(char *line, char **key, char **value, unsigned long *stored_hash) {
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

// ===========================================================
// CRUD Operations
// ===========================================================

fossil_myshell_error_t fossil_myshell_create_record(const char *file_name, const char *key, const char *value) {
    if (!fossil_myshell_validate_extension(file_name))
        return FOSSIL_MYSHELL_ERROR_INVALID_FILE;

    FILE *file = fopen(file_name, "a");
    if (!file) return FOSSIL_MYSHELL_ERROR_IO;

    char record[512];
    snprintf(record, sizeof(record), "%s=%s", key, value);
    unsigned long hash = fossil_myshell_hash(record);

    fprintf(file, "%s|%lu\n", record, hash);
    fclose(file);

    return FOSSIL_MYSHELL_ERROR_SUCCESS;
}

fossil_myshell_error_t fossil_myshell_read_record(const char *file_name, const char *key, char *value, size_t buffer_size) {
    if (!fossil_myshell_validate_extension(file_name))
        return FOSSIL_MYSHELL_ERROR_INVALID_FILE;

    FILE *file = fopen(file_name, "r");
    if (!file) return FOSSIL_MYSHELL_ERROR_IO;

    char line[512];
    while (fgets(line, sizeof(line), file)) {
        char *line_key, *line_value;
        unsigned long stored_hash;

        if (!fossil_myshell_split_record(line, &line_key, &line_value, &stored_hash))
            continue;

        char temp[512];
        snprintf(temp, sizeof(temp), "%s=%s", line_key, line_value);
        unsigned long calc_hash = fossil_myshell_hash(temp);

        if (calc_hash != stored_hash)
            return FOSSIL_MYSHELL_ERROR_CORRUPTED;

        if (strcmp(line_key, key) == 0) {
            strncpy(value, line_value, buffer_size);
            value[buffer_size - 1] = '\0';
            fclose(file);
            return FOSSIL_MYSHELL_ERROR_SUCCESS;
        }
    }

    fclose(file);
    return FOSSIL_MYSHELL_ERROR_NOT_FOUND;
}

fossil_myshell_error_t fossil_myshell_update_record(const char *file_name, const char *key, const char *new_value) {
    if (!fossil_myshell_validate_extension(file_name))
        return FOSSIL_MYSHELL_ERROR_INVALID_FILE;

    FILE *file = fopen(file_name, "r");
    if (!file) return FOSSIL_MYSHELL_ERROR_IO;

    FILE *temp = fopen("temp.fdb", "w");
    if (!temp) {
        fclose(file);
        return FOSSIL_MYSHELL_ERROR_IO;
    }

    char line[512];
    bool updated = false;

    while (fgets(line, sizeof(line), file)) {
        char *line_key, *line_value;
        unsigned long stored_hash;

        if (!fossil_myshell_split_record(line, &line_key, &line_value, &stored_hash)) {
            fputs(line, temp); // preserve invalid line
            continue;
        }

        if (strcmp(line_key, key) == 0) {
            char record[512];
            snprintf(record, sizeof(record), "%s=%s", key, new_value);
            unsigned long new_hash = fossil_myshell_hash(record);
            fprintf(temp, "%s|%lu\n", record, new_hash);
            updated = true;
        } else {
            // preserve existing
            char record[512];
            snprintf(record, sizeof(record), "%s=%s", line_key, line_value);
            fprintf(temp, "%s|%lu\n", record, stored_hash);
        }
    }

    fclose(file);
    fclose(temp);

    if (updated) {
        remove(file_name);
        rename("temp.fdb", file_name);
        return FOSSIL_MYSHELL_ERROR_SUCCESS;
    }

    remove("temp.fdb");
    return FOSSIL_MYSHELL_ERROR_NOT_FOUND;
}

fossil_myshell_error_t fossil_myshell_delete_record(const char *file_name, const char *key) {
    if (!fossil_myshell_validate_extension(file_name))
        return FOSSIL_MYSHELL_ERROR_INVALID_FILE;

    FILE *file = fopen(file_name, "r");
    if (!file) return FOSSIL_MYSHELL_ERROR_IO;

    FILE *temp = fopen("temp.fdb", "w");
    if (!temp) {
        fclose(file);
        return FOSSIL_MYSHELL_ERROR_IO;
    }

    char line[512];
    bool deleted = false;

    while (fgets(line, sizeof(line), file)) {
        char *line_key, *line_value;
        unsigned long stored_hash;

        if (!fossil_myshell_split_record(line, &line_key, &line_value, &stored_hash)) {
            fputs(line, temp);
            continue;
        }

        if (strcmp(line_key, key) == 0) {
            deleted = true;
            continue; // skip writing
        }

        char record[512];
        snprintf(record, sizeof(record), "%s=%s", line_key, line_value);
        fprintf(temp, "%s|%lu\n", record, stored_hash);
    }

    fclose(file);
    fclose(temp);

    if (deleted) {
        remove(file_name);
        rename("temp.fdb", file_name);
        return FOSSIL_MYSHELL_ERROR_SUCCESS;
    }

    remove("temp.fdb");
    return FOSSIL_MYSHELL_ERROR_NOT_FOUND;
}

// ===========================================================
// Database Management
// ===========================================================

fossil_myshell_error_t fossil_myshell_create_database(const char *file_name) {
    if (!fossil_myshell_validate_extension(file_name))
        return FOSSIL_MYSHELL_ERROR_INVALID_FILE;

    FILE *file = fopen(file_name, "w");
    if (!file) return FOSSIL_MYSHELL_ERROR_IO;

    fclose(file);
    return FOSSIL_MYSHELL_ERROR_SUCCESS;
}

fossil_myshell_error_t fossil_myshell_open_database(const char *file_name) {
    if (!fossil_myshell_validate_extension(file_name))
        return FOSSIL_MYSHELL_ERROR_INVALID_FILE;

    FILE *file = fopen(file_name, "r");
    if (!file) return FOSSIL_MYSHELL_ERROR_FILE_NOT_FOUND;

    fclose(file);
    return FOSSIL_MYSHELL_ERROR_SUCCESS;
}

fossil_myshell_error_t fossil_myshell_delete_database(const char *file_name) {
    if (remove(file_name) == 0) {
        return FOSSIL_MYSHELL_ERROR_SUCCESS;
    }

    return FOSSIL_MYSHELL_ERROR_IO;
}

fossil_myshell_error_t fossil_myshell_close_database(const char *file_name) {
    if (!file_name) return FOSSIL_MYSHELL_ERROR_INVALID_FILE;

    if (!mark_db_closed(file_name)) {
        return FOSSIL_MYSHELL_ERROR_NOT_FOUND;
    }
    return FOSSIL_MYSHELL_ERROR_SUCCESS;
}

// ===========================================================
// Backup and Restore
// ===========================================================

fossil_myshell_error_t fossil_myshell_backup_database(const char *src_file, const char *dst_file) {
    if (!fossil_myshell_validate_extension(src_file) || !fossil_myshell_validate_extension(dst_file))
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

        if (!fossil_myshell_split_record(line, &line_key, &line_value, &stored_hash)) {
            fclose(in);
            fclose(out);
            remove(dst_file);
            return FOSSIL_MYSHELL_ERROR_CORRUPTED;
        }

        char record[512];
        snprintf(record, sizeof(record), "%s=%s", line_key, line_value);
        unsigned long calc_hash = fossil_myshell_hash(record);

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

fossil_myshell_error_t fossil_myshell_restore_database(const char *backup_file, const char *dst_file) {
    if (!fossil_myshell_validate_extension(backup_file) || !fossil_myshell_validate_extension(dst_file))
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

        if (!fossil_myshell_split_record(line, &line_key, &line_value, &stored_hash)) {
            fclose(in);
            fclose(out);
            remove(dst_file);
            return FOSSIL_MYSHELL_ERROR_CORRUPTED;
        }

        char record[512];
        snprintf(record, sizeof(record), "%s=%s", line_key, line_value);
        unsigned long calc_hash = fossil_myshell_hash(record);

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

bool fossil_myshell_is_open(const char *file_name) {
    if (!file_name) return false;
    for (int i = 0; i < MAX_OPEN_DBS; i++) {
        if (open_dbs[i] && strcmp(open_dbs[i], file_name) == 0) {
            return true;
        }
    }
    return false;
}

fossil_myshell_error_t fossil_myshell_verify_database(const char *file_name) {
    if (!fossil_myshell_validate_extension(file_name))
        return FOSSIL_MYSHELL_ERROR_INVALID_FILE;

    FILE *file = fopen(file_name, "r");
    if (!file) return FOSSIL_MYSHELL_ERROR_FILE_NOT_FOUND;

    char line[512];
    while (fgets(line, sizeof(line), file)) {
        char *line_key, *line_value;
        unsigned long stored_hash;

        if (!fossil_myshell_split_record(line, &line_key, &line_value, &stored_hash)) {
            fclose(file);
            return FOSSIL_MYSHELL_ERROR_CORRUPTED;
        }

        char record[512];
        snprintf(record, sizeof(record), "%s=%s", line_key, line_value);
        unsigned long calc_hash = fossil_myshell_hash(record);

        if (calc_hash != stored_hash) {
            fclose(file);
            return FOSSIL_MYSHELL_ERROR_CORRUPTED;
        }
    }

    fclose(file);
    return FOSSIL_MYSHELL_ERROR_SUCCESS;
}

bool fossil_myshell_validate_extension(const char *file_name) {
    const char *ext = strrchr(file_name, '.');
    return ext && strcmp(ext, ".fdb") == 0;
}

bool fossil_myshell_validate_data(const char *data) {
    return data != NULL && strlen(data) > 0;
}

// ============================================================================
// Iteration Helpers
// ============================================================================
fossil_myshell_error_t fossil_myshell_first_key(const char *file_name, char *key_buffer, size_t buffer_size) {
    if (!fossil_myshell_validate_extension(file_name))
        return FOSSIL_MYSHELL_ERROR_INVALID_FILE;

    FILE *fp = fopen(file_name, "r");
    if (!fp) return FOSSIL_MYSHELL_ERROR_FILE_NOT_FOUND;

    char line[512];
    if (!fgets(line, sizeof(line), fp)) {
        fclose(fp);
        return FOSSIL_MYSHELL_ERROR_NOT_FOUND;
    }
    fclose(fp);

    char *line_key, *line_value;
    unsigned long stored_hash;
    if (!fossil_myshell_split_record(line, &line_key, &line_value, &stored_hash))
        return FOSSIL_MYSHELL_ERROR_CORRUPTED;

    char record[512];
    snprintf(record, sizeof(record), "%s=%s", line_key, line_value);
    if (fossil_myshell_hash(record) != stored_hash)
        return FOSSIL_MYSHELL_ERROR_CORRUPTED;

    strncpy(key_buffer, line_key, buffer_size);
    key_buffer[buffer_size - 1] = '\0';
    return FOSSIL_MYSHELL_ERROR_SUCCESS;
}

fossil_myshell_error_t fossil_myshell_next_key(const char *file_name, const char *prev_key, char *key_buffer, size_t buffer_size) {
    if (!fossil_myshell_validate_extension(file_name))
        return FOSSIL_MYSHELL_ERROR_INVALID_FILE;

    FILE *fp = fopen(file_name, "r");
    if (!fp) return FOSSIL_MYSHELL_ERROR_FILE_NOT_FOUND;

    char line[512];
    bool found = false;

    while (fgets(line, sizeof(line), fp)) {
        char *line_key, *line_value;
        unsigned long stored_hash;

        if (!fossil_myshell_split_record(line, &line_key, &line_value, &stored_hash))
            continue;

        char record[512];
        snprintf(record, sizeof(record), "%s=%s", line_key, line_value);
        if (fossil_myshell_hash(record) != stored_hash) {
            fclose(fp);
            return FOSSIL_MYSHELL_ERROR_CORRUPTED;
        }

        if (found) {
            strncpy(key_buffer, line_key, buffer_size);
            key_buffer[buffer_size - 1] = '\0';
            fclose(fp);
            return FOSSIL_MYSHELL_ERROR_SUCCESS;
        }

        if (strcmp(line_key, prev_key) == 0)
            found = true;
    }

    fclose(fp);
    return FOSSIL_MYSHELL_ERROR_NOT_FOUND;
}

// ============================================================================
// Metadata Helpers
// ============================================================================
fossil_myshell_error_t fossil_myshell_count_records(const char *file_name, size_t *count) {
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

fossil_myshell_error_t fossil_myshell_get_file_size(const char *file_name, size_t *size_bytes) {
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
const char* fossil_myshell_error_string(fossil_myshell_error_t error_code) {
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

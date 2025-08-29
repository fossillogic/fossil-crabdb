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
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

// ===========================================================
// CRUD Operations
// ===========================================================

fossil_myshell_error_t fossil_myshell_create_record(const char *file_name, const char *key, const char *value) {
    FILE *file = fopen(file_name, "a");
    if (!file) {
        return FOSSIL_MYSHELL_ERROR_IO;
    }

    fprintf(file, "%s=%s\n", key, value);
    fclose(file);

    return FOSSIL_MYSHELL_ERROR_SUCCESS;
}

fossil_myshell_error_t fossil_myshell_read_record(const char *file_name, const char *key, char *value, size_t buffer_size) {
    FILE *file = fopen(file_name, "r");
    if (!file) {
        return FOSSIL_MYSHELL_ERROR_IO;
    }

    char line[256];
    while (fgets(line, sizeof(line), file)) {
        char *line_key = strtok(line, "=");
        char *line_value = strtok(NULL, "\n");

        if (line_key && strcmp(line_key, key) == 0) {
            strncpy(value, line_value, buffer_size);
            fclose(file);
            return FOSSIL_MYSHELL_ERROR_SUCCESS;
        }
    }

    fclose(file);
    return FOSSIL_MYSHELL_ERROR_NOT_FOUND;
}

fossil_myshell_error_t fossil_myshell_update_record(const char *file_name, const char *key, const char *new_value) {
    FILE *file = fopen(file_name, "r+");
    if (!file) {
        return FOSSIL_MYSHELL_ERROR_IO;
    }

    char line[256];
    long pos;
    bool found = false;
    while ((pos = ftell(file)) != -1 && fgets(line, sizeof(line), file)) {
        char *line_key = strtok(line, "=");

        if (line_key && strcmp(line_key, key) == 0) {
            found = true;
            break;
        }
    }

    if (!found) {
        fclose(file);
        return FOSSIL_MYSHELL_ERROR_NOT_FOUND;
    }

    fseek(file, pos, SEEK_SET);
    fprintf(file, "%s=%s\n", key, new_value);
    fclose(file);

    return FOSSIL_MYSHELL_ERROR_SUCCESS;
}

fossil_myshell_error_t fossil_myshell_delete_record(const char *file_name, const char *key) {
    FILE *file = fopen(file_name, "r");
    if (!file) {
        return FOSSIL_MYSHELL_ERROR_IO;
    }

    FILE *temp_file = fopen("temp.crabdb", "w");
    if (!temp_file) {
        fclose(file);
        return FOSSIL_MYSHELL_ERROR_IO;
    }

    char line[256];
    bool deleted = false;
    while (fgets(line, sizeof(line), file)) {
        char *line_key = strtok(line, "=");
        if (line_key && strcmp(line_key, key) == 0) {
            deleted = true;
        } else {
            fputs(line, temp_file);
        }
    }

    fclose(file);
    fclose(temp_file);

    if (deleted) {
        remove(file_name);
        rename("temp.crabdb", file_name);
        return FOSSIL_MYSHELL_ERROR_SUCCESS;
    }

    remove("temp.crabdb");
    return FOSSIL_MYSHELL_ERROR_NOT_FOUND;
}

// ===========================================================
// Database Management
// ===========================================================

fossil_myshell_error_t fossil_myshell_create_database(const char *file_name) {
    FILE *file = fopen(file_name, "w");
    if (!file) {
        return FOSSIL_MYSHELL_ERROR_IO;
    }

    fclose(file);
    return FOSSIL_MYSHELL_ERROR_SUCCESS;
}

fossil_myshell_error_t fossil_myshell_open_database(const char *file_name) {
    FILE *file = fopen(file_name, "r");
    if (!file) {
        return FOSSIL_MYSHELL_ERROR_FILE_NOT_FOUND;
    }

    fclose(file);
    return FOSSIL_MYSHELL_ERROR_SUCCESS;
}

fossil_myshell_error_t fossil_myshell_delete_database(const char *file_name) {
    if (remove(file_name) == 0) {
        return FOSSIL_MYSHELL_ERROR_SUCCESS;
    }

    return FOSSIL_MYSHELL_ERROR_IO;
}

// ===========================================================
// Backup and Restore
// ===========================================================

fossil_myshell_error_t fossil_myshell_backup_database(const char *source_file, const char *backup_file) {
    FILE *source = fopen(source_file, "rb");
    if (!source) {
        return FOSSIL_MYSHELL_ERROR_IO;
    }

    FILE *backup = fopen(backup_file, "wb");
    if (!backup) {
        fclose(source);
        return FOSSIL_MYSHELL_ERROR_IO;
    }

    char buffer[1024];
    size_t bytes_read;
    while ((bytes_read = fread(buffer, 1, sizeof(buffer), source)) > 0) {
        fwrite(buffer, 1, bytes_read, backup);
    }

    fclose(source);
    fclose(backup);

    return FOSSIL_MYSHELL_ERROR_SUCCESS;
}

fossil_myshell_error_t fossil_myshell_restore_database(const char *backup_file, const char *destination_file) {
    FILE *backup = fopen(backup_file, "rb");
    if (!backup) {
        return FOSSIL_MYSHELL_ERROR_IO;
    }

    FILE *destination = fopen(destination_file, "wb");
    if (!destination) {
        fclose(backup);
        return FOSSIL_MYSHELL_ERROR_IO;
    }

    char buffer[1024];
    size_t bytes_read;
    while ((bytes_read = fread(buffer, 1, sizeof(buffer), backup)) > 0) {
        fwrite(buffer, 1, bytes_read, destination);
    }

    fclose(backup);
    fclose(destination);

    return FOSSIL_MYSHELL_ERROR_SUCCESS;
}

// ===========================================================
// Query and Data Validation
// ===========================================================

bool fossil_myshell_validate_extension(const char *file_name) {
    return strstr(file_name, ".crabdb") != NULL;
}

bool fossil_myshell_validate_data(const char *data) {
    return data != NULL && strlen(data) > 0;
}

// ============================================================================
// Internal: Track open databases
// ============================================================================
#define MAX_OPEN_DBS 32

static const char* open_dbs[MAX_OPEN_DBS] = {0};

// Helper: mark database as open
static bool mark_db_open(const char *file_name) {
    for (int i = 0; i < MAX_OPEN_DBS; i++) {
        if (open_dbs[i] && strcmp(open_dbs[i], file_name) == 0) {
            return false; // already open
        }
    }
    for (int i = 0; i < MAX_OPEN_DBS; i++) {
        if (open_dbs[i] == NULL) {
            open_dbs[i] = strdup(file_name);
            return true;
        }
    }
    return false; // no space
}

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
// Close / IsOpen
// ============================================================================
fossil_myshell_error_t fossil_myshell_close_database(const char *file_name) {
    if (!file_name) return FOSSIL_MYSHELL_ERROR_INVALID_FILE;

    if (!mark_db_closed(file_name)) {
        return FOSSIL_MYSHELL_ERROR_NOT_FOUND;
    }
    return FOSSIL_MYSHELL_ERROR_SUCCESS;
}

bool fossil_myshell_is_open(const char *file_name) {
    if (!file_name) return false;
    for (int i = 0; i < MAX_OPEN_DBS; i++) {
        if (open_dbs[i] && strcmp(open_dbs[i], file_name) == 0) {
            return true;
        }
    }
    return false;
}

// ============================================================================
// Iteration Helpers
// ============================================================================
fossil_myshell_error_t fossil_myshell_first_key(const char *file_name, char *key_buffer, size_t buffer_size) {
    if (!file_name || !key_buffer) return FOSSIL_MYSHELL_ERROR_INVALID_FILE;

    FILE *fp = fopen(file_name, "r");
    if (!fp) return FOSSIL_MYSHELL_ERROR_FILE_NOT_FOUND;

    char line[1024];
    if (!fgets(line, sizeof(line), fp)) {
        fclose(fp);
        return FOSSIL_MYSHELL_ERROR_NOT_FOUND;
    }

    fclose(fp);

    char *eq = strchr(line, '=');
    if (!eq) return FOSSIL_MYSHELL_ERROR_CORRUPTED;

    *eq = '\0'; // isolate key
    strncpy(key_buffer, line, buffer_size);
    key_buffer[buffer_size-1] = '\0';

    return FOSSIL_MYSHELL_ERROR_SUCCESS;
}

fossil_myshell_error_t fossil_myshell_next_key(const char *file_name, const char *prev_key, char *key_buffer, size_t buffer_size) {
    if (!file_name || !prev_key || !key_buffer) return FOSSIL_MYSHELL_ERROR_INVALID_FILE;

    FILE *fp = fopen(file_name, "r");
    if (!fp) return FOSSIL_MYSHELL_ERROR_FILE_NOT_FOUND;

    char line[1024];
    bool found = false;

    while (fgets(line, sizeof(line), fp)) {
        char *eq = strchr(line, '=');
        if (!eq) continue;
        *eq = '\0';

        if (found) {
            // this is the "next"
            strncpy(key_buffer, line, buffer_size);
            key_buffer[buffer_size-1] = '\0';
            fclose(fp);
            return FOSSIL_MYSHELL_ERROR_SUCCESS;
        }

        if (strcmp(line, prev_key) == 0) {
            found = true;
        }
    }

    fclose(fp);
    return FOSSIL_MYSHELL_ERROR_NOT_FOUND; // no more keys
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

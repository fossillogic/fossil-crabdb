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
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

// ===========================================================
// Helper functions
// ===========================================================

static FILE* open_file(const char *file_name, const char *mode) {
    return fopen(file_name, mode);
}

static void close_file(FILE *file) {
    if (file != NULL) {
        fclose(file);
    }
}

// ===========================================================
// CRUD Operations
// ===========================================================

fossil_noshell_error_t fossil_noshell_insert(const char *file_name, const char *document) {
    FILE *file = open_file(file_name, "a");
    if (!file) {
        return FOSSIL_NOSHELL_ERROR_IO;
    }

    fprintf(file, "%s\n", document);
    close_file(file);

    return FOSSIL_NOSHELL_ERROR_SUCCESS;
}

fossil_noshell_error_t fossil_noshell_find(const char *file_name, const char *query, char *result, size_t buffer_size) {
    FILE *file = open_file(file_name, "r");
    if (!file) {
        return FOSSIL_NOSHELL_ERROR_IO;
    }

    char line[256];
    while (fgets(line, sizeof(line), file)) {
        if (strstr(line, query)) {
            strncpy(result, line, buffer_size);
            close_file(file);
            return FOSSIL_NOSHELL_ERROR_SUCCESS;
        }
    }

    close_file(file);
    return FOSSIL_NOSHELL_ERROR_NOT_FOUND;
}

fossil_noshell_error_t fossil_noshell_update(const char *file_name, const char *query, const char *new_document) {
    FILE *file = open_file(file_name, "r+");
    if (!file) {
        return FOSSIL_NOSHELL_ERROR_IO;
    }

    FILE *temp_file = open_file("temp.crabdb", "w");
    if (!temp_file) {
        close_file(file);
        return FOSSIL_NOSHELL_ERROR_IO;
    }

    char line[256];
    bool updated = false;
    while (fgets(line, sizeof(line), file)) {
        if (strstr(line, query)) {
            fputs(new_document, temp_file);
            fputs("\n", temp_file);
            updated = true;
        } else {
            fputs(line, temp_file);
        }
    }

    close_file(file);
    close_file(temp_file);

    if (updated) {
        remove(file_name);
        rename("temp.crabdb", file_name);
        return FOSSIL_NOSHELL_ERROR_SUCCESS;
    }

    remove("temp.crabdb");
    return FOSSIL_NOSHELL_ERROR_NOT_FOUND;
}

fossil_noshell_error_t fossil_noshell_remove(const char *file_name, const char *query) {
    FILE *file = open_file(file_name, "r");
    if (!file) {
        return FOSSIL_NOSHELL_ERROR_IO;
    }

    FILE *temp_file = open_file("temp.crabdb", "w");
    if (!temp_file) {
        close_file(file);
        return FOSSIL_NOSHELL_ERROR_IO;
    }

    char line[256];
    bool removed = false;
    while (fgets(line, sizeof(line), file)) {
        if (!strstr(line, query)) {
            fputs(line, temp_file);
        } else {
            removed = true;
        }
    }

    close_file(file);
    close_file(temp_file);

    if (removed) {
        remove(file_name);
        rename("temp.crabdb", file_name);
        return FOSSIL_NOSHELL_ERROR_SUCCESS;
    }

    remove("temp.crabdb");
    return FOSSIL_NOSHELL_ERROR_NOT_FOUND;
}

// ===========================================================
// Database Management
// ===========================================================

fossil_noshell_error_t fossil_noshell_create_database(const char *file_name) {
    FILE *file = open_file(file_name, "w");
    if (!file) {
        return FOSSIL_NOSHELL_ERROR_IO;
    }

    close_file(file);
    return FOSSIL_NOSHELL_ERROR_SUCCESS;
}

fossil_noshell_error_t fossil_noshell_open_database(const char *file_name) {
    FILE *file = open_file(file_name, "r");
    if (!file) {
        return FOSSIL_NOSHELL_ERROR_FILE_NOT_FOUND;
    }

    close_file(file);
    return FOSSIL_NOSHELL_ERROR_SUCCESS;
}

fossil_noshell_error_t fossil_noshell_close_database(const char *file_name) {
    return FOSSIL_NOSHELL_ERROR_SUCCESS;  // No specific action required for closure.
}

fossil_noshell_error_t fossil_noshell_delete_database(const char *file_name) {
    if (remove(file_name) == 0) {
        return FOSSIL_NOSHELL_ERROR_SUCCESS;
    }

    return FOSSIL_NOSHELL_ERROR_IO;
}

// ===========================================================
// Backup and Restore
// ===========================================================

fossil_noshell_error_t fossil_noshell_backup_database(const char *source_file, const char *backup_file) {
    FILE *source = open_file(source_file, "rb");
    if (!source) {
        return FOSSIL_NOSHELL_ERROR_IO;
    }

    FILE *backup = open_file(backup_file, "wb");
    if (!backup) {
        close_file(source);
        return FOSSIL_NOSHELL_ERROR_IO;
    }

    char buffer[1024];
    size_t bytes_read;
    while ((bytes_read = fread(buffer, 1, sizeof(buffer), source)) > 0) {
        fwrite(buffer, 1, bytes_read, backup);
    }

    close_file(source);
    close_file(backup);

    return FOSSIL_NOSHELL_ERROR_SUCCESS;
}

fossil_noshell_error_t fossil_noshell_restore_database(const char *backup_file, const char *destination_file) {
    FILE *backup = open_file(backup_file, "rb");
    if (!backup) {
        return FOSSIL_NOSHELL_ERROR_IO;
    }

    FILE *destination = open_file(destination_file, "wb");
    if (!destination) {
        close_file(backup);
        return FOSSIL_NOSHELL_ERROR_IO;
    }

    char buffer[1024];
    size_t bytes_read;
    while ((bytes_read = fread(buffer, 1, sizeof(buffer), backup)) > 0) {
        fwrite(buffer, 1, bytes_read, destination);
    }

    close_file(backup);
    close_file(destination);

    return FOSSIL_NOSHELL_ERROR_SUCCESS;
}

// ===========================================================
// Query and Data Validation
// ===========================================================

bool fossil_noshell_validate_extension(const char *file_name) {
    return strstr(file_name, ".crabdb") != NULL;
}

bool fossil_noshell_validate_document(const char *document) {
    return document != NULL && strlen(document) > 0;
}

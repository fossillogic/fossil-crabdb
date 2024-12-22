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
#include "fossil/crabdb/store.h"

// *****************************************************************************
// INI Storage API
// *****************************************************************************

bool fossil_crabstore_save_to_ini(const fossil_crabdb_book_t *book, const char *filename) {
    if (book == NULL || filename == NULL) {
        return false;
    }
    FILE *file = fopen(filename, "w");
    if (file == NULL) {
        return false;
    }
    fossil_crabdb_page_t *current = book->head;
    while (current != NULL) {
        fprintf(file, "%s=%s\n", current->entry.key, current->entry.value);
        current = current->next;
    }
    fclose(file);
    return true;
}

bool fossil_crabstore_load_from_ini(fossil_crabdb_book_t *book, const char *filename) {
    if (book == NULL || filename == NULL) {
        return false;
    }
    FILE *file = fopen(filename, "r");
    if (file == NULL) {
        return false;
    }
    char line[256];
    while (fgets(line, sizeof(line), file) != NULL) {
        char *key = strtok(line, "=");
        char *value = strtok(NULL, "\n");
        fossil_crabdb_insert(book, key, value, (fossil_crabdb_attributes_t){false, false, false});
    }
    fclose(file);
    return true;
}

// *****************************************************************************
// CSV Storage API
// *****************************************************************************

bool fossil_crabstore_save_to_csv(const fossil_crabdb_book_t *book, const char *filename) {
    if (book == NULL || filename == NULL) {
        return false;
    }
    FILE *file = fopen(filename, "w");
    if (file == NULL) {
        return false;
    }
    fossil_crabdb_page_t *current = book->head;
    while (current != NULL) {
        fprintf(file, "%s,%s\n", current->entry.key, current->entry.value);
        current = current->next;
    }
    fclose(file);
    return true;
}

bool fossil_crabstore_load_from_csv(fossil_crabdb_book_t *book, const char *filename) {
    if (book == NULL || filename == NULL) {
        return false;
    }
    FILE *file = fopen(filename, "r");
    if (file == NULL) {
        return false;
    }
    char line[256];
    while (fgets(line, sizeof(line), file) != NULL) {
        char *key = strtok(line, ",");
        char *value = strtok(NULL, "\n");
        fossil_crabdb_insert(book, key, value, (fossil_crabdb_attributes_t){false, false, false});
    }
    fclose(file);
    return true;
}

// *****************************************************************************
// Utility Functions for Storage
// *****************************************************************************

bool fossil_crabstore_validate_ini(const char *filename) {
    if (filename == NULL) {
        return false;
    }
    FILE *file = fopen(filename, "r");
    if (file == NULL) {
        return false;
    }
    char line[256];
    while (fgets(line, sizeof(line), file) != NULL) {
        if (strchr(line, '=') == NULL) {
            fclose(file);
            return false;
        }
    }
    fclose(file);
    return true;
}

bool fossil_crabstore_validate_csv(const char *filename) {
    if (filename == NULL) {
        return false;
    }
    FILE *file = fopen(filename, "r");
    if (file == NULL) {
        return false;
    }
    char line[256];
    while (fgets(line, sizeof(line), file) != NULL) {
        if (strchr(line, ',') == NULL) {
            fclose(file);
            return false;
        }
    }
    fclose(file);
    return true;
}

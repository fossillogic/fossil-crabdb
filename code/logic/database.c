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
#include "fossil/crabdb/database.h"

static fossil_crabdb_book_t *db = NULL;

// *****************************************************************************
// Create and destroy
// *****************************************************************************

// Initialize the database.
void fossil_crabdb_open(const char *filename) {
    // Check if the file has a .crabdb extension
    const char *ext = strrchr(filename, '.');
    if (!ext || strcmp(ext, ".crabdb") != 0) {
        fprintf(stderr, "Invalid file extension. Expected a .crabdb file.\n");
        exit(EXIT_FAILURE);
    }

    db = (fossil_crabdb_book_t *)malloc(sizeof(fossil_crabdb_book_t));
    if (!db) {
        fprintf(stderr, "Failed to allocate memory for the database.\n");
        exit(EXIT_FAILURE);
    }
    db->head = db->tail = NULL;
    db->size = 0;

    FILE *file = fopen(filename, "rb");
    if (!file) {
        fprintf(stderr, "Failed to open the database file.\n");
        exit(EXIT_FAILURE);
    }

    char key[256];
    void *value;
    size_t value_size;
    while (!feof(file)) {
        fossil_crabdb_decode(file, key, &value, &value_size);
        fossil_crabdb_create(key, value, value_size);
        free(value);
    }

    fclose(file);
}

// Cleanup resources and close the database.
void fossil_crabdb_close(void) {
    fossil_crabdb_page_t *current = db->head;
    while (current) {
        fossil_crabdb_page_t *next = current->next;
        free(current->entry.key);
        free(current->entry.value);
        free(current);
        current = next;
    }
    free(db);
}

// *****************************************************************************
// CRUD operations
// *****************************************************************************

// Create a key-value pair.
void fossil_crabdb_create(const char *key, const void *value, size_t value_size) {
    fossil_crabdb_page_t *node = (fossil_crabdb_page_t *)malloc(sizeof(fossil_crabdb_page_t));
    if (!node) {
        fprintf(stderr, "Failed to allocate memory for the node.\n");
        exit(EXIT_FAILURE);
    }

    node->entry.key = custom_strdup(key);
    if (!node->entry.key) {
        fprintf(stderr, "Failed to allocate memory for the key.\n");
        exit(EXIT_FAILURE);
    }

    node->entry.value = malloc(value_size);
    if (!node->entry.value) {
        fprintf(stderr, "Failed to allocate memory for the value.\n");
        exit(EXIT_FAILURE);
    }

    memcpy(node->entry.value, value, value_size);
    node->entry.value_size = value_size;

    node->next = NULL;
    node->prev = db->tail;

    if (db->tail) {
        db->tail->next = node;
    } else {
        db->head = node;
    }

    db->tail = node;
    db->size++;
}

// Read a value by key.
void *fossil_crabdb_read(const char *key, size_t *value_size) {
    fossil_crabdb_page_t *current = db->head;
    while (current) {
        if (strcmp(current->entry.key, key) == 0) {
            *value_size = current->entry.value_size;
            void *value = malloc(*value_size);
            if (!value) {
                fprintf(stderr, "Failed to allocate memory for the value.\n");
                exit(EXIT_FAILURE);
            }
            memcpy(value, current->entry.value, *value_size);
            return value;
        }
        current = current->next;
    }
    return NULL;
}

// Update a key-value pair.
void fossil_crabdb_update(const char *key, const void *new_value, size_t value_size) {
    fossil_crabdb_page_t *current = db->head;
    while (current) {
        if (strcmp(current->entry.key, key) == 0) {
            free(current->entry.value);
            current->entry.value = malloc(value_size);
            if (!current->entry.value) {
                fprintf(stderr, "Failed to allocate memory for the value.\n");
                exit(EXIT_FAILURE);
            }
            memcpy(current->entry.value, new_value, value_size);
            current->entry.value_size = value_size;
            return;
        }
        current = current->next;
    }
}

// Delete a key-value pair.
void fossil_crabdb_delete(const char *key) {
    fossil_crabdb_page_t *current = db->head;
    while (current) {
        if (strcmp(current->entry.key, key) == 0) {
            if (current->prev) {
                current->prev->next = current->next;
            } else {
                db->head = current->next;
            }

            if (current->next) {
                current->next->prev = current->prev;
            } else {
                db->tail = current->prev;
            }

            free(current->entry.key);
            free(current->entry.value);
            free(current);
            db->size--;
            return;
        }
        current = current->next;
    }
}

// *****************************************************************************
// batch CRUD operations
// *****************************************************************************

void fossil_crabdb_batch_create(const fossil_crabdb_entry_t *entries, size_t count) {
    for (size_t i = 0; i < count; i++) {
        fossil_crabdb_create(entries[i].key, entries[i].value, entries[i].value_size);
    }
}

void fossil_crabdb_batch_read(const char **keys, size_t count, void ***values, size_t **value_sizes) {
    *values = (void **)malloc(count * sizeof(void *));
    if (!*values) {
        fprintf(stderr, "Failed to allocate memory for the values.\n");
        exit(EXIT_FAILURE);
    }

    *value_sizes = (size_t *)malloc(count * sizeof(size_t));
    if (!*value_sizes) {
        fprintf(stderr, "Failed to allocate memory for the value sizes.\n");
        exit(EXIT_FAILURE);
    }

    for (size_t i = 0; i < count; i++) {
        (*values)[i] = fossil_crabdb_read(keys[i], &(*value_sizes)[i]);
    }
}

void fossil_crabdb_batch_update(const fossil_crabdb_entry_t *entries, size_t count) {
    for (size_t i = 0; i < count; i++) {
        fossil_crabdb_update(entries[i].key, entries[i].value, entries[i].value_size);
    }
}

void fossil_crabdb_batch_delete(const char **keys, size_t count) {
    for (size_t i = 0; i < count; i++) {
        fossil_crabdb_delete(keys[i]);
    }
}

// *****************************************************************************
// Algorithic operations
// *****************************************************************************

fossil_crabdb_entry_t *fossil_crabdb_search(const char *partial_key, size_t *count) {
    fossil_crabdb_entry_t *matches = (fossil_crabdb_entry_t *)malloc(db->size * sizeof(fossil_crabdb_entry_t));
    if (!matches) {
        fprintf(stderr, "Failed to allocate memory for the matches.\n");
        exit(EXIT_FAILURE);
    }

    fossil_crabdb_page_t *current = db->head;
    size_t i = 0;
    while (current) {
        if (strstr(current->entry.key, partial_key)) {
            matches[i] = current->entry;
            i++;
        }
        current = current->next;
    }

    *count = i;
    return matches;
}

void fossil_crabdb_sort(void) {
    fossil_crabdb_page_t *current = db->head;
    while (current) {
        fossil_crabdb_page_t *next = current->next;
        while (next) {
            if (strcmp(current->entry.key, next->entry.key) > 0) {
                fossil_crabdb_entry_t temp = current->entry;
                current->entry = next->entry;
                next->entry = temp;
            }
            next = next->next;
        }
        current = current->next;
    }
}

fossil_crabdb_entry_t *fossil_crabdb_filter(bool (*predicate)(const fossil_crabdb_entry_t *), size_t *count) {
    fossil_crabdb_entry_t *matches = (fossil_crabdb_entry_t *)malloc(db->size * sizeof(fossil_crabdb_entry_t));
    if (!matches) {
        fprintf(stderr, "Failed to allocate memory for the matches.\n");
        exit(EXIT_FAILURE);
    }

    fossil_crabdb_page_t *current = db->head;
    size_t i = 0;
    while (current) {
        if (predicate(&current->entry)) {
            matches[i] = current->entry;
            i++;
        }
        current = current->next;
    }

    *count = i;
    return matches;
}

// *****************************************************************************
// Utility declarations
// *****************************************************************************

// Count the number of entries in the database.
size_t fossil_crabdb_count(void) {
    return db->size;
}

// Get a value by key.
void *fossil_crabdb_get(const char *key, size_t *value_size) {
    fossil_crabdb_page_t *current = db->head;
    while (current) {
        if (strcmp(current->entry.key, key) == 0) {
            *value_size = current->entry.value_size;
            void *value = malloc(*value_size);
            if (!value) {
                fprintf(stderr, "Failed to allocate memory for the value.\n");
                exit(EXIT_FAILURE);
            }
            memcpy(value, current->entry.value, *value_size);
            return value;
        }
        current = current->next;
    }
    return NULL;
}

// Check if a key exists.
bool fossil_crabdb_key_exists(const char *key) {
    fossil_crabdb_page_t *current = db->head;
    while (current) {
        if (strcmp(current->entry.key, key) == 0) {
            return true;
        }
        current = current->next;
    }
    return false;
}

// List all keys in the database.
char **fossil_crabdb_list_keys(size_t *count) {
    char **keys = (char **)malloc(db->size * sizeof(char *));
    if (!keys) {
        fprintf(stderr, "Failed to allocate memory for the keys.\n");
        exit(EXIT_FAILURE);
    }

    fossil_crabdb_page_t *current = db->head;
    size_t i = 0;
    while (current) {
        keys[i] = custom_strdup(current->entry.key);
        if (!keys[i]) {
            fprintf(stderr, "Failed to allocate memory for the key.\n");
            exit(EXIT_FAILURE);
        }
        current = current->next;
        i++;
    }

    *count = db->size;
    return keys;
}

// Clear all entries in the database.
void fossil_crabdb_clear(void) {
    fossil_crabdb_page_t *current = db->head;
    while (current) {
        fossil_crabdb_page_t *next = current->next;
        free(current->entry.key);
        free(current->entry.value);
        free(current);
        current = next;
    }
    db->head = db->tail = NULL;
    db->size = 0;
}

// *****************************************************************************
// encode and decoding
// *****************************************************************************

// Encode data to the CrabDB format.
void fossil_crabdb_encode(const char *key, const void *value, size_t value_size, FILE *file) {
    size_t key_size = strlen(key);
    fwrite(&key_size, sizeof(size_t), 1, file);
    fwrite(key, key_size, 1, file);
    fwrite(&value_size, sizeof(size_t), 1, file);
    fwrite(value, value_size, 1, file);
}

// Decode data from the CrabDB format.
void fossil_crabdb_decode(FILE *file, char *key, void **value, size_t *value_size) {
    size_t key_size;
    if (fread(&key_size, sizeof(size_t), 1, file) != 1) {
        fprintf(stderr, "Failed to read key size from the file.\n");
        exit(EXIT_FAILURE);
    }
    if (fread(key, key_size, 1, file) != 1) {
        fprintf(stderr, "Failed to read key from the file.\n");
        exit(EXIT_FAILURE);
    }
    key[key_size] = '\0';

    if (fread(value_size, sizeof(size_t), 1, file) != 1) {
        fprintf(stderr, "Failed to read value size from the file.\n");
        exit(EXIT_FAILURE);
    }
    *value = malloc(*value_size);
    if (!*value) {
        fprintf(stderr, "Failed to allocate memory for the value.\n");
        exit(EXIT_FAILURE);
    }
    if (fread(*value, *value_size, 1, file) != 1) {
        fprintf(stderr, "Failed to read value from the file.\n");
        exit(EXIT_FAILURE);
    }
}

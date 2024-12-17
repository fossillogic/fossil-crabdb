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

// Lookup table for valid strings corresponding to each tofu type.
static char *_CRABDB_TYPE_ID[] = {
    "i8",
    "i16",
    "i32",
    "i64",
    "u8",
    "u16",
    "u32",
    "u64",
    "hex",
    "octal",
    "binary",
    "float",
    "double",
    "wstr",
    "cstr",
    "cchar",
    "wchar",
    "bool",
    "size",
    "date",
    "any",
    "null"
};

static fossil_crabdb_transaction_t *active_transaction = NULL; // Current active transaction

static fossil_crabdb_book_t *db = NULL;

// *****************************************************************************
// Helper Functions
// *****************************************************************************

static bool fossil_crabdb_key_exists(const char **index, size_t index_size, const char *key) {
    for (size_t i = 0; i < index_size; ++i) {
        if (strcmp(index[i], key) == 0) {
            return true;
        }
    }
    return false;
}

static size_t fossil_crabdb_add_to_index(char ***index, size_t index_size, const char *key) {
    *index = realloc(*index, (index_size + 1) * sizeof(char *));
    if (*index == NULL) {
        fprintf(stderr, "Memory allocation failed for index\n");
        exit(EXIT_FAILURE);
    }
    (*index)[index_size] = custom_strdup(key);
    return index_size + 1;
}

char *custom_strdup(const char *str) {
    char *new_str = malloc(strlen(str) + 1);
    if (new_str == NULL) {
        return NULL;
    }
    strcpy(new_str, str);
    return new_str;
}

// *****************************************************************************
// Relational database operations
// *****************************************************************************

/**
 * Create a table-like structure in the database with attributes.
 * 
 * @param table_name The name of the table to create.
 * @param attributes The attributes that define the table schema.
 * @param attr_count The number of attributes in the schema.
 */
void fossil_crabdb_create_table(const char *table_name, fossil_crabdb_attributes_t *attributes, size_t attr_count) {
    fossil_crabdb_book_t *new_db = (fossil_crabdb_book_t *)malloc(sizeof(fossil_crabdb_book_t));
    new_db->head = NULL;
    new_db->tail = NULL;
    new_db->size = 0;
    db = new_db;
}

/**
 * Insert a new row into a table.
 * 
 * @param table_name The name of the table to insert into.
 * @param entries An array of entries to insert.
 * @param entry_count The number of entries to insert.
 */
void fossil_crabdb_insert_row(const char *table_name, const fossil_crabdb_entry_t *entries, size_t entry_count) {
    fossil_crabdb_page_t *new_page = (fossil_crabdb_page_t *)malloc(sizeof(fossil_crabdb_page_t));
    new_page->entry.key = custom_strdup(entries[0].key);
    new_page->entry.value = custom_strdup(entries[0].value);
    new_page->entry.type = entries[0].type;
    new_page->entry.attributes = entries[0].attributes;
    new_page->next = NULL;
    new_page->prev = db->tail;
    if (db->tail != NULL) {
        db->tail->next = new_page;
    }
    db->tail = new_page;
    if (db->head == NULL) {
        db->head = new_page;
    }
    db->size++;
}

/**
 * Query rows from a table using conditions.
 * 
 * @param table_name The name of the table to query from.
 * @param condition A callback function to filter rows.
 * @param result_count A pointer to store the number of matching rows.
 * 
 * @return An array of matching rows.
 */
fossil_crabdb_entry_t *fossil_crabdb_query(const char *table_name, bool (*condition)(const fossil_crabdb_entry_t *), size_t *result_count) {
    fossil_crabdb_entry_t *entries = (fossil_crabdb_entry_t *)malloc((db->size + 1) * sizeof(fossil_crabdb_entry_t));
    fossil_crabdb_page_t *current_page = db->head;
    size_t entry_index = 0;
    while (current_page != NULL) {
        entries[entry_index++] = current_page->entry;
        current_page = current_page->next;
    }
    entries[entry_index].key = NULL;
    entries[entry_index].value = NULL;
    if (condition != NULL) {
        fossil_crabdb_entry_t *filtered_entries = (fossil_crabdb_entry_t *)malloc((db->size + 1) * sizeof(fossil_crabdb_entry_t));
        size_t filtered_index = 0;
        for (size_t i = 0; entries[i].key != NULL; i++) {
            if (condition(&entries[i])) {
                filtered_entries[filtered_index++] = entries[i];
            }
        }
        *result_count = filtered_index;
        free(entries);
        return filtered_entries;
    }
    *result_count = entry_index;
    return entries;
}

fossil_crabdb_entry_t *fossil_crabdb_search(const char *table_name, const char *search_term, bool search_in_keys, size_t *result_count) {
    fossil_crabdb_entry_t *entries = fossil_crabdb_query(table_name, NULL, result_count);
    if (entries == NULL) {
        return NULL;
    }
    fossil_crabdb_entry_t *search_results = (fossil_crabdb_entry_t *)malloc(*result_count * sizeof(fossil_crabdb_entry_t));
    size_t result_index = 0;
    for (size_t i = 0; i < *result_count; i++) {
        if ((search_in_keys && strstr(entries[i].key, search_term) != NULL) ||
            (!search_in_keys && strstr(entries[i].value, search_term) != NULL)) {
            search_results[result_index++] = entries[i];
        }
    }
    *result_count = result_index;
    return search_results;
}

void fossil_crabdb_update_rows(const char *table_name, bool (*condition)(const fossil_crabdb_entry_t *), const fossil_crabdb_entry_t *new_values, size_t value_count) {
    fossil_crabdb_entry_t *entries = fossil_crabdb_query(table_name, condition, NULL);
    if (entries == NULL) {
        return;
    }
    for (size_t i = 0; entries[i].key != NULL; i++) {
        for (size_t j = 0; j < value_count; j++) {
            if (strcmp(entries[i].key, new_values[j].key) == 0) {
                free(entries[i].value);
                entries[i].value = custom_strdup(new_values[j].value);
                break;
            }
        }
    }
}

void fossil_crabdb_delete_rows(const char *table_name, bool (*condition)(const fossil_crabdb_entry_t *)) {
    fossil_crabdb_entry_t *entries = fossil_crabdb_query(table_name, condition, NULL);
    if (entries == NULL) {
        return;
    }
    for (size_t i = 0; entries[i].key != NULL; i++) {
        free(entries[i].key);
        free(entries[i].value);
    }
    free(entries);
}

void fossil_crabdb_begin_transaction(const char *transaction_name) {
    fossil_crabdb_transaction_t *transaction = (fossil_crabdb_transaction_t *)malloc(sizeof(fossil_crabdb_transaction_t));
    transaction->name = custom_strdup(transaction_name);
    transaction->next = active_transaction;
    active_transaction = transaction;
    transaction->snapshot = *db;
}

void fossil_crabdb_commit_transaction(const char *transaction_name) {
    fossil_crabdb_transaction_t *transaction = active_transaction;
    while (transaction != NULL) {
        if (strcmp(transaction->name, transaction_name) == 0) {
            *db = transaction->snapshot;
            active_transaction = transaction->next;
            free(transaction->name);
            free(transaction);
            return;
        }
        transaction = transaction->next;
    }
}

void fossil_crabdb_rollback_transaction(const char *transaction_name) {
    fossil_crabdb_transaction_t *transaction = active_transaction;
    while (transaction != NULL) {
        if (strcmp(transaction->name, transaction_name) == 0) {
            active_transaction = transaction->next;
            free(transaction->name);
            free(transaction);
            return;
        }
        transaction = transaction->next;
    }
}

void fossil_crabdb_backup(const char *file_path) {
    FILE *backup_file = fopen(file_path, "wb");
    if (backup_file == NULL) {
        fprintf(stderr, "Error: Unable to open backup file for writing.\n");
        return;
    }
    fwrite(db, sizeof(fossil_crabdb_book_t), 1, backup_file);
    fclose(backup_file);
}

void fossil_crabdb_restore(const char *file_path) {
    FILE *backup_file = fopen(file_path, "rb");
    if (backup_file == NULL) {
        fprintf(stderr, "Error: Unable to open backup file for reading.\n");
        return;
    }
    if (db == NULL) {
        db = (fossil_crabdb_book_t *)malloc(sizeof(fossil_crabdb_book_t));
    }
    if (fread(db, sizeof(fossil_crabdb_book_t), 1, backup_file) != 1) {
        fprintf(stderr, "Error: Failed to read backup file.\n");
    }
    fclose(backup_file);
}

// *****************************************************************************
// Utility functions for relational operations
// *****************************************************************************

fossil_crabdb_entry_t *fossil_crabdb_join(const char *table1, const char *table2, bool (*join_condition)(const fossil_crabdb_entry_t *, const fossil_crabdb_entry_t *), size_t *result_count) {
    fossil_crabdb_entry_t *entries1 = fossil_crabdb_query(table1, NULL, NULL);
    fossil_crabdb_entry_t *entries2 = fossil_crabdb_query(table2, NULL, NULL);
    fossil_crabdb_entry_t *joined_entries = (fossil_crabdb_entry_t *)malloc((db->size + 1) * sizeof(fossil_crabdb_entry_t));
    size_t joined_index = 0;
    for (size_t i = 0; entries1[i].key != NULL; i++) {
        for (size_t j = 0; entries2[j].key != NULL; j++) {
            if (join_condition(&entries1[i], &entries2[j])) {
                joined_entries[joined_index++] = entries1[i];
                joined_entries[joined_index++] = entries2[j];
            }
        }
    }
    *result_count = joined_index;
    return joined_entries;
}

void *fossil_crabdb_aggregate(const char *table_name, void *(*aggregate_function)(const fossil_crabdb_entry_t *, size_t)) {
    size_t result_count;
    fossil_crabdb_entry_t *entries = fossil_crabdb_query(table_name, NULL, &result_count);
    return aggregate_function(entries, result_count);
}

fossil_crabdb_entry_t *fossil_crabdb_paginate(const fossil_crabdb_entry_t *entries, size_t total_entries, size_t page_size, size_t page_number) {
    size_t start_index = page_size * page_number;
    if (start_index >= total_entries) {
        return NULL;
    }
    size_t end_index = start_index + page_size;
    if (end_index > total_entries) {
        end_index = total_entries;
    }
    size_t page_count = end_index - start_index;
    fossil_crabdb_entry_t *page_entries = (fossil_crabdb_entry_t *)malloc(page_count * sizeof(fossil_crabdb_entry_t));
    for (size_t i = 0; i < page_count; i++) {
        page_entries[i] = entries[start_index + i];
    }
    return page_entries;
}

// *****************************************************************************
// Rebuild Indices Function
// *****************************************************************************

void fossil_crabdb_rebuild_indices(const char *table_name) {
    if (table_name == NULL) {
        fprintf(stderr, "Table name cannot be NULL.\n");
        return;
    }

    printf("Rebuilding indices for table: %s\n", table_name);

    // For the sake of example, assume we have access to a global database instance.
    extern fossil_crabdb_book_t database;

    char **primary_index = NULL;   // Array to store primary keys.
    size_t primary_index_size = 0;

    char **unique_index = NULL;    // Array to store unique keys.
    size_t unique_index_size = 0;

    fossil_crabdb_page_t *current = database.head;

    while (current != NULL) {
        fossil_crabdb_entry_t *entry = &current->entry;

        // Check for primary key constraint
        if (entry->attributes.is_primary_key) {
            if (fossil_crabdb_key_exists((const char **)primary_index, primary_index_size, entry->key)) {
                fprintf(stderr, "Duplicate primary key found: %s\n", entry->key);
            } else {
                primary_index_size = fossil_crabdb_add_to_index(&primary_index, primary_index_size, entry->key);
            }
        }

        // Check for unique constraint
        if (entry->attributes.is_unique) {
            if (fossil_crabdb_key_exists((const char **)unique_index, unique_index_size, entry->key)) {
                fprintf(stderr, "Duplicate unique key found: %s\n", entry->key);
            } else {
                unique_index_size = fossil_crabdb_add_to_index(&unique_index, unique_index_size, entry->key);
            }
        }

        current = current->next;
    }

    // Cleanup: free indices
    for (size_t i = 0; i < primary_index_size; ++i) {
        free(primary_index[i]);
    }
    free(primary_index);

    for (size_t i = 0; i < unique_index_size; ++i) {
        free(unique_index[i]);
    }
    free(unique_index);

    printf("Indices successfully rebuilt for table: %s\n", table_name);
}

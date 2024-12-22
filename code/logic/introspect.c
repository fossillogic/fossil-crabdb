
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
#include "fossil/crabdb/introspect.h"

extern char *custom_strdup(const char *str);

// *****************************************************************************
// Introspection and Summary Report API
// *****************************************************************************

/**
 * @brief Provides a summary of the database structure and metadata.
 *
 * @param book Pointer to the database (fossil_crabdb_book_t).
 * @return A dynamically allocated string containing the summary report.
 *         The caller is responsible for freeing the memory.
 */
char* fossil_crabintrospect_summary(fossil_crabdb_book_t *book) {
    if (book == NULL) {
        return NULL;
    }

    size_t total_entries, unique_keys, primary_keys;
    fossil_crabintrospect_statistics(book, &total_entries, &unique_keys, &primary_keys);

    char *summary = (char *)malloc(256);
    if (summary == NULL) {
        return NULL;
    }

    snprintf(summary, 256, "Total entries: %zu\nUnique keys: %zu\nPrimary keys: %zu\n", total_entries, unique_keys, primary_keys);
    return summary;
}

/**
 * @brief Counts the number of entries matching specific attributes.
 *
 * @param book         Pointer to the database (fossil_crabdb_book_t).
 * @param attributes   Attributes to match (fossil_crabdb_attributes_t).
 * @return The number of matching entries.
 */
size_t fossil_crabintrospect_count_by_attributes(fossil_crabdb_book_t *book, fossil_crabdb_attributes_t attributes) {
    if (book == NULL) {
        return 0;
    }

    size_t count = 0;
    fossil_crabdb_page_t *current = book->head;
    while (current != NULL) {
        if (current->entry.attributes.is_primary_key == attributes.is_primary_key &&
            current->entry.attributes.is_unique == attributes.is_unique &&
            current->entry.attributes.is_nullable == attributes.is_nullable) {
            count++;
        }
        current = current->next;
    }

    return count;
}

/**
 * @brief Finds all entries that match a specific condition.
 *
 * @param book       Pointer to the database (fossil_crabdb_book_t).
 * @param predicate  Function pointer to a predicate that takes a 
 *                   fossil_crabdb_entry_t* and returns a bool.
 * @return A new database containing all matching entries.
 *         The caller is responsible for releasing the returned database.
 */
fossil_crabdb_book_t* fossil_crabintrospect_find_by_condition(
    fossil_crabdb_book_t *book,
    bool (*predicate)(fossil_crabdb_entry_t *)
) {
    if (book == NULL || predicate == NULL) {
        return NULL;
    }

    fossil_crabdb_book_t *result = fossil_crabdb_init();
    if (result == NULL) {
        return NULL;
    }

    fossil_crabdb_page_t *current = book->head;
    while (current != NULL) {
        if (predicate(&current->entry)) {
            fossil_crabdb_insert(result, custom_strdup(current->entry.key), custom_strdup(current->entry.value), current->entry.attributes);
        }
        current = current->next;
    }

    return result;
}

/**
 * @brief Retrieves statistics about the database.
 *
 * @param book           Pointer to the database (fossil_crabdb_book_t).
 * @param total_entries  Pointer to a size_t to store the total number of entries.
 * @param unique_keys    Pointer to a size_t to store the number of unique keys.
 * @param primary_keys   Pointer to a size_t to store the number of primary keys.
 */
void fossil_crabintrospect_statistics(
    fossil_crabdb_book_t *book,
    size_t *total_entries,
    size_t *unique_keys,
    size_t *primary_keys
) {
    if (book == NULL) {
        return;
    }

    *total_entries = fossil_crabdb_size(book);
    *unique_keys = fossil_crabintrospect_count_by_attributes(book, (fossil_crabdb_attributes_t){false, true, false});
    *primary_keys = fossil_crabintrospect_count_by_attributes(book, (fossil_crabdb_attributes_t){true, false, false});
}

/**
 * @brief Lists duplicate keys in the database.
 *
 * @param book Pointer to the database (fossil_crabdb_book_t).
 * @return A new database containing entries with duplicate keys.
 *         The caller is responsible for releasing the returned database.
 */
fossil_crabdb_book_t* fossil_crabintrospect_list_duplicates(fossil_crabdb_book_t *book) {
    if (book == NULL) {
        return NULL;
    }

    fossil_crabdb_book_t *result = fossil_crabdb_init();
    if (result == NULL) {
        return NULL;
    }

    fossil_crabdb_page_t *current = book->head;
    while (current != NULL) {
        if (fossil_crabdb_search(book, current->entry.key) != current) {
            fossil_crabdb_insert(result, custom_strdup(current->entry.key), custom_strdup(current->entry.value), current->entry.attributes);
        }
        current = current->next;
    }

    return result;
}

/**
 * @brief Generates a sorted list of all keys in the database.
 *
 * @param book    Pointer to the database (fossil_crabdb_book_t).
 * @param order   Sorting order (ascending or descending).
 * @return A dynamically allocated array of strings containing keys.
 *         The caller is responsible for freeing the memory and strings.
 */
char** fossil_crabintrospect_list_sorted_keys(
    fossil_crabdb_book_t *book,
    fossil_crabdb_sort_order_t order
) {
    if (book == NULL) {
        return NULL;
    }

    size_t count = fossil_crabdb_size(book);
    char **keys = (char **)malloc(count * sizeof(char *));
    if (keys == NULL) {
        return NULL;
    }

    fossil_crabdb_page_t *current = book->head;
    for (size_t i = 0; i < count; i++) {
        keys[i] = custom_strdup(current->entry.key);
        current = current->next;
    }

    fossil_crabdb_sort(book, order);

    return keys;
}

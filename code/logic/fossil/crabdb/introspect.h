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
#ifndef FOSSIL_CRABDB_INTROSPECT_H
#define FOSSIL_CRABDB_INTROSPECT_H

#include "database.h"

#ifdef __cplusplus
extern "C" {
#endif

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
char* fossil_crabintrospect_summary(fossil_crabdb_book_t *book);

/**
 * @brief Counts the number of entries matching specific attributes.
 *
 * @param book         Pointer to the database (fossil_crabdb_book_t).
 * @param attributes   Attributes to match (fossil_crabdb_attributes_t).
 * @return The number of matching entries.
 */
size_t fossil_crabintrospect_count_by_attributes(fossil_crabdb_book_t *book, fossil_crabdb_attributes_t attributes);

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
);

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
);

/**
 * @brief Lists duplicate keys in the database.
 *
 * @param book Pointer to the database (fossil_crabdb_book_t).
 * @return A new database containing entries with duplicate keys.
 *         The caller is responsible for releasing the returned database.
 */
fossil_crabdb_book_t* fossil_crabintrospect_list_duplicates(fossil_crabdb_book_t *book);

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
);

#ifdef __cplusplus
}
#endif

#endif // FOSSIL_CRABDB_QUERY_H

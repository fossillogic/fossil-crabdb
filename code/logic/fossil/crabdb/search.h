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
#ifndef FOSSIL_CRABDB_SEARCH_H
#define FOSSIL_CRABDB_SEARCH_H

#include "database.h"

#ifdef __cplusplus
extern "C" {
#endif

// *****************************************************************************
// Search API Functions
// *****************************************************************************

/**
 * @brief Finds an entry by key in the database.
 *
 * @param book  Pointer to the database (fossil_crabdb_book_t).
 * @param key   Key to search for.
 * @return      Pointer to the entry if found, NULL otherwise.
 */
fossil_crabdb_entry_t* fossil_crabsearch_by_key(fossil_crabdb_book_t *book, const char *key);

/**
 * @brief Finds all entries with a specific value in the database.
 *
 * @param book      Pointer to the database (fossil_crabdb_book_t).
 * @param value     Value to search for.
 * @return          A new database containing all matching entries.
 */
fossil_crabdb_book_t* fossil_crabsearch_by_value(fossil_crabdb_book_t *book, const char *value);

/**
 * @brief Finds all entries matching a predicate.
 *
 * @param book          Pointer to the database (fossil_crabdb_book_t).
 * @param predicate     Function pointer to the predicate.
 * @return              A new database containing all matching entries.
 */
fossil_crabdb_book_t* fossil_crabsearch_by_predicate(fossil_crabdb_book_t *book, bool (*predicate)(fossil_crabdb_entry_t *));

/**
 * @brief Finds the first entry that matches a predicate.
 *
 * @param book          Pointer to the database (fossil_crabdb_book_t).
 * @param predicate     Function pointer to the predicate.
 * @return              Pointer to the first matching entry, or NULL if none found.
 */
fossil_crabdb_entry_t* fossil_crabsearch_first_by_predicate(fossil_crabdb_book_t *book, bool (*predicate)(fossil_crabdb_entry_t *));

/**
 * @brief Checks if a key exists in the database.
 *
 * @param book  Pointer to the database (fossil_crabdb_book_t).
 * @param key   Key to search for.
 * @return      True if the key exists, false otherwise.
 */
bool fossil_crabsearch_key_exists(fossil_crabdb_book_t *book, const char *key);

/**
 * @brief Finds all primary key entries in the database.
 *
 * @param book  Pointer to the database (fossil_crabdb_book_t).
 * @return      A new database containing all primary key entries.
 */
fossil_crabdb_book_t* fossil_crabsearch_primary_keys(fossil_crabdb_book_t *book);

/**
 * @brief Counts the entries that match a predicate.
 *
 * @param book          Pointer to the database (fossil_crabdb_book_t).
 * @param predicate     Function pointer to the predicate.
 * @return              Number of matching entries.
 */
size_t fossil_crabsearch_count_by_predicate(fossil_crabdb_book_t *book, bool (*predicate)(fossil_crabdb_entry_t *));

// *****************************************************************************
// Search Utility Functions
// *****************************************************************************

/**
 * @brief Example predicate to find non-nullable entries.
 *
 * @param entry  Pointer to a database entry (fossil_crabdb_entry_t).
 * @return       True if the entry is non-nullable, false otherwise.
 */
bool fossil_crabsearch_is_non_nullable(fossil_crabdb_entry_t *entry);

/**
 * @brief Example predicate to find unique entries.
 *
 * @param entry  Pointer to a database entry (fossil_crabdb_entry_t).
 * @return       True if the entry is unique, false otherwise.
 */
bool fossil_crabsearch_is_unique(fossil_crabdb_entry_t *entry);

#ifdef __cplusplus
}
#endif

#endif // FOSSIL_CRABDB_QUERY_H

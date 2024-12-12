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
// Search operations
// *****************************************************************************

/**
 * Search for a key in the database.
 *
 * This function checks if a key exists in the database.
 * 
 * @param key The key to search for.
 * 
 * @return true if the key is found, false otherwise.
 */
bool fossil_crabsearch_key_exists(const char *key);

/**
 * Search for entries with a value that matches a given pattern.
 *
 * This function performs a search for values that match a specific pattern.
 * It can be used for partial string matching or matching a range of values.
 * 
 * @param pattern The pattern to search for in values.
 * @param count A pointer to store the number of entries matching the pattern.
 * 
 * @return An array of keys that match the pattern, or NULL if no matches are found.
 *         The caller is responsible for freeing this array.
 */
char **fossil_crabsearch_values_by_pattern(const char *pattern, size_t *count);

/**
 * Search for all keys in the database whose values match a given pattern.
 *
 * @param pattern The pattern to search for.
 * @param count A pointer to store the number of matching keys.
 * 
 * @return An array of keys that match the value pattern.
 */
char **fossil_crabsearch_keys_by_pattern(const char *pattern, size_t *count);

/**
 * Search for a value in the database by key.
 *
 * This function searches for a key in the database and returns the associated value.
 * If the key is not found, it returns NULL.
 *
 * @param key The key to search for.
 * @param value_size A pointer to store the size of the value.
 * 
 * @return A pointer to the value associated with the key, or NULL if the key is not found.
 */
void *fossil_crabsearch_value_by_key(const char *key, size_t *value_size);

/**
 * Search for a key in the database using a case-insensitive match.
 *
 * This function checks if a key exists in the database, ignoring case.
 *
 * @param key The key to search for (case-insensitive).
 * 
 * @return true if the key is found (case-insensitive), false otherwise.
 */
bool fossil_crabsearch_key_exists_case_insensitive(const char *key);

/**
 * Perform a search for multiple keys with a common prefix.
 *
 * This function checks if any keys in the database start with the given prefix.
 * 
 * @param prefix The prefix to search for.
 * @param count A pointer to store the number of matching keys.
 * 
 * @return An array of keys starting with the specified prefix.
 *         The caller is responsible for freeing this array.
 */
char **fossil_crabsearch_keys_by_prefix(const char *prefix, size_t *count);

#ifdef __cplusplus
}
#endif

#endif // FOSSIL_CRABDB_QUERY_H

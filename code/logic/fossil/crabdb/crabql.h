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
#ifndef FOSSIL_CRABDB_QUERY_H
#define FOSSIL_CRABDB_QUERY_H

#include "crabdb.h"

#ifdef __cplusplus
extern "C" {
#endif

/* Maximum sizes for keys and values */
#define FOSSIL_CRABQL_VAL_SIZE 256
#define FOSSIL_CRABQL_KEY_SIZE 1024

/* Structure for search results */
typedef struct fossil_crabql_result {
    char key[FOSSIL_CRABQL_KEY_SIZE];
    char value[FOSSIL_CRABQL_VAL_SIZE];
} fossil_crabql_result_t;

/* Structure for search result collection */
typedef struct fossil_crabql_result_collection {
    fossil_crabql_result_t* results;   // Array of search results
    size_t count;                      // Number of results found
    size_t capacity;                   // Capacity of the results array
} fossil_crabql_result_collection_t;

/* Search Operations */

/**
 * @brief Initializes a search result collection.
 * @param capacity Initial capacity for the result collection.
 * @return A pointer to the initialized result collection.
 */
fossil_crabql_result_collection_t* fossil_crabql_init_result_collection(size_t capacity);

/**
 * @brief Destroys the search result collection and frees associated memory.
 * @param collection A pointer to the result collection to be destroyed.
 */
void fossil_crabql_destroy_result_collection(fossil_crabql_result_collection_t* collection);

/**
 * @brief Searches for a key-value pair by exact key match.
 * @param db A pointer to the CrabDB instance.
 * @param key The key to search for.
 * @param result A pointer to a result structure to store the found key-value pair.
 * @return true if the key was found, false otherwise.
 */
bool fossil_crabql_search_by_key(fossil_crabdb_t* db, const char* key, fossil_crabql_result_t* result);

/**
 * @brief Searches for key-value pairs by value match.
 * @param db A pointer to the CrabDB instance.
 * @param value The value to search for.
 * @param collection A pointer to the result collection to store matching pairs.
 * @return true if any matching pairs were found, false otherwise.
 */
bool fossil_crabql_search_by_value(fossil_crabdb_t* db, const char* value, fossil_crabql_result_collection_t* collection);

/**
 * @brief Searches for key-value pairs matching a wildcard pattern.
 * @param db A pointer to the CrabDB instance.
 * @param pattern The wildcard pattern to match keys.
 * @param collection A pointer to the result collection to store matching pairs.
 * @return true if matching pairs were found, false otherwise.
 */
bool fossil_crabql_search_by_pattern(fossil_crabdb_t* db, const char* pattern, fossil_crabql_result_collection_t* collection);

/**
 * @brief Searches for key-value pairs that start with a specific prefix.
 * @param db A pointer to the CrabDB instance.
 * @param prefix The prefix to match keys.
 * @param collection A pointer to the result collection to store matching pairs.
 * @return true if matching pairs were found, false otherwise.
 */
bool fossil_crabql_search_by_prefix(fossil_crabdb_t* db, const char* prefix, fossil_crabql_result_collection_t* collection);

/**
 * @brief Searches for key-value pairs that fall within a specified range.
 * @param db A pointer to the CrabDB instance.
 * @param start_key The starting key for the range.
 * @param end_key The ending key for the range.
 * @param collection A pointer to the result collection to store matching pairs.
 * @return true if any matching pairs were found, false otherwise.
 */
bool fossil_crabql_search_by_range(fossil_crabdb_t* db, const char* start_key, const char* end_key, fossil_crabql_result_collection_t* collection);

/* Utility Functions */

/**
 * @brief Resizes the result collection to accommodate more results.
 * @param collection A pointer to the result collection.
 * @param new_capacity The new capacity for the collection.
 * @return true if resizing was successful, false otherwise.
 */
bool fossil_crabql_resize_result_collection(fossil_crabql_result_collection_t* collection, size_t new_capacity);

#ifdef __cplusplus
}
#endif

#endif // FOSSIL_CRABDB_QUERY_H

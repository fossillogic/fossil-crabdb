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

// Enumeration for search result formats
typedef enum {
    FORMAT_PLAIN_TEXT,
    FORMAT_JSON,
    FORMAT_CSV
} result_format_t;

// Enumeration for status codes
typedef enum {
    CRABSEARCH_SUCCESS,
    CRABSEARCH_INVALID_PARAM,
    CRABSEARCH_BUFFER_OVERFLOW,
    CRABSEARCH_NO_MATCHES,
} crabsearch_status_t;

/**
 * @brief Searches for key-value pairs in the CrabDB database that match a wildcard pattern.
 * 
 * @param db A pointer to the database to search in.
 * @param pattern The wildcard pattern to match keys.
 * @param result_buffer The buffer to store the matching key-value pairs.
 * @param buffer_size The size of the result buffer.
 * @param match_count Pointer to store the number of matches found.
 * @param format The format for the result output.
 * @return crabsearch_status_t Status code indicating success or error.
 */
crabsearch_status_t fossil_crabsearch_search(fossil_crabdb_t* db, const char* pattern, char* result_buffer, size_t buffer_size, size_t* match_count, result_format_t format);

/**
 * @brief Searches for key-value pairs in the CrabDB database that match multiple wildcard patterns.
 * 
 * @param db A pointer to the database to search in.
 * @param patterns An array of wildcard patterns to match keys.
 * @param num_patterns The number of patterns in the array.
 * @param result_buffer The buffer to store the matching key-value pairs.
 * @param buffer_size The size of the result buffer.
 * @param match_count Pointer to store the total number of matches found.
 * @return crabsearch_status_t Status code indicating success or error.
 */
crabsearch_status_t fossil_crabsearch_search_multiple(fossil_crabdb_t* db, const char** patterns, size_t num_patterns, char* result_buffer, size_t buffer_size, size_t* match_count);

#ifdef __cplusplus
}
#endif

#endif // FOSSIL_CRABDB_QUERY_H

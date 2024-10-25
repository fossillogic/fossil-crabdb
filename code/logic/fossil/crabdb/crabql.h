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

/**
 * @brief Finds nodes by their value type within the db, with sanitization for input validation.
 * @param db A pointer to the db instance. Must not be NULL.
 * @param type The type of value to search for; must be a valid fossil_crabdb_type_t.
 * @param result_buffer A buffer to store the results; must not be NULL.
 * @param buffer_size The size of the result buffer; must be large enough to hold expected results.
 * @return Error code (0 = success, -1 = null pointer, 1 = invalid type, 2 = buffer overflow, 3 = no match found).
 */
int fossil_crabdb_query_by_type(fossil_crabdb_t* db, fossil_crabdb_type_t type, char* result_buffer, size_t buffer_size);

/**
 * @brief Searches for nodes based on a range of values, sanitized to validate numeric and string ranges.
 * @param db A pointer to the db instance. Must not be NULL.
 * @param key The key prefix to match; must not exceed the maximum key length.
 * @param type The type of values within the range (e.g., integer, float) to verify parsing and comparison.
 * @param min_value The minimum value for the range; must be parseable to the specified type.
 * @param max_value The maximum value for the range; must be parseable to the specified type and >= min_value.
 * @param result_buffer A buffer to store the results; must not be NULL.
 * @param buffer_size The size of the result buffer; must be large enough to hold expected results.
 * @return Error code (0 = success, -1 = null pointer, 1 = invalid type or parse error, 2 = buffer overflow, 3 = no match found).
 */
int fossil_crabdb_query_range(fossil_crabdb_t* db, const char* key, fossil_crabdb_type_t type, const char* min_value, const char* max_value, char* result_buffer, size_t buffer_size);

/**
 * @brief Allows full-text search on string-type nodes, with sanitization for input validation.
 * @param db A pointer to the db instance. Must not be NULL.
 * @param search_text The text to search for within string values; must not exceed max length or contain special chars.
 * @param case_sensitive If true, performs a case-sensitive search.
 * @param result_buffer A buffer to store the matching results; must not be NULL.
 * @param buffer_size The size of the result buffer; must be large enough to hold expected results.
 * @return Error code (0 = success, -1 = null pointer, 1 = invalid text format, 2 = buffer overflow, 3 = no match found).
 */
int fossil_crabdb_full_text_search(fossil_crabdb_t* db, const char* search_text, bool case_sensitive, char* result_buffer, size_t buffer_size);

/**
 * @brief Finds nodes based on time-based criteria, with sanitization to validate the time range.
 * @param db A pointer to the db instance. Must not be NULL.
 * @param time_criteria The time to compare against; must be within a reasonable range.
 * @param newer_than If true, finds nodes newer than time_criteria; if false, older.
 * @param result_buffer A buffer to store the matching results; must not be NULL.
 * @param buffer_size The size of the result buffer; must be large enough to hold expected results.
 * @return Error code (0 = success, -1 = null pointer, 1 = invalid time, 2 = buffer overflow, 3 = no match found).
 */
int fossil_crabdb_query_by_time(fossil_crabdb_t* db, time_t time_criteria, bool newer_than, char* result_buffer, size_t buffer_size);

#ifdef __cplusplus
}
#endif

#endif // FOSSIL_CRABDB_QUERY_H

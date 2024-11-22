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
#include <fossil/test/framework.h>

#include "fossil/crabdb/framework.h"

FOSSIL_TEST_SUITE(cpp_search_fixture);
fossil_crabdb_t *cpp_search_mock;

FOSSIL_SETUP(cpp_search_fixture) {
    cpp_search_mock = fossil_crabdb_create();
    
    // Populate the mock database with test data
    fossil_crabdb_insert(cpp_search_mock, "apple", "A fruit", FOSSIL_CRABDB_TYPE_STRING);
    fossil_crabdb_insert(cpp_search_mock, "banana", "A yellow fruit", FOSSIL_CRABDB_TYPE_STRING);
    fossil_crabdb_insert(cpp_search_mock, "apricot", "Another fruit", FOSSIL_CRABDB_TYPE_STRING);
}

FOSSIL_TEARDOWN(cpp_search_fixture) {
    fossil_crabdb_destroy(cpp_search_mock);
}

// * * * * * * * * * * * * * * * * * * * * * * * *
// * Fossil Logic Test CrabSearch
// * * * * * * * * * * * * * * * * * * * * * * * *

// Test successful search with a valid pattern
FOSSIL_TEST_CASE(cpp_test_crabsearch_search_valid_pattern) {
    char result_buffer[1024];
    size_t match_count = 0;
    crabsearch_status_t status = fossil_crabsearch_search(cpp_search_mock, "a*", result_buffer, sizeof(result_buffer), &match_count, FORMAT_PLAIN_TEXT);
    ASSUME_ITS_TRUE(status == CRABSEARCH_SUCCESS);
    ASSUME_ITS_EQUAL_I32(match_count, 2); // "apple" and "apricot" should match
}

// Test successful search with no matches
FOSSIL_TEST_CASE(cpp_test_crabsearch_search_no_matches) {
    char result_buffer[1024];
    size_t match_count = 0;
    crabsearch_status_t status = fossil_crabsearch_search(cpp_search_mock, "orange*", result_buffer, sizeof(result_buffer), &match_count, FORMAT_PLAIN_TEXT);
    ASSUME_ITS_TRUE(status == CRABSEARCH_NO_MATCHES);
    ASSUME_ITS_EQUAL_I32(match_count, 0);
}

// Test buffer overflow handling
FOSSIL_TEST_CASE(cpp_test_crabsearch_search_buffer_overflow) {
    char small_buffer[10]; // Intentionally small buffer
    size_t match_count = 0;
    crabsearch_status_t status = fossil_crabsearch_search(cpp_search_mock, "a*", small_buffer, sizeof(small_buffer), &match_count, FORMAT_PLAIN_TEXT);
    ASSUME_ITS_TRUE(status == CRABSEARCH_BUFFER_OVERFLOW);
}

// Test invalid database pointer
FOSSIL_TEST_CASE(cpp_test_crabsearch_search_invalid_db) {
    char result_buffer[1024];
    size_t match_count = 0;
    crabsearch_status_t status = fossil_crabsearch_search(NULL, "a*", result_buffer, sizeof(result_buffer), &match_count, FORMAT_PLAIN_TEXT);
    ASSUME_ITS_TRUE(status == CRABSEARCH_INVALID_PARAM);
}

// Test NULL pattern handling
FOSSIL_TEST_CASE(cpp_test_crabsearch_search_null_pattern) {
    char result_buffer[1024];
    size_t match_count = 0;
    crabsearch_status_t status = fossil_crabsearch_search(cpp_search_mock, NULL, result_buffer, sizeof(result_buffer), &match_count, FORMAT_PLAIN_TEXT);
    ASSUME_ITS_TRUE(status == CRABSEARCH_INVALID_PARAM);
}

// Test successful search with multiple patterns
FOSSIL_TEST_CASE(cpp_test_crabsearch_search_multiple_patterns) {
    const char *patterns[] = {"a*", "b*"};
    char result_buffer[1024];
    size_t match_count = 0;
    crabsearch_status_t status = fossil_crabsearch_search_multiple(cpp_search_mock, patterns, 2, result_buffer, sizeof(result_buffer), &match_count);
    ASSUME_ITS_TRUE(status == CRABSEARCH_SUCCESS);
    ASSUME_ITS_EQUAL_I32(match_count, 3); // "apple", "banana", "apricot" should match
}

// Test multiple patterns with no matches
FOSSIL_TEST_CASE(cpp_test_crabsearch_search_multiple_no_matches) {
    const char *patterns[] = {"orange*", "grape*"};
    char result_buffer[1024];
    size_t match_count = 0;
    crabsearch_status_t status = fossil_crabsearch_search_multiple(cpp_search_mock, patterns, 2, result_buffer, sizeof(result_buffer), &match_count);
    ASSUME_ITS_TRUE(status == CRABSEARCH_NO_MATCHES);
    ASSUME_ITS_EQUAL_I32(match_count, 0);
}

// Test invalid parameters for multiple patterns
FOSSIL_TEST_CASE(cpp_test_crabsearch_search_multiple_invalid_params) {
    const char *patterns[] = {"a*"};
    char result_buffer[1024];
    size_t match_count = 0;

    crabsearch_status_t status = fossil_crabsearch_search_multiple(NULL, patterns, 1, result_buffer, sizeof(result_buffer), &match_count);
    ASSUME_ITS_TRUE(status == CRABSEARCH_INVALID_PARAM); // Invalid database pointer

    status = fossil_crabsearch_search_multiple(cpp_search_mock, NULL, 1, result_buffer, sizeof(result_buffer), &match_count);
    ASSUME_ITS_TRUE(status == CRABSEARCH_INVALID_PARAM); // NULL patterns

    status = fossil_crabsearch_search_multiple(cpp_search_mock, patterns, 0, result_buffer, sizeof(result_buffer), &match_count);
    ASSUME_ITS_TRUE(status == CRABSEARCH_INVALID_PARAM); // Zero patterns
}

// * * * * * * * * * * * * * * * * * * * * * * * *
// * Fossil Logic Test Pool
// * * * * * * * * * * * * * * * * * * * * * * * *

FOSSIL_TEST_CASE_GROUP(cpp_crab_search_tests) {
    FOSSIL_TEST_ADD(cpp_search_fixture, cpp_test_crabsearch_search_valid_pattern);
    FOSSIL_TEST_ADD(cpp_search_fixture, cpp_test_crabsearch_search_no_matches);
    FOSSIL_TEST_ADD(cpp_search_fixture, cpp_test_crabsearch_search_buffer_overflow);
    FOSSIL_TEST_ADD(cpp_search_fixture, cpp_test_crabsearch_search_invalid_db);
    FOSSIL_TEST_ADD(cpp_search_fixture, cpp_test_crabsearch_search_null_pattern);
    FOSSIL_TEST_ADD(cpp_search_fixture, cpp_test_crabsearch_search_multiple_patterns);
    FOSSIL_TEST_ADD(cpp_search_fixture, cpp_test_crabsearch_search_multiple_no_matches);
    FOSSIL_TEST_ADD(cpp_search_fixture, cpp_test_crabsearch_search_multiple_invalid_params);

    FOSSIL_TEST_REGISTER(cpp_search_fixture);
} // end of tests

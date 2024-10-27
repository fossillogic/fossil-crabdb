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
#include <fossil/unittest/framework.h> // Includes the Fossil Unit Test Framework
#include <fossil/mockup/framework.h>   // Includes the Fossil Mockup Framework
#include <fossil/unittest/assume.h>    // Includes the Fossil Assume Framework

#include "fossil/crabdb/framework.h"

FOSSIL_FIXTURE(search_fixture);
fossil_crabdb_t *search_mock_db;

FOSSIL_SETUP(search_fixture) {
    search_mock_db = fossil_crabdb_create();
    
    // Populate the mock database with test data
    fossil_crabdb_insert(search_mock_db, "apple", "A fruit");
    fossil_crabdb_insert(search_mock_db, "banana", "A yellow fruit");
    fossil_crabdb_insert(search_mock_db, "apricot", "Another fruit");
}

FOSSIL_TEARDOWN(search_fixture) {
    fossil_crabdb_destroy(search_mock_db);
}

// * * * * * * * * * * * * * * * * * * * * * * * *
// * Fossil Logic Test CrabSearch
// * * * * * * * * * * * * * * * * * * * * * * * *

// Test successful search with a valid pattern
FOSSIL_TEST(test_fossil_crabsearch_search_valid_pattern) {
    char result_buffer[1024];
    size_t match_count = 0;
    crabsearch_status_t status = fossil_crabsearch_search(search_mock_db, "a*", result_buffer, sizeof(result_buffer), &match_count);
    ASSUME_ITS_TRUE(status == CRABSEARCH_SUCCESS);
    ASSUME_ITS_EQUAL(match_count, 2); // "apple" and "apricot" should match
}

// Test successful search with no matches
FOSSIL_TEST(test_fossil_crabsearch_search_no_matches) {
    char result_buffer[1024];
    size_t match_count = 0;
    crabsearch_status_t status = fossil_crabsearch_search(search_mock_db, "orange*", result_buffer, sizeof(result_buffer), &match_count);
    ASSUME_ITS_TRUE(status == CRABSEARCH_NO_MATCHES);
    ASSUME_ITS_EQUAL(match_count, 0);
}

// Test buffer overflow handling
FOSSIL_TEST(test_fossil_crabsearch_search_buffer_overflow) {
    char small_buffer[10]; // Intentionally small buffer
    size_t match_count = 0;
    crabsearch_status_t status = fossil_crabsearch_search(search_mock_db, "a*", small_buffer, sizeof(small_buffer), &match_count);
    ASSUME_ITS_TRUE(status == CRABSEARCH_BUFFER_OVERFLOW);
}

// Test invalid database pointer
FOSSIL_TEST(test_fossil_crabsearch_search_invalid_db) {
    char result_buffer[1024];
    size_t match_count = 0;
    crabsearch_status_t status = fossil_crabsearch_search(NULL, "a*", result_buffer, sizeof(result_buffer), &match_count);
    ASSUME_ITS_TRUE(status == CRABSEARCH_INVALID_PARAM);
}

// Test NULL pattern handling
FOSSIL_TEST(test_fossil_crabsearch_search_null_pattern) {
    char result_buffer[1024];
    size_t match_count = 0;
    crabsearch_status_t status = fossil_crabsearch_search(search_mock_db, NULL, result_buffer, sizeof(result_buffer), &match_count);
    ASSUME_ITS_TRUE(status == CRABSEARCH_INVALID_PARAM);
}

// Test successful search with multiple patterns
FOSSIL_TEST(test_fossil_crabsearch_search_multiple_patterns) {
    const char *patterns[] = {"a*", "b*"};
    char result_buffer[1024];
    size_t match_count = 0;
    crabsearch_status_t status = fossil_crabsearch_search_multiple(search_mock_db, patterns, 2, result_buffer, sizeof(result_buffer), &match_count);
    ASSUME_ITS_TRUE(status == CRABSEARCH_SUCCESS);
    ASSUME_ITS_EQUAL(match_count, 3); // "apple", "banana", "apricot" should match
}

// Test multiple patterns with no matches
FOSSIL_TEST(test_fossil_crabsearch_search_multiple_no_matches) {
    const char *patterns[] = {"orange*", "grape*"};
    char result_buffer[1024];
    size_t match_count = 0;
    crabsearch_status_t status = fossil_crabsearch_search_multiple(search_mock_db, patterns, 2, result_buffer, sizeof(result_buffer), &match_count);
    ASSUME_ITS_TRUE(status == CRABSEARCH_NO_MATCHES);
    ASSUME_ITS_EQUAL(match_count, 0);
}

// Test invalid parameters for multiple patterns
FOSSIL_TEST(test_fossil_crabsearch_search_multiple_invalid_params) {
    const char *patterns[] = {"a*"};
    char result_buffer[1024];
    size_t match_count = 0;

    crabsearch_status_t status = fossil_crabsearch_search_multiple(NULL, patterns, 1, result_buffer, sizeof(result_buffer), &match_count);
    ASSUME_ITS_TRUE(status == CRABSEARCH_INVALID_PARAM); // Invalid database pointer

    status = fossil_crabsearch_search_multiple(search_mock_db, NULL, 1, result_buffer, sizeof(result_buffer), &match_count);
    ASSUME_ITS_TRUE(status == CRABSEARCH_INVALID_PARAM); // NULL patterns

    status = fossil_crabsearch_search_multiple(search_mock_db, patterns, 0, result_buffer, sizeof(result_buffer), &match_count);
    ASSUME_ITS_TRUE(status == CRABSEARCH_INVALID_PARAM); // Zero patterns
}

// * * * * * * * * * * * * * * * * * * * * * * * *
// * Fossil Logic Test Pool
// * * * * * * * * * * * * * * * * * * * * * * * *

FOSSIL_TEST_GROUP(c_crab_search_tests) {
    ADD_TESTF(test_fossil_crabsearch_search_valid_pattern, search_fixture);
    ADD_TESTF(test_fossil_crabsearch_search_no_matches, search_fixture);
    ADD_TESTF(test_fossil_crabsearch_search_buffer_overflow, search_fixture);
    ADD_TESTF(test_fossil_crabsearch_search_invalid_db, search_fixture);
    ADD_TESTF(test_fossil_crabsearch_search_null_pattern, search_fixture);
    ADD_TESTF(test_fossil_crabsearch_search_multiple_patterns, search_fixture);
    ADD_TESTF(test_fossil_crabsearch_search_multiple_no_matches, search_fixture);
    ADD_TESTF(test_fossil_crabsearch_search_multiple_invalid_params, search_fixture);
} // end of tests

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
#include <fossil/unittest/assume.h>            // Includes the Fossil Assume Framework

#include "fossil/crabdb/framework.h"


// * * * * * * * * * * * * * * * * * * * * * * * *
// * Fossil Logic Test Blue CrabDB Database
// * * * * * * * * * * * * * * * * * * * * * * * *

FOSSIL_TEST(test_query_by_type_null_db) {
    char result_buffer[256];
    int result = fossil_crabdb_query_by_type(NULL, FOSSIL_CRABDB_TYPE_STRING, result_buffer, sizeof(result_buffer));
    ASSUME_ITS_EQUAL_I32(-1, result); // Expect -1 for null database
}

FOSSIL_TEST(test_query_by_type_invalid_type) {
    fossil_crabdb_t db;
    char result_buffer[256];
    int result = fossil_crabdb_query_by_type(&db, FOSSIL_CRABDB_TYPE_MAX + 1, result_buffer, sizeof(result_buffer));
    ASSUME_ITS_EQUAL_I32(1, result); // Expect 1 for invalid type
}

FOSSIL_TEST(test_query_by_type_buffer_overflow) {
    fossil_crabdb_t db;
    // Setup database with nodes that could cause buffer overflow
    char result_buffer[10]; // Intentionally small buffer for testing
    int result = fossil_crabdb_query_by_type(&db, FOSSIL_CRABDB_TYPE_STRING, result_buffer, sizeof(result_buffer));
    ASSUME_ITS_EQUAL_I32(2, result); // Expect 2 for buffer overflow
}

FOSSIL_TEST(test_query_by_type_no_match_found) {
    fossil_crabdb_t db;
    char result_buffer[256];
    int result = fossil_crabdb_query_by_type(&db, FOSSIL_CRABDB_TYPE_STRING, result_buffer, sizeof(result_buffer));
    ASSUME_ITS_EQUAL_I32(3, result); // Expect 3 when no matches are found
}

FOSSIL_TEST(test_query_by_type_success) {
    fossil_crabdb_t db;
    // Populate the database with matching nodes
    char result_buffer[256];
    int result = fossil_crabdb_query_by_type(&db, FOSSIL_CRABDB_TYPE_STRING, result_buffer, sizeof(result_buffer));
    ASSUME_ITS_EQUAL_I32(0, result); // Expect 0 for successful query
    // Verify the contents of result_buffer here (e.g., assert specific expected values)
}

FOSSIL_TEST(test_query_range_null_db) {
    char result_buffer[256];
    int result = fossil_crabdb_query_range(NULL, "key", FOSSIL_CRABDB_TYPE_STRING, "0", "100", result_buffer, sizeof(result_buffer));
    ASSUME_ITS_EQUAL_I32(-1, result); // Expect -1 for null database
}

FOSSIL_TEST(test_query_range_invalid_type) {
    fossil_crabdb_t db;
    char result_buffer[256];
    int result = fossil_crabdb_query_range(&db, "key", FOSSIL_CRABDB_TYPE_MAX + 1, "0", "100", result_buffer, sizeof(result_buffer));
    ASSUME_ITS_EQUAL_I32(1, result); // Expect 1 for invalid type
}

FOSSIL_TEST(test_query_range_buffer_overflow) {
    fossil_crabdb_t db;
    // Setup database with nodes that could cause buffer overflow
    char result_buffer[10]; // Intentionally small buffer
    int result = fossil_crabdb_query_range(&db, "key", FOSSIL_CRABDB_TYPE_STRING, "0", "100", result_buffer, sizeof(result_buffer));
    ASSUME_ITS_EQUAL_I32(2, result); // Expect 2 for buffer overflow
}

FOSSIL_TEST(test_query_range_no_match_found) {
    fossil_crabdb_t db;
    char result_buffer[256];
    int result = fossil_crabdb_query_range(&db, "key", FOSSIL_CRABDB_TYPE_STRING, "0", "100", result_buffer, sizeof(result_buffer));
    ASSUME_ITS_EQUAL_I32(3, result); // Expect 3 when no matches are found
}

FOSSIL_TEST(test_query_range_success) {
    fossil_crabdb_t db;
    // Populate the database with matching nodes
    char result_buffer[256];
    int result = fossil_crabdb_query_range(&db, "key", FOSSIL_CRABDB_TYPE_STRING, "0", "100", result_buffer, sizeof(result_buffer));
    ASSUME_ITS_EQUAL_I32(0, result); // Expect 0 for successful query
    // Verify the contents of result_buffer here (e.g., assert specific expected values)
}

FOSSIL_TEST(test_full_text_search_null_db) {
    char result_buffer[256];
    int result = fossil_crabdb_full_text_search(NULL, "search", false, result_buffer, sizeof(result_buffer));
    ASSUME_ITS_EQUAL_I32(-1, result); // Expect -1 for null database
}

FOSSIL_TEST(test_full_text_search_buffer_overflow) {
    fossil_crabdb_t db;
    // Setup database with nodes that could cause buffer overflow
    char result_buffer[10]; // Intentionally small buffer
    int result = fossil_crabdb_full_text_search(&db, "search", false, result_buffer, sizeof(result_buffer));
    ASSUME_ITS_EQUAL_I32(2, result); // Expect 2 for buffer overflow
}

FOSSIL_TEST(test_full_text_search_no_match_found) {
    fossil_crabdb_t db;
    char result_buffer[256];
    int result = fossil_crabdb_full_text_search(&db, "search", false, result_buffer, sizeof(result_buffer));
    ASSUME_ITS_EQUAL_I32(3, result); // Expect 3 when no matches are found
}

FOSSIL_TEST(test_full_text_search_success) {
    fossil_crabdb_t db;
    // Populate the database with nodes containing the search text
    char result_buffer[256];
    int result = fossil_crabdb_full_text_search(&db, "search", false, result_buffer, sizeof(result_buffer));
    ASSUME_ITS_EQUAL_I32(0, result); // Expect 0 for successful search
    // Verify the contents of result_buffer here (e.g., assert specific expected values)
}

FOSSIL_TEST(test_query_by_time_null_db) {
    char result_buffer[256];
    time_t time_criteria = time(NULL);
    int result = fossil_crabdb_query_by_time(NULL, time_criteria, true, result_buffer, sizeof(result_buffer));
    ASSUME_ITS_EQUAL_I32(-1, result); // Expect -1 for null database
}

FOSSIL_TEST(test_query_by_time_buffer_overflow) {
    fossil_crabdb_t db;
    // Setup database with nodes that could cause buffer overflow
    char result_buffer[10]; // Intentionally small buffer
    time_t time_criteria = time(NULL);
    int result = fossil_crabdb_query_by_time(&db, time_criteria, true, result_buffer, sizeof(result_buffer));
    ASSUME_ITS_EQUAL_I32(2, result); // Expect 2 for buffer overflow
}

FOSSIL_TEST(test_query_by_time_no_match_found) {
    fossil_crabdb_t db;
    char result_buffer[256];
    time_t time_criteria = time(NULL);
    int result = fossil_crabdb_query_by_time(&db, time_criteria, true, result_buffer, sizeof(result_buffer));
    ASSUME_ITS_EQUAL_I32(3, result); // Expect 3 when no matches are found
}

FOSSIL_TEST(test_query_by_time_success) {
    fossil_crabdb_t db;
    // Populate the database with nodes that should match the time criteria
    char result_buffer[256];
    time_t time_criteria = time(NULL);
    int result = fossil_crabdb_query_by_time(&db, time_criteria, true, result_buffer, sizeof(result_buffer));
    ASSUME_ITS_EQUAL_I32(0, result); // Expect 0 for successful query
    // Verify the contents of result_buffer here (e.g., assert specific expected values)
}

// * * * * * * * * * * * * * * * * * * * * * * * *
// * Fossil Logic Test Pool
// * * * * * * * * * * * * * * * * * * * * * * * *

FOSSIL_TEST_GROUP(c_crab_qsearch_tests) {    
    ADD_TEST(test_query_by_type_null_db);
    ADD_TEST(test_query_by_type_invalid_type);
    ADD_TEST(test_query_by_type_buffer_overflow);
    ADD_TEST(test_query_by_type_no_match_found);
    ADD_TEST(test_query_by_type_success);
    ADD_TEST(test_query_range_null_db);
    ADD_TEST(test_query_range_invalid_type);
    ADD_TEST(test_query_range_buffer_overflow);
    ADD_TEST(test_query_range_no_match_found);
    ADD_TEST(test_query_range_success);
    ADD_TEST(test_full_text_search_null_db);
    ADD_TEST(test_full_text_search_buffer_overflow);
    ADD_TEST(test_full_text_search_no_match_found);
    ADD_TEST(test_full_text_search_success);
    ADD_TEST(test_query_by_time_null_db);
    ADD_TEST(test_query_by_time_buffer_overflow);
    ADD_TEST(test_query_by_time_no_match_found);
    ADD_TEST(test_query_by_time_success);
} // end of tests

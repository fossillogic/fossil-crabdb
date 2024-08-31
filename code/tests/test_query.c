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
#include <fossil/unittest/framework.h>
#include <fossil/mockup/framework.h>
#include <fossil/xassume.h>
#include "fossil/crabdb/framework.h"

FOSSIL_FIXTURE(crabdb_query_fixture);
fossil_crabdb_deque_t *query_test_db;

FOSSIL_SETUP(crabdb_query_fixture) {
    query_test_db = fossil_crabdb_create();
}

FOSSIL_TEARDOWN(crabdb_query_fixture) {
    fossil_crabdb_destroy(query_test_db);
}

// * * * * * * * * * * * * * * * * * * * * * * * *
// * Fossil Logic Test Blue CrabDB Database
// * * * * * * * * * * * * * * * * * * * * * * * *

// Test case 1: Valid .crabql file with multiple commands
FOSSIL_TEST(test_fossil_crabdb_exec_valid_file) {
    bool result = fossil_crabdb_exec("test_valid.crabql", &query_test_db);
    ASSUME_ITS_TRUE(result);
    char value[1024];
    ASSUME_ITS_TRUE(fossil_crabdb_select(&query_test_db, "key1", value, sizeof(value)));
    ASSUME_ITS_EQUAL_CSTR("value1", value);
}

// Test case 2: File with a condition that is met
FOSSIL_TEST(test_fossil_crabdb_exec_condition_met) {
    bool result = fossil_crabdb_exec("test_condition_met.crabql", &query_test_db);
    ASSUME_ITS_TRUE(result);
    char value[1024];
    ASSUME_ITS_TRUE(fossil_crabdb_select(&query_test_db, "key2", value, sizeof(value)));
    ASSUME_ITS_EQUAL_CSTR("value2", value);
}

// Test case 3: File with a condition that is not met
FOSSIL_TEST(test_fossil_crabdb_exec_condition_not_met) {
    bool result = fossil_crabdb_exec("test_condition_not_met.crabql", &query_test_db);
    ASSUME_ITS_TRUE(result);
    char value[1024];
    ASSUME_NOT_TRUE(fossil_crabdb_select(&query_test_db, "key3", value, sizeof(value)));
}

// Test case 4: File with an unknown command
FOSSIL_TEST(test_fossil_crabdb_exec_unknown_command) {
    bool result = fossil_crabdb_exec("test_unknown_command.crabql", &query_test_db);
    ASSUME_ITS_TRUE(result);
    char value[1024];
    ASSUME_ITS_FALSE(fossil_crabdb_select(&query_test_db, "key4", value, sizeof(value)));
}

// Test case 5: Invalid file extension
FOSSIL_TEST(test_fossil_crabdb_exec_invalid_extension) {
    bool result = fossil_crabdb_exec("test_invalid.txt", &query_test_db);
    ASSUME_NOT_TRUE(result);
}


// * * * * * * * * * * * * * * * * * * * * * * * *
// * Fossil Logic Test Pool
// * * * * * * * * * * * * * * * * * * * * * * * *

FOSSIL_TEST_GROUP(c_crabql_query_tests) {
    ADD_TESTF(test_fossil_crabdb_exec_valid_file, crabdb_query_fixture);
    ADD_TESTF(test_fossil_crabdb_exec_condition_met, crabdb_query_fixture);
    ADD_TESTF(test_fossil_crabdb_exec_condition_not_met, crabdb_query_fixture);
    ADD_TESTF(test_fossil_crabdb_exec_unknown_command, crabdb_query_fixture);
    ADD_TESTF(test_fossil_crabdb_exec_invalid_extension, crabdb_query_fixture);
}

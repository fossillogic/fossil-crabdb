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

FOSSIL_FIXTURE(crabdb_script_fixture);
fossil_crabdb_deque_t *script_test_db;

FOSSIL_SETUP(crabdb_script_fixture) {
    script_test_db = fossil_crabdb_create();
}

FOSSIL_TEARDOWN(crabdb_script_fixture) {
    fossil_crabdb_destroy(script_test_db);
}

// * * * * * * * * * * * * * * * * * * * * * * * *
// * Fossil Logic Test Blue CrabDB Database
// * * * * * * * * * * * * * * * * * * * * * * * *

FOSSIL_TEST(test_insert_commands) {
    bool result = fossil_crabdb_script("insert_commands.crabql", script_test_db);
    ASSUME_ITS_TRUE(result);

    char value[1024];
    ASSUME_ITS_TRUE(fossil_crabdb_select(script_test_db, "myKey1", value, sizeof(value)));
    ASSUME_ITS_EQUAL_CSTR("myValue1", value);

    ASSUME_ITS_TRUE(fossil_crabdb_select(script_test_db, "myKey2", value, sizeof(value)));
    ASSUME_ITS_EQUAL_CSTR("myValue2", value);
}

FOSSIL_TEST(test_update_command) {
    bool result = fossil_crabdb_script("update_command.crabql", script_test_db);
    ASSUME_ITS_TRUE(result);

    char value[1024];
    ASSUME_ITS_TRUE(fossil_crabdb_select(script_test_db, "myKey1", value, sizeof(value)));
    ASSUME_ITS_EQUAL_CSTR("updatedValue1", value);
}

FOSSIL_TEST(test_select_command) {
    bool result = fossil_crabdb_script("select_command.crabql", script_test_db);
    ASSUME_ITS_TRUE(result);

    char value[1024];
    ASSUME_ITS_TRUE(fossil_crabdb_select(script_test_db, "myKey2", value, sizeof(value)));
    ASSUME_ITS_EQUAL_CSTR("myValue2", value);
}

FOSSIL_TEST(test_delete_command) {
    bool result = fossil_crabdb_script("delete_command.crabql", script_test_db);
    ASSUME_ITS_TRUE(result);

    char value[1024];
    ASSUME_ITS_FALSE(fossil_crabdb_select(script_test_db, "myKey1", value, sizeof(value)));
}

FOSSIL_TEST(test_sort_key_command) {
    bool result = fossil_crabdb_script("sort_key_command.crabql", script_test_db);
    ASSUME_ITS_TRUE(result);

    char value[1024];
    ASSUME_ITS_TRUE(fossil_crabdb_search_by_key(script_test_db, "myKey2", value, sizeof(value)));
    ASSUME_ITS_EQUAL_CSTR("myValue2", value);
}

// * * * * * * * * * * * * * * * * * * * * * * * *
// * Fossil Logic Test Pool
// * * * * * * * * * * * * * * * * * * * * * * * *

FOSSIL_TEST_GROUP(c_crabql_script_tests) {
    (void)test_env; // Silence the unused variable warning, must resume this later for 0.1.3
    // ADD_TESTF(test_insert_commands, crabdb_script_fixture);
    // ADD_TESTF(test_update_command, crabdb_script_fixture);
    // ADD_TESTF(test_select_command, crabdb_script_fixture);
    // ADD_TESTF(test_delete_command, crabdb_script_fixture);
    // ADD_TESTF(test_sort_key_command, crabdb_script_fixture);
}

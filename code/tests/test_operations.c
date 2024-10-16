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

FOSSIL_FIXTURE(crabdb_operation_fixture);
fossil_crabdb_deque_t *operation_test_db;

FOSSIL_SETUP(crabdb_operation_fixture) {
    operation_test_db = fossil_crabdb_create();
}

FOSSIL_TEARDOWN(crabdb_operation_fixture) {
    fossil_crabdb_destroy(operation_test_db);
}

// * * * * * * * * * * * * * * * * * * * * * * * *
// * Fossil Logic Test Blue CrabDB Database
// * * * * * * * * * * * * * * * * * * * * * * * *

FOSSIL_TEST(test_insert) {
    bool result = fossil_crabdb_insert(operation_test_db, "key1", "value1", FOSSIL_CRABDB_TYPE_STRING);
    ASSUME_ITS_TRUE(result);
    
    char value[1024];
    result = fossil_crabdb_select(operation_test_db, "key1", value, sizeof(value));
    ASSUME_ITS_TRUE(result);
    ASSUME_ITS_EQUAL_CSTR("value1", value);
}

FOSSIL_TEST(test_update) {
    fossil_crabdb_insert(operation_test_db, "key1", "value1", FOSSIL_CRABDB_TYPE_STRING);
    bool result = fossil_crabdb_update(operation_test_db, "key1", "updatedValue1");
    ASSUME_ITS_TRUE(result);
    
    char value[1024];
    result = fossil_crabdb_select(operation_test_db, "key1", value, sizeof(value));
    ASSUME_ITS_TRUE(result);
    ASSUME_ITS_EQUAL_CSTR("updatedValue1", value);
}

FOSSIL_TEST(test_delete) {
    fossil_crabdb_insert(operation_test_db, "key1", "value1", FOSSIL_CRABDB_TYPE_STRING);
    bool result = fossil_crabdb_delete(operation_test_db, "key1");
    ASSUME_ITS_TRUE(result);
    
    char value[1024];
    result = fossil_crabdb_select(operation_test_db, "key1", value, sizeof(value));
    ASSUME_NOT_TRUE(result); // Should return false as the key is deleted
}

FOSSIL_TEST(test_list) {
    fossil_crabdb_insert(operation_test_db, "key1", "value1", FOSSIL_CRABDB_TYPE_STRING);
    fossil_crabdb_insert(operation_test_db, "key2", "value2", FOSSIL_CRABDB_TYPE_STRING);

    char list_buffer[2048];
    bool result = fossil_crabdb_list(operation_test_db, list_buffer, sizeof(list_buffer));
    ASSUME_ITS_TRUE(result);

    // Check if the list contains the expected entries
    ASSUME_ITS_TRUE(strstr(list_buffer, "key1: value1") != NULL);
    ASSUME_ITS_TRUE(strstr(list_buffer, "key2: value2") != NULL);
}

FOSSIL_TEST(test_clear) {
    fossil_crabdb_insert(operation_test_db, "key1", "value1", FOSSIL_CRABDB_TYPE_STRING);
    fossil_crabdb_insert(operation_test_db, "key2", "value2", FOSSIL_CRABDB_TYPE_STRING);

    bool result = fossil_crabdb_clear(operation_test_db);
    ASSUME_ITS_TRUE(result);

    char value[1024];
    result = fossil_crabdb_select(operation_test_db, "key1", value, sizeof(value));
    ASSUME_NOT_TRUE(result); // Should return false as the deque is cleared

    result = fossil_crabdb_select(operation_test_db, "key2", value, sizeof(value));
    ASSUME_NOT_TRUE(result); // Should return false as the deque is cleared
}

FOSSIL_TEST(test_show) {
    fossil_crabdb_insert(operation_test_db, "key1", "value1", FOSSIL_CRABDB_TYPE_STRING);

    bool result = fossil_crabdb_show(operation_test_db);
    ASSUME_ITS_TRUE(result);
}

FOSSIL_TEST(test_drop) {
    fossil_crabdb_insert(operation_test_db, "key1", "value1", FOSSIL_CRABDB_TYPE_STRING);
    
    bool result = fossil_crabdb_drop(operation_test_db);
    ASSUME_ITS_TRUE(result);

    char value[1024];
    result = fossil_crabdb_select(operation_test_db, "key1", value, sizeof(value));
    ASSUME_NOT_TRUE(result); // Should return false as the deque is dropped
}

FOSSIL_TEST(test_exist) {
    fossil_crabdb_insert(operation_test_db, "key1", "value1", FOSSIL_CRABDB_TYPE_STRING);

    bool result = fossil_crabdb_exist(operation_test_db, "key1");
    ASSUME_ITS_TRUE(result);

    result = fossil_crabdb_exist(operation_test_db, "nonExistingKey");
    ASSUME_NOT_TRUE(result);
}

// * * * * * * * * * * * * * * * * * * * * * * * *
// * Fossil Logic Test Pool
// * * * * * * * * * * * * * * * * * * * * * * * *

FOSSIL_TEST_GROUP(c_crabdb_operations_tests) {    
    ADD_TESTF(test_insert, crabdb_operation_fixture);
    ADD_TESTF(test_update, crabdb_operation_fixture);
    ADD_TESTF(test_delete, crabdb_operation_fixture);
    ADD_TESTF(test_list, crabdb_operation_fixture);
    ADD_TESTF(test_clear, crabdb_operation_fixture);
    ADD_TESTF(test_show, crabdb_operation_fixture);
    ADD_TESTF(test_drop, crabdb_operation_fixture);
    ADD_TESTF(test_exist, crabdb_operation_fixture);
} // end of tests

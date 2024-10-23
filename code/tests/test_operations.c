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
fossil_crabdb_t *operation_test_db;

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
    ASSUME_NOT_TRUE(result); // Should return false as the db is cleared

    result = fossil_crabdb_select(operation_test_db, "key2", value, sizeof(value));
    ASSUME_NOT_TRUE(result); // Should return false as the db is cleared
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
    ASSUME_NOT_TRUE(result); // Should return false as the db is dropped
}

FOSSIL_TEST(test_exist) {
    fossil_crabdb_insert(operation_test_db, "key1", "value1", FOSSIL_CRABDB_TYPE_STRING);

    bool result = fossil_crabdb_exist(operation_test_db, "key1");
    ASSUME_ITS_TRUE(result);

    result = fossil_crabdb_exist(operation_test_db, "nonExistingKey");
    ASSUME_NOT_TRUE(result);
}

FOSSIL_TEST(test_rename_key) {
    // Insert initial key-value pair
    fossil_crabdb_insert(operation_test_db, "old_key", "value1", FOSSIL_CRABDB_TYPE_STRING);

    // Rename the key from "old_key" to "new_key"
    bool result = fossil_crabdb_rename_key(operation_test_db, "old_key", "new_key");
    ASSUME_ITS_TRUE(result);

    // Verify that the old key no longer exists
    result = fossil_crabdb_exist(operation_test_db, "old_key");
    ASSUME_NOT_TRUE(result);

    // Verify that the new key exists and the value is still intact
    result = fossil_crabdb_exist(operation_test_db, "new_key");
    ASSUME_ITS_TRUE(result);

    char value[1024];
    result = fossil_crabdb_select(operation_test_db, "new_key", value, sizeof(value));
    ASSUME_ITS_TRUE(result);
    ASSUME_ITS_EQUAL_CSTR("value1", value);
}

FOSSIL_TEST(test_check_duplicates) {
    // Insert duplicate values
    fossil_crabdb_insert(operation_test_db, "key1", "duplicateValue", FOSSIL_CRABDB_TYPE_STRING);
    fossil_crabdb_insert(operation_test_db, "key2", "duplicateValue", FOSSIL_CRABDB_TYPE_STRING);

    // Check for duplicates
    bool result = fossil_crabdb_check_duplicates(operation_test_db);
    ASSUME_ITS_TRUE(result);

    // Insert non-duplicate values
    fossil_crabdb_insert(operation_test_db, "key3", "uniqueValue", FOSSIL_CRABDB_TYPE_STRING);

    // Check again for duplicates
    result = fossil_crabdb_check_duplicates(operation_test_db);
    ASSUME_ITS_TRUE(result); // Still true since "duplicateValue" is a duplicate

    // Clear duplicates
    fossil_crabdb_delete(operation_test_db, "key1");
    fossil_crabdb_delete(operation_test_db, "key2");

    // Ensure no duplicates remain
    result = fossil_crabdb_check_duplicates(operation_test_db);
    ASSUME_NOT_TRUE(result); // No more duplicates should be present
}

FOSSIL_TEST(test_merge) {
    // Create two databases for merging
    fossil_crabdb_t* source_db = fossil_crabdb_create();
    fossil_crabdb_insert(operation_test_db, "key1", "value1", FOSSIL_CRABDB_TYPE_STRING);
    fossil_crabdb_insert(source_db, "key2", "value2", FOSSIL_CRABDB_TYPE_STRING);

    // Merge source_db into operation_test_db
    bool result = fossil_crabdb_merge(operation_test_db, source_db);
    ASSUME_ITS_TRUE(result);

    // Verify both key1 and key2 exist in operation_test_db after merging
    char value[1024];
    result = fossil_crabdb_select(operation_test_db, "key1", value, sizeof(value));
    ASSUME_ITS_TRUE(result);
    ASSUME_ITS_EQUAL_CSTR("value1", value);

    result = fossil_crabdb_select(operation_test_db, "key2", value, sizeof(value));
    ASSUME_ITS_TRUE(result);
    ASSUME_ITS_EQUAL_CSTR("value2", value);

    // Clean up the source_db
    fossil_crabdb_destroy(source_db);
}

FOSSIL_TEST(test_copy) {
    // Create a destination database for copying
    fossil_crabdb_t* dest_db = fossil_crabdb_create();
    fossil_crabdb_insert(operation_test_db, "key1", "value1", FOSSIL_CRABDB_TYPE_STRING);

    // Copy operation_test_db to dest_db
    bool result = fossil_crabdb_copy(operation_test_db, dest_db);
    ASSUME_ITS_TRUE(result);

    // Verify key1 exists in dest_db with the correct value
    char value[1024];
    result = fossil_crabdb_select(dest_db, "key1", value, sizeof(value));
    ASSUME_ITS_TRUE(result);
    ASSUME_ITS_EQUAL_CSTR("value1", value);

    // Clean up the dest_db
    fossil_crabdb_destroy(dest_db);
}

FOSSIL_TEST(test_memory_usage) {
    // Insert some key-value pairs
    fossil_crabdb_insert(operation_test_db, "key1", "value1", FOSSIL_CRABDB_TYPE_STRING);
    fossil_crabdb_insert(operation_test_db, "key2", "value2", FOSSIL_CRABDB_TYPE_STRING);

    // Calculate memory usage
    size_t memory_usage = fossil_crabdb_memory_usage(operation_test_db);
    ASSUME_ITS_TRUE(memory_usage > 0); // Ensure some memory is being used

    // Clear the database
    fossil_crabdb_clear(operation_test_db);

    // Recalculate memory usage after clearing
    memory_usage = fossil_crabdb_memory_usage(operation_test_db);
    ASSUME_ITS_TRUE(memory_usage == 0); // Memory usage should be minimal or zero
}

void sample_callback(const char* key, const char* value) {
    // Example callback function
    printf("Iterating over key: %s, value: %s\n", key, value);
}

FOSSIL_TEST(test_iterate) {
    // Insert some key-value pairs
    fossil_crabdb_insert(operation_test_db, "key1", "value1", FOSSIL_CRABDB_TYPE_STRING);
    fossil_crabdb_insert(operation_test_db, "key2", "value2", FOSSIL_CRABDB_TYPE_STRING);

    // Use iterate to apply callback to each key-value pair
    bool result = fossil_crabdb_iterate(operation_test_db, sample_callback);
    ASSUME_ITS_TRUE(result);
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
    ADD_TESTF(test_rename_key, crabdb_operation_fixture);
    ADD_TESTF(test_check_duplicates, crabdb_operation_fixture);
    ADD_TESTF(test_merge, crabdb_operation_fixture);
    ADD_TESTF(test_copy, crabdb_operation_fixture);
    ADD_TESTF(test_memory_usage, crabdb_operation_fixture);
    ADD_TESTF(test_iterate, crabdb_operation_fixture);
} // end of tests

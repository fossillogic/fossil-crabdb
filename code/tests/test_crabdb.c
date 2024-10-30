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

FOSSIL_TEST(test_fossil_crabdb_create) {
    fossil_crabdb_t* db = fossil_crabdb_create();
    ASSUME_NOT_CNULL(db);
    ASSUME_ITS_EQUAL_I32(0, db->node_count);
    fossil_crabdb_destroy(db);
}

FOSSIL_TEST(test_fossil_crabdb_destroy) {
    fossil_crabdb_t* db = fossil_crabdb_create();
    fossil_crabdb_destroy(db); // Ensure no crash or memory leak
}

FOSSIL_TEST(test_fossil_crabdb_insert) {
    fossil_crabdb_t* db = fossil_crabdb_create();
    bool result = fossil_crabdb_insert(db, "key1", "value1", FOSSIL_CRABDB_TYPE_STRING);
    ASSUME_ITS_TRUE(result);
    ASSUME_ITS_EQUAL_I32(1, db->node_count);
    fossil_crabdb_destroy(db);
}

FOSSIL_TEST(test_fossil_crabdb_insert_duplicate_key) {
    fossil_crabdb_t* db = fossil_crabdb_create();
    fossil_crabdb_insert(db, "key1", "value1", FOSSIL_CRABDB_TYPE_STRING);
    bool result = fossil_crabdb_insert(db, "key1", "value2", FOSSIL_CRABDB_TYPE_STRING); // Should fail or update
    ASSUME_ITS_TRUE(result); // Assuming it updates; change to ASSUME_ITS_FALSE if not.
    ASSUME_ITS_EQUAL_I32(1, db->node_count);
    char value[FOSSIL_CRABDB_VAL_SIZE];
    fossil_crabdb_select(db, "key1", value, sizeof(value));
    ASSUME_ITS_EQUAL_CSTR("value2", value); // Verify updated value
    fossil_crabdb_destroy(db);
}

FOSSIL_TEST(test_fossil_crabdb_select_existing_key) {
    fossil_crabdb_t* db = fossil_crabdb_create();
    fossil_crabdb_insert(db, "key1", "value1", FOSSIL_CRABDB_TYPE_STRING);
    char value[FOSSIL_CRABDB_VAL_SIZE];
    bool result = fossil_crabdb_select(db, "key1", value, sizeof(value));
    ASSUME_ITS_TRUE(result);
    ASSUME_ITS_EQUAL_CSTR("value1", value);
    fossil_crabdb_destroy(db);
}

FOSSIL_TEST(test_fossil_crabdb_select_non_existing_key) {
    fossil_crabdb_t* db = fossil_crabdb_create();
    char value[FOSSIL_CRABDB_VAL_SIZE];
    bool result = fossil_crabdb_select(db, "non_existing_key", value, sizeof(value));
    ASSUME_ITS_FALSE(result); // Should return false
    fossil_crabdb_destroy(db);
}

FOSSIL_TEST(test_fossil_crabdb_update) {
    fossil_crabdb_t* db = fossil_crabdb_create();
    fossil_crabdb_insert(db, "key1", "value1", FOSSIL_CRABDB_TYPE_STRING);
    bool result = fossil_crabdb_update(db, "key1", "value_updated");
    ASSUME_ITS_TRUE(result);
    char value[FOSSIL_CRABDB_VAL_SIZE];
    fossil_crabdb_select(db, "key1", value, sizeof(value));
    ASSUME_ITS_EQUAL_CSTR("value_updated", value);
    fossil_crabdb_destroy(db);
}

FOSSIL_TEST(test_fossil_crabdb_delete) {
    fossil_crabdb_t* db = fossil_crabdb_create();
    fossil_crabdb_insert(db, "key1", "value1", FOSSIL_CRABDB_TYPE_STRING);
    bool result = fossil_crabdb_delete(db, "key1");
    ASSUME_ITS_TRUE(result);
    ASSUME_ITS_EQUAL_I32(0, db->node_count);
    fossil_crabdb_destroy(db);
}

FOSSIL_TEST(test_fossil_crabdb_delete_non_existing_key) {
    fossil_crabdb_t* db = fossil_crabdb_create();
    bool result = fossil_crabdb_delete(db, "non_existing_key");
    ASSUME_ITS_FALSE(result); // Should return false
    fossil_crabdb_destroy(db);
}

FOSSIL_TEST(test_fossil_crabdb_cleanup_expired) {
    fossil_crabdb_t* db = fossil_crabdb_create();
    fossil_crabdb_insert_with_ttl(db, "key1", "value1", FOSSIL_CRABDB_TYPE_STRING, 1); // 1 second TTL
    // Delay loop to allow expiration
    time_t start_time = time(NULL);
    while (time(NULL) - start_time < 2) {
        // Busy-wait loop for 2 seconds
    }
    bool result = fossil_crabdb_cleanup_expired(db);
    ASSUME_ITS_TRUE(result);
    ASSUME_ITS_EQUAL_I32(0, db->node_count);
    fossil_crabdb_destroy(db);
}

FOSSIL_TEST(test_fossil_crabdb_update_batch) {
    fossil_crabdb_t* db = fossil_crabdb_create();
    fossil_crabdb_insert(db, "key1", "value1", FOSSIL_CRABDB_TYPE_STRING);
    fossil_crabdb_insert(db, "key2", "value2", FOSSIL_CRABDB_TYPE_STRING);
    fossil_crabdb_insert(db, "key3", "value3", FOSSIL_CRABDB_TYPE_STRING);
    const char* keys[] = {"key1", "key2", "key3"};
    const char* values[] = {"value_updated1", "value_updated2", "value_updated3"};
    bool result = fossil_crabdb_update_batch(db, keys, values, 3);
    ASSUME_ITS_TRUE(result);
    char value[FOSSIL_CRABDB_VAL_SIZE];
    fossil_crabdb_select(db, "key1", value, sizeof(value));
    ASSUME_ITS_EQUAL_CSTR("value_updated1", value);
    fossil_crabdb_select(db, "key2", value, sizeof(value));
    ASSUME_ITS_EQUAL_CSTR("value_updated2", value);
    fossil_crabdb_select(db, "key3", value, sizeof(value));
    ASSUME_ITS_EQUAL_CSTR("value_updated3", value);
    fossil_crabdb_destroy(db);
}

FOSSIL_TEST(test_fossil_crabdb_delete_batch) {
    fossil_crabdb_t* db = fossil_crabdb_create();
    fossil_crabdb_insert(db, "key1", "value1", FOSSIL_CRABDB_TYPE_STRING);
    fossil_crabdb_insert(db, "key2", "value2", FOSSIL_CRABDB_TYPE_STRING);
    fossil_crabdb_insert(db, "key3", "value3", FOSSIL_CRABDB_TYPE_STRING);
    const char* keys[] = {"key1", "key2", "key3"};
    bool result = fossil_crabdb_delete_batch(db, keys, 3);
    ASSUME_ITS_TRUE(result);
    ASSUME_ITS_EQUAL_I32(0, db->node_count);
    fossil_crabdb_destroy(db);
}

// Test case for inserting NULL key
FOSSIL_TEST(test_fossil_crabdb_insert_null_key) {
    fossil_crabdb_t* db = fossil_crabdb_create();
    bool result = fossil_crabdb_insert(db, NULL, "value1", FOSSIL_CRABDB_TYPE_STRING);
    ASSUME_ITS_FALSE(result); // Inserting NULL key should fail
    fossil_crabdb_destroy(db);
}

// Test case for inserting with NULL value
FOSSIL_TEST(test_fossil_crabdb_insert_null_value) {
    fossil_crabdb_t* db = fossil_crabdb_create();
    bool result = fossil_crabdb_insert(db, "key1", NULL, FOSSIL_CRABDB_TYPE_STRING);
    ASSUME_ITS_FALSE(result); // Inserting NULL value should fail
    fossil_crabdb_destroy(db);
}

// Test case for selecting with NULL key
FOSSIL_TEST(test_fossil_crabdb_select_null_key) {
    fossil_crabdb_t* db = fossil_crabdb_create();
    char value[FOSSIL_CRABDB_VAL_SIZE];
    bool result = fossil_crabdb_select(db, NULL, value, sizeof(value));
    ASSUME_ITS_FALSE(result); // Selecting with NULL key should fail
    fossil_crabdb_destroy(db);
}

// Test case for updating a NULL key
FOSSIL_TEST(test_fossil_crabdb_update_null_key) {
    fossil_crabdb_t* db = fossil_crabdb_create();
    bool result = fossil_crabdb_update(db, NULL, "new_value");
    ASSUME_ITS_FALSE(result); // Updating NULL key should fail
    fossil_crabdb_destroy(db);
}

// Test case for updating with NULL value
FOSSIL_TEST(test_fossil_crabdb_update_null_value) {
    fossil_crabdb_t* db = fossil_crabdb_create();
    fossil_crabdb_insert(db, "key1", "value1", FOSSIL_CRABDB_TYPE_STRING);
    bool result = fossil_crabdb_update(db, "key1", NULL);
    ASSUME_ITS_FALSE(result); // Updating with NULL value should fail
    fossil_crabdb_destroy(db);
}

// Test case for deleting with NULL key
FOSSIL_TEST(test_fossil_crabdb_delete_null_key) {
    fossil_crabdb_t* db = fossil_crabdb_create();
    bool result = fossil_crabdb_delete(NULL, NULL);
    ASSUME_ITS_FALSE(result); // Deleting with NULL key should fail
    fossil_crabdb_destroy(db);
}

// Test case for concurrent access handling (basic)
FOSSIL_TEST(test_fossil_crabdb_concurrent_access) {
    fossil_crabdb_t* db = fossil_crabdb_create();
    // Simulate concurrent inserts (this is a simplification; real concurrent tests would use threads)
    bool result1 = fossil_crabdb_insert(db, "key1", "value1", FOSSIL_CRABDB_TYPE_STRING);
    bool result2 = fossil_crabdb_insert(db, "key1", "value2", FOSSIL_CRABDB_TYPE_STRING); // Should update
    ASSUME_ITS_TRUE(result1);
    ASSUME_ITS_TRUE(result2);
    char value[FOSSIL_CRABDB_VAL_SIZE];
    fossil_crabdb_select(db, "key1", value, sizeof(value));
    ASSUME_ITS_EQUAL_CSTR("value2", value); // Verify updated value
    fossil_crabdb_destroy(db);
}

// Test case for inserting large data
FOSSIL_TEST(test_fossil_crabdb_insert_large_data) {
    fossil_crabdb_t* db = fossil_crabdb_create();
    char large_value[FOSSIL_CRABDB_VAL_SIZE];
    memset(large_value, 'A', sizeof(large_value) - 1);
    large_value[sizeof(large_value) - 1] = '\0'; // Ensure null-terminated
    bool result = fossil_crabdb_insert(db, "large_key", large_value, FOSSIL_CRABDB_TYPE_STRING);
    ASSUME_ITS_TRUE(result);
    char value[FOSSIL_CRABDB_VAL_SIZE];
    fossil_crabdb_select(db, "large_key", value, sizeof(value));
    ASSUME_ITS_EQUAL_CSTR(large_value, value); // Verify large value inserted correctly
    fossil_crabdb_destroy(db);
}

// Test case for selecting with insufficient buffer size
FOSSIL_TEST(test_fossil_crabdb_select_insufficient_buffer) {
    fossil_crabdb_t* db = fossil_crabdb_create();
    fossil_crabdb_insert(db, "key1", "value1", FOSSIL_CRABDB_TYPE_STRING);
    char value[5]; // Insufficient buffer
    bool result = fossil_crabdb_select(db, "key1", value, sizeof(value));
    ASSUME_ITS_TRUE(result); // Should succeed, but may not null-terminate
    ASSUME_NOT_EQUAL_CSTR("value1", value); // Should not equal due to buffer overflow
    fossil_crabdb_destroy(db);
}

// Test case for checking total count of keys
FOSSIL_TEST(test_fossil_crabdb_count_keys) {
    fossil_crabdb_t* db = fossil_crabdb_create();
    ASSUME_ITS_EQUAL_I32(0, fossil_crabdb_count_keys(db)); // Initially should be 0
    fossil_crabdb_insert(db, "key1", "value1", FOSSIL_CRABDB_TYPE_STRING);
    fossil_crabdb_insert(db, "key2", "value2", FOSSIL_CRABDB_TYPE_STRING);
    ASSUME_ITS_EQUAL_I32(2, fossil_crabdb_count_keys(db)); // Should be 2
    fossil_crabdb_destroy(db);
}

// Test case for handling invalid data types
FOSSIL_TEST(test_fossil_crabdb_invalid_data_type) {
    fossil_crabdb_t* db = fossil_crabdb_create();
    bool result = fossil_crabdb_insert(db, "key1", "value1", -1); // Invalid type
    ASSUME_ITS_FALSE(result); // Should fail due to invalid type
    fossil_crabdb_destroy(db);
}

// Test case for batch update with invalid data
FOSSIL_TEST(test_fossil_crabdb_update_batch_invalid_data) {
    fossil_crabdb_t* db = fossil_crabdb_create();
    fossil_crabdb_insert(db, "key1", "value1", FOSSIL_CRABDB_TYPE_STRING);
    const char* keys[] = {"key1", "key2", "key_invalid"};
    const char* values[] = {"new_value1", "new_value2", "new_value_invalid"};
    bool result = fossil_crabdb_update_batch(db, keys, values, 3); // One invalid key
    ASSUME_ITS_FALSE(result); // Should fail
    fossil_crabdb_destroy(db);
}

// Test case for batch delete with non-existing keys
FOSSIL_TEST(test_fossil_crabdb_delete_batch_non_existing_keys) {
    fossil_crabdb_t* db = fossil_crabdb_create();
    const char* keys[] = {"non_existing_key1", "non_existing_key2"};
    bool result = fossil_crabdb_delete_batch(db, keys, 2); // All non-existing
    ASSUME_ITS_FALSE(result); // Should fail
    fossil_crabdb_destroy(db);
}

// Test case for creating a new table
FOSSIL_TEST(test_fossil_crabdb_create_table) {
    fossil_crabdb_t* db = fossil_crabdb_create();
    bool result = fossil_crabdb_create_table(db, "table1");
    ASSUME_ITS_TRUE(result); // Creating a new table should succeed
    fossil_crabdb_destroy(db);
}

// Test case for creating a table with a duplicate name
FOSSIL_TEST(test_fossil_crabdb_create_duplicate_table) {
    fossil_crabdb_t* db = fossil_crabdb_create();
    fossil_crabdb_create_table(db, "table1");
    bool result = fossil_crabdb_create_table(db, "table1"); // Duplicate creation should fail
    ASSUME_ITS_FALSE(result);
    fossil_crabdb_destroy(db);
}

// Test case for deleting a table
FOSSIL_TEST(test_fossil_crabdb_delete_table) {
    fossil_crabdb_t* db = fossil_crabdb_create();
    fossil_crabdb_create_table(db, "table1");
    bool result = fossil_crabdb_delete_table(db, "table1");
    ASSUME_ITS_TRUE(result); // Deleting the existing table should succeed
    fossil_crabdb_destroy(db);
}

// Test case for deleting a non-existing table
FOSSIL_TEST(test_fossil_crabdb_delete_non_existing_table) {
    fossil_crabdb_t* db = fossil_crabdb_create();
    bool result = fossil_crabdb_delete_table(db, "non_existing_table"); // Should fail
    ASSUME_ITS_FALSE(result);
    fossil_crabdb_destroy(db);
}

// Test case for selecting from a table
FOSSIL_TEST(test_fossil_crabdb_select_from_table) {
    fossil_crabdb_t* db = fossil_crabdb_create();
    fossil_crabdb_create_table(db, "table1");
    fossil_crabdb_insert(db, "table1.key1", "value1", FOSSIL_CRABDB_TYPE_STRING); // Use table-prefixed keys
    char value[FOSSIL_CRABDB_VAL_SIZE];
    bool result = fossil_crabdb_select(db, "table1.key1", value, sizeof(value));
    ASSUME_ITS_TRUE(result); // Selecting from the table should succeed
    ASSUME_ITS_EQUAL_CSTR("value1", value); // Verify correct value
    fossil_crabdb_destroy(db);
}

// Test case for selecting from a non-existing table
FOSSIL_TEST(test_fossil_crabdb_select_non_existing_table) {
    fossil_crabdb_t* db = fossil_crabdb_create();
    char value[FOSSIL_CRABDB_VAL_SIZE];
    bool result = fossil_crabdb_select(db, "non_existing_table.key1", value, sizeof(value)); // Should fail
    ASSUME_ITS_FALSE(result);
    fossil_crabdb_destroy(db);
}

// Test case for updating a record in a table
FOSSIL_TEST(test_fossil_crabdb_update_table_record) {
    fossil_crabdb_t* db = fossil_crabdb_create();
    fossil_crabdb_create_table(db, "table1");
    fossil_crabdb_insert(db, "table1.key1", "value1", FOSSIL_CRABDB_TYPE_STRING);
    bool result = fossil_crabdb_update(db, "table1.key1", "new_value1"); // Update should succeed
    ASSUME_ITS_TRUE(result);
    char value[FOSSIL_CRABDB_VAL_SIZE];
    fossil_crabdb_select(db, "table1.key1", value, sizeof(value));
    ASSUME_ITS_EQUAL_CSTR("new_value1", value); // Verify updated value
    fossil_crabdb_destroy(db);
}

// Test case for batch insert into a table
FOSSIL_TEST(test_fossil_crabdb_batch_insert_table) {
    fossil_crabdb_t* db = fossil_crabdb_create();
    fossil_crabdb_create_table(db, "table1");
    const char* keys[] = {"table1.key1", "table1.key2", "table1.key3"};
    const char* values[] = {"value1", "value2", "value3"};
    fossil_crabdb_type_t types[] = {FOSSIL_CRABDB_TYPE_STRING, FOSSIL_CRABDB_TYPE_STRING, FOSSIL_CRABDB_TYPE_STRING};
    bool result = fossil_crabdb_insert_batch(db, keys, values, types, 3); // Should succeed
    ASSUME_ITS_TRUE(result);
    char value[FOSSIL_CRABDB_VAL_SIZE];
    fossil_crabdb_select(db, "table1.key1", value, sizeof(value));
    ASSUME_ITS_EQUAL_CSTR("value1", value); // Verify first value
    fossil_crabdb_destroy(db);
}

// Test case for batch update in a table
FOSSIL_TEST(test_fossil_crabdb_batch_update_table) {
    fossil_crabdb_t* db = fossil_crabdb_create();
    fossil_crabdb_create_table(db, "table1");
    fossil_crabdb_insert(db, "table1.key1", "value1", FOSSIL_CRABDB_TYPE_STRING);
    const char* keys[] = {"table1.key1", "table1.key2"};
    const char* values[] = {"new_value1", "new_value2"}; // Update existing and new key
    bool result = fossil_crabdb_update_batch(db, keys, values, 2); // One exists, one doesn't
    ASSUME_ITS_TRUE(result); // Updating existing key should succeed
    char value[FOSSIL_CRABDB_VAL_SIZE];
    fossil_crabdb_select(db, "table1.key1", value, sizeof(value));
    ASSUME_ITS_EQUAL_CSTR("new_value1", value); // Verify updated value
    fossil_crabdb_destroy(db);
}

// Test case for checking table existence
FOSSIL_TEST(test_fossil_crabdb_table_exists) {
    fossil_crabdb_t* db = fossil_crabdb_create();
    fossil_crabdb_create_table(db, "table1");
    bool exists = fossil_crabdb_table_exists(db, "table1"); // Check existence
    ASSUME_ITS_TRUE(exists);
    fossil_crabdb_destroy(db);
}

// Test case for checking non-existing table
FOSSIL_TEST(test_fossil_crabdb_non_existing_table) {
    fossil_crabdb_t* db = fossil_crabdb_create();
    bool exists = fossil_crabdb_table_exists(db, "non_existing_table"); // Should return false
    ASSUME_ITS_FALSE(exists);
    fossil_crabdb_destroy(db);
}

// * * * * * * * * * * * * * * * * * * * * * * * *
// * Fossil Logic Test Pool
// * * * * * * * * * * * * * * * * * * * * * * * *

FOSSIL_TEST_GROUP(c_crab_database_tests) {    
    ADD_TEST(test_fossil_crabdb_create);
    ADD_TEST(test_fossil_crabdb_destroy);
    ADD_TEST(test_fossil_crabdb_insert);
    ADD_TEST(test_fossil_crabdb_insert_duplicate_key);
    ADD_TEST(test_fossil_crabdb_select_existing_key);
    ADD_TEST(test_fossil_crabdb_select_non_existing_key);
    ADD_TEST(test_fossil_crabdb_update);
    ADD_TEST(test_fossil_crabdb_delete);
    ADD_TEST(test_fossil_crabdb_delete_non_existing_key);
    ADD_TEST(test_fossil_crabdb_cleanup_expired);
    ADD_TEST(test_fossil_crabdb_update_batch);
    ADD_TEST(test_fossil_crabdb_delete_batch);
    ADD_TEST(test_fossil_crabdb_insert_null_key);
    ADD_TEST(test_fossil_crabdb_insert_null_value);
    ADD_TEST(test_fossil_crabdb_select_null_key);
    ADD_TEST(test_fossil_crabdb_update_null_key);
    ADD_TEST(test_fossil_crabdb_update_null_value);
    ADD_TEST(test_fossil_crabdb_delete_null_key);
    ADD_TEST(test_fossil_crabdb_concurrent_access);
    ADD_TEST(test_fossil_crabdb_insert_large_data);
    ADD_TEST(test_fossil_crabdb_select_insufficient_buffer);
    ADD_TEST(test_fossil_crabdb_count_keys);
    ADD_TEST(test_fossil_crabdb_invalid_data_type);
    ADD_TEST(test_fossil_crabdb_update_batch_invalid_data);
    ADD_TEST(test_fossil_crabdb_delete_batch_non_existing_keys);
    ADD_TEST(test_fossil_crabdb_create_table);
    ADD_TEST(test_fossil_crabdb_create_duplicate_table);
    ADD_TEST(test_fossil_crabdb_delete_table);
    ADD_TEST(test_fossil_crabdb_delete_non_existing_table);
    ADD_TEST(test_fossil_crabdb_select_from_table);
    ADD_TEST(test_fossil_crabdb_select_non_existing_table);
    ADD_TEST(test_fossil_crabdb_update_table_record);
    ADD_TEST(test_fossil_crabdb_batch_insert_table);
    ADD_TEST(test_fossil_crabdb_batch_update_table);
    ADD_TEST(test_fossil_crabdb_table_exists);
    ADD_TEST(test_fossil_crabdb_non_existing_table);
} // end of tests

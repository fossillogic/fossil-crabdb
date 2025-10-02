/**
 * -----------------------------------------------------------------------------
 * Project: Fossil Logic
 *
 * This file is part of the Fossil Logic project, which aims to develop
 * high-performance, cross-platform applications and libraries. The code
 * contained herein is licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License. You may obtain
 * a copy of the License at:
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS, WITHOUT
 * WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied. See the
 * License for the specific language governing permissions and limitations
 * under the License.
 *
 * Author: Michael Gene Brockus (Dreamer)
 * Date: 04/05/2014
 *
 * Copyright (C) 2014-2025 Fossil Logic. All rights reserved.
 * -----------------------------------------------------------------------------
 */
#include <fossil/pizza/framework.h>

#include "fossil/crabdb/framework.h"

// * * * * * * * * * * * * * * * * * * * * * * * *
// * Fossil Logic Test Utilities
// * * * * * * * * * * * * * * * * * * * * * * * *
// Setup steps for things like test fixtures and
// mock objects are set here.
// * * * * * * * * * * * * * * * * * * * * * * * *

FOSSIL_SUITE(c_myshell_fixture);

FOSSIL_SETUP(c_myshell_fixture) {
    // Setup the test fixture
}

FOSSIL_TEARDOWN(c_myshell_fixture) {
    // Teardown the test fixture
}

// * * * * * * * * * * * * * * * * * * * * * * * *
// * Fossil Logic Test Blue CrabDB Database
// * * * * * * * * * * * * * * * * * * * * * * * *

/*
 * Test case for creating a new record in the database file (FSON encoding)
 */
FOSSIL_TEST(c_test_myshell_create_record) {
    const char *file_name = "test.myshell";
    fossil_bluecrab_myshell_create_database(file_name);

    // Store a value as FSON type:value
    fossil_bluecrab_myshell_error_t result = fossil_bluecrab_myshell_create_record(file_name, "key1", "i32:42");
    ASSUME_ITS_TRUE(result == FOSSIL_MYSHELL_ERROR_SUCCESS);

    char value[256];
    result = fossil_bluecrab_myshell_read_record(file_name, "key1", value, sizeof(value));
    ASSUME_ITS_TRUE(result == FOSSIL_MYSHELL_ERROR_SUCCESS);
    ASSUME_ITS_TRUE(strcmp(value, "i32:42") == 0);

    // Try to create duplicate key
    result = fossil_bluecrab_myshell_create_record(file_name, "key1", "i32:99");
    ASSUME_ITS_TRUE(result == FOSSIL_MYSHELL_ERROR_ALREADY_EXISTS);

    fossil_bluecrab_myshell_delete_database(file_name);
}

/*
 * Test case for reading a non-existent record from the database file
 */
FOSSIL_TEST(c_test_myshell_read_nonexistent_record) {
    const char *file_name = "test.myshell";
    fossil_bluecrab_myshell_create_database(file_name);

    char value[256];
    fossil_bluecrab_myshell_error_t result = fossil_bluecrab_myshell_read_record(file_name, "nonexistent_key", value, sizeof(value));
    ASSUME_ITS_TRUE(result == FOSSIL_MYSHELL_ERROR_NOT_FOUND);

    fossil_bluecrab_myshell_delete_database(file_name);
}

/*
 * Test case for updating a non-existent record in the database file
 */
FOSSIL_TEST(c_test_myshell_update_nonexistent_record) {
    const char *file_name = "test.myshell";
    fossil_bluecrab_myshell_create_database(file_name);

    fossil_bluecrab_myshell_error_t result = fossil_bluecrab_myshell_update_record(file_name, "nonexistent_key", "i8:7");
    ASSUME_ITS_TRUE(result == FOSSIL_MYSHELL_ERROR_NOT_FOUND);

    fossil_bluecrab_myshell_delete_database(file_name);
}

/*
 * Test case for deleting a non-existent record from the database file
 */
FOSSIL_TEST(c_test_myshell_delete_nonexistent_record) {
    const char *file_name = "test.myshell";
    fossil_bluecrab_myshell_create_database(file_name);

    fossil_bluecrab_myshell_error_t result = fossil_bluecrab_myshell_delete_record(file_name, "nonexistent_key");
    ASSUME_ITS_TRUE(result == FOSSIL_MYSHELL_ERROR_NOT_FOUND);

    fossil_bluecrab_myshell_delete_database(file_name);
}

/*
 * Test case for validating the file extension of a database file
 */
FOSSIL_TEST(c_test_myshell_validate_extension) {
    ASSUME_ITS_TRUE(fossil_bluecrab_myshell_validate_extension("test.myshell"));
    ASSUME_ITS_FALSE(fossil_bluecrab_myshell_validate_extension("test.txt"));
}

/*
 * Test case for validating data
 */
FOSSIL_TEST(c_test_myshell_validate_data) {
    ASSUME_ITS_TRUE(fossil_bluecrab_myshell_validate_data("valid_data"));
    ASSUME_ITS_FALSE(fossil_bluecrab_myshell_validate_data(NULL));
    ASSUME_ITS_FALSE(fossil_bluecrab_myshell_validate_data(""));
}

/*
 * Test case for file size
 */
FOSSIL_TEST(c_test_myshell_get_file_size) {
    const char *file_name = "test.myshell";
    fossil_bluecrab_myshell_create_database(file_name);
    fossil_bluecrab_myshell_create_record(file_name, "key1", "i32:1");

    size_t size_bytes = 0;
    fossil_bluecrab_myshell_error_t result = fossil_bluecrab_myshell_get_file_size(file_name, &size_bytes);
    ASSUME_ITS_TRUE(result == FOSSIL_MYSHELL_ERROR_SUCCESS);
    ASSUME_ITS_TRUE(size_bytes > 0);

    fossil_bluecrab_myshell_delete_database(file_name);
}

/*
 * Test case for error string conversion
 */
FOSSIL_TEST(c_test_myshell_error_string) {
    ASSUME_ITS_TRUE(strcmp(fossil_bluecrab_myshell_error_string(FOSSIL_MYSHELL_ERROR_SUCCESS), "Success") == 0);
    ASSUME_ITS_TRUE(strcmp(fossil_bluecrab_myshell_error_string(FOSSIL_MYSHELL_ERROR_NOT_FOUND), "Record not found") == 0);
}

/*
 * Test case for opening and closing a database
 */
FOSSIL_TEST(c_test_myshell_open_close_database) {
    const char *file_name = "test.myshell";
    fossil_bluecrab_myshell_create_database(file_name);

    fossil_bluecrab_myshell_error_t result = fossil_bluecrab_myshell_open_database(file_name);
    ASSUME_ITS_TRUE(result == FOSSIL_MYSHELL_ERROR_SUCCESS);
    ASSUME_ITS_TRUE(fossil_bluecrab_myshell_is_open(file_name));

    // Try to open again (should get ALREADY_EXISTS)
    result = fossil_bluecrab_myshell_open_database(file_name);
    ASSUME_ITS_TRUE(result == FOSSIL_MYSHELL_ERROR_ALREADY_EXISTS);

    result = fossil_bluecrab_myshell_close_database(file_name);
    ASSUME_ITS_TRUE(result == FOSSIL_MYSHELL_ERROR_SUCCESS);
    ASSUME_ITS_FALSE(fossil_bluecrab_myshell_is_open(file_name));

    fossil_bluecrab_myshell_delete_database(file_name);
}

/*
 * Test case for verifying database integrity (hash check)
 */
FOSSIL_TEST(c_test_myshell_verify_database) {
    const char *file_name = "test.myshell";
    fossil_bluecrab_myshell_create_database(file_name);
    fossil_bluecrab_myshell_create_record(file_name, "key1", "i16:123");
    fossil_bluecrab_myshell_create_record(file_name, "key2", "u32:456");

    fossil_bluecrab_myshell_error_t result = fossil_bluecrab_myshell_verify_database(file_name);
    ASSUME_ITS_TRUE(result == FOSSIL_MYSHELL_ERROR_SUCCESS);

    fossil_bluecrab_myshell_delete_database(file_name);
}

/*
 * Test case for creating a record with empty key or value
 */
FOSSIL_TEST(c_test_myshell_create_empty_key_value) {
    const char *file_name = "test.myshell";
    fossil_bluecrab_myshell_create_database(file_name);

    fossil_bluecrab_myshell_error_t result = fossil_bluecrab_myshell_create_record(file_name, "", "i32:10");
    ASSUME_ITS_TRUE(result == FOSSIL_MYSHELL_ERROR_PARSE_FAILED);

    result = fossil_bluecrab_myshell_create_record(file_name, "key1", "");
    ASSUME_ITS_TRUE(result == FOSSIL_MYSHELL_ERROR_PARSE_FAILED);

    fossil_bluecrab_myshell_delete_database(file_name);
}

/*
 * Test case for deleting a database file that does not exist
 */
FOSSIL_TEST(c_test_myshell_delete_nonexistent_database) {
    const char *file_name = "doesnotexist.myshell";
    fossil_bluecrab_myshell_error_t result = fossil_bluecrab_myshell_delete_database(file_name);
    ASSUME_ITS_TRUE(result == FOSSIL_MYSHELL_ERROR_IO);
}

/*
 * Edge case: Create record with special characters in key and value
 */
FOSSIL_TEST(c_test_myshell_create_special_chars) {
    const char *file_name = "test.myshell";
    fossil_bluecrab_myshell_create_database(file_name);

    const char *special_key = "key!@#$%^&*()_+";
    const char *special_value = "val~`<>?/|\\{}[]";
    fossil_bluecrab_myshell_error_t result = fossil_bluecrab_myshell_create_record(file_name, special_key, special_value);
    ASSUME_ITS_TRUE(result == FOSSIL_MYSHELL_ERROR_SUCCESS || result == FOSSIL_MYSHELL_ERROR_PARSE_FAILED);

    if (result == FOSSIL_MYSHELL_ERROR_SUCCESS) {
        char value[256];
        result = fossil_bluecrab_myshell_read_record(file_name, special_key, value, sizeof(value));
        ASSUME_ITS_TRUE(result == FOSSIL_MYSHELL_ERROR_SUCCESS);
        ASSUME_ITS_TRUE(strcmp(value, special_value) == 0);
    }

    fossil_bluecrab_myshell_delete_database(file_name);
}

/*
 * Test case: Creating a record with duplicate key returns ALREADY_EXISTS
 */
FOSSIL_TEST(c_test_myshell_create_duplicate_key) {
    const char *file_name = "test.myshell";
    fossil_bluecrab_myshell_create_database(file_name);

    fossil_bluecrab_myshell_error_t result = fossil_bluecrab_myshell_create_record(file_name, "dupkey", "i32:1");
    ASSUME_ITS_TRUE(result == FOSSIL_MYSHELL_ERROR_SUCCESS);

    result = fossil_bluecrab_myshell_create_record(file_name, "dupkey", "i32:2");
    ASSUME_ITS_TRUE(result == FOSSIL_MYSHELL_ERROR_ALREADY_EXISTS);

    fossil_bluecrab_myshell_delete_database(file_name);
}

/*
 * Test case: Creating a record in a file with invalid extension returns INVALID_FILE
 */
FOSSIL_TEST(c_test_myshell_create_invalid_extension) {
    fossil_bluecrab_myshell_error_t result = fossil_bluecrab_myshell_create_record("test.txt", "key", "i32:1");
    ASSUME_ITS_TRUE(result == FOSSIL_MYSHELL_ERROR_INVALID_FILE);
}

/*
 * Test case: Reading a record with invalid extension returns INVALID_FILE
 */
FOSSIL_TEST(c_test_myshell_read_invalid_extension) {
    char value[128];
    fossil_bluecrab_myshell_error_t result = fossil_bluecrab_myshell_read_record("test.txt", "key", value, sizeof(value));
    ASSUME_ITS_TRUE(result == FOSSIL_MYSHELL_ERROR_INVALID_FILE);
}

/*
 * Test case: Updating a record with invalid extension returns INVALID_FILE
 */
FOSSIL_TEST(c_test_myshell_update_invalid_extension) {
    fossil_bluecrab_myshell_error_t result = fossil_bluecrab_myshell_update_record("test.txt", "key", "i32:2");
    ASSUME_ITS_TRUE(result == FOSSIL_MYSHELL_ERROR_INVALID_FILE);
}

/*
 * Test case: Deleting a record with invalid extension returns INVALID_FILE
 */
FOSSIL_TEST(c_test_myshell_delete_invalid_extension) {
    fossil_bluecrab_myshell_error_t result = fossil_bluecrab_myshell_delete_record("test.txt", "key");
    ASSUME_ITS_TRUE(result == FOSSIL_MYSHELL_ERROR_INVALID_FILE);
}

/*
 * Test case: Creating a record with a key containing '=' returns PARSE_FAILED
 */
FOSSIL_TEST(c_test_myshell_create_key_with_equal) {
    const char *file_name = "test.myshell";
    fossil_bluecrab_myshell_create_database(file_name);

    fossil_bluecrab_myshell_error_t result = fossil_bluecrab_myshell_create_record(file_name, "bad=key", "i32:1");
    ASSUME_ITS_TRUE(result == FOSSIL_MYSHELL_ERROR_PARSE_FAILED || result == FOSSIL_MYSHELL_ERROR_SUCCESS);

    fossil_bluecrab_myshell_delete_database(file_name);
}

/*
 * Test case: Reading a record with empty key returns PARSE_FAILED
 */
FOSSIL_TEST(c_test_myshell_read_empty_key) {
    const char *file_name = "test.myshell";
    fossil_bluecrab_myshell_create_database(file_name);

    char value[128];
    fossil_bluecrab_myshell_error_t result = fossil_bluecrab_myshell_read_record(file_name, "", value, sizeof(value));
    ASSUME_ITS_TRUE(result == FOSSIL_MYSHELL_ERROR_PARSE_FAILED);

    fossil_bluecrab_myshell_delete_database(file_name);
}

/*
 * Test case: Updating a record with empty value returns PARSE_FAILED
 */
FOSSIL_TEST(c_test_myshell_update_empty_value) {
    const char *file_name = "test.myshell";
    fossil_bluecrab_myshell_create_database(file_name);

    fossil_bluecrab_myshell_create_record(file_name, "key", "i32:1");
    fossil_bluecrab_myshell_error_t result = fossil_bluecrab_myshell_update_record(file_name, "key", "");
    ASSUME_ITS_TRUE(result == FOSSIL_MYSHELL_ERROR_PARSE_FAILED);

    fossil_bluecrab_myshell_delete_database(file_name);
}

/*
 * Test case: Deleting a record with empty key returns PARSE_FAILED
 */
FOSSIL_TEST(c_test_myshell_delete_empty_key) {
    const char *file_name = "test.myshell";
    fossil_bluecrab_myshell_create_database(file_name);

    fossil_bluecrab_myshell_error_t result = fossil_bluecrab_myshell_delete_record(file_name, "");
    ASSUME_ITS_TRUE(result == FOSSIL_MYSHELL_ERROR_PARSE_FAILED);

    fossil_bluecrab_myshell_delete_database(file_name);
}

/*
 * Test case: Creating a record with a value that is not FSON (plain string)
 */
FOSSIL_TEST(c_test_myshell_create_plain_string_value) {
    const char *file_name = "test.myshell";
    fossil_bluecrab_myshell_create_database(file_name);

    fossil_bluecrab_myshell_error_t result = fossil_bluecrab_myshell_create_record(file_name, "plain", "hello world");
    ASSUME_ITS_TRUE(result == FOSSIL_MYSHELL_ERROR_SUCCESS);

    char value[128];
    result = fossil_bluecrab_myshell_read_record(file_name, "plain", value, sizeof(value));
    ASSUME_ITS_TRUE(result == FOSSIL_MYSHELL_ERROR_SUCCESS);
    ASSUME_ITS_TRUE(strcmp(value, "hello world") == 0);

    fossil_bluecrab_myshell_delete_database(file_name);
}

/*
 * Test case: Creating a record with a value containing special FSON types
 */
FOSSIL_TEST(c_test_myshell_create_fson_types) {
    const char *file_name = "test.myshell";
    fossil_bluecrab_myshell_create_database(file_name);

    fossil_bluecrab_myshell_create_record(file_name, "int8", "i8:-128");
    fossil_bluecrab_myshell_create_record(file_name, "uint64", "u64:18446744073709551615");
    fossil_bluecrab_myshell_create_record(file_name, "float", "f32:3.14");
    fossil_bluecrab_myshell_create_record(file_name, "char", "char:A");

    char value[128];
    fossil_bluecrab_myshell_error_t result;

    result = fossil_bluecrab_myshell_read_record(file_name, "int8", value, sizeof(value));
    ASSUME_ITS_TRUE(result == FOSSIL_MYSHELL_ERROR_SUCCESS);
    ASSUME_ITS_TRUE(strcmp(value, "i8:-128") == 0);

    result = fossil_bluecrab_myshell_read_record(file_name, "uint64", value, sizeof(value));
    ASSUME_ITS_TRUE(result == FOSSIL_MYSHELL_ERROR_SUCCESS);
    ASSUME_ITS_TRUE(strcmp(value, "u64:18446744073709551615") == 0);

    result = fossil_bluecrab_myshell_read_record(file_name, "float", value, sizeof(value));
    ASSUME_ITS_TRUE(result == FOSSIL_MYSHELL_ERROR_SUCCESS);
    ASSUME_ITS_TRUE(strncmp(value, "f32:3.14", 7) == 0);

    result = fossil_bluecrab_myshell_read_record(file_name, "char", value, sizeof(value));
    ASSUME_ITS_TRUE(result == FOSSIL_MYSHELL_ERROR_SUCCESS);
    ASSUME_ITS_TRUE(strcmp(value, "char:A") == 0);

    fossil_bluecrab_myshell_delete_database(file_name);
}

// * * * * * * * * * * * * * * * * * * * * * * * *
// * Fossil Logic Test Pool
// * * * * * * * * * * * * * * * * * * * * * * * *
FOSSIL_TEST_GROUP(c_myshell_database_tests) {
    FOSSIL_TEST_ADD(c_myshell_fixture, c_test_myshell_create_record);
    FOSSIL_TEST_ADD(c_myshell_fixture, c_test_myshell_read_nonexistent_record);
    FOSSIL_TEST_ADD(c_myshell_fixture, c_test_myshell_update_nonexistent_record);
    FOSSIL_TEST_ADD(c_myshell_fixture, c_test_myshell_delete_nonexistent_record);
    FOSSIL_TEST_ADD(c_myshell_fixture, c_test_myshell_validate_extension);
    FOSSIL_TEST_ADD(c_myshell_fixture, c_test_myshell_validate_data);
    FOSSIL_TEST_ADD(c_myshell_fixture, c_test_myshell_get_file_size);
    FOSSIL_TEST_ADD(c_myshell_fixture, c_test_myshell_error_string);
    FOSSIL_TEST_ADD(c_myshell_fixture, c_test_myshell_open_close_database);
    FOSSIL_TEST_ADD(c_myshell_fixture, c_test_myshell_verify_database);
    FOSSIL_TEST_ADD(c_myshell_fixture, c_test_myshell_create_empty_key_value);
    FOSSIL_TEST_ADD(c_myshell_fixture, c_test_myshell_delete_nonexistent_database);
    FOSSIL_TEST_ADD(c_myshell_fixture, c_test_myshell_create_special_chars);
    FOSSIL_TEST_ADD(c_myshell_fixture, c_test_myshell_create_duplicate_key);
    FOSSIL_TEST_ADD(c_myshell_fixture, c_test_myshell_create_invalid_extension);
    FOSSIL_TEST_ADD(c_myshell_fixture, c_test_myshell_read_invalid_extension);
    FOSSIL_TEST_ADD(c_myshell_fixture, c_test_myshell_update_invalid_extension);
    FOSSIL_TEST_ADD(c_myshell_fixture, c_test_myshell_delete_invalid_extension);
    FOSSIL_TEST_ADD(c_myshell_fixture, c_test_myshell_create_key_with_equal);
    FOSSIL_TEST_ADD(c_myshell_fixture, c_test_myshell_read_empty_key);
    FOSSIL_TEST_ADD(c_myshell_fixture, c_test_myshell_update_empty_value);
    FOSSIL_TEST_ADD(c_myshell_fixture, c_test_myshell_delete_empty_key);
    FOSSIL_TEST_ADD(c_myshell_fixture, c_test_myshell_create_plain_string_value);
    FOSSIL_TEST_ADD(c_myshell_fixture, c_test_myshell_create_fson_types);

    FOSSIL_TEST_REGISTER(c_myshell_fixture);
} // end of tests

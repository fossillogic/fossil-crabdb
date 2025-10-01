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

FOSSIL_SUITE(cpp_myshell_fixture);

FOSSIL_SETUP(cpp_myshell_fixture) {
    // Setup the test fixture
}

FOSSIL_TEARDOWN(cpp_myshell_fixture) {
    // Teardown the test fixture
}

// * * * * * * * * * * * * * * * * * * * * * * * *
// * Fossil Logic Test Blue CrabDB Database
// * * * * * * * * * * * * * * * * * * * * * * * *

/*
 * Test case for creating a new record in the database file (FSON encoding)
 */
FOSSIL_TEST(cpp_test_myshell_create_record) {
    std::string file_name = "test.myshell";
    fossil::bluecrab::MyShell::create_database(file_name);

    // Store a value as FSON type:value
    fossil_bluecrab_myshell_error_t result = fossil::bluecrab::MyShell::create_record(file_name, "key1", "i32:42");
    ASSUME_ITS_TRUE(result == FOSSIL_MYSHELL_ERROR_SUCCESS);

    std::string value;
    result = fossil::bluecrab::MyShell::read_record(file_name, "key1", value);
    ASSUME_ITS_TRUE(result == FOSSIL_MYSHELL_ERROR_SUCCESS);
    ASSUME_ITS_TRUE(value == "i32:42");

    fossil::bluecrab::MyShell::delete_database(file_name);
}

/*
 * Test case for reading a non-existent record from the database file
 */
FOSSIL_TEST(cpp_test_myshell_read_nonexistent_record) {
    std::string file_name = "test.myshell";
    fossil::bluecrab::MyShell::create_database(file_name);

    std::string value;
    fossil_bluecrab_myshell_error_t result = fossil::bluecrab::MyShell::read_record(file_name, "nonexistent_key", value);
    ASSUME_ITS_TRUE(result == FOSSIL_MYSHELL_ERROR_NOT_FOUND);

    fossil::bluecrab::MyShell::delete_database(file_name);
}

/*
 * Test case for updating a record in the database file
 */
FOSSIL_TEST(cpp_test_myshell_update_record) {
    std::string file_name = "test.myshell";
    fossil::bluecrab::MyShell::create_database(file_name);

    fossil::bluecrab::MyShell::create_record(file_name, "key1", "bool:true");
    fossil_bluecrab_myshell_error_t result = fossil::bluecrab::MyShell::update_record(file_name, "key1", "bool:false");
    ASSUME_ITS_TRUE(result == FOSSIL_MYSHELL_ERROR_SUCCESS);

    std::string value;
    result = fossil::bluecrab::MyShell::read_record(file_name, "key1", value);
    ASSUME_ITS_TRUE(result == FOSSIL_MYSHELL_ERROR_SUCCESS);
    ASSUME_ITS_TRUE(value == "bool:false");

    fossil::bluecrab::MyShell::delete_database(file_name);
}

/*
 * Test case for updating a non-existent record in the database file
 */
FOSSIL_TEST(cpp_test_myshell_update_nonexistent_record) {
    std::string file_name = "test.myshell";
    fossil::bluecrab::MyShell::create_database(file_name);

    fossil_bluecrab_myshell_error_t result = fossil::bluecrab::MyShell::update_record(file_name, "nonexistent_key", "i8:7");
    ASSUME_ITS_TRUE(result == FOSSIL_MYSHELL_ERROR_NOT_FOUND);

    fossil::bluecrab::MyShell::delete_database(file_name);
}

/*
 * Test case for deleting a record from the database file
 */
FOSSIL_TEST(cpp_test_myshell_delete_record) {
    std::string file_name = "test.myshell";
    fossil::bluecrab::MyShell::create_database(file_name);

    fossil::bluecrab::MyShell::create_record(file_name, "key1", "f64:3.14159");
    fossil_bluecrab_myshell_error_t result = fossil::bluecrab::MyShell::delete_record(file_name, "key1");
    ASSUME_ITS_TRUE(result == FOSSIL_MYSHELL_ERROR_SUCCESS);

    std::string value;
    result = fossil::bluecrab::MyShell::read_record(file_name, "key1", value);
    ASSUME_ITS_TRUE(result == FOSSIL_MYSHELL_ERROR_NOT_FOUND);

    fossil::bluecrab::MyShell::delete_database(file_name);
}

/*
 * Test case for deleting a non-existent record from the database file
 */
FOSSIL_TEST(cpp_test_myshell_delete_nonexistent_record) {
    std::string file_name = "test.myshell";
    fossil::bluecrab::MyShell::create_database(file_name);

    fossil_bluecrab_myshell_error_t result = fossil::bluecrab::MyShell::delete_record(file_name, "nonexistent_key");
    ASSUME_ITS_TRUE(result == FOSSIL_MYSHELL_ERROR_NOT_FOUND);

    fossil::bluecrab::MyShell::delete_database(file_name);
}

/*
 * Test case for backing up and restoring a database file
 */
FOSSIL_TEST(cpp_test_myshell_backup_restore) {
    std::string file_name = "test.myshell";
    std::string backup_file = "backup.myshell";
    fossil::bluecrab::MyShell::create_database(file_name);
    fossil::bluecrab::MyShell::create_record(file_name, "key1", "cstr:hello");

    fossil_bluecrab_myshell_error_t result = fossil::bluecrab::MyShell::backup_database(file_name, backup_file);
    ASSUME_ITS_TRUE(result == FOSSIL_MYSHELL_ERROR_SUCCESS);

    fossil::bluecrab::MyShell::delete_database(file_name);
    result = fossil::bluecrab::MyShell::restore_database(backup_file, file_name);
    ASSUME_ITS_TRUE(result == FOSSIL_MYSHELL_ERROR_SUCCESS);

    std::string value;
    result = fossil::bluecrab::MyShell::read_record(file_name, "key1", value);
    ASSUME_ITS_TRUE(result == FOSSIL_MYSHELL_ERROR_SUCCESS);
    ASSUME_ITS_TRUE(value == "cstr:hello");

    fossil::bluecrab::MyShell::delete_database(file_name);
    fossil::bluecrab::MyShell::delete_database(backup_file);
}

/*
 * Test case for validating the file extension of a database file
 */
FOSSIL_TEST(cpp_test_myshell_validate_extension) {
    ASSUME_ITS_TRUE(fossil::bluecrab::MyShell::validate_extension("test.myshell"));
    ASSUME_ITS_FALSE(fossil::bluecrab::MyShell::validate_extension("test.txt"));
}

/*
 * Test case for validating data
 */
FOSSIL_TEST(cpp_test_myshell_validate_data) {
    ASSUME_ITS_TRUE(fossil::bluecrab::MyShell::validate_data("valid_data"));
    ASSUME_ITS_FALSE(fossil::bluecrab::MyShell::validate_data(""));
}

/*
 * Test case for record counting
 */
FOSSIL_TEST(cpp_test_myshell_count_records) {
    std::string file_name = "test.myshell";
    fossil::bluecrab::MyShell::create_database(file_name);
    fossil::bluecrab::MyShell::create_record(file_name, "key1", "i32:1");
    fossil::bluecrab::MyShell::create_record(file_name, "key2", "i32:2");
    fossil::bluecrab::MyShell::create_record(file_name, "key3", "i32:3");

    size_t count = 0;
    fossil_bluecrab_myshell_error_t result = fossil::bluecrab::MyShell::count_records(file_name, count);
    ASSUME_ITS_TRUE(result == FOSSIL_MYSHELL_ERROR_SUCCESS);
    ASSUME_ITS_TRUE(count == 3);

    fossil::bluecrab::MyShell::delete_database(file_name);
}

/*
 * Test case for file size
 */
FOSSIL_TEST(cpp_test_myshell_get_file_size) {
    std::string file_name = "test.myshell";
    fossil::bluecrab::MyShell::create_database(file_name);
    fossil::bluecrab::MyShell::create_record(file_name, "key1", "i32:1");

    size_t size_bytes = 0;
    fossil_bluecrab_myshell_error_t result = fossil::bluecrab::MyShell::get_file_size(file_name, size_bytes);
    ASSUME_ITS_TRUE(result == FOSSIL_MYSHELL_ERROR_SUCCESS);
    ASSUME_ITS_TRUE(size_bytes > 0);

    fossil::bluecrab::MyShell::delete_database(file_name);
}

/*
 * Test case for error string conversion
 */
FOSSIL_TEST(cpp_test_myshell_error_string) {
    ASSUME_ITS_TRUE(fossil::bluecrab::MyShell::error_string(FOSSIL_MYSHELL_ERROR_SUCCESS) == "Success");
    ASSUME_ITS_TRUE(fossil::bluecrab::MyShell::error_string(FOSSIL_MYSHELL_ERROR_NOT_FOUND) == "Record not found");
}

// * * * * * * * * * * * * * * * * * * * * * * * *
// * Fossil Logic Test Pool
// * * * * * * * * * * * * * * * * * * * * * * * *
FOSSIL_TEST_GROUP(cpp_myshell_database_tests) {
    FOSSIL_TEST_ADD(cpp_myshell_fixture, cpp_test_myshell_create_record);
    FOSSIL_TEST_ADD(cpp_myshell_fixture, cpp_test_myshell_read_nonexistent_record);
    FOSSIL_TEST_ADD(cpp_myshell_fixture, cpp_test_myshell_update_record);
    FOSSIL_TEST_ADD(cpp_myshell_fixture, cpp_test_myshell_update_nonexistent_record);
    FOSSIL_TEST_ADD(cpp_myshell_fixture, cpp_test_myshell_delete_record);
    FOSSIL_TEST_ADD(cpp_myshell_fixture, cpp_test_myshell_delete_nonexistent_record);
    FOSSIL_TEST_ADD(cpp_myshell_fixture, cpp_test_myshell_backup_restore);
    FOSSIL_TEST_ADD(cpp_myshell_fixture, cpp_test_myshell_validate_extension);
    FOSSIL_TEST_ADD(cpp_myshell_fixture, cpp_test_myshell_validate_data);
    FOSSIL_TEST_ADD(cpp_myshell_fixture, cpp_test_myshell_count_records);
    FOSSIL_TEST_ADD(cpp_myshell_fixture, cpp_test_myshell_get_file_size);
    FOSSIL_TEST_ADD(cpp_myshell_fixture, cpp_test_myshell_error_string);

    FOSSIL_TEST_REGISTER(cpp_myshell_fixture);
} // end of tests

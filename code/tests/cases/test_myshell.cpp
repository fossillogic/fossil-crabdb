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

// Test case for creating a new record in the database file
FOSSIL_TEST(cpp_test_myshell_create_record) {
    const std::string file_name = "test.crabdb";
    fossil::bluecrab::MyShell::create_database(file_name);
    fossil_bluecrab_myshell_error_t result = fossil::bluecrab::MyShell::create_record(file_name, "key1", "value1");
    ASSUME_ITS_TRUE(result == FOSSIL_MYSHELL_ERROR_SUCCESS);

    std::string value;
    result = fossil::bluecrab::MyShell::read_record(file_name, "key1", value);
    ASSUME_ITS_TRUE(result == FOSSIL_MYSHELL_ERROR_SUCCESS);
    ASSUME_ITS_TRUE(value == "value1");

    fossil::bluecrab::MyShell::delete_database(file_name);
}

// Test case for reading a non-existent record from the database file
FOSSIL_TEST(cpp_test_myshell_read_nonexistent_record) {
    const std::string file_name = "test.crabdb";
    fossil::bluecrab::MyShell::create_database(file_name);

    std::string value;
    fossil_bluecrab_myshell_error_t result = fossil::bluecrab::MyShell::read_record(file_name, "nonexistent_key", value);
    ASSUME_ITS_TRUE(result == FOSSIL_MYSHELL_ERROR_NOT_FOUND);

    fossil::bluecrab::MyShell::delete_database(file_name);
}

// Test case for updating a non-existent record in the database file
FOSSIL_TEST(cpp_test_myshell_update_nonexistent_record) {
    const std::string file_name = "test.crabdb";
    fossil::bluecrab::MyShell::create_database(file_name);

    fossil_bluecrab_myshell_error_t result = fossil::bluecrab::MyShell::update_record(file_name, "nonexistent_key", "new_value");
    ASSUME_ITS_TRUE(result == FOSSIL_MYSHELL_ERROR_NOT_FOUND);

    fossil::bluecrab::MyShell::delete_database(file_name);
}

// Test case for deleting a non-existent record from the database file
FOSSIL_TEST(cpp_test_myshell_delete_nonexistent_record) {
    const std::string file_name = "test.crabdb";
    fossil::bluecrab::MyShell::create_database(file_name);

    fossil_bluecrab_myshell_error_t result = fossil::bluecrab::MyShell::delete_record(file_name, "nonexistent_key");
    ASSUME_ITS_TRUE(result == FOSSIL_MYSHELL_ERROR_NOT_FOUND);

    fossil::bluecrab::MyShell::delete_database(file_name);
}

// Test case for backing up and restoring a database file
FOSSIL_TEST(cpp_test_myshell_backup_restore) {
    const std::string file_name = "test.crabdb";
    const std::string backup_file = "backup.crabdb";
    fossil::bluecrab::MyShell::create_database(file_name);
    fossil::bluecrab::MyShell::create_record(file_name, "key1", "value1");

    fossil_bluecrab_myshell_error_t result = fossil::bluecrab::MyShell::backup_database(file_name, backup_file);
    ASSUME_ITS_TRUE(result == FOSSIL_MYSHELL_ERROR_SUCCESS);

    fossil::bluecrab::MyShell::delete_database(file_name);
    result = fossil::bluecrab::MyShell::restore_database(backup_file, file_name);
    ASSUME_ITS_TRUE(result == FOSSIL_MYSHELL_ERROR_SUCCESS);

    std::string value;
    result = fossil::bluecrab::MyShell::read_record(file_name, "key1", value);
    ASSUME_ITS_TRUE(result == FOSSIL_MYSHELL_ERROR_SUCCESS);
    ASSUME_ITS_TRUE(value == "value1");

    fossil::bluecrab::MyShell::delete_database(file_name);
    fossil::bluecrab::MyShell::delete_database(backup_file);
}

// Test case for validating the file extension of a database file
FOSSIL_TEST(cpp_test_myshell_validate_extension) {
    ASSUME_ITS_TRUE(fossil::bluecrab::MyShell::validate_extension("test.crabdb"));
    ASSUME_ITS_FALSE(fossil::bluecrab::MyShell::validate_extension("test.txt"));
}

// Test case for validating data
FOSSIL_TEST(cpp_test_myshell_validate_data) {
    ASSUME_ITS_TRUE(fossil::bluecrab::MyShell::validate_data("valid_data"));
    ASSUME_ITS_FALSE(fossil::bluecrab::MyShell::validate_data(""));
}

// * * * * * * * * * * * * * * * * * * * * * * * *
// * Fossil Logic Test Pool
// * * * * * * * * * * * * * * * * * * * * * * * *
FOSSIL_TEST_GROUP(cpp_myshell_database_tests) {
    FOSSIL_TEST_ADD(cpp_myshell_fixture, cpp_test_myshell_create_record);
    FOSSIL_TEST_ADD(cpp_myshell_fixture, cpp_test_myshell_read_nonexistent_record);
    FOSSIL_TEST_ADD(cpp_myshell_fixture, cpp_test_myshell_update_nonexistent_record);
    FOSSIL_TEST_ADD(cpp_myshell_fixture, cpp_test_myshell_delete_nonexistent_record);
    FOSSIL_TEST_ADD(cpp_myshell_fixture, cpp_test_myshell_backup_restore);
    FOSSIL_TEST_ADD(cpp_myshell_fixture, cpp_test_myshell_validate_extension);
    FOSSIL_TEST_ADD(cpp_myshell_fixture, cpp_test_myshell_validate_data);

    FOSSIL_TEST_REGISTER(cpp_myshell_fixture);
} // end of tests

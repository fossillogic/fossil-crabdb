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
#include <fossil/test/framework.h>

#include "fossil/crabdb/framework.h"
#include <string>

// * * * * * * * * * * * * * * * * * * * * * * * *
// * Fossil Logic Test Utilities
// * * * * * * * * * * * * * * * * * * * * * * * *
// Setup steps for things like test fixtures and
// mock objects are set here.
// * * * * * * * * * * * * * * * * * * * * * * * *

FOSSIL_TEST_SUITE(cpp_myshell_fixture);

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
FOSSIL_TEST_CASE(cpp_test_myshell_create_record) {
    std::string file_name = "test.crabdb";
    fossil_myshell_create_database(file_name.c_str());
    fossil_myshell_error_t result = fossil_myshell_create_record(file_name.c_str(), "key1", "value1");
    ASSUME_ITS_TRUE(result == FOSSIL_MYSHELL_ERROR_SUCCESS);

    char value[256];
    result = fossil_myshell_read_record(file_name.c_str(), "key1", value, sizeof(value));
    ASSUME_ITS_TRUE(result == FOSSIL_MYSHELL_ERROR_SUCCESS);
    ASSUME_ITS_TRUE(strcmp(value, "value1") == 0);

    fossil_myshell_delete_database(file_name.c_str());
}

// Test case for reading a non-existent record from the database file
FOSSIL_TEST_CASE(cpp_test_myshell_read_nonexistent_record) {
    std::string file_name = "test.crabdb";
    fossil_myshell_create_database(file_name.c_str());

    char value[256];
    fossil_myshell_error_t result = fossil_myshell_read_record(file_name.c_str(), "nonexistent_key", value, sizeof(value));
    ASSUME_ITS_TRUE(result == FOSSIL_MYSHELL_ERROR_NOT_FOUND);

    fossil_myshell_delete_database(file_name.c_str());
}

// Test case for updating a non-existent record in the database file
FOSSIL_TEST_CASE(cpp_test_myshell_update_nonexistent_record) {
    std::string file_name = "test.crabdb";
    fossil_myshell_create_database(file_name.c_str());

    fossil_myshell_error_t result = fossil_myshell_update_record(file_name.c_str(), "nonexistent_key", "new_value");
    ASSUME_ITS_TRUE(result == FOSSIL_MYSHELL_ERROR_NOT_FOUND);

    fossil_myshell_delete_database(file_name.c_str());
}

// Test case for deleting a non-existent record from the database file
FOSSIL_TEST_CASE(cpp_test_myshell_delete_nonexistent_record) {
    std::string file_name = "test.crabdb";
    fossil_myshell_create_database(file_name.c_str());

    fossil_myshell_error_t result = fossil_myshell_delete_record(file_name.c_str(), "nonexistent_key");
    ASSUME_ITS_TRUE(result == FOSSIL_MYSHELL_ERROR_NOT_FOUND);

    fossil_myshell_delete_database(file_name.c_str());
}

// Test case for backing up and restoring a database file
FOSSIL_TEST_CASE(cpp_test_myshell_backup_restore) {
    std::string file_name = "test.crabdb";
    std::string backup_file = "backup.crabdb";
    fossil_myshell_create_database(file_name.c_str());
    fossil_myshell_create_record(file_name.c_str(), "key1", "value1");

    fossil_myshell_error_t result = fossil_myshell_backup_database(file_name.c_str(), backup_file.c_str());
    ASSUME_ITS_TRUE(result == FOSSIL_MYSHELL_ERROR_SUCCESS);

    fossil_myshell_delete_database(file_name.c_str());
    result = fossil_myshell_restore_database(backup_file.c_str(), file_name.c_str());
    ASSUME_ITS_TRUE(result == FOSSIL_MYSHELL_ERROR_SUCCESS);

    char value[256];
    result = fossil_myshell_read_record(file_name.c_str(), "key1", value, sizeof(value));
    ASSUME_ITS_TRUE(result == FOSSIL_MYSHELL_ERROR_SUCCESS);
    ASSUME_ITS_TRUE(strcmp(value, "value1") == 0);

    fossil_myshell_delete_database(file_name.c_str());
    fossil_myshell_delete_database(backup_file.c_str());
}

// Test case for validating the file extension of a database file
FOSSIL_TEST_CASE(cpp_test_myshell_validate_extension) {
    ASSUME_ITS_TRUE(fossil_myshell_validate_extension("test.crabdb"));
    ASSUME_ITS_FALSE(fossil_myshell_validate_extension("test.txt"));
}

// Test case for validating data
FOSSIL_TEST_CASE(cpp_test_myshell_validate_data) {
    ASSUME_ITS_TRUE(fossil_myshell_validate_data("valid_data"));
    ASSUME_ITS_FALSE(fossil_myshell_validate_data(NULL));
    ASSUME_ITS_FALSE(fossil_myshell_validate_data(""));
}

// Test case for creating a new record using MyShell class
FOSSIL_TEST_CASE(cpp_test_myshell_class_create_record) {
    std::string file_name = "test.crabdb";
    fossil::MyShell::createDatabase(file_name);
    fossil_myshell_error_t result = fossil::MyShell::createRecord(file_name, "key1", "value1");
    ASSUME_ITS_TRUE(result == FOSSIL_MYSHELL_ERROR_SUCCESS);

    std::string value;
    result = fossil::MyShell::readRecord(file_name, "key1", value);
    ASSUME_ITS_TRUE(result == FOSSIL_MYSHELL_ERROR_SUCCESS);
    ASSUME_ITS_TRUE(value == "value1");

    fossil::MyShell::deleteDatabase(file_name);
}

// Test case for reading a non-existent record using MyShell class
FOSSIL_TEST_CASE(cpp_test_myshell_class_read_nonexistent_record) {
    std::string file_name = "test.crabdb";
    fossil::MyShell::createDatabase(file_name);

    std::string value;
    fossil_myshell_error_t result = fossil::MyShell::readRecord(file_name, "nonexistent_key", value);
    ASSUME_ITS_TRUE(result == FOSSIL_MYSHELL_ERROR_NOT_FOUND);

    fossil::MyShell::deleteDatabase(file_name);
}

// Test case for updating a non-existent record using MyShell class
FOSSIL_TEST_CASE(cpp_test_myshell_class_update_nonexistent_record) {
    std::string file_name = "test.crabdb";
    fossil::MyShell::createDatabase(file_name);

    fossil_myshell_error_t result = fossil::MyShell::updateRecord(file_name, "nonexistent_key", "new_value");
    ASSUME_ITS_TRUE(result == FOSSIL_MYSHELL_ERROR_NOT_FOUND);

    fossil::MyShell::deleteDatabase(file_name);
}

// Test case for deleting a non-existent record using MyShell class
FOSSIL_TEST_CASE(cpp_test_myshell_class_delete_nonexistent_record) {
    std::string file_name = "test.crabdb";
    fossil::MyShell::createDatabase(file_name);

    fossil_myshell_error_t result = fossil::MyShell::deleteRecord(file_name, "nonexistent_key");
    ASSUME_ITS_TRUE(result == FOSSIL_MYSHELL_ERROR_NOT_FOUND);

    fossil::MyShell::deleteDatabase(file_name);
}

// Test case for backing up and restoring a database using MyShell class
FOSSIL_TEST_CASE(cpp_test_myshell_class_backup_restore) {
    std::string file_name = "test.crabdb";
    std::string backup_file = "backup.crabdb";
    fossil::MyShell::createDatabase(file_name);
    fossil::MyShell::createRecord(file_name, "key1", "value1");

    fossil_myshell_error_t result = fossil::MyShell::backupDatabase(file_name, backup_file);
    ASSUME_ITS_TRUE(result == FOSSIL_MYSHELL_ERROR_SUCCESS);

    fossil::MyShell::deleteDatabase(file_name);
    result = fossil::MyShell::restoreDatabase(backup_file, file_name);
    ASSUME_ITS_TRUE(result == FOSSIL_MYSHELL_ERROR_SUCCESS);

    std::string value;
    result = fossil::MyShell::readRecord(file_name, "key1", value);
    ASSUME_ITS_TRUE(result == FOSSIL_MYSHELL_ERROR_SUCCESS);
    ASSUME_ITS_TRUE(value == "value1");

    fossil::MyShell::deleteDatabase(file_name);
    fossil::MyShell::deleteDatabase(backup_file);
}

// Test case for validating the file extension using MyShell class
FOSSIL_TEST_CASE(cpp_test_myshell_class_validate_extension) {
    ASSUME_ITS_TRUE(fossil::MyShell::validateExtension("test.crabdb"));
    ASSUME_ITS_FALSE(fossil::MyShell::validateExtension("test.txt"));
}

// Test case for validating data using MyShell class
FOSSIL_TEST_CASE(cpp_test_myshell_class_validate_data) {
    ASSUME_ITS_TRUE(fossil::MyShell::validateData("valid_data"));
    ASSUME_ITS_FALSE(fossil::MyShell::validateData(NULL));
    ASSUME_ITS_FALSE(fossil::MyShell::validateData(""));
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
    FOSSIL_TEST_ADD(cpp_myshell_fixture, cpp_test_myshell_class_create_record);
    FOSSIL_TEST_ADD(cpp_myshell_fixture, cpp_test_myshell_class_read_nonexistent_record);
    FOSSIL_TEST_ADD(cpp_myshell_fixture, cpp_test_myshell_class_update_nonexistent_record);
    FOSSIL_TEST_ADD(cpp_myshell_fixture, cpp_test_myshell_class_delete_nonexistent_record);
    FOSSIL_TEST_ADD(cpp_myshell_fixture, cpp_test_myshell_class_backup_restore);
    FOSSIL_TEST_ADD(cpp_myshell_fixture, cpp_test_myshell_class_validate_extension);
    FOSSIL_TEST_ADD(cpp_myshell_fixture, cpp_test_myshell_class_validate_data);

    FOSSIL_TEST_REGISTER(cpp_myshell_fixture);
} // end of tests

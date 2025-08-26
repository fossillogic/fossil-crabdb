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
#include <string>

// * * * * * * * * * * * * * * * * * * * * * * * *
// * Fossil Logic Test Utilities
// * * * * * * * * * * * * * * * * * * * * * * * *
// Setup steps for things like test fixtures and
// mock objects are set here.
// * * * * * * * * * * * * * * * * * * * * * * * *

FOSSIL_SUITE(cpp_noshell_fixture);

FOSSIL_SETUP(cpp_noshell_fixture) {
    // Setup the test fixture
}

FOSSIL_TEARDOWN(cpp_noshell_fixture) {
    // Teardown the test fixture
}

// * * * * * * * * * * * * * * * * * * * * * * * *
// * Fossil Logic Test Blue CrabDB Database
// * * * * * * * * * * * * * * * * * * * * * * * *

// Test case for creating a new database
FOSSIL_TEST(cpp_test_noshell_create_database) {
    fossil_noshell_error_t result = fossil_noshell_create_database("test.crabdb");
    ASSUME_ITS_TRUE(result == FOSSIL_NOSHELL_ERROR_SUCCESS);
    std::remove("test.crabdb");
}

// Test case for opening an existing database
FOSSIL_TEST(cpp_test_noshell_open_database) {
    FILE *file = fopen("test.crabdb", "w");
    fclose(file);
    fossil_noshell_error_t result = fossil_noshell_open_database("test.crabdb");
    ASSUME_ITS_TRUE(result == FOSSIL_NOSHELL_ERROR_SUCCESS);
    std::remove("test.crabdb");
}

// Test case for deleting a database
FOSSIL_TEST(cpp_test_noshell_delete_database) {
    FILE *file = fopen("test.crabdb", "w");
    fclose(file);
    fossil_noshell_error_t result = fossil_noshell_delete_database("test.crabdb");
    ASSUME_ITS_TRUE(result == FOSSIL_NOSHELL_ERROR_SUCCESS);
}

// Test case for inserting a document into the database
FOSSIL_TEST(cpp_test_noshell_insert_document) {
    FILE *file = fopen("test.crabdb", "w");
    fclose(file);
    fossil_noshell_error_t result = fossil_noshell_insert("test.crabdb", "document1");
    ASSUME_ITS_TRUE(result == FOSSIL_NOSHELL_ERROR_SUCCESS);
    std::remove("test.crabdb");
}

// Test case for finding a document in the database
FOSSIL_TEST(cpp_test_noshell_find_document) {
    FILE *file = fopen("test.crabdb", "w");
    fprintf(file, "document1\n");
    fclose(file);
    char result[256];
    fossil_noshell_error_t error = fossil_noshell_find("test.crabdb", "document1", result, sizeof(result));
    ASSUME_ITS_TRUE(error == FOSSIL_NOSHELL_ERROR_SUCCESS);
    ASSUME_ITS_TRUE(strcmp(result, "document1\n") == 0);
    std::remove("test.crabdb");
}

// Test case for updating a document in the database
FOSSIL_TEST(cpp_test_noshell_update_document) {
    FILE *file = fopen("test.crabdb", "w");
    fprintf(file, "document1\n");
    fclose(file);
    fossil_noshell_error_t result = fossil_noshell_update("test.crabdb", "document1", "new_document1");
    ASSUME_ITS_TRUE(result == FOSSIL_NOSHELL_ERROR_SUCCESS);
    char updated_result[256];
    fossil_noshell_find("test.crabdb", "new_document1", updated_result, sizeof(updated_result));
    ASSUME_ITS_TRUE(strcmp(updated_result, "new_document1\n") == 0);
    std::remove("test.crabdb");
}

// Test case for removing a document from the database
FOSSIL_TEST(cpp_test_noshell_remove_document) {
    FILE *file = fopen("test.crabdb", "w");
    fprintf(file, "document1\n");
    fclose(file);
    fossil_noshell_error_t result = fossil_noshell_remove("test.crabdb", "document1");
    ASSUME_ITS_TRUE(result == FOSSIL_NOSHELL_ERROR_SUCCESS);
    char removed_result[256];
    fossil_noshell_error_t error = fossil_noshell_find("test.crabdb", "document1", removed_result, sizeof(removed_result));
    ASSUME_ITS_TRUE(error == FOSSIL_NOSHELL_ERROR_NOT_FOUND);
    std::remove("test.crabdb");
}

// Test case for backing up a database
FOSSIL_TEST(cpp_test_noshell_backup_database) {
    FILE *file = fopen("test.crabdb", "w");
    fprintf(file, "document1\n");
    fclose(file);
    fossil_noshell_error_t result = fossil_noshell_backup_database("test.crabdb", "backup.crabdb");
    ASSUME_ITS_TRUE(result == FOSSIL_NOSHELL_ERROR_SUCCESS);
    FILE *backup = fopen("backup.crabdb", "r");
    ASSUME_NOT_CNULL(backup);
    char buffer[256];
    (void)fgets(buffer, sizeof(buffer), backup);
    ASSUME_ITS_TRUE(strcmp(buffer, "document1\n") == 0);
    fclose(backup);
    std::remove("test.crabdb");
    std::remove("backup.crabdb");
}

// Test case for restoring a database from a backup
FOSSIL_TEST(cpp_test_noshell_restore_database) {
    FILE *backup = fopen("backup.crabdb", "w");
    fprintf(backup, "document1\n");
    fclose(backup);
    fossil_noshell_error_t result = fossil_noshell_restore_database("backup.crabdb", "restored.crabdb");
    ASSUME_ITS_TRUE(result == FOSSIL_NOSHELL_ERROR_SUCCESS);
    FILE *restored = fopen("restored.crabdb", "r");
    ASSUME_NOT_CNULL(restored);
    char buffer[256];
    (void)fgets(buffer, sizeof(buffer), restored);
    ASSUME_ITS_TRUE(strcmp(buffer, "document1\n") == 0);
    fclose(restored);
    std::remove("backup.crabdb");
    std::remove("restored.crabdb");
}

// Test case for validating the file extension
FOSSIL_TEST(cpp_test_noshell_validate_extension) {
    bool result = fossil_noshell_validate_extension("test.crabdb");
    ASSUME_ITS_TRUE(result);
    result = fossil_noshell_validate_extension("test.txt");
    ASSUME_ITS_FALSE(result);
}

// Test case for validating a document
FOSSIL_TEST(cpp_test_noshell_validate_document) {
    bool result = fossil_noshell_validate_document("document1");
    ASSUME_ITS_TRUE(result);
    result = fossil_noshell_validate_document("");
    ASSUME_ITS_FALSE(result);
    result = fossil_noshell_validate_document(NULL);
    ASSUME_ITS_FALSE(result);
}

// Test case for creating a new database using NoShell class
FOSSIL_TEST(cpp_test_noshell_class_create_database) {
    fossil_noshell_error_t result = fossil::NoShell::createDatabase("test_class.crabdb");
    ASSUME_ITS_TRUE(result == FOSSIL_NOSHELL_ERROR_SUCCESS);
    std::remove("test_class.crabdb");
}

// Test case for opening an existing database using NoShell class
FOSSIL_TEST(cpp_test_noshell_class_open_database) {
    FILE *file = fopen("test_class.crabdb", "w");
    fclose(file);
    fossil_noshell_error_t result = fossil::NoShell::openDatabase("test_class.crabdb");
    ASSUME_ITS_TRUE(result == FOSSIL_NOSHELL_ERROR_SUCCESS);
    std::remove("test_class.crabdb");
}

// Test case for deleting a database using NoShell class
FOSSIL_TEST(cpp_test_noshell_class_delete_database) {
    FILE *file = fopen("test_class.crabdb", "w");
    fclose(file);
    fossil_noshell_error_t result = fossil::NoShell::deleteDatabase("test_class.crabdb");
    ASSUME_ITS_TRUE(result == FOSSIL_NOSHELL_ERROR_SUCCESS);
}

// Test case for inserting a document into the database using NoShell class
FOSSIL_TEST(cpp_test_noshell_class_insert_document) {
    FILE *file = fopen("test_class.crabdb", "w");
    fclose(file);
    fossil_noshell_error_t result = fossil::NoShell::insert("test_class.crabdb", "document1");
    ASSUME_ITS_TRUE(result == FOSSIL_NOSHELL_ERROR_SUCCESS);
    std::remove("test_class.crabdb");
}

// Test case for finding a document in the database using NoShell class
FOSSIL_TEST(cpp_test_noshell_class_find_document) {
    FILE *file = fopen("test_class.crabdb", "w");
    fprintf(file, "document1\n");
    fclose(file);
    std::string result;
    fossil_noshell_error_t error = fossil::NoShell::find("test_class.crabdb", "document1", result);
    ASSUME_ITS_TRUE(error == FOSSIL_NOSHELL_ERROR_SUCCESS);
    ASSUME_ITS_TRUE(result == "document1\n");
    std::remove("test_class.crabdb");
}

// Test case for updating a document in the database using NoShell class
FOSSIL_TEST(cpp_test_noshell_class_update_document) {
    FILE *file = fopen("test_class.crabdb", "w");
    fprintf(file, "document1\n");
    fclose(file);
    fossil_noshell_error_t result = fossil::NoShell::update("test_class.crabdb", "document1", "new_document1");
    ASSUME_ITS_TRUE(result == FOSSIL_NOSHELL_ERROR_SUCCESS);
    std::string updated_result;
    fossil::NoShell::find("test_class.crabdb", "new_document1", updated_result);
    ASSUME_ITS_TRUE(updated_result == "new_document1\n");
    std::remove("test_class.crabdb");
}

// Test case for removing a document from the database using NoShell class
FOSSIL_TEST(cpp_test_noshell_class_remove_document) {
    FILE *file = fopen("test_class.crabdb", "w");
    fprintf(file, "document1\n");
    fclose(file);
    fossil_noshell_error_t result = fossil::NoShell::remove("test_class.crabdb", "document1");
    ASSUME_ITS_TRUE(result == FOSSIL_NOSHELL_ERROR_SUCCESS);
    std::string removed_result;
    fossil_noshell_error_t error = fossil::NoShell::find("test_class.crabdb", "document1", removed_result);
    ASSUME_ITS_TRUE(error == FOSSIL_NOSHELL_ERROR_NOT_FOUND);
    std::remove("test_class.crabdb");
}

// Test case for backing up a database using NoShell class
FOSSIL_TEST(cpp_test_noshell_class_backup_database) {
    FILE *file = fopen("test_class.crabdb", "w");
    fprintf(file, "document1\n");
    fclose(file);
    fossil_noshell_error_t result = fossil::NoShell::backupDatabase("test_class.crabdb", "backup_class.crabdb");
    ASSUME_ITS_TRUE(result == FOSSIL_NOSHELL_ERROR_SUCCESS);
    FILE *backup = fopen("backup_class.crabdb", "r");
    ASSUME_NOT_CNULL(backup);
    char buffer[256];
    (void)fgets(buffer, sizeof(buffer), backup);
    ASSUME_ITS_TRUE(strcmp(buffer, "document1\n") == 0);
    fclose(backup);
    std::remove("test_class.crabdb");
    std::remove("backup_class.crabdb");
}

// Test case for restoring a database from a backup using NoShell class
FOSSIL_TEST(cpp_test_noshell_class_restore_database) {
    FILE *backup = fopen("backup_class.crabdb", "w");
    fprintf(backup, "document1\n");
    fclose(backup);
    fossil_noshell_error_t result = fossil::NoShell::restoreDatabase("backup_class.crabdb", "restored_class.crabdb");
    ASSUME_ITS_TRUE(result == FOSSIL_NOSHELL_ERROR_SUCCESS);
    FILE *restored = fopen("restored_class.crabdb", "r");
    ASSUME_NOT_CNULL(restored);
    char buffer[256];
    (void)fgets(buffer, sizeof(buffer), restored);
    ASSUME_ITS_TRUE(strcmp(buffer, "document1\n") == 0);
    fclose(restored);
    std::remove("backup_class.crabdb");
    std::remove("restored_class.crabdb");
}

// Test case for validating the file extension using NoShell class
FOSSIL_TEST(cpp_test_noshell_class_validate_extension) {
    bool result = fossil::NoShell::validateExtension("test_class.crabdb");
    ASSUME_ITS_TRUE(result);
    result = fossil::NoShell::validateExtension("test.txt");
    ASSUME_ITS_FALSE(result);
}

// * * * * * * * * * * * * * * * * * * * * * * * *
// * Fossil Logic Test Pool
// * * * * * * * * * * * * * * * * * * * * * * * *
FOSSIL_TEST_GROUP(cpp_noshell_database_tests) {
    FOSSIL_TEST_ADD(cpp_noshell_fixture, cpp_test_noshell_create_database);
    FOSSIL_TEST_ADD(cpp_noshell_fixture, cpp_test_noshell_open_database);
    FOSSIL_TEST_ADD(cpp_noshell_fixture, cpp_test_noshell_delete_database);
    FOSSIL_TEST_ADD(cpp_noshell_fixture, cpp_test_noshell_insert_document);
    FOSSIL_TEST_ADD(cpp_noshell_fixture, cpp_test_noshell_find_document);
    FOSSIL_TEST_ADD(cpp_noshell_fixture, cpp_test_noshell_update_document);
    FOSSIL_TEST_ADD(cpp_noshell_fixture, cpp_test_noshell_remove_document);
    FOSSIL_TEST_ADD(cpp_noshell_fixture, cpp_test_noshell_backup_database);
    FOSSIL_TEST_ADD(cpp_noshell_fixture, cpp_test_noshell_restore_database);
    FOSSIL_TEST_ADD(cpp_noshell_fixture, cpp_test_noshell_validate_extension);
    FOSSIL_TEST_ADD(cpp_noshell_fixture, cpp_test_noshell_validate_document);

    // Adding new class-based test cases
    FOSSIL_TEST_ADD(cpp_noshell_fixture, cpp_test_noshell_class_create_database);
    FOSSIL_TEST_ADD(cpp_noshell_fixture, cpp_test_noshell_class_open_database);
    FOSSIL_TEST_ADD(cpp_noshell_fixture, cpp_test_noshell_class_delete_database);
    FOSSIL_TEST_ADD(cpp_noshell_fixture, cpp_test_noshell_class_insert_document);
    FOSSIL_TEST_ADD(cpp_noshell_fixture, cpp_test_noshell_class_find_document);
    FOSSIL_TEST_ADD(cpp_noshell_fixture, cpp_test_noshell_class_update_document);
    FOSSIL_TEST_ADD(cpp_noshell_fixture, cpp_test_noshell_class_remove_document);
    FOSSIL_TEST_ADD(cpp_noshell_fixture, cpp_test_noshell_class_backup_database);
    FOSSIL_TEST_ADD(cpp_noshell_fixture, cpp_test_noshell_class_restore_database);
    FOSSIL_TEST_ADD(cpp_noshell_fixture, cpp_test_noshell_class_validate_extension);

    FOSSIL_TEST_REGISTER(cpp_noshell_fixture);
} // end of tests

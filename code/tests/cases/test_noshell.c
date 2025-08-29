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

FOSSIL_SUITE(c_noshell_fixture);

FOSSIL_SETUP(c_noshell_fixture) {
    // Setup the test fixture
}

FOSSIL_TEARDOWN(c_noshell_fixture) {
    // Teardown the test fixture
}

// * * * * * * * * * * * * * * * * * * * * * * * *
// * Fossil Logic Test Blue CrabDB Database
// * * * * * * * * * * * * * * * * * * * * * * * *

// Test case for creating a new database
FOSSIL_TEST(c_test_noshell_create_database) {
    fossil_noshell_error_t result = fossil_noshell_create_database("test.crabdb");
    ASSUME_ITS_TRUE(result == FOSSIL_NOSHELL_ERROR_SUCCESS);
    remove("test.crabdb");
}

// Test case for opening an existing database
FOSSIL_TEST(c_test_noshell_open_database) {
    FILE *file = fopen("test.crabdb", "w");
    fclose(file);
    fossil_noshell_error_t result = fossil_noshell_open_database("test.crabdb");
    ASSUME_ITS_TRUE(result == FOSSIL_NOSHELL_ERROR_SUCCESS);
    remove("test.crabdb");
}

// Test case for deleting a database
FOSSIL_TEST(c_test_noshell_delete_database) {
    FILE *file = fopen("test.crabdb", "w");
    fclose(file);
    fossil_noshell_error_t result = fossil_noshell_delete_database("test.crabdb");
    ASSUME_ITS_TRUE(result == FOSSIL_NOSHELL_ERROR_SUCCESS);
}

// Test case for inserting a document into the database
FOSSIL_TEST(c_test_noshell_insert_document) {
    FILE *file = fopen("test.crabdb", "w");
    fclose(file);
    fossil_noshell_error_t result = fossil_noshell_insert("test.crabdb", "document1");
    ASSUME_ITS_TRUE(result == FOSSIL_NOSHELL_ERROR_SUCCESS);
    remove("test.crabdb");
}

// Test case for finding a document in the database
FOSSIL_TEST(c_test_noshell_find_document) {
    FILE *file = fopen("test.crabdb", "w");
    fprintf(file, "document1\n");
    fclose(file);
    char result[256];
    fossil_noshell_error_t error = fossil_noshell_find("test.crabdb", "document1", result, sizeof(result));
    ASSUME_ITS_TRUE(error == FOSSIL_NOSHELL_ERROR_SUCCESS);
    ASSUME_ITS_TRUE(strcmp(result, "document1\n") == 0);
    remove("test.crabdb");
}

// Test case for updating a document in the database
FOSSIL_TEST(c_test_noshell_update_document) {
    FILE *file = fopen("test.crabdb", "w");
    fprintf(file, "document1\n");
    fclose(file);
    fossil_noshell_error_t result = fossil_noshell_update("test.crabdb", "document1", "new_document1");
    ASSUME_ITS_TRUE(result == FOSSIL_NOSHELL_ERROR_SUCCESS);
    char updated_result[256];
    fossil_noshell_find("test.crabdb", "new_document1", updated_result, sizeof(updated_result));
    ASSUME_ITS_TRUE(strcmp(updated_result, "new_document1\n") == 0);
    remove("test.crabdb");
}

// Test case for removing a document from the database
FOSSIL_TEST(c_test_noshell_remove_document) {
    FILE *file = fopen("test.crabdb", "w");
    fprintf(file, "document1\n");
    fclose(file);
    fossil_noshell_error_t result = fossil_noshell_remove("test.crabdb", "document1");
    ASSUME_ITS_TRUE(result == FOSSIL_NOSHELL_ERROR_SUCCESS);
    char removed_result[256];
    fossil_noshell_error_t error = fossil_noshell_find("test.crabdb", "document1", removed_result, sizeof(removed_result));
    ASSUME_ITS_TRUE(error == FOSSIL_NOSHELL_ERROR_NOT_FOUND);
    remove("test.crabdb");
}

// Test case for validating the file extension
FOSSIL_TEST(c_test_noshell_validate_extension) {
    bool result = fossil_noshell_validate_extension("test.crabdb");
    ASSUME_ITS_TRUE(result);
    result = fossil_noshell_validate_extension("test.txt");
    ASSUME_ITS_FALSE(result);
}

// Test case for validating a document
FOSSIL_TEST(c_test_noshell_validate_document) {
    bool result = fossil_noshell_validate_document("document1");
    ASSUME_ITS_TRUE(result);
    result = fossil_noshell_validate_document("");
    ASSUME_ITS_FALSE(result);
    result = fossil_noshell_validate_document(NULL);
    ASSUME_ITS_FALSE(result);
}

// * * * * * * * * * * * * * * * * * * * * * * * *
// * Fossil Logic Test Pool
// * * * * * * * * * * * * * * * * * * * * * * * *
FOSSIL_TEST_GROUP(c_noshell_database_tests) {
    FOSSIL_TEST_ADD(c_noshell_fixture, c_test_noshell_create_database);
    FOSSIL_TEST_ADD(c_noshell_fixture, c_test_noshell_open_database);
    FOSSIL_TEST_ADD(c_noshell_fixture, c_test_noshell_delete_database);
    FOSSIL_TEST_ADD(c_noshell_fixture, c_test_noshell_insert_document);
    FOSSIL_TEST_ADD(c_noshell_fixture, c_test_noshell_find_document);
    FOSSIL_TEST_ADD(c_noshell_fixture, c_test_noshell_update_document);
    FOSSIL_TEST_ADD(c_noshell_fixture, c_test_noshell_remove_document);
    FOSSIL_TEST_ADD(c_noshell_fixture, c_test_noshell_validate_extension);
    FOSSIL_TEST_ADD(c_noshell_fixture, c_test_noshell_validate_document);

    FOSSIL_TEST_REGISTER(c_noshell_fixture);
} // end of tests

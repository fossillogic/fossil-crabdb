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
#include <fossil/xassume.h>            // Includes the Fossil Assume Framework

#include "fossil/crabdb/framework.h"

FOSSIL_FIXTURE(core_crabdb_datastore_fixture);
fossil_crabdb_t *db_dna = NULL;

FOSSIL_SETUP(core_crabdb_datastore_fixture) {
    db_dna = fossil_crabdb_create();
}

FOSSIL_TEARDOWN(core_crabdb_datastore_fixture) {
    fossil_crabdb_erase(db_dna);
}

/**
 * @brief Test case for serializing the database to a file.
 */
FOSSIL_TEST(test_serialize_crabdb_file) {
    ASSUME_NOT_CNULL(db_dna);

    // Create and populate a namespace
    fossil_crabdb_error_t result = fossil_crabdb_create_namespace(db_dna, "namespace1");
    ASSUME_ITS_EQUAL_I32(CRABDB_OK, result);

    result = fossil_crabdb_insert(db_dna, "namespace1", "key1", "value1");
    ASSUME_ITS_EQUAL_I32(CRABDB_OK, result);

    // Serialize the database to a file
    const char *filename = "test_db_serialized.crabdb";
    result = fossil_crabdb_serialize_to_file(db_dna, filename);
    ASSUME_ITS_EQUAL_I32(CRABDB_OK, result);

    // Clean up by removing the test file
    if (remove(filename) != 0) {
        fprintf(stderr, "Error deleting file %s\n", filename);
    }
}

/**
 * @brief Test case for deserializing the database from a file.
 */
FOSSIL_TEST(test_deserialize_crabdb_file) {
    ASSUME_NOT_CNULL(db_dna);

    // Create and populate a namespace
    fossil_crabdb_error_t result = fossil_crabdb_create_namespace(db_dna, "namespace1");
    ASSUME_ITS_EQUAL_I32(CRABDB_OK, result);

    result = fossil_crabdb_insert(db_dna, "namespace1", "key1", "value1");
    ASSUME_ITS_EQUAL_I32(CRABDB_OK, result);

    // Serialize the database to a file for deserialization test
    const char *filename = "test_db_serialized.crabdb";
    result = fossil_crabdb_serialize_to_file(db_dna, filename);
    ASSUME_ITS_EQUAL_I32(CRABDB_OK, result);

    // Create a new database instance for loading data
    fossil_crabdb_t *new_db_dna = fossil_crabdb_create();
    ASSUME_NOT_CNULL(new_db_dna);

    // Deserialize the database from the file
    result = fossil_crabdb_deserialize_from_file(new_db_dna, filename);
    ASSUME_ITS_EQUAL_I32(CRABDB_OK, result);

    // Verify the loaded data
    char *value = NULL;
    result = fossil_crabdb_get(new_db_dna, "namespace1", "key1", &value);
    ASSUME_ITS_EQUAL_I32(CRABDB_OK, result);
    ASSUME_ITS_EQUAL_CSTR("value1", value);

    fossil_crabdb_free(value);
    fossil_crabdb_erase(new_db_dna);

    // Clean up by removing the test file
    if (remove(filename) != 0) {
        fprintf(stderr, "Error deleting file %s\n", filename);
    }
}

/**
 * @brief Test case for saving the database to a file.
 */
FOSSIL_TEST(test_save_crabdb_file) {
    ASSUME_NOT_CNULL(db_dna);

    // Create and populate a namespace
    fossil_crabdb_error_t result = fossil_crabdb_create_namespace(db_dna, "namespace1");
    ASSUME_ITS_EQUAL_I32(CRABDB_OK, result);

    result = fossil_crabdb_insert(db_dna, "namespace1", "key1", "value1");
    ASSUME_ITS_EQUAL_I32(CRABDB_OK, result);

    // Save the database to a file
    const char *filename = "test_db_saved.crabdb";
    int save_result = fossil_crabdb_save_to_file(db_dna, filename);
    ASSUME_ITS_EQUAL_I32(0, save_result);

    // Clean up by removing the test file
    if (remove(filename) != 0) {
        fprintf(stderr, "Error deleting file %s\n", filename);
    }
}

/**
 * @brief Test case for loading the database from a file.
 */
FOSSIL_TEST(test_load_crabdb_file) {
    ASSUME_NOT_CNULL(db_dna);

    // Create and populate a namespace
    fossil_crabdb_error_t result = fossil_crabdb_create_namespace(db_dna, "namespace1");
    ASSUME_ITS_EQUAL_I32(CRABDB_OK, result);

    result = fossil_crabdb_insert(db_dna, "namespace1", "key1", "value1");
    ASSUME_ITS_EQUAL_I32(CRABDB_OK, result);

    // Save the database to a file for loading test
    const char *filename = "test_db_saved.crabdb";
    int save_result = fossil_crabdb_save_to_file(db_dna, filename);
    ASSUME_ITS_EQUAL_I32(0, save_result);

    // Create a new database instance for loading data
    fossil_crabdb_t *new_db_dna = fossil_crabdb_create();
    ASSUME_NOT_CNULL(new_db_dna);

    // Load the database from the file
    int load_result = fossil_crabdb_load_from_file(new_db_dna, filename);
    ASSUME_ITS_EQUAL_I32(0, load_result);

    // Verify the loaded data
    char *value = NULL;
    result = fossil_crabdb_get(new_db_dna, "namespace1", "key1", &value);
    ASSUME_ITS_EQUAL_I32(CRABDB_OK, result);
    ASSUME_ITS_EQUAL_CSTR("value1", value);

    fossil_crabdb_free(value);
    fossil_crabdb_erase(new_db_dna);

    // Clean up by removing the test file
    if (remove(filename) != 0) {
        fprintf(stderr, "Error deleting file %s\n", filename);
    }
}

/**
 * @brief Test group for datastore-related tests.
 */
FOSSIL_TEST_GROUP(c_crabdb_datastore_tests) {
    ADD_TESTF(test_serialize_crabdb_file, core_crabdb_datastore_fixture);
    ADD_TESTF(test_deserialize_crabdb_file, core_crabdb_datastore_fixture);
    ADD_TESTF(test_save_crabdb_file, core_crabdb_datastore_fixture);
    //ADD_TESTF(test_load_crabdb_file, core_crabdb_datastore_fixture);
}

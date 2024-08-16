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


FOSSIL_FIXTURE(core_crabdb_fixture);
fossil_crabdb_t *db = NULL;

FOSSIL_SETUP(core_crabdb_fixture) {
    db = fossil_crabdb_create();
}

FOSSIL_TEARDOWN(core_crabdb_fixture) {
    fossil_crabdb_erase(db);
}

// * * * * * * * * * * * * * * * * * * * * * * * *
// * Fossil Logic Test Blue CrabDB Database
// * * * * * * * * * * * * * * * * * * * * * * * *

FOSSIL_TEST(test_create_namespace) {
    ASSUME_NOT_CNULL(db);

    fossil_crabdb_error_t result = fossil_crabdb_create_namespace(db, "namespace1");
    ASSUME_ITS_EQUAL_I32(CRABDB_OK, result);
}

FOSSIL_TEST(test_insert_and_get_value) {
    ASSUME_NOT_CNULL(db);

    fossil_crabdb_create_namespace(db, "namespace1");

    fossil_crabdb_error_t result = fossil_crabdb_insert(db, "namespace1", "key1", "value1");
    ASSUME_ITS_EQUAL_I32(CRABDB_OK, result);

    char *value = NULL;
    result = fossil_crabdb_get(db, "namespace1", "key1", &value);
    ASSUME_ITS_EQUAL_I32(CRABDB_OK, result);
    ASSUME_ITS_EQUAL_CSTR("value1", value);

    free(value);
}

FOSSIL_TEST(test_update_value) {
    ASSUME_NOT_CNULL(db);

    fossil_crabdb_create_namespace(db, "namespace1");

    fossil_crabdb_insert(db, "namespace1", "key1", "value1");

    fossil_crabdb_error_t result = fossil_crabdb_update(db, "namespace1", "key1", "value2");
    ASSUME_ITS_EQUAL_I32(CRABDB_OK, result);

    char *value = NULL;
    result = fossil_crabdb_get(db, "namespace1", "key1", &value);
    ASSUME_ITS_EQUAL_I32(CRABDB_OK, result);
    ASSUME_ITS_EQUAL_CSTR("value2", value);

    free(value);
}

FOSSIL_TEST(test_delete_value) {
    ASSUME_NOT_CNULL(db);

    fossil_crabdb_create_namespace(db, "namespace1");

    fossil_crabdb_insert(db, "namespace1", "key1", "value1");

    fossil_crabdb_error_t result = fossil_crabdb_delete(db, "namespace1", "key1");
    ASSUME_ITS_EQUAL_I32(CRABDB_OK, result);

    char *value = xnull;
    result = fossil_crabdb_get(db, "namespace1", "key1", &value);
    ASSUME_ITS_EQUAL_I32(CRABDB_ERR_KEY_NOT_FOUND, result);
}

// Test case for serializing and deserializing the database
FOSSIL_TEST(test_crabdb_serialize_and_deserialize) {
    ASSUME_NOT_CNULL(db);

    // Create and populate the database
    fossil_crabdb_create_namespace(db, "namespace1");
    fossil_crabdb_insert(db, "namespace1", "key1", "value1");

    // Serialize the database
    fossil_crabdb_error_t result = fossil_crabdb_serialize(db, "db_file.crabdb");
    ASSUME_ITS_EQUAL_I32(CRABDB_OK, result);

    // Erase the current database and create a new one
    fossil_crabdb_erase(db);
    db = fossil_crabdb_create();  // Create a new instance

    // Deserialize the database
    fossil_crabdb_t *db_deserialized = fossil_crabdb_deserialize("db_file.crabdb");
    ASSUME_NOT_CNULL(db_deserialized);

    // Check the deserialized data
    char *value = NULL;
    result = fossil_crabdb_get(db_deserialized, "namespace1", "key1", &value);
    ASSUME_ITS_EQUAL_I32(CRABDB_OK, result);
    ASSUME_ITS_EQUAL_CSTR("value1", value);

    free(value);
    fossil_crabdb_erase(db_deserialized);
}

// Test case for executing a custom query
FOSSIL_TEST(test_crabdb_execute_query) {
    ASSUME_NOT_CNULL(db);

    // Execute a custom query to create a namespace and insert a value
    fossil_crabdb_error_t result = fossil_crabdb_execute_query(db, "create_namespace namespace1");
    ASSUME_ITS_EQUAL_I32(CRABDB_OK, result);

    result = fossil_crabdb_execute_query(db, "insert namespace1 key1 value1");
    ASSUME_ITS_EQUAL_I32(CRABDB_OK, result);

    // Verify the inserted value via get
    char *value = NULL;
    result = fossil_crabdb_get(db, "namespace1", "key1", &value);
    ASSUME_ITS_EQUAL_I32(CRABDB_OK, result);
    ASSUME_ITS_EQUAL_CSTR("value1", value);

    free(value);
}

// Test case for backing up and restoring the database
FOSSIL_TEST(test_crabdb_backup_and_restore) {
    ASSUME_NOT_CNULL(db);

    // Create and populate the database
    fossil_crabdb_create_namespace(db, "namespace1");
    fossil_crabdb_insert(db, "namespace1", "key1", "value1");

    // Backup the database
    fossil_crabdb_error_t result = fossil_crabdb_backup(db, "backup_file.crabdb");
    ASSUME_ITS_EQUAL_I32(CRABDB_OK, result);

    // Erase the current database and create a new one
    fossil_crabdb_erase(db);
    db = fossil_crabdb_create();  // Create a new instance

    // Restore the database from backup
    result = fossil_crabdb_restore(db, "backup_file.crabdb");
    ASSUME_ITS_EQUAL_I32(CRABDB_OK, result);

    // Verify the restored data
    char *value = NULL;
    result = fossil_crabdb_get(db, "namespace1", "key1", &value);
    ASSUME_ITS_EQUAL_I32(CRABDB_OK, result);
    ASSUME_ITS_EQUAL_CSTR("value1", value);

    free(value);
}

// * * * * * * * * * * * * * * * * * * * * * * * *
// * Fossil Logic Test Pool
// * * * * * * * * * * * * * * * * * * * * * * * *

FOSSIL_TEST_GROUP(c_crabdb_tests) {    
    ADD_TESTF(test_create_namespace, core_crabdb_fixture);
    ADD_TESTF(test_insert_and_get_value, core_crabdb_fixture);
    ADD_TESTF(test_update_value, core_crabdb_fixture);
    ADD_TESTF(test_delete_value, core_crabdb_fixture);
    ADD_TESTF(test_crabdb_serialize_and_deserialize, core_crabdb_fixture);
    ADD_TESTF(test_crabdb_execute_query, core_crabdb_fixture);
    ADD_TESTF(test_crabdb_backup_and_restore, core_crabdb_fixture);
} // end of tests

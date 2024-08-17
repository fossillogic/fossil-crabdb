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

// Test case for serializing the database
FOSSIL_TEST(test_crabdb_serialize) {
    ASSUME_NOT_CNULL(db);

    // Create and populate the database
    fossil_crabdb_create_namespace(db, "namespace1");
    fossil_crabdb_insert(db, "namespace1", "key1", "value1");

    // Serialize the database to a file
    fossil_crabdb_error_t result = fossil_crabdb_serialize(db, "serialize_test.crabdb");
    ASSUME_ITS_EQUAL_I32(CRABDB_OK, result);

    // Clean up: Erase the database and verify the file exists
    fossil_crabdb_erase(db);
    // Check if file "serialize_test.crabdb" exists (implement file existence check)
}

// Test case for deserializing the database
FOSSIL_TEST(test_crabdb_deserialize) {
    // First, serialize a database
    fossil_crabdb_t *db_source = fossil_crabdb_create();
    ASSUME_NOT_CNULL(db_source);

    fossil_crabdb_create_namespace(db_source, "namespace1");
    fossil_crabdb_insert(db_source, "namespace1", "key1", "value1");

    fossil_crabdb_error_t result = fossil_crabdb_serialize(db_source, "deserialize_test.crabdb");
    ASSUME_ITS_EQUAL_I32(CRABDB_OK, result);

    // Deserialize into a new database
    fossil_crabdb_t *db_dest = fossil_crabdb_deserialize("deserialize_test.crabdb");
    ASSUME_NOT_CNULL(db_dest);

    // Check deserialized data
    char *value = NULL;
    result = fossil_crabdb_get(db_dest, "namespace1", "key1", &value);
    ASSUME_ITS_EQUAL_I32(CRABDB_OK, result);
    ASSUME_ITS_EQUAL_CSTR("value1", value);

    free(value);

    // Clean up
    fossil_crabdb_erase(db_source);
    free(db_source);

    fossil_crabdb_erase(db_dest);
    free(db_dest);
}

// Test case for backing up the database
FOSSIL_TEST(test_crabdb_backup) {
    ASSUME_NOT_CNULL(db);

    // Create and populate the database
    fossil_crabdb_create_namespace(db, "namespace1");
    fossil_crabdb_insert(db, "namespace1", "key1", "value1");

    // Backup the database
    fossil_crabdb_error_t result = fossil_crabdb_backup(db, "backup_test.crabdb");
    ASSUME_ITS_EQUAL_I32(CRABDB_OK, result);

    // Clean up: Erase the database and verify the backup file exists
    fossil_crabdb_erase(db);
    // Check if file "backup_test.crabdb" exists (implement file existence check)
}

// Test case for restoring the database from a backup
FOSSIL_TEST(test_crabdb_restore) {
    ASSUME_NOT_CNULL(db);

    // Create and populate the original database
    fossil_crabdb_create_namespace(db, "namespace1");
    fossil_crabdb_insert(db, "namespace1", "key1", "value1");

    // Backup the database
    fossil_crabdb_backup(db, "restore_test_backup.crabdb");

    // Erase the current database
    fossil_crabdb_erase(db);

    // Create a new database instance to restore the backup
    fossil_crabdb_t *db_restore = fossil_crabdb_create();
    ASSUME_NOT_CNULL(db_restore);

    // Restore the database from the backup
    fossil_crabdb_error_t result = fossil_crabdb_restore(db_restore, "restore_test_backup.crabdb");
    ASSUME_ITS_EQUAL_I32(CRABDB_OK, result);

    // Verify the restored data
    char *value = NULL;
    result = fossil_crabdb_get(db_restore, "namespace1", "key1", &value);
    ASSUME_ITS_EQUAL_I32(CRABDB_OK, result);
    ASSUME_ITS_EQUAL_CSTR("value1", value);

    free(value);

    // Clean up
    fossil_crabdb_erase(db_restore);
    free(db_restore);
}

// * * * * * * * * * * * * * * * * * * * * * * * *
// * Fossil Logic Test Pool
// * * * * * * * * * * * * * * * * * * * * * * * *

FOSSIL_TEST_GROUP(c_crabdb_tests) {    
    ADD_TESTF(test_create_namespace, core_crabdb_fixture);
    ADD_TESTF(test_insert_and_get_value, core_crabdb_fixture);
    ADD_TESTF(test_update_value, core_crabdb_fixture);
    ADD_TESTF(test_delete_value, core_crabdb_fixture);
} // end of tests

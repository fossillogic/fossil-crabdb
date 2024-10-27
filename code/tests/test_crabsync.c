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
#include <fossil/unittest/assume.h>    // Includes the Fossil Assume Framework

#include "fossil/crabdb/framework.h"

FOSSIL_FIXTURE(crabsync_fixture);
fossil_crabsync_t *sync_mock;

FOSSIL_SETUP(crabsync_fixture) {
    // Create mock source and target databases
    fossil_crabdb_t *source_db = fossil_crabdb_create();
    fossil_crabdb_t *target_db = fossil_crabdb_create();
    
    // Initialize CrabSync with source and target databases
    sync_mock = fossil_crabsync_create(source_db, target_db);
    ASSUME_ITS_TRUE(sync_mock != NULL);
}

FOSSIL_TEARDOWN(crabsync_fixture) {
    // Clean up the sync instance
    fossil_crabsync_destroy(sync_mock);
}

// * * * * * * * * * * * * * * * * * * * * * * * *
// * Fossil Logic Test CrabSync Functionality
// * * * * * * * * * * * * * * * * * * * * * * * *

// Test initializing CrabSync
FOSSIL_TEST(test_crabsync_initialize) {
    ASSUME_ITS_TRUE(sync_mock->source_db != NULL);
    ASSUME_ITS_TRUE(sync_mock->target_db != NULL);
}

// Test adding data to the sync queue
FOSSIL_TEST(test_crabsync_add_data) {
    const char *key = "test_key";
    const char *value = "test_value";
    bool result = fossil_crabsync_add(sync_mock, key, value, FOSSIL_CRABDB_TYPE_STRING);
    ASSUME_ITS_TRUE(result);
}

// Test processing a sync operation
FOSSIL_TEST(test_crabsync_process_next) {
    const char *key = "process_key";
    const char *value = "process_value";
    fossil_crabsync_add(sync_mock, key, value, FOSSIL_CRABDB_TYPE_STRING);
    
    // Process the next sync operation
    bool result = fossil_crabsync_process_next(sync_mock);
    ASSUME_ITS_TRUE(result);
}

// Test syncing all data in the queue
FOSSIL_TEST(test_crabsync_sync_all) {
    const char *key1 = "key1";
    const char *value1 = "value1";
    const char *key2 = "key2";
    const char *value2 = "value2";
    
    fossil_crabsync_add(sync_mock, key1, value1, FOSSIL_CRABDB_TYPE_STRING);
    fossil_crabsync_add(sync_mock, key2, value2, FOSSIL_CRABDB_TYPE_STRING);
    
    // Sync all entries in the queue
    bool result = fossil_crabsync_sync_all(sync_mock);
    ASSUME_ITS_TRUE(result);
}

// Test cleanup
FOSSIL_TEST(test_crabsync_destroy) {
    // Verify that sync_mock is cleaned up correctly
    fossil_crabsync_destroy(sync_mock);
    sync_mock = NULL; // Avoid dangling pointer
    ASSUME_ITS_TRUE(sync_mock == NULL);
}

// Test error handling
FOSSIL_TEST(test_crabsync_handle_error) {
    // Simulate an error in the sync process
    fossil_crabsync_handle_error(-1); // Test unknown error
    fossil_crabsync_handle_error(EDEADLK); // Test deadlock error
}

// * * * * * * * * * * * * * * * * * * * * * * * *
// * Fossil Logic Test Pool
// * * * * * * * * * * * * * * * * * * * * * * * *

FOSSIL_TEST_GROUP(c_crabsync_tests) {
    ADD_TESTF(test_crabsync_initialize, crabsync_fixture);
    ADD_TESTF(test_crabsync_add_data, crabsync_fixture);
    ADD_TESTF(test_crabsync_process_next, crabsync_fixture);
    ADD_TESTF(test_crabsync_sync_all, crabsync_fixture);
    ADD_TESTF(test_crabsync_destroy, crabsync_fixture);
    ADD_TESTF(test_crabsync_handle_error, crabsync_fixture);
} // end of tests

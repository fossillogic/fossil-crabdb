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

FOSSIL_TEST_SUITE(c_crabsync_fixture);
fossil_crabsync_t *c_sync_mock;

FOSSIL_SETUP(c_crabsync_fixture) {
    // Create mock source and target databases
    fossil_crabdb_t *source_db = fossil_crabdb_create();
    fossil_crabdb_t *target_db = fossil_crabdb_create();
    
    // Initialize CrabSync with source and target databases
    c_sync_mock = fossil_crabsync_create(source_db, target_db);
    ASSUME_ITS_TRUE(c_sync_mock != NULL);
}

FOSSIL_TEARDOWN(c_crabsync_fixture) {
    // Clean up the sync instance
    fossil_crabsync_destroy(c_sync_mock);
}

// * * * * * * * * * * * * * * * * * * * * * * * *
// * Fossil Logic Test CrabSync Functionality
// * * * * * * * * * * * * * * * * * * * * * * * *

// Test initializing CrabSync
FOSSIL_TEST_CASE(c_test_crabsync_initialize) {
    ASSUME_ITS_TRUE(c_sync_mock->source_db != NULL);
    ASSUME_ITS_TRUE(c_sync_mock->target_db != NULL);
}

// Test adding data to the sync queue
FOSSIL_TEST_CASE(c_test_crabsync_add_data) {
    const char *key = "test_key";
    const char *value = "test_value";
    bool result = fossil_crabsync_add(c_sync_mock, key, value, FOSSIL_CRABDB_TYPE_STRING);
    ASSUME_ITS_TRUE(result);
}

// Test processing a sync operation
FOSSIL_TEST_CASE(c_test_crabsync_process_next) {
    const char *key = "process_key";
    const char *value = "process_value";
    fossil_crabsync_add(c_sync_mock, key, value, FOSSIL_CRABDB_TYPE_STRING);
    
    // Process the next sync operation
    bool result = fossil_crabsync_process_next(c_sync_mock);
    ASSUME_ITS_TRUE(result);
}

// Test syncing all data in the queue
FOSSIL_TEST_CASE(c_test_crabsync_sync_all) {
    const char *key1 = "key1";
    const char *value1 = "value1";
    const char *key2 = "key2";
    const char *value2 = "value2";
    
    fossil_crabsync_add(c_sync_mock, key1, value1, FOSSIL_CRABDB_TYPE_STRING);
    fossil_crabsync_add(c_sync_mock, key2, value2, FOSSIL_CRABDB_TYPE_STRING);
    
    // Sync all entries in the queue
    bool result = fossil_crabsync_sync_all(c_sync_mock);
    ASSUME_ITS_TRUE(result);
}

// * * * * * * * * * * * * * * * * * * * * * * * *
// * Fossil Logic Test Pool
// * * * * * * * * * * * * * * * * * * * * * * * *

FOSSIL_TEST_CASE_GROUP(c_crabsync_tests) {
    FOSSIL_TEST_ADD(c_crabsync_fixture, c_test_crabsync_initialize);
    FOSSIL_TEST_ADD(c_crabsync_fixture, c_test_crabsync_add_data);
    FOSSIL_TEST_ADD(c_crabsync_fixture, c_test_crabsync_process_next);
    FOSSIL_TEST_ADD(c_crabsync_fixture, c_test_crabsync_sync_all);

    FOSSIL_TEST_REGISTER(c_crabsync_fixture);
} // end of tests

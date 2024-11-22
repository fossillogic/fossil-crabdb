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

FOSSIL_TEST_SUITE(cpp_crabsyncpp_fixture);
fossil_crabsyncpp_t *cpp_syncpp_mock;

FOSSIL_SETUP(cpp_crabsyncpp_fixture) {
    // Create mock source and target databases
    fossil_crabdb_t *source_db = fossil_crabdb_create();
    fossil_crabdb_t *target_db = fossil_crabdb_create();
    
    // Initialize CrabSync with source and target databases
    cpp_syncpp_mock = fossil_crabsyncpp_create(source_db, target_db);
    ASSUME_ITS_TRUE(cpp_syncpp_mock != NULL);
}

FOSSIL_TEARDOWN(cpp_crabsyncpp_fixture) {
    // Clean up the sync instance
    fossil_crabsyncpp_destroy(cpp_syncpp_mock);
}

// * * * * * * * * * * * * * * * * * * * * * * * *
// * Fossil Logic Test CrabSync Functionality
// * * * * * * * * * * * * * * * * * * * * * * * *

// Test initializing CrabSync
FOSSIL_TEST_CASE(cpp_test_crabsyncpp_initialize) {
    ASSUME_ITS_TRUE(cpp_syncpp_mock->source_db != NULL);
    ASSUME_ITS_TRUE(cpp_syncpp_mock->target_db != NULL);
}

// Test adding data to the sync queue
FOSSIL_TEST_CASE(cpp_test_crabsyncpp_add_data) {
    const char *key = "test_key";
    const char *value = "test_value";
    bool result = fossil_crabsyncpp_add(cpp_syncpp_mock, key, value, FOSSIL_CRABDB_TYPE_STRING);
    ASSUME_ITS_TRUE(result);
}

// Test processing a sync operation
FOSSIL_TEST_CASE(cpp_test_crabsyncpp_process_next) {
    const char *key = "process_key";
    const char *value = "process_value";
    fossil_crabsyncpp_add(cpp_syncpp_mock, key, value, FOSSIL_CRABDB_TYPE_STRING);
    
    // Process the next sync operation
    bool result = fossil_crabsyncpp_process_next(cpp_syncpp_mock);
    ASSUME_ITS_TRUE(result);
}

// Test syncing all data in the queue
FOSSIL_TEST_CASE(cpp_test_crabsyncpp_syncpp_all) {
    const char *key1 = "key1";
    const char *value1 = "value1";
    const char *key2 = "key2";
    const char *value2 = "value2";
    
    fossil_crabsyncpp_add(cpp_syncpp_mock, key1, value1, FOSSIL_CRABDB_TYPE_STRING);
    fossil_crabsyncpp_add(cpp_syncpp_mock, key2, value2, FOSSIL_CRABDB_TYPE_STRING);
    
    // Sync all entries in the queue
    bool result = fossil_crabsyncpp_syncpp_all(cpp_syncpp_mock);
    ASSUME_ITS_TRUE(result);
}

// * * * * * * * * * * * * * * * * * * * * * * * *
// * Fossil Logic Test Pool
// * * * * * * * * * * * * * * * * * * * * * * * *

FOSSIL_TEST_CASE_GROUP(cpp_crabsyncpp_tests) {
    FOSSIL_TEST_ADD(cpp_crabsyncpp_fixture, cpp_test_crabsyncpp_initialize);
    FOSSIL_TEST_ADD(cpp_crabsyncpp_fixture, cpp_test_crabsyncpp_add_data);
    FOSSIL_TEST_ADD(cpp_crabsyncpp_fixture, cpp_test_crabsyncpp_process_next);
    FOSSIL_TEST_ADD(cpp_crabsyncpp_fixture, cpp_test_crabsyncpp_syncpp_all);

    FOSSIL_TEST_REGISTER(cpp_crabsyncpp_fixture);
} // end of tests

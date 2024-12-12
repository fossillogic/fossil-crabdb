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

// * * * * * * * * * * * * * * * * * * * * * * * *
// * Fossil Logic Test Utilities
// * * * * * * * * * * * * * * * * * * * * * * * *
// Setup steps for things like test fixtures and
// mock objects are set here.
// * * * * * * * * * * * * * * * * * * * * * * * *

FOSSIL_TEST_SUITE(c_crabdb_fixture);

FOSSIL_SETUP(c_crabdb_fixture) {
    // Setup the test fixture
}

FOSSIL_TEARDOWN(c_crabdb_fixture) {
    // Teardown the test fixture
}

// * * * * * * * * * * * * * * * * * * * * * * * *
// * Fossil Logic Test Blue CrabDB Database
// * * * * * * * * * * * * * * * * * * * * * * * *

// Test case for opening a database with an invalid file extension
FOSSIL_TEST_CASE(c_test_crabdb_open_invalid_extension) {
    bool result = fossil_crabdb_open("invalid_file.txt");
    ASSUME_ITS_FALSE(result); // Should fail due to invalid file extension
}

// Test case for opening a non-existing database file
FOSSIL_TEST_CASE(c_test_crabdb_open_non_existing_file) {
    bool result = fossil_crabdb_open("non_existing_file.crabdb");
    ASSUME_ITS_FALSE(result); // Should fail due to non-existing file
}

// Test case for closing an already closed database
FOSSIL_TEST_CASE(c_test_crabdb_close_already_closed) {
    fossil_crabdb_open("test_file.crabdb");
    fossil_crabdb_close();
    fossil_crabdb_close(); // Should handle gracefully without crashing
}

// * * * * * * * * * * * * * * * * * * * * * * * *
// * Fossil Logic Test Pool
// * * * * * * * * * * * * * * * * * * * * * * * *

FOSSIL_TEST_GROUP(c_crab_database_tests) {    
    FOSSIL_TEST_ADD(c_crabdb_fixture, c_test_crabdb_open_invalid_extension);
    FOSSIL_TEST_ADD(c_crabdb_fixture, c_test_crabdb_open_non_existing_file);
    FOSSIL_TEST_ADD(c_crabdb_fixture, c_test_crabdb_close_already_closed);
    FOSSIL_TEST_ADD(c_crabdb_fixture, c_test_crabdb_batch_create_invalid_entries);
    FOSSIL_TEST_ADD(c_crabdb_fixture, c_test_crabdb_batch_read_non_existing_keys);
    FOSSIL_TEST_ADD(c_crabdb_fixture, c_test_crabdb_batch_delete_mixed_keys);
    FOSSIL_TEST_ADD(c_crabdb_fixture, c_test_crabdb_search_partial_key);
    FOSSIL_TEST_ADD(c_crabdb_fixture, c_test_crabdb_filter_custom_predicate);
    FOSSIL_TEST_ADD(c_crabdb_fixture, c_test_crabdb_sort_entries);
    FOSSIL_TEST_ADD(c_crabdb_fixture, c_test_crabdb_clear_entries);
    FOSSIL_TEST_ADD(c_crabdb_fixture, c_test_crabdb_encode_decode);

    FOSSIL_TEST_REGISTER(c_crabdb_fixture);
} // end of tests

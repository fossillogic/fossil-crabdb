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

FOSSIL_SUITE(c_timeshell_fixture);

FOSSIL_SETUP(c_timeshell_fixture) {
    // Setup the test fixture
}

FOSSIL_TEARDOWN(c_timeshell_fixture) {
    // Teardown the test fixture
}

// * * * * * * * * * * * * * * * * * * * * * * * *
// * Fossil Logic Test Blue CrabDB Database
// * * * * * * * * * * * * * * * * * * * * * * * *

// Test case for inserting a new interval into the database
FOSSIL_TEST(c_test_timeshell_insert_interval) {
    const char *file_name = "test.crabdb";
    fossil_bluecrab_timeshell_create_database(file_name);

    fossil_timeshell_interval_t interval = { .start = 100, .end = 200 };
    fossil_timeshell_error_t result = fossil_bluecrab_timeshell_insert(file_name, &interval);
    ASSUME_ITS_TRUE(result == FOSSIL_TIMESHELL_ERROR_SUCCESS);

    size_t count = 0;
    result = fossil_bluecrab_timeshell_count_intervals(file_name, &count);
    ASSUME_ITS_TRUE(result == FOSSIL_TIMESHELL_ERROR_SUCCESS);
    ASSUME_ITS_TRUE(count == 1);

    fossil_bluecrab_timeshell_delete_database(file_name);
}

// Test case for finding overlapping intervals
FOSSIL_TEST(c_test_timeshell_find_overlap) {
    const char *file_name = "test.crabdb";
    fossil_bluecrab_timeshell_create_database(file_name);

    fossil_timeshell_interval_t intervals[] = {
        { .start = 10, .end = 20 },
        { .start = 15, .end = 25 },
        { .start = 30, .end = 40 }
    };
    for (size_t i = 0; i < 3; ++i)
        fossil_bluecrab_timeshell_insert(file_name, &intervals[i]);

    fossil_timeshell_interval_t query = { .start = 18, .end = 35 };
    fossil_timeshell_interval_t results[3];
    size_t found = 0;
    fossil_timeshell_error_t result = fossil_bluecrab_timeshell_find(file_name, &query, results, 3, &found);
    ASSUME_ITS_TRUE(result == FOSSIL_TIMESHELL_ERROR_SUCCESS);
    ASSUME_ITS_TRUE(found == 2);

    fossil_bluecrab_timeshell_delete_database(file_name);
}

// Test case for updating an existing interval
FOSSIL_TEST(c_test_timeshell_update_interval) {
    const char *file_name = "test.crabdb";
    fossil_bluecrab_timeshell_create_database(file_name);

    fossil_timeshell_interval_t old_interval = { .start = 50, .end = 60 };
    fossil_timeshell_interval_t new_interval = { .start = 55, .end = 65 };
    fossil_bluecrab_timeshell_insert(file_name, &old_interval);

    fossil_timeshell_error_t result = fossil_bluecrab_timeshell_update(file_name, &old_interval, &new_interval);
    ASSUME_ITS_TRUE(result == FOSSIL_TIMESHELL_ERROR_SUCCESS);

    fossil_timeshell_interval_t query = { .start = 55, .end = 65 };
    fossil_timeshell_interval_t results[1];
    size_t found = 0;
    result = fossil_bluecrab_timeshell_find(file_name, &query, results, 1, &found);
    ASSUME_ITS_TRUE(result == FOSSIL_TIMESHELL_ERROR_SUCCESS);
    ASSUME_ITS_TRUE(found == 1);
    ASSUME_ITS_TRUE(results[0].start == 55 && results[0].end == 65);

    fossil_bluecrab_timeshell_delete_database(file_name);
}

// Test case for removing an interval
FOSSIL_TEST(c_test_timeshell_remove_interval) {
    const char *file_name = "test.crabdb";
    fossil_bluecrab_timeshell_create_database(file_name);

    fossil_timeshell_interval_t interval = { .start = 70, .end = 80 };
    fossil_bluecrab_timeshell_insert(file_name, &interval);

    fossil_timeshell_error_t result = fossil_bluecrab_timeshell_remove(file_name, &interval);
    ASSUME_ITS_TRUE(result == FOSSIL_TIMESHELL_ERROR_SUCCESS);

    size_t count = 0;
    result = fossil_bluecrab_timeshell_count_intervals(file_name, &count);
    ASSUME_ITS_TRUE(result == FOSSIL_TIMESHELL_ERROR_SUCCESS);
    ASSUME_ITS_TRUE(count == 0);

    fossil_bluecrab_timeshell_delete_database(file_name);
}

// Test case for backing up and restoring a timeshell database
FOSSIL_TEST(c_test_timeshell_backup_restore) {
    const char *file_name = "test.crabdb";
    const char *backup_file = "backup.crabdb";
    fossil_bluecrab_timeshell_create_database(file_name);

    fossil_timeshell_interval_t interval = { .start = 1, .end = 2 };
    fossil_bluecrab_timeshell_insert(file_name, &interval);

    fossil_timeshell_error_t result = fossil_bluecrab_timeshell_backup_database(file_name, backup_file);
    ASSUME_ITS_TRUE(result == FOSSIL_TIMESHELL_ERROR_SUCCESS);

    fossil_bluecrab_timeshell_delete_database(file_name);
    result = fossil_bluecrab_timeshell_restore_database(backup_file, file_name);
    ASSUME_ITS_TRUE(result == FOSSIL_TIMESHELL_ERROR_SUCCESS);

    size_t count = 0;
    result = fossil_bluecrab_timeshell_count_intervals(file_name, &count);
    ASSUME_ITS_TRUE(result == FOSSIL_TIMESHELL_ERROR_SUCCESS);
    ASSUME_ITS_TRUE(count == 1);

    fossil_bluecrab_timeshell_delete_database(file_name);
    fossil_bluecrab_timeshell_delete_database(backup_file);
}

// Test case for validating file extension
FOSSIL_TEST(c_test_timeshell_validate_extension) {
    ASSUME_ITS_TRUE(fossil_bluecrab_timeshell_validate_extension("test.crabdb"));
    ASSUME_ITS_FALSE(fossil_bluecrab_timeshell_validate_extension("test.txt"));
}

// Test case for validating time intervals
FOSSIL_TEST(c_test_timeshell_validate_interval) {
    fossil_timeshell_interval_t valid = { .start = 1, .end = 2 };
    fossil_timeshell_interval_t invalid = { .start = 5, .end = 2 };
    ASSUME_ITS_TRUE(fossil_bluecrab_timeshell_validate_interval(&valid));
    ASSUME_ITS_FALSE(fossil_bluecrab_timeshell_validate_interval(&invalid));
}

// * * * * * * * * * * * * * * * * * * * * * * * *
// * Fossil Logic Test Pool
// * * * * * * * * * * * * * * * * * * * * * * * *
FOSSIL_TEST_GROUP(c_timeshell_database_tests) {
    FOSSIL_TEST_ADD(c_timeshell_fixture, c_test_timeshell_create_record);
    FOSSIL_TEST_ADD(c_timeshell_fixture, c_test_timeshell_read_nonexistent_record);
    FOSSIL_TEST_ADD(c_timeshell_fixture, c_test_timeshell_update_nonexistent_record);
    FOSSIL_TEST_ADD(c_timeshell_fixture, c_test_timeshell_delete_nonexistent_record);
    FOSSIL_TEST_ADD(c_timeshell_fixture, c_test_timeshell_backup_restore);
    FOSSIL_TEST_ADD(c_timeshell_fixture, c_test_timeshell_validate_extension);
    FOSSIL_TEST_ADD(c_timeshell_fixture, c_test_timeshell_validate_interval);

    FOSSIL_TEST_REGISTER(c_timeshell_fixture);
} // end of tests

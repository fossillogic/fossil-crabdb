/**
 * -----------------------------------------------------------------------------
 * Project: Fossil Logic
 *
 * This file is part of the Fossil Logic project, which aims to develop
 * high-performance, cross-platform applications and libraries. The code
 * contained herein is licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License. You may obtain
 * a copy of the License at:
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS, WITHOUT
 * WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied. See the
 * License for the specific language governing permissions and limitations
 * under the License.
 *
 * Author: Michael Gene Brockus (Dreamer)
 * Date: 04/05/2014
 *
 * Copyright (C) 2014-2025 Fossil Logic. All rights reserved.
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

/* Test case for inserting a new interval into the database */
FOSSIL_TEST(c_test_timeshell_insert_interval) {
    const char *file_name = "test.tmshell";
    fossil_bluecrab_timeshell_create_database(file_name);

    fossil_timeshell_interval_t interval;
    interval.start_time = 100;
    interval.end_time = 200;
    fossil_timeshell_error_t result = fossil_bluecrab_timeshell_insert(file_name, &interval);
    ASSUME_ITS_TRUE(result == FOSSIL_TIMESHELL_ERROR_SUCCESS);

    size_t count = 0;
    result = fossil_bluecrab_timeshell_count_intervals(file_name, &count);
    ASSUME_ITS_TRUE(result == FOSSIL_TIMESHELL_ERROR_SUCCESS);
    ASSUME_ITS_TRUE(count == 1);

    fossil_bluecrab_timeshell_delete_database(file_name);
}

/* Test case for finding overlapping intervals */
FOSSIL_TEST(c_test_timeshell_find_overlap) {
    const char *file_name = "test.tmshell";
    fossil_bluecrab_timeshell_create_database(file_name);

    fossil_timeshell_interval_t intervals[3];
    intervals[0].start_time = 10; intervals[0].end_time = 20;
    intervals[1].start_time = 15; intervals[1].end_time = 25;
    intervals[2].start_time = 30; intervals[2].end_time = 40;
    for (size_t i = 0; i < 3; ++i)
        fossil_bluecrab_timeshell_insert(file_name, &intervals[i]);

    fossil_timeshell_interval_t query;
    query.start_time = 18;
    query.end_time = 35;
    fossil_timeshell_interval_t results[3];
    size_t found = 0;
    fossil_timeshell_error_t result = fossil_bluecrab_timeshell_find(file_name, &query, results, 3, &found);
    ASSUME_ITS_TRUE(result == FOSSIL_TIMESHELL_ERROR_SUCCESS);
    //ASSUME_ITS_TRUE(found == 2);

    fossil_bluecrab_timeshell_delete_database(file_name);
}

/* Test case for updating an existing interval */
FOSSIL_TEST(c_test_timeshell_update_interval) {
    const char *file_name = "test.tmshell";
    fossil_bluecrab_timeshell_create_database(file_name);

    fossil_timeshell_interval_t old_interval;
    old_interval.start_time = 50;
    old_interval.end_time = 60;
    fossil_timeshell_interval_t new_interval;
    new_interval.start_time = 55;
    new_interval.end_time = 65;
    fossil_bluecrab_timeshell_insert(file_name, &old_interval);

    fossil_timeshell_error_t result = fossil_bluecrab_timeshell_update(file_name, &old_interval, &new_interval);
    ASSUME_ITS_TRUE(result == FOSSIL_TIMESHELL_ERROR_SUCCESS);

    fossil_timeshell_interval_t query;
    query.start_time = 55;
    query.end_time = 65;
    fossil_timeshell_interval_t results[1];
    size_t found = 0;
    result = fossil_bluecrab_timeshell_find(file_name, &query, results, 1, &found);
    ASSUME_ITS_TRUE(result == FOSSIL_TIMESHELL_ERROR_SUCCESS);
    ASSUME_ITS_TRUE(found == 1);
    ASSUME_ITS_TRUE(results[0].start_time == 55 && results[0].end_time == 65);

    fossil_bluecrab_timeshell_delete_database(file_name);
}

/* Test case for removing an interval */
FOSSIL_TEST(c_test_timeshell_remove_interval) {
    const char *file_name = "test.tmshell";
    fossil_bluecrab_timeshell_create_database(file_name);

    fossil_timeshell_interval_t interval;
    interval.start_time = 70;
    interval.end_time = 80;
    fossil_bluecrab_timeshell_insert(file_name, &interval);

    fossil_timeshell_error_t result = fossil_bluecrab_timeshell_remove(file_name, &interval);
    ASSUME_ITS_TRUE(result == FOSSIL_TIMESHELL_ERROR_SUCCESS);

    size_t count = 0;
    result = fossil_bluecrab_timeshell_count_intervals(file_name, &count);
    ASSUME_ITS_TRUE(result == FOSSIL_TIMESHELL_ERROR_SUCCESS);
    ASSUME_ITS_TRUE(count == 0);

    fossil_bluecrab_timeshell_delete_database(file_name);
}

/* Test case for backing up and restoring a timeshell database */
FOSSIL_TEST(c_test_timeshell_backup_restore) {
    const char *file_name = "test.tmshell";
    const char *backup_file = "backup.tmshell";
    fossil_bluecrab_timeshell_create_database(file_name);

    fossil_timeshell_interval_t interval;
    interval.start_time = 1;
    interval.end_time = 2;
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

/* Test case for validating file extension */
FOSSIL_TEST(c_test_timeshell_validate_extension) {
    ASSUME_ITS_TRUE(fossil_bluecrab_timeshell_validate_extension("test.tmshell"));
    ASSUME_ITS_FALSE(fossil_bluecrab_timeshell_validate_extension("test.txt"));
}

/* Test case for validating time intervals */
FOSSIL_TEST(c_test_timeshell_validate_interval) {
    fossil_timeshell_interval_t valid;
    valid.start_time = 1;
    valid.end_time = 2;
    fossil_timeshell_interval_t invalid;
    invalid.start_time = 5;
    invalid.end_time = 2;
    ASSUME_ITS_TRUE(fossil_bluecrab_timeshell_validate_interval(&valid));
    ASSUME_ITS_FALSE(fossil_bluecrab_timeshell_validate_interval(&invalid));
}

// * * * * * * * * * * * * * * * * * * * * * * * *
// * Fossil Logic Test Pool
// * * * * * * * * * * * * * * * * * * * * * * * *
FOSSIL_TEST_GROUP(c_timeshell_database_tests) {
    FOSSIL_TEST_ADD(c_timeshell_fixture, c_test_timeshell_insert_interval);
    FOSSIL_TEST_ADD(c_timeshell_fixture, c_test_timeshell_find_overlap);
    FOSSIL_TEST_ADD(c_timeshell_fixture, c_test_timeshell_update_interval);
    FOSSIL_TEST_ADD(c_timeshell_fixture, c_test_timeshell_remove_interval);
    FOSSIL_TEST_ADD(c_timeshell_fixture, c_test_timeshell_backup_restore);
    FOSSIL_TEST_ADD(c_timeshell_fixture, c_test_timeshell_validate_extension);
    FOSSIL_TEST_ADD(c_timeshell_fixture, c_test_timeshell_validate_interval);

    FOSSIL_TEST_REGISTER(c_timeshell_fixture);
} // end of tests

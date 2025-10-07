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

FOSSIL_SUITE(c_myshell_fixture);

FOSSIL_SETUP(c_myshell_fixture) {
    // Setup the test fixture
}

FOSSIL_TEARDOWN(c_myshell_fixture) {
    // Teardown the test fixture
}

// * * * * * * * * * * * * * * * * * * * * * * * *
// * Fossil Logic Test Blue CrabDB Database
// * * * * * * * * * * * * * * * * * * * * * * * *

/*
 * Test case for creating a new record in the database file (FSON encoding)
 */
FOSSIL_TEST(c_test_myshell_open_create_close) {
    fossil_bluecrab_myshell_error_t err;
    const char *file_name = "test2.myshell";

    fossil_bluecrab_myshell_t *db = fossil_myshell_create(file_name, &err);
    ASSUME_ITS_TRUE(db != NULL);
    ASSUME_ITS_TRUE(err == FOSSIL_MYSHELL_ERROR_SUCCESS);

    fossil_myshell_close(db);

    db = fossil_myshell_open(file_name, &err);
    ASSUME_ITS_TRUE(db != NULL);
    ASSUME_ITS_TRUE(err == FOSSIL_MYSHELL_ERROR_SUCCESS);

    fossil_myshell_close(db);

    remove(file_name);
}

FOSSIL_TEST(c_test_myshell_commit_branch_checkout) {
    fossil_bluecrab_myshell_error_t err;
    const char *file_name = "test4.myshell";
    fossil_bluecrab_myshell_t *db = fossil_myshell_create(file_name, &err);
    ASSUME_ITS_TRUE(db != NULL);

    err = fossil_myshell_put(db, "key", "cstr", "val");
    ASSUME_ITS_TRUE(err == FOSSIL_MYSHELL_ERROR_SUCCESS);

    err = fossil_myshell_commit(db, "Initial commit");
    ASSUME_ITS_TRUE(err == FOSSIL_MYSHELL_ERROR_SUCCESS);

    err = fossil_myshell_branch(db, "feature");
    ASSUME_ITS_TRUE(err == FOSSIL_MYSHELL_ERROR_SUCCESS);

    err = fossil_myshell_checkout(db, "feature");
    ASSUME_ITS_TRUE(err == FOSSIL_MYSHELL_ERROR_SUCCESS);

    fossil_myshell_close(db);
    remove(file_name);
}

FOSSIL_TEST(c_test_myshell_errstr) {
    ASSUME_ITS_EQUAL_CSTR(fossil_myshell_errstr(FOSSIL_MYSHELL_ERROR_SUCCESS), "Success");
    ASSUME_ITS_EQUAL_CSTR(fossil_myshell_errstr(FOSSIL_MYSHELL_ERROR_NOT_FOUND), "Not found");
    ASSUME_ITS_EQUAL_CSTR(fossil_myshell_errstr(FOSSIL_MYSHELL_ERROR_INVALID_FILE), "Invalid file");
    ASSUME_ITS_EQUAL_CSTR(fossil_myshell_errstr(9999), "Unknown error");
}

FOSSIL_TEST(c_test_myshell_put_get_del) {
    fossil_bluecrab_myshell_error_t err;
    const char *file_name = "test_put_get_del.myshell";
    fossil_bluecrab_myshell_t *db = fossil_myshell_create(file_name, &err);
    ASSUME_ITS_TRUE(db != NULL);

    // Put key-value pairs
    err = fossil_myshell_put(db, "username", "cstr", "alice");
    ASSUME_ITS_TRUE(err == FOSSIL_MYSHELL_ERROR_SUCCESS);

    err = fossil_myshell_put(db, "password", "cstr", "secret");
    ASSUME_ITS_TRUE(err == FOSSIL_MYSHELL_ERROR_SUCCESS);

    // Get key-value pairs
    char value[128];
    err = fossil_myshell_get(db, "username", value, sizeof(value));
    ASSUME_ITS_TRUE(err == FOSSIL_MYSHELL_ERROR_SUCCESS);
    ASSUME_ITS_EQUAL_CSTR(value, "alice");

    err = fossil_myshell_get(db, "password", value, sizeof(value));
    ASSUME_ITS_TRUE(err == FOSSIL_MYSHELL_ERROR_SUCCESS);
    ASSUME_ITS_EQUAL_CSTR(value, "secret");

    // Delete key
    err = fossil_myshell_del(db, "username");
    ASSUME_ITS_TRUE(err == FOSSIL_MYSHELL_ERROR_SUCCESS);

    err = fossil_myshell_get(db, "username", value, sizeof(value));
    ASSUME_ITS_TRUE(err == FOSSIL_MYSHELL_ERROR_NOT_FOUND);

    fossil_myshell_close(db);
    remove(file_name);
}

FOSSIL_TEST(c_test_myshell_put_all_types) {
    fossil_bluecrab_myshell_error_t err;
    const char *file_name = "test_put_all_types.myshell";
    fossil_bluecrab_myshell_t *db = fossil_myshell_create(file_name, &err);
    ASSUME_ITS_TRUE(db != NULL);

    const char *types[] = {
        "null", "bool", "i8", "i16", "i32", "i64", "u8", "u16", "u32", "u64",
        "f32", "f64", "oct", "hex", "bin", "char", "cstr", "array", "object",
        "enum", "datetime", "duration"
    };
    const char *values[] = {
        "", "true", "127", "32767", "2147483647", "9223372036854775807",
        "255", "65535", "4294967295", "18446744073709551615",
        "3.14", "2.71828", "0755", "0xFF", "0b1010", "A", "hello", "[1,2]", "{\"k\":1}", "VAL", "2024-06-01T12:00:00Z", "1h30m"
    };

    char value[128];
    for (size_t i = 0; i < sizeof(types)/sizeof(types[0]); ++i) {
        char key[32];
        snprintf(key, sizeof(key), "key_%s", types[i]);
        err = fossil_myshell_put(db, key, types[i], values[i]);
        ASSUME_ITS_TRUE(err == FOSSIL_MYSHELL_ERROR_SUCCESS);

        err = fossil_myshell_get(db, key, value, sizeof(value));
        ASSUME_ITS_TRUE(err == FOSSIL_MYSHELL_ERROR_SUCCESS);
        ASSUME_ITS_EQUAL_CSTR(value, values[i]);
    }

    fossil_myshell_close(db);
    remove(file_name);
}

FOSSIL_TEST(c_test_myshell_put_invalid_type) {
    fossil_bluecrab_myshell_error_t err;
    const char *file_name = "test_put_invalid_type.myshell";
    fossil_bluecrab_myshell_t *db = fossil_myshell_create(file_name, &err);
    ASSUME_ITS_TRUE(db != NULL);

    err = fossil_myshell_put(db, "badkey", "notatype", "value");
    ASSUME_ITS_TRUE(err == FOSSIL_MYSHELL_ERROR_INVALID_TYPE);

    fossil_myshell_close(db);
    remove(file_name);
}

FOSSIL_TEST(c_test_myshell_get_not_found) {
    fossil_bluecrab_myshell_error_t err;
    const char *file_name = "test_get_not_found.myshell";
    fossil_bluecrab_myshell_t *db = fossil_myshell_create(file_name, &err);
    ASSUME_ITS_TRUE(db != NULL);

    char value[128];
    err = fossil_myshell_get(db, "nonexistent", value, sizeof(value));
    ASSUME_ITS_TRUE(err == FOSSIL_MYSHELL_ERROR_NOT_FOUND);

    fossil_myshell_close(db);
    remove(file_name);
}

FOSSIL_TEST(c_test_myshell_get_buffer_too_small) {
    fossil_bluecrab_myshell_error_t err;
    const char *file_name = "test_get_buffer_small.myshell";
    fossil_bluecrab_myshell_t *db = fossil_myshell_create(file_name, &err);
    ASSUME_ITS_TRUE(db != NULL);

    err = fossil_myshell_put(db, "shortkey", "cstr", "longvalue");
    ASSUME_ITS_TRUE(err == FOSSIL_MYSHELL_ERROR_SUCCESS);

    char value[4]; // too small for "longvalue"
    err = fossil_myshell_get(db, "shortkey", value, sizeof(value));
    ASSUME_ITS_TRUE(err == FOSSIL_MYSHELL_ERROR_BUFFER_TOO_SMALL);

    fossil_myshell_close(db);
    remove(file_name);
}

FOSSIL_TEST(c_test_myshell_del_not_found) {
    fossil_bluecrab_myshell_error_t err;
    const char *file_name = "test_del_not_found.myshell";
    fossil_bluecrab_myshell_t *db = fossil_myshell_create(file_name, &err);
    ASSUME_ITS_TRUE(db != NULL);

    // Try to delete a key that does not exist
    err = fossil_myshell_del(db, "nonexistent_key");
    ASSUME_ITS_TRUE(err == FOSSIL_MYSHELL_ERROR_NOT_FOUND);

    fossil_myshell_close(db);
    remove(file_name);
}

FOSSIL_TEST(c_test_myshell_del_twice) {
    fossil_bluecrab_myshell_error_t err;
    const char *file_name = "test_del_twice.myshell";
    fossil_bluecrab_myshell_t *db = fossil_myshell_create(file_name, &err);
    ASSUME_ITS_TRUE(db != NULL);

    // Put a key and delete it
    err = fossil_myshell_put(db, "key", "cstr", "value");
    ASSUME_ITS_TRUE(err == FOSSIL_MYSHELL_ERROR_SUCCESS);

    err = fossil_myshell_del(db, "key");
    ASSUME_ITS_TRUE(err == FOSSIL_MYSHELL_ERROR_SUCCESS);

    // Try to delete again
    err = fossil_myshell_del(db, "key");
    ASSUME_ITS_TRUE(err == FOSSIL_MYSHELL_ERROR_NOT_FOUND);

    fossil_myshell_close(db);
    remove(file_name);
}

FOSSIL_TEST(c_test_myshell_stage_unstage) {
    fossil_bluecrab_myshell_error_t err;
    const char *file_name = "test_stage_unstage.myshell";
    fossil_bluecrab_myshell_t *db = fossil_myshell_create(file_name, &err);
    ASSUME_ITS_TRUE(db != NULL);

    err = fossil_myshell_stage(db, "foo", "cstr", "bar");
    ASSUME_ITS_TRUE(err == FOSSIL_MYSHELL_ERROR_SUCCESS);

    err = fossil_myshell_unstage(db, "foo");
    ASSUME_ITS_TRUE(err == FOSSIL_MYSHELL_ERROR_SUCCESS);

    // Unstage again should return NOT_FOUND
    err = fossil_myshell_unstage(db, "foo");
    ASSUME_ITS_TRUE(err == FOSSIL_MYSHELL_ERROR_NOT_FOUND);

    fossil_myshell_close(db);
    remove(file_name);
}

FOSSIL_TEST(c_test_myshell_backup_restore) {
    fossil_bluecrab_myshell_error_t err;
    const char *file_name = "test_backup_restore.myshell";
    const char *backup_file = "test_backup_restore.bak";
    const char *restore_file = "test_backup_restore_restored.myshell";
    fossil_bluecrab_myshell_t *db = fossil_myshell_create(file_name, &err);
    ASSUME_ITS_TRUE(db != NULL);

    err = fossil_myshell_put(db, "alpha", "cstr", "beta");
    ASSUME_ITS_TRUE(err == FOSSIL_MYSHELL_ERROR_SUCCESS);

    err = fossil_myshell_backup(db, backup_file);
    ASSUME_ITS_TRUE(err == FOSSIL_MYSHELL_ERROR_SUCCESS);

    fossil_myshell_close(db);

    err = fossil_myshell_restore(backup_file, restore_file);
    ASSUME_ITS_TRUE(err == FOSSIL_MYSHELL_ERROR_SUCCESS);

    db = fossil_myshell_open(restore_file, &err);
    ASSUME_ITS_TRUE(db != NULL);

    char value[128];
    err = fossil_myshell_get(db, "alpha", value, sizeof(value));
    ASSUME_ITS_TRUE(err == FOSSIL_MYSHELL_ERROR_SUCCESS);
    ASSUME_ITS_EQUAL_CSTR(value, "beta");

    fossil_myshell_close(db);
    remove(file_name);
    remove(backup_file);
    remove(restore_file);
}

FOSSIL_TEST(c_test_myshell_open_invalid_path) {
    fossil_bluecrab_myshell_error_t err;
    fossil_bluecrab_myshell_t *db = fossil_myshell_open(NULL, &err);
    ASSUME_ITS_TRUE(db == NULL);
    ASSUME_ITS_TRUE(err == FOSSIL_MYSHELL_ERROR_INVALID_FILE);

    db = fossil_myshell_open("not_a_myshell.txt", &err);
    ASSUME_ITS_TRUE(db == NULL);
    ASSUME_ITS_TRUE(err == FOSSIL_MYSHELL_ERROR_INVALID_FILE);
}

FOSSIL_TEST(c_test_myshell_create_existing_file) {
    fossil_bluecrab_myshell_error_t err;
    const char *file_name = "test_existing.myshell";
    fossil_bluecrab_myshell_t *db = fossil_myshell_create(file_name, &err);
    ASSUME_ITS_TRUE(db != NULL);
    fossil_myshell_close(db);

    db = fossil_myshell_create(file_name, &err);
    ASSUME_ITS_TRUE(db == NULL);
    ASSUME_ITS_TRUE(err == FOSSIL_MYSHELL_ERROR_ALREADY_EXISTS);

    remove(file_name);
}

FOSSIL_TEST(c_test_myshell_backup_restore_null_args) {
    fossil_bluecrab_myshell_error_t err;
    const char *file_name = "test_backup_restore_null.myshell";
    fossil_bluecrab_myshell_t *db = fossil_myshell_create(file_name, &err);
    ASSUME_ITS_TRUE(db != NULL);

    err = fossil_myshell_backup(NULL, "backup.bak");
    ASSUME_ITS_TRUE(err == FOSSIL_MYSHELL_ERROR_INVALID_FILE);

    err = fossil_myshell_backup(db, NULL);
    ASSUME_ITS_TRUE(err == FOSSIL_MYSHELL_ERROR_CONFIG_INVALID);

    err = fossil_myshell_backup(db, "");
    ASSUME_ITS_TRUE(err == FOSSIL_MYSHELL_ERROR_CONFIG_INVALID);

    err = fossil_myshell_restore(NULL, "target.myshell");
    ASSUME_ITS_TRUE(err == FOSSIL_MYSHELL_ERROR_INVALID_FILE);

    err = fossil_myshell_restore("backup.bak", NULL);
    ASSUME_ITS_TRUE(err == FOSSIL_MYSHELL_ERROR_INVALID_FILE);

    fossil_myshell_close(db);
    remove(file_name);
}

FOSSIL_TEST(c_test_myshell_diff_null_args) {
    fossil_bluecrab_myshell_error_t err;
    char diff[128];
    err = fossil_myshell_diff(NULL, NULL, diff, sizeof(diff));
    ASSUME_ITS_TRUE(err == FOSSIL_MYSHELL_ERROR_INVALID_FILE);

    err = fossil_myshell_diff(NULL, NULL, NULL, sizeof(diff));
    ASSUME_ITS_TRUE(err == FOSSIL_MYSHELL_ERROR_INVALID_FILE);

    err = fossil_myshell_diff(NULL, NULL, diff, 0);
    ASSUME_ITS_TRUE(err == FOSSIL_MYSHELL_ERROR_INVALID_FILE);
}

FOSSIL_TEST(c_test_myshell_check_integrity_null) {
    fossil_bluecrab_myshell_error_t err = fossil_myshell_check_integrity(NULL);
    ASSUME_ITS_TRUE(err == FOSSIL_MYSHELL_ERROR_INVALID_FILE);
}

// * * * * * * * * * * * * * * * * * * * * * * * *
// * Fossil Logic Test Pool
// * * * * * * * * * * * * * * * * * * * * * * * *
FOSSIL_TEST_GROUP(c_myshell_database_tests) {
    FOSSIL_TEST_ADD(c_myshell_fixture, c_test_myshell_open_create_close);
    FOSSIL_TEST_ADD(c_myshell_fixture, c_test_myshell_commit_branch_checkout);
    FOSSIL_TEST_ADD(c_myshell_fixture, c_test_myshell_errstr);
    FOSSIL_TEST_ADD(c_myshell_fixture, c_test_myshell_put_get_del);
    FOSSIL_TEST_ADD(c_myshell_fixture, c_test_myshell_put_all_types);
    FOSSIL_TEST_ADD(c_myshell_fixture, c_test_myshell_put_invalid_type);
    FOSSIL_TEST_ADD(c_myshell_fixture, c_test_myshell_get_not_found);
    FOSSIL_TEST_ADD(c_myshell_fixture, c_test_myshell_get_buffer_too_small);
    FOSSIL_TEST_ADD(c_myshell_fixture, c_test_myshell_del_not_found);
    FOSSIL_TEST_ADD(c_myshell_fixture, c_test_myshell_del_twice);
    FOSSIL_TEST_ADD(c_myshell_fixture, c_test_myshell_stage_unstage);
    FOSSIL_TEST_ADD(c_myshell_fixture, c_test_myshell_backup_restore);
    FOSSIL_TEST_ADD(c_myshell_fixture, c_test_myshell_open_invalid_path);
    FOSSIL_TEST_ADD(c_myshell_fixture, c_test_myshell_create_existing_file);
    FOSSIL_TEST_ADD(c_myshell_fixture, c_test_myshell_backup_restore_null_args);
    FOSSIL_TEST_ADD(c_myshell_fixture, c_test_myshell_diff_null_args);
    FOSSIL_TEST_ADD(c_myshell_fixture, c_test_myshell_check_integrity_null);

    FOSSIL_TEST_REGISTER(c_myshell_fixture);
} // end of tests

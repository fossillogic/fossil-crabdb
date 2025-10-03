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

FOSSIL_TEST(c_test_myshell_put_get_del) {
    fossil_bluecrab_myshell_error_t err;
    const char *file_name = "test3.myshell";
    fossil_bluecrab_myshell_t *db = fossil_myshell_create(file_name, &err);
    ASSUME_ITS_TRUE(db != NULL);

    err = fossil_myshell_put(db, "foo", "bar");
    ASSUME_ITS_TRUE(err == FOSSIL_MYSHELL_ERROR_SUCCESS);

    char value[128];
    err = fossil_myshell_get(db, "foo", value, sizeof(value));
    ASSUME_ITS_TRUE(err == FOSSIL_MYSHELL_ERROR_SUCCESS);
    ASSUME_ITS_EQUAL_CSTR(value, "bar");

    err = fossil_myshell_del(db, "foo");
    ASSUME_ITS_TRUE(err == FOSSIL_MYSHELL_ERROR_SUCCESS);

    err = fossil_myshell_get(db, "foo", value, sizeof(value));
    ASSUME_ITS_TRUE(err == FOSSIL_MYSHELL_ERROR_NOT_FOUND);

    fossil_myshell_close(db);
    remove(file_name);
}

FOSSIL_TEST(c_test_myshell_commit_branch_checkout) {
    fossil_bluecrab_myshell_error_t err;
    const char *file_name = "test4.myshell";
    fossil_bluecrab_myshell_t *db = fossil_myshell_create(file_name, &err);
    ASSUME_ITS_TRUE(db != NULL);

    err = fossil_myshell_put(db, "key", "val");
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

static bool log_cb(const char *commit_hash, const char *message, void *user) {
    int *count = (int *)user;
    (*count)++;
    return true;
}

FOSSIL_TEST(c_test_myshell_log_history) {
    fossil_bluecrab_myshell_error_t err;
    const char *file_name = "test5.myshell";
    fossil_bluecrab_myshell_t *db = fossil_myshell_create(file_name, &err);
    ASSUME_ITS_TRUE(db != NULL);

    fossil_myshell_commit(db, "Commit 1");
    fossil_myshell_commit(db, "Commit 2");
    fossil_myshell_commit(db, "Commit 3");

    int count = 0;
    err = fossil_myshell_log(db, log_cb, &count);
    ASSUME_ITS_TRUE(err == FOSSIL_MYSHELL_ERROR_SUCCESS);
    ASSUME_ITS_TRUE(count == 3);

    fossil_myshell_close(db);
    remove(file_name);
}

FOSSIL_TEST(c_test_myshell_backup_restore) {
    fossil_bluecrab_myshell_error_t err;
    const char *file_name = "test6.myshell";
    const char *backup_name = "test6_backup.myshell";
    const char *restore_name = "test6_restored.myshell";

    fossil_bluecrab_myshell_t *db = fossil_myshell_create(file_name, &err);
    ASSUME_ITS_TRUE(db != NULL);

    fossil_myshell_put(db, "a", "b");
    fossil_myshell_commit(db, "Backup commit");

    err = fossil_myshell_backup(db, backup_name);
    ASSUME_ITS_TRUE(err == FOSSIL_MYSHELL_ERROR_SUCCESS);

    fossil_myshell_close(db);

    err = fossil_myshell_restore(backup_name, restore_name);
    ASSUME_ITS_TRUE(err == FOSSIL_MYSHELL_ERROR_SUCCESS);

    fossil_bluecrab_myshell_t *db2 = fossil_myshell_open(restore_name, &err);
    ASSUME_ITS_TRUE(db2 != NULL);

    char value[32];
    err = fossil_myshell_get(db2, "a", value, sizeof(value));
    ASSUME_ITS_TRUE(err == FOSSIL_MYSHELL_ERROR_SUCCESS);
    ASSUME_ITS_EQUAL_CSTR(value, "b");

    fossil_myshell_close(db2);
    remove(file_name);
    remove(backup_name);
    remove(restore_name);
}

FOSSIL_TEST(c_test_myshell_errstr) {
    ASSUME_ITS_EQUAL_CSTR(fossil_myshell_errstr(FOSSIL_MYSHELL_ERROR_SUCCESS), "Success");
    ASSUME_ITS_EQUAL_CSTR(fossil_myshell_errstr(FOSSIL_MYSHELL_ERROR_NOT_FOUND), "Not found");
    ASSUME_ITS_EQUAL_CSTR(fossil_myshell_errstr(FOSSIL_MYSHELL_ERROR_INVALID_FILE), "Invalid file");
    ASSUME_ITS_EQUAL_CSTR(fossil_myshell_errstr(9999), "Unknown error");
}

// Edge case tests for myshell

FOSSIL_TEST(c_test_myshell_empty_strings) {
    fossil_bluecrab_myshell_error_t err;
    const char *file_name = "emptystr.myshell";
    fossil_bluecrab_myshell_t *db = fossil_myshell_create(file_name, &err);
    ASSUME_ITS_TRUE(db != NULL);

    // Empty key
    err = fossil_myshell_put(db, "", "value");
    ASSUME_ITS_TRUE(err != FOSSIL_MYSHELL_ERROR_SUCCESS);

    // Empty value
    err = fossil_myshell_put(db, "key", "");
    ASSUME_ITS_TRUE(err == FOSSIL_MYSHELL_ERROR_SUCCESS);

    char value[16];
    err = fossil_myshell_get(db, "key", value, sizeof(value));
    ASSUME_ITS_TRUE(err == FOSSIL_MYSHELL_ERROR_SUCCESS);
    ASSUME_ITS_EQUAL_CSTR(value, "");

    fossil_myshell_close(db);
    remove(file_name);
}

FOSSIL_TEST(c_test_myshell_corrupted_key_hash) {
    fossil_bluecrab_myshell_error_t err;
    const char *file_name = "corrupt_key.myshell";
    fossil_bluecrab_myshell_t *db = fossil_myshell_create(file_name, &err);
    ASSUME_ITS_TRUE(db != NULL);

    fossil_myshell_put(db, "corruptkey", "corruptval");
    fossil_myshell_commit(db, "commit");

    fossil_myshell_close(db);

    // Corrupt the key hash in the file
    FILE *file = fopen(file_name, "rb+");
    ASSUME_ITS_TRUE(file != NULL);
    char line[1024];
    long pos = 0;
    while (fgets(line, sizeof(line), file)) {
        char *hash_comment = strstr(line, "#hash=");
        if (hash_comment) {
            pos = ftell(file) - strlen(line) + (hash_comment - line) + 6;
            break;
        }
    }
    fseek(file, pos, SEEK_SET);
    fputs("deadbeefdeadbeef", file); // overwrite hash with invalid value
    fclose(file);

    db = fossil_myshell_open(file_name, &err);
    ASSUME_ITS_TRUE(db != NULL);

    err = fossil_myshell_check_integrity(db);
    ASSUME_ITS_TRUE(err == FOSSIL_MYSHELL_ERROR_INTEGRITY);

    fossil_myshell_close(db);
    remove(file_name);
}

FOSSIL_TEST(c_test_myshell_corrupted_file_size) {
    fossil_bluecrab_myshell_error_t err;
    const char *file_name = "corrupt_size.myshell";
    fossil_bluecrab_myshell_t *db = fossil_myshell_create(file_name, &err);
    ASSUME_ITS_TRUE(db != NULL);

    fossil_myshell_put(db, "sizekey", "sizeval");
    fossil_myshell_commit(db, "commit");

    // Artificially change db->file_size to simulate corruption
    db->file_size += 10;

    err = fossil_myshell_check_integrity(db);
    ASSUME_ITS_TRUE(err == FOSSIL_MYSHELL_ERROR_CORRUPTED);

    fossil_myshell_close(db);
    remove(file_name);
}

FOSSIL_TEST(c_test_myshell_parse_failed_commit) {
    fossil_bluecrab_myshell_error_t err;
    const char *file_name = "parsefail.myshell";
    fossil_bluecrab_myshell_t *db = fossil_myshell_create(file_name, &err);
    ASSUME_ITS_TRUE(db != NULL);

    fossil_myshell_put(db, "parsekey", "parseval");
    fossil_myshell_commit(db, "parse commit");

    fossil_myshell_close(db);

    // Remove timestamp from commit line to cause parse failure
    FILE *file = fopen(file_name, "rb+");
    ASSUME_ITS_TRUE(file != NULL);
    char lines[10][1024];
    int count = 0;
    while (fgets(lines[count], sizeof(lines[count]), file)) {
        count++;
    }
    fclose(file);

    file = fopen(file_name, "wb");
    ASSUME_ITS_TRUE(file != NULL);
    for (int i = 0; i < count; ++i) {
        if (strncmp(lines[i], "#commit ", 8) == 0) {
            char hash_str[17], msg[512];
            int n = sscanf(lines[i], "#commit %16s %511[^\n]", hash_str, msg);
            if (n == 2) {
                fprintf(file, "#commit %s %s\n", hash_str, msg); // omit timestamp
            }
        } else {
            fputs(lines[i], file);
        }
    }
    fclose(file);

    db = fossil_myshell_open(file_name, &err);
    ASSUME_ITS_TRUE(db != NULL);

    err = fossil_myshell_check_integrity(db);
    ASSUME_ITS_TRUE(err == FOSSIL_MYSHELL_ERROR_PARSE_FAILED);

    fossil_myshell_close(db);
    remove(file_name);
}

// * * * * * * * * * * * * * * * * * * * * * * * *
// * Fossil Logic Test Pool
// * * * * * * * * * * * * * * * * * * * * * * * *
FOSSIL_TEST_GROUP(c_myshell_database_tests) {
    FOSSIL_TEST_ADD(c_myshell_fixture, c_test_myshell_open_create_close);
    FOSSIL_TEST_ADD(c_myshell_fixture, c_test_myshell_put_get_del);
    FOSSIL_TEST_ADD(c_myshell_fixture, c_test_myshell_commit_branch_checkout);
    FOSSIL_TEST_ADD(c_myshell_fixture, c_test_myshell_log_history);
    FOSSIL_TEST_ADD(c_myshell_fixture, c_test_myshell_backup_restore);
    FOSSIL_TEST_ADD(c_myshell_fixture, c_test_myshell_errstr);
    FOSSIL_TEST_ADD(c_myshell_fixture, c_test_myshell_empty_strings);
    FOSSIL_TEST_ADD(c_myshell_fixture, c_test_myshell_corrupted_key_hash);
    FOSSIL_TEST_ADD(c_myshell_fixture, c_test_myshell_corrupted_file_size);
    FOSSIL_TEST_ADD(c_myshell_fixture, c_test_myshell_parse_failed_commit);

    FOSSIL_TEST_REGISTER(c_myshell_fixture);
} // end of tests

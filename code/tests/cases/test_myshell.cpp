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

FOSSIL_SUITE(cpp_myshell_fixture);

FOSSIL_SETUP(cpp_myshell_fixture) {
    // Setup the test fixture
}

FOSSIL_TEARDOWN(cpp_myshell_fixture) {
    // Teardown the test fixture
}

// * * * * * * * * * * * * * * * * * * * * * * * *
// * Fossil Logic Test Blue CrabDB Database
// * * * * * * * * * * * * * * * * * * * * * * * *

/*
 * Test case for creating a new record in the database file (FSON encoding)
 * Uses C++ RAII wrapper
 */
FOSSIL_TEST(cpp_test_myshell_open_create_close) {
    fossil_bluecrab_myshell_error_t err;
    const std::string file_name = "test2.myshell";

    auto db = fossil::bluecrab::MyShell::create(file_name, err);
    ASSUME_ITS_TRUE(db.is_open());
    ASSUME_ITS_TRUE(err == FOSSIL_MYSHELL_ERROR_SUCCESS);

    db.close();

    fossil::bluecrab::MyShell db2(file_name, err);
    ASSUME_ITS_TRUE(db2.is_open());
    ASSUME_ITS_TRUE(err == FOSSIL_MYSHELL_ERROR_SUCCESS);

    db2.close();

    remove(file_name.c_str());
}

FOSSIL_TEST(cpp_test_myshell_commit_branch_checkout) {
    fossil_bluecrab_myshell_error_t err;
    const std::string file_name = "test4.myshell";
    auto db = fossil::bluecrab::MyShell::create(file_name, err);
    ASSUME_ITS_TRUE(db.is_open());

    err = db.put("key", "cstr", "val");
    ASSUME_ITS_TRUE(err == FOSSIL_MYSHELL_ERROR_SUCCESS);

    err = db.commit("Initial commit");
    ASSUME_ITS_TRUE(err == FOSSIL_MYSHELL_ERROR_SUCCESS);

    err = db.branch("feature");
    ASSUME_ITS_TRUE(err == FOSSIL_MYSHELL_ERROR_SUCCESS);

    err = db.checkout("feature");
    ASSUME_ITS_TRUE(err == FOSSIL_MYSHELL_ERROR_SUCCESS);

    db.close();
    remove(file_name.c_str());
}

FOSSIL_TEST(cpp_test_myshell_errstr) {
    ASSUME_ITS_EQUAL_CSTR(fossil::bluecrab::MyShell::errstr(FOSSIL_MYSHELL_ERROR_SUCCESS), "Success");
    ASSUME_ITS_EQUAL_CSTR(fossil::bluecrab::MyShell::errstr(FOSSIL_MYSHELL_ERROR_NOT_FOUND), "Not found");
    ASSUME_ITS_EQUAL_CSTR(fossil::bluecrab::MyShell::errstr(FOSSIL_MYSHELL_ERROR_INVALID_FILE), "Invalid file");
}

// Edge case tests for myshell

FOSSIL_TEST(cpp_test_myshell_corrupted_key_hash) {
    fossil_bluecrab_myshell_error_t err;
    const std::string file_name = "corrupt_key.myshell";
    auto db = fossil::bluecrab::MyShell::create(file_name, err);
    ASSUME_ITS_TRUE(db.is_open());

    db.put("corruptkey", "cstr", "corruptval");
    db.commit("commit");

    db.close();

    // Corrupt the key hash in the file
    FILE *file = fopen(file_name.c_str(), "rb+");
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

    fossil::bluecrab::MyShell db2(file_name, err);
    ASSUME_ITS_TRUE(db2.is_open());

    err = db2.check_integrity();
    ASSUME_ITS_TRUE(err == FOSSIL_MYSHELL_ERROR_INTEGRITY);

    db2.close();
    remove(file_name.c_str());
}

FOSSIL_TEST(cpp_test_myshell_corrupted_file_size) {
    fossil_bluecrab_myshell_error_t err;
    const std::string file_name = "corrupt_size.myshell";
    auto db = fossil::bluecrab::MyShell::create(file_name, err);
    ASSUME_ITS_TRUE(db.is_open());

    db.put("sizekey", "cstr", "sizeval");
    db.commit("commit");

    // Artificially change db->file_size to simulate corruption
    db.handle()->file_size += 10;

    err = db.check_integrity();
    ASSUME_ITS_TRUE(err == FOSSIL_MYSHELL_ERROR_CORRUPTED);

    db.close();
    remove(file_name.c_str());
}

FOSSIL_TEST(cpp_test_myshell_parse_failed_commit) {
    fossil_bluecrab_myshell_error_t err;
    const std::string file_name = "parsefail.myshell";
    auto db = fossil::bluecrab::MyShell::create(file_name, err);
    ASSUME_ITS_TRUE(db.is_open());

    db.put("parsekey", "cstr", "parseval");
    db.commit("parse commit");

    db.close();

    // Remove timestamp from commit line to cause parse failure
    FILE *file = fopen(file_name.c_str(), "rb+");
    ASSUME_ITS_TRUE(file != NULL);
    char lines[10][1024];
    int count = 0;
    while (fgets(lines[count], sizeof(lines[count]), file)) {
        count++;
    }
    fclose(file);

    file = fopen(file_name.c_str(), "wb");
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

    fossil::bluecrab::MyShell db2(file_name, err);
    ASSUME_ITS_TRUE(db2.is_open());

    err = db2.check_integrity();
    ASSUME_ITS_TRUE(err == FOSSIL_MYSHELL_ERROR_PARSE_FAILED);

    db2.close();
    remove(file_name.c_str());
}

// * * * * * * * * * * * * * * * * * * * * * * * *
// * Fossil Logic Test Pool
// * * * * * * * * * * * * * * * * * * * * * * * *
FOSSIL_TEST_GROUP(cpp_myshell_database_tests) {
    FOSSIL_TEST_ADD(cpp_myshell_fixture, cpp_test_myshell_open_create_close);
    FOSSIL_TEST_ADD(cpp_myshell_fixture, cpp_test_myshell_commit_branch_checkout);
    FOSSIL_TEST_ADD(cpp_myshell_fixture, cpp_test_myshell_errstr);
    FOSSIL_TEST_ADD(cpp_myshell_fixture, cpp_test_myshell_corrupted_key_hash);
    FOSSIL_TEST_ADD(cpp_myshell_fixture, cpp_test_myshell_corrupted_file_size);
    FOSSIL_TEST_ADD(cpp_myshell_fixture, cpp_test_myshell_parse_failed_commit);

    FOSSIL_TEST_REGISTER(cpp_myshell_fixture);
} // end of tests

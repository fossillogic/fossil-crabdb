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
 * Now using fossil::bluecrab::MyShell C++ RAII wrapper.
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

FOSSIL_TEST(cpp_test_myshell_put_get_del) {
    fossil_bluecrab_myshell_error_t err;
    const std::string file_name = "test3.myshell";
    auto db = fossil::bluecrab::MyShell::create(file_name, err);
    ASSUME_ITS_TRUE(db.is_open());

    err = db.put("foo", "bar");
    ASSUME_ITS_TRUE(err == FOSSIL_MYSHELL_ERROR_SUCCESS);

    std::string value;
    err = db.get("foo", value);
    ASSUME_ITS_TRUE(err == FOSSIL_MYSHELL_ERROR_SUCCESS);
    ASSUME_ITS_EQUAL_CSTR(value.c_str(), "bar");

    err = db.del("foo");
    ASSUME_ITS_TRUE(err == FOSSIL_MYSHELL_ERROR_SUCCESS);

    err = db.get("foo", value);
    ASSUME_ITS_TRUE(err == FOSSIL_MYSHELL_ERROR_NOT_FOUND);

    db.close();
    remove(file_name.c_str());
}

FOSSIL_TEST(cpp_test_myshell_commit_branch_checkout) {
    fossil_bluecrab_myshell_error_t err;
    const std::string file_name = "test4.myshell";
    auto db = fossil::bluecrab::MyShell::create(file_name, err);
    ASSUME_ITS_TRUE(db.is_open());

    err = db.put("key", "val");
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

static bool log_cb(const char *commit_hash, const char *message, void *user) {
    int *count = (int *)user;
    (*count)++;
    return true;
}

FOSSIL_TEST(cpp_test_myshell_log_history) {
    fossil_bluecrab_myshell_error_t err;
    const std::string file_name = "test5.myshell";
    auto db = fossil::bluecrab::MyShell::create(file_name, err);
    ASSUME_ITS_TRUE(db.is_open());

    db.commit("Commit 1");
    db.commit("Commit 2");
    db.commit("Commit 3");

    int count = 0;
    err = db.log(log_cb, &count);
    ASSUME_ITS_TRUE(err == FOSSIL_MYSHELL_ERROR_SUCCESS);
    ASSUME_ITS_TRUE(count == 3);

    db.close();
    remove(file_name.c_str());
}

FOSSIL_TEST(cpp_test_myshell_backup_restore) {
    fossil_bluecrab_myshell_error_t err;
    const std::string file_name = "test6.myshell";
    const std::string backup_name = "test6_backup.myshell";
    const std::string restore_name = "test6_restored.myshell";

    auto db = fossil::bluecrab::MyShell::create(file_name, err);
    ASSUME_ITS_TRUE(db.is_open());

    db.put("a", "b");
    db.commit("Backup commit");

    err = db.backup(backup_name);
    ASSUME_ITS_TRUE(err == FOSSIL_MYSHELL_ERROR_SUCCESS);

    db.close();

    err = fossil::bluecrab::MyShell::restore(backup_name, restore_name);
    ASSUME_ITS_TRUE(err == FOSSIL_MYSHELL_ERROR_SUCCESS);

    fossil::bluecrab::MyShell db2(restore_name, err);
    ASSUME_ITS_TRUE(db2.is_open());

    std::string value;
    err = db2.get("a", value);
    ASSUME_ITS_TRUE(err == FOSSIL_MYSHELL_ERROR_SUCCESS);
    ASSUME_ITS_EQUAL_CSTR(value.c_str(), "b");

    db2.close();
    remove(file_name.c_str());
    remove(backup_name.c_str());
    remove(restore_name.c_str());
}

FOSSIL_TEST(cpp_test_myshell_errstr) {
    ASSUME_ITS_EQUAL_CSTR(fossil::bluecrab::MyShell::errstr(FOSSIL_MYSHELL_ERROR_SUCCESS), "Success");
    ASSUME_ITS_EQUAL_CSTR(fossil::bluecrab::MyShell::errstr(FOSSIL_MYSHELL_ERROR_NOT_FOUND), "Not found");
    ASSUME_ITS_EQUAL_CSTR(fossil::bluecrab::MyShell::errstr(FOSSIL_MYSHELL_ERROR_INVALID_FILE), "Invalid file");
    ASSUME_ITS_EQUAL_CSTR(fossil::bluecrab::MyShell::errstr(static_cast<fossil_bluecrab_myshell_error_t>(9999)), "Unknown error");
}

// Edge case tests for myshell

FOSSIL_TEST(cpp_test_myshell_empty_strings) {
    fossil_bluecrab_myshell_error_t err;
    const std::string file_name = "emptystr.myshell";
    auto db = fossil::bluecrab::MyShell::create(file_name, err);
    ASSUME_ITS_TRUE(db.is_open());

    // Empty key
    err = db.put("", "value");
    ASSUME_ITS_TRUE(err != FOSSIL_MYSHELL_ERROR_SUCCESS);

    // Empty value
    err = db.put("key", "");
    ASSUME_ITS_TRUE(err == FOSSIL_MYSHELL_ERROR_SUCCESS);

    std::string value;
    err = db.get("key", value);
    ASSUME_ITS_TRUE(err == FOSSIL_MYSHELL_ERROR_SUCCESS);
    ASSUME_ITS_EQUAL_CSTR(value.c_str(), "");

    db.close();
    remove(file_name.c_str());
}

// * * * * * * * * * * * * * * * * * * * * * * * *
// * Fossil Logic Test Pool
// * * * * * * * * * * * * * * * * * * * * * * * *
FOSSIL_TEST_GROUP(cpp_myshell_database_tests) {
    FOSSIL_TEST_ADD(cpp_myshell_fixture, cpp_test_myshell_open_create_close);
    FOSSIL_TEST_ADD(cpp_myshell_fixture, cpp_test_myshell_put_get_del);
    FOSSIL_TEST_ADD(cpp_myshell_fixture, cpp_test_myshell_commit_branch_checkout);
    FOSSIL_TEST_ADD(cpp_myshell_fixture, cpp_test_myshell_log_history);
    FOSSIL_TEST_ADD(cpp_myshell_fixture, cpp_test_myshell_backup_restore);
    FOSSIL_TEST_ADD(cpp_myshell_fixture, cpp_test_myshell_errstr);
    FOSSIL_TEST_ADD(cpp_myshell_fixture, cpp_test_myshell_empty_strings);

    FOSSIL_TEST_REGISTER(cpp_myshell_fixture);
} // end of tests

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
 * Now uses fossil::bluecrab::MyShell C++ RAII wrapper.
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
    ASSUME_ITS_EQUAL_CSTR(fossil::bluecrab::MyShell::errstr(static_cast<fossil_bluecrab_myshell_error_t>(9999)), "Unknown error");
}

FOSSIL_TEST(cpp_test_myshell_put_get_del) {
    fossil_bluecrab_myshell_error_t err;
    const std::string file_name = "test_put_get_del.myshell";
    auto db = fossil::bluecrab::MyShell::create(file_name, err);
    ASSUME_ITS_TRUE(db.is_open());

    // Put key-value pairs
    err = db.put("username", "cstr", "alice");
    ASSUME_ITS_TRUE(err == FOSSIL_MYSHELL_ERROR_SUCCESS);

    err = db.put("password", "cstr", "secret");
    ASSUME_ITS_TRUE(err == FOSSIL_MYSHELL_ERROR_SUCCESS);

    // Get key-value pairs
    std::string value;
    err = db.get("username", value);
    ASSUME_ITS_TRUE(err == FOSSIL_MYSHELL_ERROR_SUCCESS);
    ASSUME_ITS_EQUAL_CSTR(value.c_str(), "alice");

    err = db.get("password", value);
    ASSUME_ITS_TRUE(err == FOSSIL_MYSHELL_ERROR_SUCCESS);
    ASSUME_ITS_EQUAL_CSTR(value.c_str(), "secret");

    // Delete key
    err = db.del("username");
    ASSUME_ITS_TRUE(err == FOSSIL_MYSHELL_ERROR_SUCCESS);

    err = db.get("username", value);
    ASSUME_ITS_TRUE(err == FOSSIL_MYSHELL_ERROR_NOT_FOUND);

    db.close();
    remove(file_name.c_str());
}

FOSSIL_TEST(cpp_test_myshell_put_all_types) {
    fossil_bluecrab_myshell_error_t err;
    const std::string file_name = "test_put_all_types.myshell";
    auto db = fossil::bluecrab::MyShell::create(file_name, err);
    ASSUME_ITS_TRUE(db.is_open());

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

    for (size_t i = 0; i < sizeof(types)/sizeof(types[0]); ++i) {
        std::string key = std::string("key_") + types[i];
        err = db.put(key, types[i], values[i]);
        ASSUME_ITS_TRUE(err == FOSSIL_MYSHELL_ERROR_SUCCESS);

        std::string value;
        err = db.get(key, value);
        ASSUME_ITS_TRUE(err == FOSSIL_MYSHELL_ERROR_SUCCESS);
        ASSUME_ITS_EQUAL_CSTR(value.c_str(), values[i]);
    }

    db.close();
    remove(file_name.c_str());
}

FOSSIL_TEST(cpp_test_myshell_put_invalid_type) {
    fossil_bluecrab_myshell_error_t err;
    const std::string file_name = "test_put_invalid_type.myshell";
    auto db = fossil::bluecrab::MyShell::create(file_name, err);
    ASSUME_ITS_TRUE(db.is_open());

    err = db.put("badkey", "notatype", "value");
    ASSUME_ITS_TRUE(err == FOSSIL_MYSHELL_ERROR_INVALID_TYPE);

    db.close();
    remove(file_name.c_str());
}

FOSSIL_TEST(cpp_test_myshell_get_not_found) {
    fossil_bluecrab_myshell_error_t err;
    const std::string file_name = "test_get_not_found.myshell";
    auto db = fossil::bluecrab::MyShell::create(file_name, err);
    ASSUME_ITS_TRUE(db.is_open());

    std::string value;
    err = db.get("nonexistent", value);
    ASSUME_ITS_TRUE(err == FOSSIL_MYSHELL_ERROR_NOT_FOUND);

    db.close();
    remove(file_name.c_str());
}

FOSSIL_TEST(cpp_test_myshell_get_buffer_too_small) {
    fossil_bluecrab_myshell_error_t err;
    const std::string file_name = "test_get_buffer_small.myshell";
    auto db = fossil::bluecrab::MyShell::create(file_name, err);
    ASSUME_ITS_TRUE(db.is_open());

    err = db.put("shortkey", "cstr", "longvalue");
    ASSUME_ITS_TRUE(err == FOSSIL_MYSHELL_ERROR_SUCCESS);

    // Direct C API call for buffer-too-small test
    char value[4]; // too small for "longvalue"
    err = fossil_myshell_get(db.handle(), "shortkey", value, sizeof(value));
    ASSUME_ITS_TRUE(err == FOSSIL_MYSHELL_ERROR_BUFFER_TOO_SMALL);

    db.close();
    remove(file_name.c_str());
}

FOSSIL_TEST(cpp_test_myshell_del_not_found) {
    fossil_bluecrab_myshell_error_t err;
    const std::string file_name = "test_del_not_found.myshell";
    auto db = fossil::bluecrab::MyShell::create(file_name, err);
    ASSUME_ITS_TRUE(db.is_open());

    // Try to delete a key that does not exist
    err = db.del("nonexistent_key");
    ASSUME_ITS_TRUE(err == FOSSIL_MYSHELL_ERROR_NOT_FOUND);

    db.close();
    remove(file_name.c_str());
}

FOSSIL_TEST(cpp_test_myshell_del_twice) {
    fossil_bluecrab_myshell_error_t err;
    const std::string file_name = "test_del_twice.myshell";
    auto db = fossil::bluecrab::MyShell::create(file_name, err);
    ASSUME_ITS_TRUE(db.is_open());

    // Put a key and delete it
    err = db.put("key", "cstr", "value");
    ASSUME_ITS_TRUE(err == FOSSIL_MYSHELL_ERROR_SUCCESS);

    err = db.del("key");
    ASSUME_ITS_TRUE(err == FOSSIL_MYSHELL_ERROR_SUCCESS);

    // Try to delete again
    err = db.del("key");
    ASSUME_ITS_TRUE(err == FOSSIL_MYSHELL_ERROR_NOT_FOUND);

    db.close();
    remove(file_name.c_str());
}

FOSSIL_TEST(cpp_test_myshell_stage_unstage) {
    fossil_bluecrab_myshell_error_t err;
    const std::string file_name = "test_stage_unstage.myshell";
    auto db = fossil::bluecrab::MyShell::create(file_name, err);
    ASSUME_ITS_TRUE(db.is_open());

    err = db.stage("foo", "cstr", "bar");
    ASSUME_ITS_TRUE(err == FOSSIL_MYSHELL_ERROR_SUCCESS);

    err = db.unstage("foo");
    ASSUME_ITS_TRUE(err == FOSSIL_MYSHELL_ERROR_SUCCESS);

    // Unstage again should return NOT_FOUND
    err = db.unstage("foo");
    ASSUME_ITS_TRUE(err == FOSSIL_MYSHELL_ERROR_NOT_FOUND);

    db.close();
    remove(file_name.c_str());
}

FOSSIL_TEST(cpp_test_myshell_backup_restore) {
    fossil_bluecrab_myshell_error_t err;
    const std::string file_name = "test_backup_restore.myshell";
    const std::string backup_file = "test_backup_restore.bak";
    const std::string restore_file = "test_backup_restore_restored.myshell";
    auto db = fossil::bluecrab::MyShell::create(file_name, err);
    ASSUME_ITS_TRUE(db.is_open());

    err = db.put("alpha", "cstr", "beta");
    ASSUME_ITS_TRUE(err == FOSSIL_MYSHELL_ERROR_SUCCESS);

    err = db.backup(backup_file);
    ASSUME_ITS_TRUE(err == FOSSIL_MYSHELL_ERROR_SUCCESS);

    db.close();

    err = fossil::bluecrab::MyShell::restore(backup_file, restore_file);
    ASSUME_ITS_TRUE(err == FOSSIL_MYSHELL_ERROR_SUCCESS);

    fossil::bluecrab::MyShell db2(restore_file, err);
    ASSUME_ITS_TRUE(db2.is_open());

    std::string value;
    err = db2.get("alpha", value);
    ASSUME_ITS_TRUE(err == FOSSIL_MYSHELL_ERROR_SUCCESS);
    ASSUME_ITS_EQUAL_CSTR(value.c_str(), "beta");

    db2.close();
    remove(file_name.c_str());
    remove(backup_file.c_str());
    remove(restore_file.c_str());
}

FOSSIL_TEST(cpp_test_myshell_open_invalid_path) {
    fossil_bluecrab_myshell_error_t err;
    fossil::bluecrab::MyShell db1("", err);
    ASSUME_ITS_TRUE(!db1.is_open());
    ASSUME_ITS_TRUE(err == FOSSIL_MYSHELL_ERROR_INVALID_FILE);

    fossil::bluecrab::MyShell db2("not_a_myshell.txt", err);
    ASSUME_ITS_TRUE(!db2.is_open());
    ASSUME_ITS_TRUE(err == FOSSIL_MYSHELL_ERROR_INVALID_FILE);
}

FOSSIL_TEST(cpp_test_myshell_create_existing_file) {
    fossil_bluecrab_myshell_error_t err;
    const std::string file_name = "test_existing.myshell";
    auto db = fossil::bluecrab::MyShell::create(file_name, err);
    ASSUME_ITS_TRUE(db.is_open());
    db.close();

    auto db2 = fossil::bluecrab::MyShell::create(file_name, err);
    ASSUME_ITS_TRUE(!db2.is_open());
    ASSUME_ITS_TRUE(err == FOSSIL_MYSHELL_ERROR_ALREADY_EXISTS);

    remove(file_name.c_str());
}

FOSSIL_TEST(cpp_test_myshell_backup_restore_null_args) {
    fossil_bluecrab_myshell_error_t err;
    const std::string file_name = "test_backup_restore_null.myshell";
    auto db = fossil::bluecrab::MyShell::create(file_name, err);
    ASSUME_ITS_TRUE(db.is_open());

    err = fossil_myshell_backup(nullptr, "backup.bak");
    ASSUME_ITS_TRUE(err == FOSSIL_MYSHELL_ERROR_INVALID_FILE);

    err = fossil_myshell_backup(db.handle(), nullptr);
    ASSUME_ITS_TRUE(err == FOSSIL_MYSHELL_ERROR_CONFIG_INVALID);

    err = fossil_myshell_backup(db.handle(), "");
    ASSUME_ITS_TRUE(err == FOSSIL_MYSHELL_ERROR_CONFIG_INVALID);

    err = fossil_myshell_restore(nullptr, "target.myshell");
    ASSUME_ITS_TRUE(err == FOSSIL_MYSHELL_ERROR_INVALID_FILE);

    err = fossil_myshell_restore("backup.bak", nullptr);
    ASSUME_ITS_TRUE(err == FOSSIL_MYSHELL_ERROR_INVALID_FILE);

    db.close();
    remove(file_name.c_str());
}

FOSSIL_TEST(cpp_test_myshell_diff_null_args) {
    fossil_bluecrab_myshell_error_t err;
    char diff[128];
    err = fossil_myshell_diff(nullptr, nullptr, diff, sizeof(diff));
    ASSUME_ITS_TRUE(err == FOSSIL_MYSHELL_ERROR_INVALID_FILE);

    err = fossil_myshell_diff(nullptr, nullptr, nullptr, sizeof(diff));
    ASSUME_ITS_TRUE(err == FOSSIL_MYSHELL_ERROR_INVALID_FILE);

    err = fossil_myshell_diff(nullptr, nullptr, diff, 0);
    ASSUME_ITS_TRUE(err == FOSSIL_MYSHELL_ERROR_INVALID_FILE);
}

FOSSIL_TEST(cpp_test_myshell_check_integrity_null) {
    fossil_bluecrab_myshell_error_t err = fossil_myshell_check_integrity(nullptr);
    ASSUME_ITS_TRUE(err == FOSSIL_MYSHELL_ERROR_INVALID_FILE);
}

// * * * * * * * * * * * * * * * * * * * * * * * *
// * Fossil Logic Test Pool
// * * * * * * * * * * * * * * * * * * * * * * * *
FOSSIL_TEST_GROUP(cpp_myshell_database_tests) {
    FOSSIL_TEST_ADD(cpp_myshell_fixture, cpp_test_myshell_open_create_close);
    FOSSIL_TEST_ADD(cpp_myshell_fixture, cpp_test_myshell_commit_branch_checkout);
    FOSSIL_TEST_ADD(cpp_myshell_fixture, cpp_test_myshell_errstr);
    FOSSIL_TEST_ADD(cpp_myshell_fixture, cpp_test_myshell_put_get_del);
    FOSSIL_TEST_ADD(cpp_myshell_fixture, cpp_test_myshell_put_all_types);
    FOSSIL_TEST_ADD(cpp_myshell_fixture, cpp_test_myshell_put_invalid_type);
    FOSSIL_TEST_ADD(cpp_myshell_fixture, cpp_test_myshell_get_not_found);
    FOSSIL_TEST_ADD(cpp_myshell_fixture, cpp_test_myshell_get_buffer_too_small);
    FOSSIL_TEST_ADD(cpp_myshell_fixture, cpp_test_myshell_del_not_found);
    FOSSIL_TEST_ADD(cpp_myshell_fixture, cpp_test_myshell_del_twice);
    FOSSIL_TEST_ADD(cpp_myshell_fixture, cpp_test_myshell_stage_unstage);
    FOSSIL_TEST_ADD(cpp_myshell_fixture, cpp_test_myshell_backup_restore);
    FOSSIL_TEST_ADD(cpp_myshell_fixture, cpp_test_myshell_open_invalid_path);
    FOSSIL_TEST_ADD(cpp_myshell_fixture, cpp_test_myshell_create_existing_file);
    FOSSIL_TEST_ADD(cpp_myshell_fixture, cpp_test_myshell_backup_restore_null_args);
    FOSSIL_TEST_ADD(cpp_myshell_fixture, cpp_test_myshell_diff_null_args);
    FOSSIL_TEST_ADD(cpp_myshell_fixture, cpp_test_myshell_check_integrity_null);

    FOSSIL_TEST_REGISTER(cpp_myshell_fixture);
} // end of tests

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

FOSSIL_SUITE(cpp_noshell_fixture);

FOSSIL_SETUP(cpp_noshell_fixture) {
    // Setup the test fixture
}

FOSSIL_TEARDOWN(cpp_noshell_fixture) {
    // Teardown the test fixture
}

// * * * * * * * * * * * * * * * * * * * * * * * *
// * Fossil Logic Test Blue CrabDB Database
// * * * * * * * * * * * * * * * * * * * * * * * *

// * * * * * * * * * * * * * * * * * * * * * * * *
// * Fossil Logic Test Blue CrabDB NoShell Database
// * * * * * * * * * * * * * * * * * * * * * * * *

FOSSIL_TEST(cpp_test_noshell_create_open_delete) {
    using fossil::bluecrab::NoShell;
    const std::string file_name = "test_noshell.crabdb";

    auto err = NoShell::create_database(file_name);
    ASSUME_ITS_TRUE(err == FOSSIL_NOSHELL_ERROR_SUCCESS);

    err = NoShell::open_database(file_name);
    ASSUME_ITS_TRUE(err == FOSSIL_NOSHELL_ERROR_SUCCESS);

    err = NoShell::delete_database(file_name);
    ASSUME_ITS_TRUE(err == FOSSIL_NOSHELL_ERROR_SUCCESS);
}

FOSSIL_TEST(cpp_test_noshell_insert_find_remove) {
    using fossil::bluecrab::NoShell;
    const std::string file_name = "test_noshell_insert.crabdb";
    const std::string doc = "{\"user\":\"alice\",\"pass\":\"secret\"}";

    auto err = NoShell::create_database(file_name);
    ASSUME_ITS_TRUE(err == FOSSIL_NOSHELL_ERROR_SUCCESS);

    err = NoShell::insert(file_name, doc);
    ASSUME_ITS_TRUE(err == FOSSIL_NOSHELL_ERROR_SUCCESS);

    std::string result;
    err = NoShell::find(file_name, "user=alice", result);
    ASSUME_ITS_TRUE(err == FOSSIL_NOSHELL_ERROR_SUCCESS);
    ASSUME_ITS_EQUAL_CSTR(result.c_str(), doc.c_str());

    err = NoShell::remove(file_name, "user=alice");
    ASSUME_ITS_TRUE(err == FOSSIL_NOSHELL_ERROR_SUCCESS);

    err = NoShell::find(file_name, "user=alice", result);
    ASSUME_ITS_TRUE(err == FOSSIL_NOSHELL_ERROR_NOT_FOUND);

    NoShell::delete_database(file_name);
}

FOSSIL_TEST(cpp_test_noshell_insert_with_id) {
    using fossil::bluecrab::NoShell;
    const std::string file_name = "test_noshell_with_id.crabdb";
    const std::string doc = "{\"item\":\"book\"}";
    std::string docpp_id;

    auto err = NoShell::create_database(file_name);
    ASSUME_ITS_TRUE(err == FOSSIL_NOSHELL_ERROR_SUCCESS);

    err = NoShell::insert_with_id(file_name, doc, "", docpp_id);
    ASSUME_ITS_TRUE(err == FOSSIL_NOSHELL_ERROR_SUCCESS);

    std::string result;
    err = NoShell::find(file_name, docpp_id, result);
    ASSUME_ITS_TRUE(err == FOSSIL_NOSHELL_ERROR_SUCCESS);
    ASSUME_ITS_EQUAL_CSTR(result.c_str(), doc.c_str());

    NoShell::delete_database(file_name);
}

FOSSIL_TEST(cpp_test_noshell_update) {
    using fossil::bluecrab::NoShell;
    const std::string file_name = "test_noshell_update.crabdb";
    const std::string doc = "{\"name\":\"bob\"}";
    const std::string new_doc = "{\"name\":\"bob\",\"age\":30}";

    auto err = NoShell::create_database(file_name);
    ASSUME_ITS_TRUE(err == FOSSIL_NOSHELL_ERROR_SUCCESS);

    err = NoShell::insert(file_name, doc);
    ASSUME_ITS_TRUE(err == FOSSIL_NOSHELL_ERROR_SUCCESS);

    err = NoShell::update(file_name, "name=bob", new_doc);
    ASSUME_ITS_TRUE(err == FOSSIL_NOSHELL_ERROR_SUCCESS);

    std::string result;
    err = NoShell::find(file_name, "name=bob", result);
    ASSUME_ITS_TRUE(err == FOSSIL_NOSHELL_ERROR_SUCCESS);
    ASSUME_ITS_EQUAL_CSTR(result.c_str(), new_doc.c_str());

    NoShell::delete_database(file_name);
}

FOSSIL_TEST(cpp_test_noshell_backup_restore) {
    using fossil::bluecrab::NoShell;
    const std::string file_name = "test_noshell_backup.crabdb";
    const std::string backup_file = "test_noshell_backup.bak";
    const std::string restore_file = "test_noshell_restored.crabdb";
    const std::string doc = "{\"x\":1}";

    auto err = NoShell::create_database(file_name);
    ASSUME_ITS_TRUE(err == FOSSIL_NOSHELL_ERROR_SUCCESS);

    err = NoShell::insert(file_name, doc);
    ASSUME_ITS_TRUE(err == FOSSIL_NOSHELL_ERROR_SUCCESS);

    err = NoShell::backup_database(file_name, backup_file);
    ASSUME_ITS_TRUE(err == FOSSIL_NOSHELL_ERROR_SUCCESS);

    err = NoShell::restore_database(backup_file, restore_file);
    ASSUME_ITS_TRUE(err == FOSSIL_NOSHELL_ERROR_SUCCESS);

    std::string result;
    err = NoShell::find(restore_file, "x=1", result);
    ASSUME_ITS_TRUE(err == FOSSIL_NOSHELL_ERROR_SUCCESS);
    ASSUME_ITS_EQUAL_CSTR(result.c_str(), doc.c_str());

    NoShell::delete_database(file_name);
    NoShell::delete_database(restore_file);
    remove(backup_file.c_str());
}

FOSSIL_TEST(cpp_test_noshell_count_and_size) {
    using fossil::bluecrab::NoShell;
    const std::string file_name = "test_noshell_count.crabdb";
    auto err = NoShell::create_database(file_name);
    ASSUME_ITS_TRUE(err == FOSSIL_NOSHELL_ERROR_SUCCESS);

    err = NoShell::insert(file_name, "{\"a\":1}");
    ASSUME_ITS_TRUE(err == FOSSIL_NOSHELL_ERROR_SUCCESS);
    err = NoShell::insert(file_name, "{\"b\":2}");
    ASSUME_ITS_TRUE(err == FOSSIL_NOSHELL_ERROR_SUCCESS);

    size_t count = 0;
    err = NoShell::count_documents(file_name, count);
    ASSUME_ITS_TRUE(err == FOSSIL_NOSHELL_ERROR_SUCCESS);
    ASSUME_ITS_TRUE(count == 2);

    size_t size_bytes = 0;
    err = NoShell::get_file_size(file_name, size_bytes);
    ASSUME_ITS_TRUE(err == FOSSIL_NOSHELL_ERROR_SUCCESS);
    ASSUME_ITS_TRUE(size_bytes > 0);

    NoShell::delete_database(file_name);
}

FOSSIL_TEST(cpp_test_noshell_first_next_document) {
    using fossil::bluecrab::NoShell;
    const std::string file_name = "test_noshell_iter.crabdb";
    auto err = NoShell::create_database(file_name);
    ASSUME_ITS_TRUE(err == FOSSIL_NOSHELL_ERROR_SUCCESS);

    err = NoShell::insert(file_name, "{\"id\":1}");
    ASSUME_ITS_TRUE(err == FOSSIL_NOSHELL_ERROR_SUCCESS);
    err = NoShell::insert(file_name, "{\"id\":2}");
    ASSUME_ITS_TRUE(err == FOSSIL_NOSHELL_ERROR_SUCCESS);

    std::string id1, id2;
    err = NoShell::first_document(file_name, id1);
    ASSUME_ITS_TRUE(err == FOSSIL_NOSHELL_ERROR_SUCCESS);

    err = NoShell::next_document(file_name, id1, id2);
    ASSUME_ITS_TRUE(err == FOSSIL_NOSHELL_ERROR_SUCCESS);

    std::string id3;
    err = NoShell::next_document(file_name, id2, id3);
    ASSUME_ITS_TRUE(err == FOSSIL_NOSHELL_ERROR_NOT_FOUND);

    NoShell::delete_database(file_name);
}

FOSSIL_TEST(cpp_test_noshell_verify_database) {
    using fossil::bluecrab::NoShell;
    const std::string file_name = "test_noshell_verify.crabdb";
    auto err = NoShell::create_database(file_name);
    ASSUME_ITS_TRUE(err == FOSSIL_NOSHELL_ERROR_SUCCESS);

    err = NoShell::insert(file_name, "{\"v\":42}");
    ASSUME_ITS_TRUE(err == FOSSIL_NOSHELL_ERROR_SUCCESS);

    err = NoShell::verify_database(file_name);
    ASSUME_ITS_TRUE(err == FOSSIL_NOSHELL_ERROR_SUCCESS);

    NoShell::delete_database(file_name);
}

FOSSIL_TEST(cpp_test_noshell_validate_helpers) {
    using fossil::bluecrab::NoShell;
    ASSUME_ITS_TRUE(NoShell::validate_extension("foo.crabdb"));
    ASSUME_ITS_TRUE(!NoShell::validate_extension("foo.txt"));
    ASSUME_ITS_TRUE(NoShell::validate_document("{\"ok\":true}"));
    ASSUME_ITS_TRUE(!NoShell::validate_document("not_a_json"));
}

FOSSIL_TEST(cpp_test_noshell_lock_unlock_is_locked) {
    using fossil::bluecrab::NoShell;
    const std::string file_name = "test_noshell_lock.crabdb";
    auto err = NoShell::create_database(file_name);
    ASSUME_ITS_TRUE(err == FOSSIL_NOSHELL_ERROR_SUCCESS);

    err = NoShell::lock_database(file_name);
    ASSUME_ITS_TRUE(err == FOSSIL_NOSHELL_ERROR_SUCCESS);

    ASSUME_ITS_TRUE(NoShell::is_locked(file_name));

    err = NoShell::unlock_database(file_name);
    ASSUME_ITS_TRUE(err == FOSSIL_NOSHELL_ERROR_SUCCESS);

    ASSUME_ITS_TRUE(!NoShell::is_locked(file_name));

    NoShell::delete_database(file_name);
}

// * * * * * * * * * * * * * * * * * * * * * * * *
// * Fossil Logic Test Pool
// * * * * * * * * * * * * * * * * * * * * * * * *
FOSSIL_TEST_GROUP(cpp_noshell_database_tests) {
    FOSSIL_TEST_ADD(cpp_noshell_fixture, cpp_test_noshell_create_open_delete);
    FOSSIL_TEST_ADD(cpp_noshell_fixture, cpp_test_noshell_insert_find_remove);
    FOSSIL_TEST_ADD(cpp_noshell_fixture, cpp_test_noshell_insert_with_id);
    FOSSIL_TEST_ADD(cpp_noshell_fixture, cpp_test_noshell_update);
    FOSSIL_TEST_ADD(cpp_noshell_fixture, cpp_test_noshell_backup_restore);
    FOSSIL_TEST_ADD(cpp_noshell_fixture, cpp_test_noshell_count_and_size);
    FOSSIL_TEST_ADD(cpp_noshell_fixture, cpp_test_noshell_first_next_document);
    FOSSIL_TEST_ADD(cpp_noshell_fixture, cpp_test_noshell_verify_database);
    FOSSIL_TEST_ADD(cpp_noshell_fixture, cpp_test_noshell_validate_helpers);
    FOSSIL_TEST_ADD(cpp_noshell_fixture, cpp_test_noshell_lock_unlock_is_locked);

    FOSSIL_TEST_REGISTER(cpp_noshell_fixture);
} // end of tests

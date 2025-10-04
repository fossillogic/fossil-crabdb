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

// Test case for inserting a new document into the database
FOSSIL_TEST(cpp_test_noshell_insert_document) {
    const std::string file_name = "test.noshell";
    fossil::bluecrab::NoShell::create_database(file_name);

    fossil_bluecrab_noshell_error_t result = fossil::bluecrab::NoShell::insert(file_name, "{\"name\":\"Alice\"}");
    ASSUME_ITS_TRUE(result == FOSSIL_NOSHELL_ERROR_SUCCESS);

    fossil::bluecrab::NoShell::delete_database(file_name);
}

// Test case for inserting a document and retrieving its ID
FOSSIL_TEST(cpp_test_noshell_insert_with_id) {
    const std::string file_name = "test.noshell";
    std::string id;
    fossil::bluecrab::NoShell::create_database(file_name);

    fossil_bluecrab_noshell_error_t result = fossil::bluecrab::NoShell::insert_with_id(file_name, "{\"name\":\"Bob\"}", id);
    ASSUME_ITS_TRUE(result == FOSSIL_NOSHELL_ERROR_SUCCESS);
    ASSUME_ITS_TRUE(!id.empty());

    fossil::bluecrab::NoShell::delete_database(file_name);
}

// Test case for finding a document by query
FOSSIL_TEST(cpp_test_noshell_find_document) {
    const std::string file_name = "test.noshell";
    fossil::bluecrab::NoShell::create_database(file_name);
    fossil::bluecrab::NoShell::insert(file_name, "{\"name\":\"Carol\"}");

    std::string result_doc;
    fossil_bluecrab_noshell_error_t result = fossil::bluecrab::NoShell::find(file_name, "Carol", result_doc);
    ASSUME_ITS_TRUE(result == FOSSIL_NOSHELL_ERROR_SUCCESS);
    ASSUME_ITS_TRUE(result_doc.find("Carol") != std::string::npos);

    fossil::bluecrab::NoShell::delete_database(file_name);
}

// Test case for updating a document by query
FOSSIL_TEST(cpp_test_noshell_update_document) {
    const std::string file_name = "test.noshell";
    fossil::bluecrab::NoShell::create_database(file_name);
    fossil::bluecrab::NoShell::insert(file_name, "{\"name\":\"Dave\"}");

    fossil_bluecrab_noshell_error_t result = fossil::bluecrab::NoShell::update(file_name, "Dave", "{\"name\":\"David\"}");
    ASSUME_ITS_TRUE(result == FOSSIL_NOSHELL_ERROR_SUCCESS);

    std::string result_doc;
    result = fossil::bluecrab::NoShell::find(file_name, "David", result_doc);
    ASSUME_ITS_TRUE(result == FOSSIL_NOSHELL_ERROR_SUCCESS);
    ASSUME_ITS_TRUE(result_doc.find("David") != std::string::npos);

    fossil::bluecrab::NoShell::delete_database(file_name);
}

// Test case for removing a document by query
FOSSIL_TEST(cpp_test_noshell_remove_document) {
    const std::string file_name = "test.noshell";
    fossil::bluecrab::NoShell::create_database(file_name);
    fossil::bluecrab::NoShell::insert(file_name, "{\"name\":\"Eve\"}");

    fossil_bluecrab_noshell_error_t result = fossil::bluecrab::NoShell::remove(file_name, "Eve");
    ASSUME_ITS_TRUE(result == FOSSIL_NOSHELL_ERROR_SUCCESS);

    std::string result_doc;
    result = fossil::bluecrab::NoShell::find(file_name, "Eve", result_doc);
    ASSUME_ITS_TRUE(result != FOSSIL_NOSHELL_ERROR_SUCCESS);

    fossil::bluecrab::NoShell::delete_database(file_name);
}

// Test case for creating and deleting a database file
FOSSIL_TEST(cpp_test_noshell_create_delete_database) {
    const std::string file_name = "test.noshell";
    fossil_bluecrab_noshell_error_t result = fossil::bluecrab::NoShell::create_database(file_name);
    ASSUME_ITS_TRUE(result == FOSSIL_NOSHELL_ERROR_SUCCESS);

    result = fossil::bluecrab::NoShell::delete_database(file_name);
    ASSUME_ITS_TRUE(result == FOSSIL_NOSHELL_ERROR_SUCCESS);
}

// Test case for locking and unlocking a database file
FOSSIL_TEST(cpp_test_noshell_lock_unlock_database) {
    const std::string file_name = "test.noshell";
    fossil::bluecrab::NoShell::create_database(file_name);

    fossil_bluecrab_noshell_error_t result = fossil::bluecrab::NoShell::lock_database(file_name);
    ASSUME_ITS_TRUE(result == FOSSIL_NOSHELL_ERROR_SUCCESS);
    ASSUME_ITS_TRUE(fossil::bluecrab::NoShell::is_locked(file_name));

    result = fossil::bluecrab::NoShell::unlock_database(file_name);
    ASSUME_ITS_TRUE(result == FOSSIL_NOSHELL_ERROR_SUCCESS);
    ASSUME_ITS_FALSE(fossil::bluecrab::NoShell::is_locked(file_name));

    fossil::bluecrab::NoShell::delete_database(file_name);
}

// Test case for backing up and restoring a database file
FOSSIL_TEST(cpp_test_noshell_backup_restore_database) {
    const std::string file_name = "test.noshell";
    const std::string backup_file = "backup.noshell";
    fossil::bluecrab::NoShell::create_database(file_name);
    fossil::bluecrab::NoShell::insert(file_name, "{\"name\":\"Frank\"}");

    fossil_bluecrab_noshell_error_t result = fossil::bluecrab::NoShell::backup_database(file_name, backup_file);
    ASSUME_ITS_TRUE(result == FOSSIL_NOSHELL_ERROR_SUCCESS);

    fossil::bluecrab::NoShell::delete_database(file_name);
    result = fossil::bluecrab::NoShell::restore_database(backup_file, file_name);
    ASSUME_ITS_TRUE(result == FOSSIL_NOSHELL_ERROR_SUCCESS);

    std::string result_doc;
    result = fossil::bluecrab::NoShell::find(file_name, "Frank", result_doc);
    ASSUME_ITS_TRUE(result == FOSSIL_NOSHELL_ERROR_SUCCESS);

    fossil::bluecrab::NoShell::delete_database(file_name);
    fossil::bluecrab::NoShell::delete_database(backup_file);
}

// Test case for verifying the integrity of the database
FOSSIL_TEST(cpp_test_noshell_verify_database) {
    const std::string file_name = "test.noshell";
    fossil::bluecrab::NoShell::create_database(file_name);
    fossil::bluecrab::NoShell::insert(file_name, "{\"name\":\"Grace\"}");

    fossil_bluecrab_noshell_error_t result = fossil::bluecrab::NoShell::verify_database(file_name);
    ASSUME_ITS_TRUE(result == FOSSIL_NOSHELL_ERROR_SUCCESS);

    fossil::bluecrab::NoShell::delete_database(file_name);
}

// Test case for iterating over document IDs
FOSSIL_TEST(cpp_test_noshell_iterate_documents) {
    const std::string file_name = "test.noshell";
    std::string id, next_id;
    fossil::bluecrab::NoShell::create_database(file_name);
    fossil::bluecrab::NoShell::insert(file_name, "{\"name\":\"Heidi\"}");
    fossil::bluecrab::NoShell::insert(file_name, "{\"name\":\"Ivan\"}");

    fossil_bluecrab_noshell_error_t result = fossil::bluecrab::NoShell::first_document(file_name, id);
    ASSUME_ITS_TRUE(result == FOSSIL_NOSHELL_ERROR_SUCCESS);
    ASSUME_ITS_TRUE(!id.empty());

    result = fossil::bluecrab::NoShell::next_document(file_name, id, next_id);
    ASSUME_ITS_TRUE(result == FOSSIL_NOSHELL_ERROR_SUCCESS || result == FOSSIL_NOSHELL_ERROR_NOT_FOUND);

    fossil::bluecrab::NoShell::delete_database(file_name);
}

// Test case for counting documents in the database
FOSSIL_TEST(cpp_test_noshell_count_documents) {
    const std::string file_name = "test.noshell";
    size_t count = 0;
    fossil::bluecrab::NoShell::create_database(file_name);
    fossil::bluecrab::NoShell::insert(file_name, "{\"name\":\"Judy\"}");
    fossil::bluecrab::NoShell::insert(file_name, "{\"name\":\"Karl\"}");

    fossil_bluecrab_noshell_error_t result = fossil::bluecrab::NoShell::count_documents(file_name, count);
    ASSUME_ITS_TRUE(result == FOSSIL_NOSHELL_ERROR_SUCCESS);
    ASSUME_ITS_TRUE(count == 2);

    fossil::bluecrab::NoShell::delete_database(file_name);
}

// Test case for getting the file size of the database
FOSSIL_TEST(cpp_test_noshell_get_file_size) {
    const std::string file_name = "test.noshell";
    size_t size_bytes = 0;
    fossil::bluecrab::NoShell::create_database(file_name);
    fossil::bluecrab::NoShell::insert(file_name, "{\"name\":\"Leo\"}");

    fossil_bluecrab_noshell_error_t result = fossil::bluecrab::NoShell::get_file_size(file_name, size_bytes);
    ASSUME_ITS_TRUE(result == FOSSIL_NOSHELL_ERROR_SUCCESS);
    ASSUME_ITS_TRUE(size_bytes > 0);

    fossil::bluecrab::NoShell::delete_database(file_name);
}

// Test case for validating the file extension of a database file
FOSSIL_TEST(cpp_test_noshell_validate_extension) {
    ASSUME_ITS_TRUE(fossil::bluecrab::NoShell::validate_extension("test.noshell"));
    ASSUME_ITS_FALSE(fossil::bluecrab::NoShell::validate_extension("test.txt"));
}

// Test case for validating a document string
FOSSIL_TEST(cpp_test_noshell_validate_document) {
    ASSUME_ITS_TRUE(fossil::bluecrab::NoShell::validate_document("{\"name\":\"Mona\"}"));
    ASSUME_ITS_FALSE(fossil::bluecrab::NoShell::validate_document(""));
}

// * * * * * * * * * * * * * * * * * * * * * * * *
// * Fossil Logic Test Pool
// * * * * * * * * * * * * * * * * * * * * * * * *
FOSSIL_TEST_GROUP(cpp_noshell_database_tests) {
    FOSSIL_TEST_ADD(cpp_noshell_fixture, cpp_test_noshell_insert_document);
    FOSSIL_TEST_ADD(cpp_noshell_fixture, cpp_test_noshell_insert_with_id);
    FOSSIL_TEST_ADD(cpp_noshell_fixture, cpp_test_noshell_find_document);
    FOSSIL_TEST_ADD(cpp_noshell_fixture, cpp_test_noshell_update_document);
    FOSSIL_TEST_ADD(cpp_noshell_fixture, cpp_test_noshell_remove_document);
    FOSSIL_TEST_ADD(cpp_noshell_fixture, cpp_test_noshell_create_delete_database);
    FOSSIL_TEST_ADD(cpp_noshell_fixture, cpp_test_noshell_lock_unlock_database);
    FOSSIL_TEST_ADD(cpp_noshell_fixture, cpp_test_noshell_backup_restore_database);
    FOSSIL_TEST_ADD(cpp_noshell_fixture, cpp_test_noshell_verify_database);
    FOSSIL_TEST_ADD(cpp_noshell_fixture, cpp_test_noshell_iterate_documents);
    FOSSIL_TEST_ADD(cpp_noshell_fixture, cpp_test_noshell_count_documents);
    FOSSIL_TEST_ADD(cpp_noshell_fixture, cpp_test_noshell_get_file_size);
    FOSSIL_TEST_ADD(cpp_noshell_fixture, cpp_test_noshell_validate_extension);
    FOSSIL_TEST_ADD(cpp_noshell_fixture, cpp_test_noshell_validate_document);

    FOSSIL_TEST_REGISTER(cpp_noshell_fixture);
} // end of tests

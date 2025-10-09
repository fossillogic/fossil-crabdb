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

FOSSIL_SUITE(c_noshell_fixture);

FOSSIL_SETUP(c_noshell_fixture) {
    // Setup the test fixture
}

FOSSIL_TEARDOWN(c_noshell_fixture) {
    // Teardown the test fixture
}

// * * * * * * * * * * * * * * * * * * * * * * * *
// * Fossil Logic Test Blue CrabDB Database
// * * * * * * * * * * * * * * * * * * * * * * * *

// * * * * * * * * * * * * * * * * * * * * * * * *
// * Fossil Logic Test Blue CrabDB NoShell Database
// * * * * * * * * * * * * * * * * * * * * * * * *

FOSSIL_TEST(c_test_noshell_create_open_delete) {
    fossil_bluecrab_noshell_error_t err;
    const char *file_name = "test_noshell.noshell";

    err = fossil_bluecrab_noshell_create_database(file_name);
    ASSUME_ITS_TRUE(err == FOSSIL_NOSHELL_ERROR_SUCCESS);

    err = fossil_bluecrab_noshell_open_database(file_name);
    ASSUME_ITS_TRUE(err == FOSSIL_NOSHELL_ERROR_SUCCESS);

    err = fossil_bluecrab_noshell_delete_database(file_name);
    ASSUME_ITS_TRUE(err == FOSSIL_NOSHELL_ERROR_SUCCESS);
}

FOSSIL_TEST(c_test_noshell_insert_find_remove) {
    fossil_bluecrab_noshell_error_t err;
    const char *file_name = "test_noshell_insert.noshell";
    const char *doc = "{ username: cstr: \"alice\", password: cstr: \"secret\" }";
    const char *type = "object";

    err = fossil_bluecrab_noshell_create_database(file_name);
    ASSUME_ITS_TRUE(err == FOSSIL_NOSHELL_ERROR_SUCCESS);

    err = fossil_bluecrab_noshell_insert(file_name, doc, NULL, type);
    ASSUME_ITS_TRUE(err == FOSSIL_NOSHELL_ERROR_SUCCESS);

    char result[128];
    err = fossil_bluecrab_noshell_find(file_name, "username", result, sizeof(result), type);
    ASSUME_ITS_TRUE(err == FOSSIL_NOSHELL_ERROR_SUCCESS);
    ASSUME_ITS_TRUE(strstr(result, "alice") != NULL);

    err = fossil_bluecrab_noshell_remove(file_name, "username");
    ASSUME_ITS_TRUE(err == FOSSIL_NOSHELL_ERROR_SUCCESS);

    err = fossil_bluecrab_noshell_find(file_name, "username", result, sizeof(result), type);
    ASSUME_ITS_TRUE(err == FOSSIL_NOSHELL_ERROR_NOT_FOUND);

    fossil_bluecrab_noshell_delete_database(file_name);
}

FOSSIL_TEST(c_test_noshell_insert_with_id) {
    fossil_bluecrab_noshell_error_t err;
    const char *file_name = "test_noshell_with_id.noshell";
    const char *doc = "{ item: cstr: \"book\" }";
    const char *type = "object";
    char doc_id[64];

    err = fossil_bluecrab_noshell_create_database(file_name);
    ASSUME_ITS_TRUE(err == FOSSIL_NOSHELL_ERROR_SUCCESS);

    err = fossil_bluecrab_noshell_insert_with_id(file_name, doc, NULL, type, doc_id, sizeof(doc_id));
    ASSUME_ITS_TRUE(err == FOSSIL_NOSHELL_ERROR_SUCCESS);

    char result[128];
    err = fossil_bluecrab_noshell_find(file_name, doc_id, result, sizeof(result), type);
    ASSUME_ITS_TRUE(err == FOSSIL_NOSHELL_ERROR_SUCCESS);
    ASSUME_ITS_TRUE(strstr(result, "book") != NULL);

    fossil_bluecrab_noshell_delete_database(file_name);
}

FOSSIL_TEST(c_test_noshell_update) {
    fossil_bluecrab_noshell_error_t err;
    const char *file_name = "test_noshell_update.noshell";
    const char *doc = "{ name: cstr: \"bob\" }";
    const char *new_doc = "{ name: cstr: \"bob\", age: i32: 30 }";
    const char *type = "object";

    err = fossil_bluecrab_noshell_create_database(file_name);
    ASSUME_ITS_TRUE(err == FOSSIL_NOSHELL_ERROR_SUCCESS);

    err = fossil_bluecrab_noshell_insert(file_name, doc, NULL, type);
    ASSUME_ITS_TRUE(err == FOSSIL_NOSHELL_ERROR_SUCCESS);

    err = fossil_bluecrab_noshell_update(file_name, "name", new_doc, NULL, type);
    ASSUME_ITS_TRUE(err == FOSSIL_NOSHELL_ERROR_SUCCESS);

    char result[128];
    err = fossil_bluecrab_noshell_find(file_name, "name", result, sizeof(result), type);
    ASSUME_ITS_TRUE(err == FOSSIL_NOSHELL_ERROR_SUCCESS);
    ASSUME_ITS_TRUE(strstr(result, "age: i32: 30") != NULL);

    fossil_bluecrab_noshell_delete_database(file_name);
}

FOSSIL_TEST(c_test_noshell_backup_restore) {
    fossil_bluecrab_noshell_error_t err;
    const char *file_name = "test_noshell_backup.noshell";
    const char *backup_file = "test_noshell_backup_file.noshell";
    const char *restore_file = "test_noshell_restored.noshell";
    const char *doc = "{ x: i32: 1 }";
    const char *type = "object";

    err = fossil_bluecrab_noshell_create_database(file_name);
    ASSUME_ITS_TRUE(err == FOSSIL_NOSHELL_ERROR_SUCCESS);

    err = fossil_bluecrab_noshell_insert(file_name, doc, NULL, type);
    ASSUME_ITS_TRUE(err == FOSSIL_NOSHELL_ERROR_SUCCESS);

    err = fossil_bluecrab_noshell_backup_database(file_name, backup_file);
    ASSUME_ITS_TRUE(err == FOSSIL_NOSHELL_ERROR_SUCCESS);

    err = fossil_bluecrab_noshell_restore_database(backup_file, restore_file);
    ASSUME_ITS_TRUE(err == FOSSIL_NOSHELL_ERROR_SUCCESS);

    // Ensure the restored database is opened before querying
    err = fossil_bluecrab_noshell_open_database(restore_file);
    ASSUME_ITS_TRUE(err == FOSSIL_NOSHELL_ERROR_SUCCESS);

    char result[128];
    err = fossil_bluecrab_noshell_find(restore_file, "x", result, sizeof(result), type);
    ASSUME_ITS_TRUE(err == FOSSIL_NOSHELL_ERROR_SUCCESS);
    ASSUME_ITS_TRUE(strstr(result, "x: i32: 1") != NULL);

    fossil_bluecrab_noshell_delete_database(file_name);
    fossil_bluecrab_noshell_delete_database(restore_file);
    remove(backup_file);
}

FOSSIL_TEST(c_test_noshell_count_and_size) {
    fossil_bluecrab_noshell_error_t err;
    const char *file_name = "test_noshell_count.noshell";
    const char *type = "object";
    err = fossil_bluecrab_noshell_create_database(file_name);
    ASSUME_ITS_TRUE(err == FOSSIL_NOSHELL_ERROR_SUCCESS);

    err = fossil_bluecrab_noshell_insert(file_name, "{ a: i32: 1 }", NULL, type);
    ASSUME_ITS_TRUE(err == FOSSIL_NOSHELL_ERROR_SUCCESS);
    err = fossil_bluecrab_noshell_insert(file_name, "{ b: i32: 2 }", NULL, type);
    ASSUME_ITS_TRUE(err == FOSSIL_NOSHELL_ERROR_SUCCESS);

    size_t count = 0;
    err = fossil_bluecrab_noshell_count_documents(file_name, &count);
    ASSUME_ITS_TRUE(err == FOSSIL_NOSHELL_ERROR_SUCCESS);
    ASSUME_ITS_TRUE(count == 2);

    size_t size_bytes = 0;
    err = fossil_bluecrab_noshell_get_file_size(file_name, &size_bytes);
    ASSUME_ITS_TRUE(err == FOSSIL_NOSHELL_ERROR_SUCCESS);
    ASSUME_ITS_TRUE(size_bytes > 0);

    fossil_bluecrab_noshell_delete_database(file_name);
}

FOSSIL_TEST(c_test_noshell_first_next_document) {
    fossil_bluecrab_noshell_error_t err;
    const char *file_name = "test_noshell_iter.noshell";
    const char *type = "object";
    err = fossil_bluecrab_noshell_create_database(file_name);
    ASSUME_ITS_TRUE(err == FOSSIL_NOSHELL_ERROR_SUCCESS);

    err = fossil_bluecrab_noshell_insert(file_name, "{ id: i32: 1 }", NULL, type);
    ASSUME_ITS_TRUE(err == FOSSIL_NOSHELL_ERROR_SUCCESS);
    err = fossil_bluecrab_noshell_insert(file_name, "{ id: i32: 2 }", NULL, type);
    ASSUME_ITS_TRUE(err == FOSSIL_NOSHELL_ERROR_SUCCESS);

    char id1[64], id2[64];
    err = fossil_bluecrab_noshell_first_document(file_name, id1, sizeof(id1));
    ASSUME_ITS_TRUE(err == FOSSIL_NOSHELL_ERROR_SUCCESS);

    err = fossil_bluecrab_noshell_next_document(file_name, id1, id2, sizeof(id2));
    ASSUME_ITS_TRUE(err == FOSSIL_NOSHELL_ERROR_SUCCESS);

    char id3[64];
    err = fossil_bluecrab_noshell_next_document(file_name, id2, id3, sizeof(id3));
    ASSUME_ITS_TRUE(err == FOSSIL_NOSHELL_ERROR_NOT_FOUND);

    fossil_bluecrab_noshell_delete_database(file_name);
}

FOSSIL_TEST(c_test_noshell_verify_database) {
    fossil_bluecrab_noshell_error_t err;
    const char *file_name = "test_noshell_verify.noshell";
    const char *type = "object";
    err = fossil_bluecrab_noshell_create_database(file_name);
    ASSUME_ITS_TRUE(err == FOSSIL_NOSHELL_ERROR_SUCCESS);

    err = fossil_bluecrab_noshell_insert(file_name, "{ v: i32: 42 }", NULL, type);
    ASSUME_ITS_TRUE(err == FOSSIL_NOSHELL_ERROR_SUCCESS);

    err = fossil_bluecrab_noshell_verify_database(file_name);
    ASSUME_ITS_TRUE(err == FOSSIL_NOSHELL_ERROR_SUCCESS);

    fossil_bluecrab_noshell_delete_database(file_name);
}

FOSSIL_TEST(c_test_noshell_validate_helpers) {
    ASSUME_ITS_TRUE(fossil_bluecrab_noshell_validate_extension("foo.noshell"));
    ASSUME_ITS_TRUE(!fossil_bluecrab_noshell_validate_extension("foo.txt"));
    ASSUME_ITS_TRUE(fossil_bluecrab_noshell_validate_document("{ ok: bool: true }"));
    ASSUME_ITS_TRUE(!fossil_bluecrab_noshell_validate_document("not_a_json"));
}

FOSSIL_TEST(c_test_noshell_lock_unlock_is_locked) {
    fossil_bluecrab_noshell_error_t err;
    const char *file_name = "test_noshell_lock.noshell";
    err = fossil_bluecrab_noshell_create_database(file_name);
    ASSUME_ITS_TRUE(err == FOSSIL_NOSHELL_ERROR_SUCCESS);

    err = fossil_bluecrab_noshell_lock_database(file_name);
    ASSUME_ITS_TRUE(err == FOSSIL_NOSHELL_ERROR_SUCCESS);

    ASSUME_ITS_TRUE(fossil_bluecrab_noshell_is_locked(file_name));

    err = fossil_bluecrab_noshell_unlock_database(file_name);
    ASSUME_ITS_TRUE(err == FOSSIL_NOSHELL_ERROR_SUCCESS);

    ASSUME_ITS_TRUE(!fossil_bluecrab_noshell_is_locked(file_name));

    fossil_bluecrab_noshell_delete_database(file_name);
}

// * * * * * * * * * * * * * * * * * * * * * * * *
// * Fossil Logic Test Pool
// * * * * * * * * * * * * * * * * * * * * * * * *
FOSSIL_TEST_GROUP(c_noshell_database_tests) {
    FOSSIL_TEST_ADD(c_noshell_fixture, c_test_noshell_create_open_delete);
    FOSSIL_TEST_ADD(c_noshell_fixture, c_test_noshell_insert_find_remove);
    FOSSIL_TEST_ADD(c_noshell_fixture, c_test_noshell_insert_with_id);
    FOSSIL_TEST_ADD(c_noshell_fixture, c_test_noshell_update);
    FOSSIL_TEST_ADD(c_noshell_fixture, c_test_noshell_backup_restore);
    FOSSIL_TEST_ADD(c_noshell_fixture, c_test_noshell_count_and_size);
    FOSSIL_TEST_ADD(c_noshell_fixture, c_test_noshell_first_next_document);
    FOSSIL_TEST_ADD(c_noshell_fixture, c_test_noshell_verify_database);
    FOSSIL_TEST_ADD(c_noshell_fixture, c_test_noshell_validate_helpers);
    FOSSIL_TEST_ADD(c_noshell_fixture, c_test_noshell_lock_unlock_is_locked);

    FOSSIL_TEST_REGISTER(c_noshell_fixture);
} // end of tests

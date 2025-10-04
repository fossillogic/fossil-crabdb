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

FOSSIL_SUITE(c_fileshell_fixture);

FOSSIL_SETUP(c_fileshell_fixture) {
    // Setup the test fixture
}

FOSSIL_TEARDOWN(c_fileshell_fixture) {
    // Teardown the test fixture
}

// * * * * * * * * * * * * * * * * * * * * * * * *
// * Fossil Logic Test Blue CrabDB Database
// * * * * * * * * * * * * * * * * * * * * * * * *

// Tests for fossil_bluecrab_fileshell_write, append, read, delete, exists, size, list
FOSSIL_TEST(c_test_fileshell_write_and_read) {
    const char *file_name = "test_file.txt";
    const char *data = "Hello, Fossil!";
    char buffer[64];

    // Write data
    ASSUME_ITS_TRUE(fossil_bluecrab_fileshell_write(file_name, data));

    // Read data
    ASSUME_ITS_TRUE(fossil_bluecrab_fileshell_read(file_name, buffer, sizeof(buffer)));
    ASSUME_ITS_TRUE(strcmp(buffer, data) == 0);

    // Cleanup
    fossil_bluecrab_fileshell_delete(file_name);
}

FOSSIL_TEST(c_test_fileshell_append) {
    const char *file_name = "test_append.txt";
    const char *data1 = "First line.\n";
    const char *data2 = "Second line.";
    char buffer[128];

    // Write initial data
    ASSUME_ITS_TRUE(fossil_bluecrab_fileshell_write(file_name, data1));
    // Append data
    ASSUME_ITS_TRUE(fossil_bluecrab_fileshell_append(file_name, data2));

    // Read and check
    ASSUME_ITS_TRUE(fossil_bluecrab_fileshell_read(file_name, buffer, sizeof(buffer)));
    ASSUME_ITS_TRUE(strstr(buffer, data1) != NULL);
    ASSUME_ITS_TRUE(strstr(buffer, data2) != NULL);

    fossil_bluecrab_fileshell_delete(file_name);
}

FOSSIL_TEST(c_test_fileshell_size) {
    const char *file_name = "test_size.txt";
    const char *data = "1234567890";
    fossil_bluecrab_fileshell_write(file_name, data);

    long size = fossil_bluecrab_fileshell_size(file_name);
    ASSUME_ITS_TRUE(size == (long)strlen(data));

    fossil_bluecrab_fileshell_delete(file_name);
}

FOSSIL_TEST(c_test_fileshell_list) {
    const char *dir_name = ".";
    char *files[16];
    int count = fossil_bluecrab_fileshell_list(dir_name, files, 16);

    ASSUME_ITS_TRUE(count > 0);
    for (int i = 0; i < count; ++i) {
        free(files[i]);
    }
    ASSUME_ITS_TRUE(count > 0);
}

FOSSIL_TEST(c_test_fileshell_read_nonexistent_file) {
    char buffer[32];
    ASSUME_ITS_FALSE(fossil_bluecrab_fileshell_read("no_such_file.txt", buffer, sizeof(buffer)));
}

FOSSIL_TEST(c_test_fileshell_write_null_args) {
    ASSUME_ITS_FALSE(fossil_bluecrab_fileshell_write(NULL, "data"));
    ASSUME_ITS_FALSE(fossil_bluecrab_fileshell_write("file.txt", NULL));
}

FOSSIL_TEST(c_test_fileshell_append_null_args) {
    ASSUME_ITS_FALSE(fossil_bluecrab_fileshell_append(NULL, "data"));
    ASSUME_ITS_FALSE(fossil_bluecrab_fileshell_append("file.txt", NULL));
}

FOSSIL_TEST(c_test_fileshell_list_invalid_dir) {
    char *files[4];
    int count = fossil_bluecrab_fileshell_list("no_such_dir", files, 4);
    ASSUME_ITS_TRUE(count == -1);
}

// * * * * * * * * * * * * * * * * * * * * * * * *
// * Fossil Logic Test Pool
// * * * * * * * * * * * * * * * * * * * * * * * *
FOSSIL_TEST_GROUP(c_fileshell_database_tests) {
    FOSSIL_TEST_ADD(c_fileshell_fixture, c_test_fileshell_write_and_read);
    FOSSIL_TEST_ADD(c_fileshell_fixture, c_test_fileshell_append);
    FOSSIL_TEST_ADD(c_fileshell_fixture, c_test_fileshell_size);
    FOSSIL_TEST_ADD(c_fileshell_fixture, c_test_fileshell_list);
    FOSSIL_TEST_ADD(c_fileshell_fixture, c_test_fileshell_read_nonexistent_file);
    FOSSIL_TEST_ADD(c_fileshell_fixture, c_test_fileshell_write_null_args);
    FOSSIL_TEST_ADD(c_fileshell_fixture, c_test_fileshell_append_null_args);
    FOSSIL_TEST_ADD(c_fileshell_fixture, c_test_fileshell_list_invalid_dir);

    FOSSIL_TEST_REGISTER(c_fileshell_fixture);
} // end of tests

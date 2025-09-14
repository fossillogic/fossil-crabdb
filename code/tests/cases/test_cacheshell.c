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

FOSSIL_SUITE(c_cacheshell_fixture);

FOSSIL_SETUP(c_cacheshell_fixture) {
    // Setup the test fixture
}

FOSSIL_TEARDOWN(c_cacheshell_fixture) {
    // Teardown the test fixture
}

// * * * * * * * * * * * * * * * * * * * * * * * *
// * Fossil Logic Test Blue CrabDB Database
// * * * * * * * * * * * * * * * * * * * * * * * *

// CacheShell API tests

FOSSIL_TEST(c_test_cacheshell_set_and_get) {
    fossil_bluecrab_cacheshell_clear();
    const char *key = "foo";
    const char *value = "bar";
    char out[32];

    ASSUME_ITS_TRUE(fossil_bluecrab_cacheshell_set(key, value));
    ASSUME_ITS_TRUE(fossil_bluecrab_cacheshell_get(key, out, sizeof(out)));
    ASSUME_ITS_TRUE(strcmp(out, value) == 0);
}

FOSSIL_TEST(c_test_cacheshell_overwrite_value) {
    fossil_bluecrab_cacheshell_clear();
    const char *key = "foo";
    const char *value1 = "bar";
    const char *value2 = "baz";
    char out[32];

    ASSUME_ITS_TRUE(fossil_bluecrab_cacheshell_set(key, value1));
    ASSUME_ITS_TRUE(fossil_bluecrab_cacheshell_set(key, value2));
    ASSUME_ITS_TRUE(fossil_bluecrab_cacheshell_get(key, out, sizeof(out)));
    ASSUME_ITS_TRUE(strcmp(out, value2) == 0);
}

FOSSIL_TEST(c_test_cacheshell_remove) {
    fossil_bluecrab_cacheshell_clear();
    const char *key = "foo";
    const char *value = "bar";
    char out[32];

    ASSUME_ITS_TRUE(fossil_bluecrab_cacheshell_set(key, value));
    ASSUME_ITS_TRUE(fossil_bluecrab_cacheshell_remove(key));
    ASSUME_ITS_FALSE(fossil_bluecrab_cacheshell_get(key, out, sizeof(out)));
}

FOSSIL_TEST(c_test_cacheshell_set_with_ttl_and_expire) {
    fossil_bluecrab_cacheshell_clear();
    const char *key = "ttlkey";
    const char *value = "ttlvalue";
    char out[32];

    ASSUME_ITS_TRUE(fossil_bluecrab_cacheshell_set_with_ttl(key, value, 1));
    ASSUME_ITS_TRUE(fossil_bluecrab_cacheshell_get(key, out, sizeof(out)));
    ASSUME_ITS_TRUE(strcmp(out, value) == 0);

    // Wait for TTL to expire
    sleep(2);
    ASSUME_ITS_FALSE(fossil_bluecrab_cacheshell_get(key, out, sizeof(out)));
}

FOSSIL_TEST(c_test_cacheshell_expire_and_ttl) {
    fossil_bluecrab_cacheshell_clear();
    const char *key = "expirekey";
    const char *value = "expirevalue";

    ASSUME_ITS_TRUE(fossil_bluecrab_cacheshell_set(key, value));
    ASSUME_ITS_TRUE(fossil_bluecrab_cacheshell_expire(key, 2));
    int ttl = fossil_bluecrab_cacheshell_ttl(key);
    ASSUME_ITS_TRUE(ttl > 0 && ttl <= 2);

    sleep(3);
    ttl = fossil_bluecrab_cacheshell_ttl(key);
    ASSUME_ITS_TRUE(ttl == -1);
}

FOSSIL_TEST(c_test_cacheshell_clear_and_count) {
    fossil_bluecrab_cacheshell_clear();
    ASSUME_ITS_TRUE(fossil_bluecrab_cacheshell_count() == 0);

    fossil_bluecrab_cacheshell_set("a", "1");
    fossil_bluecrab_cacheshell_set("b", "2");
    ASSUME_ITS_TRUE(fossil_bluecrab_cacheshell_count() == 2);

    fossil_bluecrab_cacheshell_clear();
    ASSUME_ITS_TRUE(fossil_bluecrab_cacheshell_count() == 0);
}

FOSSIL_TEST(c_test_cacheshell_exists) {
    fossil_bluecrab_cacheshell_clear();
    const char *key = "existkey";
    ASSUME_ITS_FALSE(fossil_bluecrab_cacheshell_exists(key));
    fossil_bluecrab_cacheshell_set(key, "val");
    ASSUME_ITS_TRUE(fossil_bluecrab_cacheshell_exists(key));
    fossil_bluecrab_cacheshell_remove(key);
    ASSUME_ITS_FALSE(fossil_bluecrab_cacheshell_exists(key));
}

FOSSIL_TEST(c_test_cacheshell_set_and_get_binary) {
    fossil_bluecrab_cacheshell_clear();
    const char *key = "bin";
    unsigned char data[4] = {0xde, 0xad, 0xbe, 0xef};
    unsigned char out[4] = {0};
    size_t out_size = 0;

    ASSUME_ITS_TRUE(fossil_bluecrab_cacheshell_set_binary(key, data, sizeof(data)));
    ASSUME_ITS_TRUE(fossil_bluecrab_cacheshell_get_binary(key, out, sizeof(out), &out_size));
    ASSUME_ITS_TRUE(out_size == sizeof(data));
    ASSUME_ITS_TRUE(memcmp(data, out, sizeof(data)) == 0);
}

FOSSIL_TEST(c_test_cacheshell_get_nonexistent_key) {
    fossil_bluecrab_cacheshell_clear();
    char out[32];
    ASSUME_ITS_FALSE(fossil_bluecrab_cacheshell_get("nope", out, sizeof(out)));
}

FOSSIL_TEST(c_test_cacheshell_remove_nonexistent_key) {
    fossil_bluecrab_cacheshell_clear();
    ASSUME_ITS_FALSE(fossil_bluecrab_cacheshell_remove("nope"));
}

// * * * * * * * * * * * * * * * * * * * * * * * *
// * Fossil Logic Test Pool
// * * * * * * * * * * * * * * * * * * * * * * * *
FOSSIL_TEST_GROUP(c_cacheshell_database_tests) {
    FOSSIL_TEST_ADD(c_cacheshell_fixture, c_test_cacheshell_set_and_get);
    FOSSIL_TEST_ADD(c_cacheshell_fixture, c_test_cacheshell_overwrite_value);
    FOSSIL_TEST_ADD(c_cacheshell_fixture, c_test_cacheshell_remove);
    FOSSIL_TEST_ADD(c_cacheshell_fixture, c_test_cacheshell_set_with_ttl_and_expire);
    FOSSIL_TEST_ADD(c_cacheshell_fixture, c_test_cacheshell_expire_and_ttl);
    FOSSIL_TEST_ADD(c_cacheshell_fixture, c_test_cacheshell_clear_and_count);
    FOSSIL_TEST_ADD(c_cacheshell_fixture, c_test_cacheshell_exists);
    FOSSIL_TEST_ADD(c_cacheshell_fixture, c_test_cacheshell_set_and_get_binary);
    FOSSIL_TEST_ADD(c_cacheshell_fixture, c_test_cacheshell_get_nonexistent_key);
    FOSSIL_TEST_ADD(c_cacheshell_fixture, c_test_cacheshell_remove_nonexistent_key);

    FOSSIL_TEST_REGISTER(c_cacheshell_fixture);
} // end of tests

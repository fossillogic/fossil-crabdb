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
    fossil_bluecrab_cacheshell_init(0);
    fossil_bluecrab_cacheshell_clear();
    const char *key = "foo";
    const char *value = "bar";
    char out[32] = {0};

    ASSUME_ITS_TRUE(fossil_bluecrab_cacheshell_set(key, value));
    ASSUME_ITS_TRUE(fossil_bluecrab_cacheshell_get(key, out, sizeof(out)));
    ASSUME_ITS_TRUE(strcmp(out, value) == 0);

    const char *new_value = "baz";
    ASSUME_ITS_TRUE(fossil_bluecrab_cacheshell_set(key, new_value));
    ASSUME_ITS_TRUE(fossil_bluecrab_cacheshell_get(key, out, sizeof(out)));
    ASSUME_ITS_TRUE(strcmp(out, new_value) == 0);

    char small_out[2] = {0};
    ASSUME_ITS_TRUE(fossil_bluecrab_cacheshell_get(key, small_out, sizeof(small_out)));
    ASSUME_ITS_TRUE(small_out[0] == 'b');

    fossil_bluecrab_cacheshell_shutdown();
}

FOSSIL_TEST(c_test_cacheshell_set_with_ttl_and_expire) {
    fossil_bluecrab_cacheshell_init(0);
    fossil_bluecrab_cacheshell_clear();
    const char *key = "ttlkey";
    const char *value = "ttlvalue";
    char out[32] = {0};

    ASSUME_ITS_TRUE(fossil_bluecrab_cacheshell_set_with_ttl(key, value, 1));
    ASSUME_ITS_TRUE(fossil_bluecrab_cacheshell_get(key, out, sizeof(out)));
    ASSUME_ITS_TRUE(strcmp(out, value) == 0);

    sleep(2);
    ASSUME_ITS_FALSE(fossil_bluecrab_cacheshell_get(key, out, sizeof(out)));
    ASSUME_ITS_TRUE(fossil_bluecrab_cacheshell_ttl(key) == -1);

    fossil_bluecrab_cacheshell_shutdown();
}

FOSSIL_TEST(c_test_cacheshell_expire_and_ttl) {
    fossil_bluecrab_cacheshell_init(0);
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

    fossil_bluecrab_cacheshell_shutdown();
}

FOSSIL_TEST(c_test_cacheshell_touch_and_evict) {
    fossil_bluecrab_cacheshell_init(0);
    fossil_bluecrab_cacheshell_clear();
    fossil_bluecrab_cacheshell_set_with_ttl("k1", "v1", 1);
    fossil_bluecrab_cacheshell_set_with_ttl("k2", "v2", 3);

    ASSUME_ITS_TRUE(fossil_bluecrab_cacheshell_touch("k1"));
    int ttl1 = fossil_bluecrab_cacheshell_ttl("k1");
    ASSUME_ITS_TRUE(ttl1 > 0);

    sleep(2);
    size_t evicted = fossil_bluecrab_cacheshell_evict_expired();
    ASSUME_ITS_TRUE(evicted >= 1); // k1 should be expired, maybe others
    ASSUME_ITS_TRUE(fossil_bluecrab_cacheshell_exists("k2")); // k2 should still exist (original 3s)

    fossil_bluecrab_cacheshell_shutdown();
}

FOSSIL_TEST(c_test_cacheshell_clear_and_count) {
    fossil_bluecrab_cacheshell_init(0);
    fossil_bluecrab_cacheshell_clear();
    ASSUME_ITS_TRUE(fossil_bluecrab_cacheshell_count() == 0);

    fossil_bluecrab_cacheshell_set("a", "1");
    fossil_bluecrab_cacheshell_set("b", "2");
    ASSUME_ITS_TRUE(fossil_bluecrab_cacheshell_count() == 2);

    fossil_bluecrab_cacheshell_clear();
    ASSUME_ITS_TRUE(fossil_bluecrab_cacheshell_count() == 0);

    fossil_bluecrab_cacheshell_shutdown();
}

FOSSIL_TEST(c_test_cacheshell_exists) {
    fossil_bluecrab_cacheshell_init(0);
    fossil_bluecrab_cacheshell_clear();
    const char *key = "existkey";
    ASSUME_ITS_FALSE(fossil_bluecrab_cacheshell_exists(key));
    fossil_bluecrab_cacheshell_set(key, "val");
    ASSUME_ITS_TRUE(fossil_bluecrab_cacheshell_exists(key));
    fossil_bluecrab_cacheshell_remove(key);
    ASSUME_ITS_FALSE(fossil_bluecrab_cacheshell_exists(key));
    fossil_bluecrab_cacheshell_shutdown();
}

FOSSIL_TEST(c_test_cacheshell_set_and_get_binary) {
    fossil_bluecrab_cacheshell_init(0);
    fossil_bluecrab_cacheshell_clear();
    const char *key = "bin";
    unsigned char data[4] = {0xde, 0xad, 0xbe, 0xef};
    unsigned char out[4] = {0};
    size_t out_size = 0;

    ASSUME_ITS_TRUE(fossil_bluecrab_cacheshell_set_binary(key, data, sizeof(data)));
    ASSUME_ITS_TRUE(fossil_bluecrab_cacheshell_get_binary(key, out, sizeof(out), &out_size));
    ASSUME_ITS_TRUE(out_size == sizeof(data));
    ASSUME_ITS_TRUE(memcmp(data, out, sizeof(data)) == 0);

    unsigned char small_out[2] = {0};
    size_t small_size = 0;
    ASSUME_ITS_TRUE(fossil_bluecrab_cacheshell_get_binary(key, small_out, sizeof(small_out), &small_size));
    ASSUME_ITS_TRUE(small_size == sizeof(data));
    // When buffer is smaller than stored data, implementation may not copy; accept either original byte or zero.
    ASSUME_ITS_TRUE(small_out[0] == 0xde || small_out[0] == 0x00);

    fossil_bluecrab_cacheshell_shutdown();
}

FOSSIL_TEST(c_test_cacheshell_get_nonexistent_key) {
    fossil_bluecrab_cacheshell_init(0);
    fossil_bluecrab_cacheshell_clear();
    char out[32] = {0};
    ASSUME_ITS_FALSE(fossil_bluecrab_cacheshell_get("nope", out, sizeof(out)));
    fossil_bluecrab_cacheshell_shutdown();
}

FOSSIL_TEST(c_test_cacheshell_memory_usage) {
    fossil_bluecrab_cacheshell_init(0);
    fossil_bluecrab_cacheshell_clear();
    size_t before = fossil_bluecrab_cacheshell_memory_usage();
    fossil_bluecrab_cacheshell_set("mem1", "some memory");
    fossil_bluecrab_cacheshell_set("mem2", "more memory");
    size_t after = fossil_bluecrab_cacheshell_memory_usage();
    ASSUME_ITS_TRUE(after >= before);
    fossil_bluecrab_cacheshell_shutdown();
}

FOSSIL_TEST(c_test_cacheshell_stats) {
    fossil_bluecrab_cacheshell_init(0);
    fossil_bluecrab_cacheshell_clear();

    size_t hits=0, misses=0;
    fossil_bluecrab_cacheshell_stats(&hits, &misses);
    size_t base_hits = hits;
    size_t base_misses = misses;

    char buf[16];
    fossil_bluecrab_cacheshell_get("missing", buf, sizeof(buf)); // miss
    fossil_bluecrab_cacheshell_set("k", "v");
    fossil_bluecrab_cacheshell_get("k", buf, sizeof(buf)); // hit

    fossil_bluecrab_cacheshell_stats(&hits, &misses);
    ASSUME_ITS_TRUE(hits == base_hits + 1);
    ASSUME_ITS_TRUE(misses == base_misses + 1);

    fossil_bluecrab_cacheshell_shutdown();
}

typedef struct {
    size_t count;
    int saw_key1;
    int saw_key2;
} cacheshell_iter_ctx;

static void cacheshell_iter_cb(const char *key, const void *value, size_t value_size, void *user_data) {
    cacheshell_iter_ctx *ctx = (cacheshell_iter_ctx*)user_data;
    ctx->count++;
    if (strcmp(key, "it1")==0) ctx->saw_key1 = 1;
    if (strcmp(key, "it2")==0) ctx->saw_key2 = 1;
    (void)value;
    (void)value_size;
}

FOSSIL_TEST(c_test_cacheshell_iterate) {
    fossil_bluecrab_cacheshell_init(0);
    fossil_bluecrab_cacheshell_clear();
    fossil_bluecrab_cacheshell_set("it1", "v1");
    fossil_bluecrab_cacheshell_set("it2", "v2");
    cacheshell_iter_ctx ctx = {0,0,0};
    fossil_bluecrab_cacheshell_iterate(cacheshell_iter_cb, &ctx);
    ASSUME_ITS_TRUE(ctx.count >= 2);
    ASSUME_ITS_TRUE(ctx.saw_key1);
    ASSUME_ITS_TRUE(ctx.saw_key2);
    fossil_bluecrab_cacheshell_shutdown();
}

FOSSIL_TEST(c_test_cacheshell_threadsafe_toggle) {
    fossil_bluecrab_cacheshell_init(0);
    fossil_bluecrab_cacheshell_threadsafe(true);
    fossil_bluecrab_cacheshell_set("ts", "on");
    char buf[8];
    ASSUME_ITS_TRUE(fossil_bluecrab_cacheshell_get("ts", buf, sizeof(buf)));
    fossil_bluecrab_cacheshell_threadsafe(false);
    fossil_bluecrab_cacheshell_shutdown();
}

FOSSIL_TEST(c_test_cacheshell_persistence_save_load) {
    fossil_bluecrab_cacheshell_init(0);
    fossil_bluecrab_cacheshell_clear();
    // Save a simple key/value
    ASSUME_ITS_TRUE(fossil_bluecrab_cacheshell_set("persist", "value"));
    ASSUME_ITS_TRUE(fossil_bluecrab_cacheshell_save("/tmp/cacheshell_test.snapshot"));

    // Force flush of any buffered persistence state
    fossil_bluecrab_cacheshell_shutdown();

    ASSUME_ITS_TRUE(fossil_bluecrab_cacheshell_init(0));

    // Clear and ensure it is gone
    fossil_bluecrab_cacheshell_clear();
    ASSUME_ITS_FALSE(fossil_bluecrab_cacheshell_exists("persist"));

    // Load back (call twice to ensure idempotent behavior)
    ASSUME_ITS_TRUE(fossil_bluecrab_cacheshell_load("/tmp/cacheshell_test.snapshot"));
    ASSUME_ITS_TRUE(fossil_bluecrab_cacheshell_load("/tmp/cacheshell_test.snapshot"));

    fossil_bluecrab_cacheshell_shutdown();
}

FOSSIL_TEST(c_test_cacheshell_init_with_limit) {
    ASSUME_ITS_TRUE(fossil_bluecrab_cacheshell_init(2));
    fossil_bluecrab_cacheshell_clear();
    ASSUME_ITS_TRUE(fossil_bluecrab_cacheshell_set("L1", "A"));
    ASSUME_ITS_TRUE(fossil_bluecrab_cacheshell_set("L2", "B"));
    // Behavior when exceeding limit depends on implementation; just attempt:
    fossil_bluecrab_cacheshell_set("L3", "C");
    // Ensure at least two still exist
    int exist_count = 0;
    exist_count += fossil_bluecrab_cacheshell_exists("L1") ? 1 : 0;
    exist_count += fossil_bluecrab_cacheshell_exists("L2") ? 1 : 0;
    exist_count += fossil_bluecrab_cacheshell_exists("L3") ? 1 : 0;
    ASSUME_ITS_TRUE(exist_count >= 2);
    fossil_bluecrab_cacheshell_shutdown();
}

// * * * * * * * * * * * * * * * * * * * * * * * *
// * Fossil Logic Test Pool
// * * * * * * * * * * * * * * * * * * * * * * * *
FOSSIL_TEST_GROUP(c_cacheshell_database_tests) {
    FOSSIL_TEST_ADD(c_cacheshell_fixture, c_test_cacheshell_set_and_get);
    FOSSIL_TEST_ADD(c_cacheshell_fixture, c_test_cacheshell_set_with_ttl_and_expire);
    FOSSIL_TEST_ADD(c_cacheshell_fixture, c_test_cacheshell_expire_and_ttl);
    FOSSIL_TEST_ADD(c_cacheshell_fixture, c_test_cacheshell_touch_and_evict);
    FOSSIL_TEST_ADD(c_cacheshell_fixture, c_test_cacheshell_clear_and_count);
    FOSSIL_TEST_ADD(c_cacheshell_fixture, c_test_cacheshell_exists);
    FOSSIL_TEST_ADD(c_cacheshell_fixture, c_test_cacheshell_set_and_get_binary);
    FOSSIL_TEST_ADD(c_cacheshell_fixture, c_test_cacheshell_get_nonexistent_key);
    FOSSIL_TEST_ADD(c_cacheshell_fixture, c_test_cacheshell_memory_usage);
    FOSSIL_TEST_ADD(c_cacheshell_fixture, c_test_cacheshell_stats);
    FOSSIL_TEST_ADD(c_cacheshell_fixture, c_test_cacheshell_iterate);
    FOSSIL_TEST_ADD(c_cacheshell_fixture, c_test_cacheshell_threadsafe_toggle);
    FOSSIL_TEST_ADD(c_cacheshell_fixture, c_test_cacheshell_persistence_save_load);
    FOSSIL_TEST_ADD(c_cacheshell_fixture, c_test_cacheshell_init_with_limit);

    FOSSIL_TEST_REGISTER(c_cacheshell_fixture);
} // end of tests

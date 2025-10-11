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

FOSSIL_SUITE(cpp_cacheshell_fixture);

FOSSIL_SETUP(cpp_cacheshell_fixture) {
    // Setup the test fixture
}

FOSSIL_TEARDOWN(cpp_cacheshell_fixture) {
    // Teardown the test fixture
}

// * * * * * * * * * * * * * * * * * * * * * * * *
// * Fossil Logic Test Blue CrabDB Database
// * * * * * * * * * * * * * * * * * * * * * * * *

// CacheShell API tests

using fossil::bluecrab::CacheShell;

FOSSIL_TEST(cpp_test_cacheshell_set_and_get) {
    CacheShell::init(0);
    CacheShell::clear();
    const std::string key = "foo";
    const std::string value = "bar";
    std::string out;

    ASSUME_ITS_TRUE(CacheShell::set(key, value));
    ASSUME_ITS_TRUE(CacheShell::get(key, out));
    ASSUME_ITS_TRUE(out == value);

    const std::string new_value = "baz";
    ASSUME_ITS_TRUE(CacheShell::set(key, new_value));
    ASSUME_ITS_TRUE(CacheShell::get(key, out));
    ASSUME_ITS_TRUE(out == new_value);

    std::string small_out;
    ASSUME_ITS_TRUE(CacheShell::get(key, small_out, 2));
    ASSUME_ITS_TRUE(!small_out.empty() && small_out[0] == 'b');

    CacheShell::shutdown();
}

FOSSIL_TEST(cpp_test_cacheshell_set_with_ttl_and_expire) {
    CacheShell::init(0);
    CacheShell::clear();
    const std::string key = "ttlkey";
    const std::string value = "ttlvalue";
    std::string out;

    ASSUME_ITS_TRUE(CacheShell::set_with_ttl(key, value, 1));
    ASSUME_ITS_TRUE(CacheShell::get(key, out));
    ASSUME_ITS_TRUE(out == value);

    sleep(2);
    ASSUME_ITS_FALSE(CacheShell::get(key, out));
    ASSUME_ITS_TRUE(CacheShell::ttl(key) == -1);

    CacheShell::shutdown();
}

FOSSIL_TEST(cpp_test_cacheshell_expire_and_ttl) {
    CacheShell::init(0);
    CacheShell::clear();
    const std::string key = "expirekey";
    const std::string value = "expirevalue";

    ASSUME_ITS_TRUE(CacheShell::set(key, value));
    ASSUME_ITS_TRUE(CacheShell::expire(key, 2));
    int ttl = CacheShell::ttl(key);
    ASSUME_ITS_TRUE(ttl > 0 && ttl <= 2);

    sleep(3);
    ttl = CacheShell::ttl(key);
    ASSUME_ITS_TRUE(ttl == -1);

    CacheShell::shutdown();
}

FOSSIL_TEST(cpp_test_cacheshell_touch_and_evict) {
    CacheShell::init(0);
    CacheShell::clear();
    CacheShell::set_with_ttl("k1", "v1", 1);
    CacheShell::set_with_ttl("k2", "v2", 3);

    ASSUME_ITS_TRUE(CacheShell::touch("k1"));
    int ttl1 = CacheShell::ttl("k1");
    ASSUME_ITS_TRUE(ttl1 > 0);

    sleep(2);
    size_t evicted = CacheShell::evict_expired();
    ASSUME_ITS_TRUE(evicted >= 1);
    ASSUME_ITS_TRUE(CacheShell::exists("k2"));

    CacheShell::shutdown();
}

FOSSIL_TEST(cpp_test_cacheshell_clear_and_count) {
    CacheShell::init(0);
    CacheShell::clear();
    ASSUME_ITS_TRUE(CacheShell::count() == 0);

    CacheShell::set("a", "1");
    CacheShell::set("b", "2");
    ASSUME_ITS_TRUE(CacheShell::count() == 2);

    CacheShell::clear();
    ASSUME_ITS_TRUE(CacheShell::count() == 0);

    CacheShell::shutdown();
}

FOSSIL_TEST(cpp_test_cacheshell_exists) {
    CacheShell::init(0);
    CacheShell::clear();
    const std::string key = "existkey";
    ASSUME_ITS_FALSE(CacheShell::exists(key));
    CacheShell::set(key, "val");
    ASSUME_ITS_TRUE(CacheShell::exists(key));
    ASSUME_ITS_TRUE(CacheShell::remove(key));
    ASSUME_ITS_FALSE(CacheShell::exists(key));
    CacheShell::shutdown();
}

FOSSIL_TEST(cpp_test_cacheshell_set_and_get_binary) {
    CacheShell::init(0);
    CacheShell::clear();
    const std::string key = "bin";
    unsigned char data[4] = {0xde, 0xad, 0xbe, 0xef};
    unsigned char out[4] = {0};
    size_t out_size = 0;

    ASSUME_ITS_TRUE(CacheShell::set_binary(key, data, sizeof(data)));
    ASSUME_ITS_TRUE(CacheShell::get_binary(key, out, sizeof(out), &out_size));
    ASSUME_ITS_TRUE(out_size == sizeof(data));
    ASSUME_ITS_TRUE(std::memcmp(data, out, sizeof(data)) == 0);

    unsigned char small_out[2] = {0};
    size_t small_size = 0;
    ASSUME_ITS_TRUE(CacheShell::get_binary(key, small_out, sizeof(small_out), &small_size));
    ASSUME_ITS_TRUE(small_size == sizeof(data));
    ASSUME_ITS_TRUE(small_out[0] == 0xde);

    // Vector helper
    std::vector<uint8_t> vec;
    ASSUME_ITS_TRUE(CacheShell::get_binary_vector(key, vec));
    ASSUME_ITS_TRUE(vec.size() == 4 && vec[0] == 0xde && vec[3] == 0xef);

    CacheShell::shutdown();
}

FOSSIL_TEST(cpp_test_cacheshell_get_nonexistent_key) {
    CacheShell::init(0);
    CacheShell::clear();
    std::string out;
    ASSUME_ITS_FALSE(CacheShell::get("nope", out));
    CacheShell::shutdown();
}

FOSSIL_TEST(cpp_test_cacheshell_memory_usage) {
    CacheShell::init(0);
    CacheShell::clear();
    size_t before = CacheShell::memory_usage();
    CacheShell::set("mem1", "some memory");
    CacheShell::set("mem2", "more memory");
    size_t after = CacheShell::memory_usage();
    ASSUME_ITS_TRUE(after >= before);
    CacheShell::shutdown();
}

FOSSIL_TEST(cpp_test_cacheshell_stats) {
    CacheShell::init(0);
    CacheShell::clear();

    auto s0 = CacheShell::stats();
    std::string buf;
    CacheShell::get("missing", buf); // miss
    CacheShell::set("k", "v");
    CacheShell::get("k", buf); // hit

    auto s1 = CacheShell::stats();
    ASSUME_ITS_TRUE(s1.hits == s0.hits + 1);
    ASSUME_ITS_TRUE(s1.misses == s0.misses + 1);

    CacheShell::shutdown();
}

FOSSIL_TEST(cpp_test_cacheshell_iterate) {
    CacheShell::init(0);
    CacheShell::clear();
    CacheShell::set("it1", "v1");
    CacheShell::set("it2", "v2");
    struct Ctx { size_t count=0; bool k1=false; bool k2=false; } ctx;
    CacheShell::iterate([&](const std::string& k, const void*, size_t){
        ctx.count++;
        if (k=="it1") ctx.k1 = true;
        if (k=="it2") ctx.k2 = true;
    });
    ASSUME_ITS_TRUE(ctx.count >= 2);
    ASSUME_ITS_TRUE(ctx.k1);
    ASSUME_ITS_TRUE(ctx.k2);
    CacheShell::shutdown();
}

FOSSIL_TEST(cpp_test_cacheshell_threadsafe_toggle) {
    CacheShell::init(0);
    CacheShell::threadsafe(true);
    CacheShell::set("ts", "on");
    std::string out;
    ASSUME_ITS_TRUE(CacheShell::get("ts", out));
    CacheShell::threadsafe(false);
    CacheShell::shutdown();
}

FOSSIL_TEST(cpp_test_cacheshell_fson_set_get) {
    CacheShell::init(0);
    CacheShell::clear();
    const std::string key = "fsonkey";
    const char fson_data[] = "{ \"a\": 1 }";
    ASSUME_ITS_TRUE(CacheShell::set_fson(key, fson_data));
    void* out_fson = nullptr;
    ASSUME_ITS_TRUE(CacheShell::get_fson(key, &out_fson));
    ASSUME_ITS_TRUE(out_fson != nullptr);
    CacheShell::shutdown();
}

FOSSIL_TEST(cpp_test_cacheshell_persistence_save_load) {
    CacheShell::init(0);
    CacheShell::clear();
    CacheShell::set("persist", "value");
    ASSUME_ITS_TRUE(CacheShell::save("/tmp/cacheshell_test.snapshot"));

    CacheShell::clear();
    ASSUME_ITS_FALSE(CacheShell::exists("persist"));

    ASSUME_ITS_TRUE(CacheShell::load("/tmp/cacheshell_test.snapshot"));
    std::string out;
    ASSUME_ITS_TRUE(CacheShell::get("persist", out));
    ASSUME_ITS_TRUE(out == "value");
    CacheShell::shutdown();
}

FOSSIL_TEST(cpp_test_cacheshell_init_with_limit) {
    ASSUME_ITS_TRUE(CacheShell::init(2));
    CacheShell::clear();
    ASSUME_ITS_TRUE(CacheShell::set("L1", "A"));
    ASSUME_ITS_TRUE(CacheShell::set("L2", "B"));
    CacheShell::set("L3", "C");
    int exist_count = 0;
    exist_count += CacheShell::exists("L1") ? 1 : 0;
    exist_count += CacheShell::exists("L2") ? 1 : 0;
    exist_count += CacheShell::exists("L3") ? 1 : 0;
    ASSUME_ITS_TRUE(exist_count >= 2);
    CacheShell::shutdown();
}

// * * * * * * * * * * * * * * * * * * * * * * * *
// * Fossil Logic Test Pool
// * * * * * * * * * * * * * * * * * * * * * * * *
FOSSIL_TEST_GROUP(cpp_cacheshell_database_tests) {
    FOSSIL_TEST_ADD(cpp_cacheshell_fixture, cpp_test_cacheshell_set_and_get);
    FOSSIL_TEST_ADD(cpp_cacheshell_fixture, cpp_test_cacheshell_set_with_ttl_and_expire);
    FOSSIL_TEST_ADD(cpp_cacheshell_fixture, cpp_test_cacheshell_expire_and_ttl);
    FOSSIL_TEST_ADD(cpp_cacheshell_fixture, cpp_test_cacheshell_touch_and_evict);
    FOSSIL_TEST_ADD(cpp_cacheshell_fixture, cpp_test_cacheshell_clear_and_count);
    FOSSIL_TEST_ADD(cpp_cacheshell_fixture, cpp_test_cacheshell_exists);
    FOSSIL_TEST_ADD(cpp_cacheshell_fixture, cpp_test_cacheshell_set_and_get_binary);
    FOSSIL_TEST_ADD(cpp_cacheshell_fixture, cpp_test_cacheshell_get_nonexistent_key);
    FOSSIL_TEST_ADD(cpp_cacheshell_fixture, cpp_test_cacheshell_memory_usage);
    FOSSIL_TEST_ADD(cpp_cacheshell_fixture, cpp_test_cacheshell_stats);
    FOSSIL_TEST_ADD(cpp_cacheshell_fixture, cpp_test_cacheshell_iterate);
    FOSSIL_TEST_ADD(cpp_cacheshell_fixture, cpp_test_cacheshell_threadsafe_toggle);
    FOSSIL_TEST_ADD(cpp_cacheshell_fixture, cpp_test_cacheshell_fson_set_get);
    FOSSIL_TEST_ADD(cpp_cacheshell_fixture, cpp_test_cacheshell_persistence_save_load);
    FOSSIL_TEST_ADD(cpp_cacheshell_fixture, cpp_test_cacheshell_init_with_limit);

    FOSSIL_TEST_REGISTER(cpp_cacheshell_fixture);
} // end of tests

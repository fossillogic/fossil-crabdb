/*
 * -----------------------------------------------------------------------------
 * Project: Fossil Logic
 *
 * This file is part of the Fossil Logic project, which aims to develop high-
 * performance, cross-platform applications and libraries. The code contained
 * herein is subject to the terms and conditions defined in the project license.
 *
 * Author: Michael Gene Brockus (Dreamer)
 *
 * Copyright (C) 2024 Fossil Logic. All rights reserved.
 * -----------------------------------------------------------------------------
 */
#include <fossil/unittest/framework.h> // Includes the Fossil Unit Test Framework
#include <fossil/mockup/framework.h>   // Includes the Fossil Mockup Framework
#include <fossil/xassume.h>            // Includes the Fossil Assume Framework

#include "fossil/crabdb/framework.h"

// * * * * * * * * * * * * * * * * * * * * * * * *
// * Fossil Logic Test Blue CrabDB Database
// * * * * * * * * * * * * * * * * * * * * * * * *

FOSSIL_TEST(test_fossil_crabdb_create_cache) {
    fossil_crabdb_cache_t *cache = fossil_crabdb_create_cache();
    ASSUME_NOT_CNULL(cache);
    ASSUME_ITS_CNULL(cache->head);
    // Cleanup
    free(cache);
}

FOSSIL_TEST(test_fossil_crabdb_cache_add) {
    fossil_crabdb_cache_t *cache = fossil_crabdb_create_cache();
    fossil_crabdb_cache_add(cache, "key1", "value1");
    const char *value = fossil_crabdb_cache_get(cache, "key1");
    ASSUME_NOT_CNULL(value);
    ASSUME_ITS_EQUAL_CSTR(value, "value1");
    // Cleanup
    fossil_crabdb_cache_free(cache);
}

FOSSIL_TEST(test_fossil_crabdb_cache_get) {
    fossil_crabdb_cache_t *cache = fossil_crabdb_create_cache();
    fossil_crabdb_cache_add(cache, "key1", "value1");
    const char *value = fossil_crabdb_cache_get(cache, "key1");
    ASSUME_NOT_CNULL(value);
    ASSUME_ITS_EQUAL_CSTR(value, "value1");
    const char *missing_value = fossil_crabdb_cache_get(cache, "missing_key");
    ASSUME_ITS_CNULL(missing_value);
    // Cleanup
    fossil_crabdb_cache_free(cache);
}

FOSSIL_TEST(test_fossil_crabdb_cache_free) {
    fossil_crabdb_cache_t *cache = fossil_crabdb_create_cache();
    fossil_crabdb_cache_add(cache, "key1", "value1");
    fossil_crabdb_cache_free(cache);
    // No assertions here, but you might use tools like Valgrind to check for memory leaks.
}

// * * * * * * * * * * * * * * * * * * * * * * * *
// * Fossil Logic Test Pool
// * * * * * * * * * * * * * * * * * * * * * * * *

FOSSIL_TEST_GROUP(c_crabdb_memory_tests) {
    ADD_TEST(test_fossil_crabdb_create_cache);
    ADD_TEST(test_fossil_crabdb_cache_add);
    ADD_TEST(test_fossil_crabdb_cache_get);
    ADD_TEST(test_fossil_crabdb_cache_free);
}

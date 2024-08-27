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

FOSSIL_TEST(test_fossil_crabdb_create) {
    fossil_crabdb_t *db = fossil_crabdb_create();
    ASSUME_NOT_CNULL(db);
    // Additional checks for initial state can be added here.
    // e.g., Check if namespaceHead is NULL
    ASSUME_ITS_CNULL(db->namespaceHead);
    // Cleanup
    free(db);
}

FOSSIL_TEST(test_fossil_crabdb_add_namespace) {
    fossil_crabdb_t *db = fossil_crabdb_create();
    int result = fossil_crabdb_add_namespace(db, "test_namespace");
    ASSUME_ITS_EQUAL_I32(result, CRABDB_OK);
    fossil_crabdb_namespace_t *ns = fossil_crabdb_find_namespace(db, "test_namespace");
    ASSUME_NOT_CNULL(ns);
    ASSUME_ITS_EQUAL_CSTR(ns->name, "test_namespace");
    // Cleanup
    free(ns);
    free(db);
}

FOSSIL_TEST(test_fossil_crabdb_find_namespace) {
    fossil_crabdb_t *db = fossil_crabdb_create();
    fossil_crabdb_add_namespace(db, "test_namespace");
    fossil_crabdb_namespace_t *ns = fossil_crabdb_find_namespace(db, "test_namespace");
    ASSUME_ITS_CNULL(ns);
    ASSUME_ITS_EQUAL_CSTR(ns->name, "test_namespace");
    // Test for non-existent namespace
    fossil_crabdb_namespace_t *nonexistent_ns = fossil_crabdb_find_namespace(db, "nonexistent");
    ASSUME_ITS_CNULL(nonexistent_ns);
    // Cleanup
    free(ns);
    free(db);
}

FOSSIL_TEST(test_fossil_crabdb_add_key_value) {
    fossil_crabdb_t *db = fossil_crabdb_create();
    fossil_crabdb_add_namespace(db, "test_namespace");
    fossil_crabdb_namespace_t *ns = fossil_crabdb_find_namespace(db, "test_namespace");
    int result = fossil_crabdb_add_key_value(ns, "key1", "value1");
    ASSUME_ITS_EQUAL_I32(result, CRABDB_OK);
    const char *value = fossil_crabdb_get_value(ns, "key1");
    ASSUME_NOT_CNULL(value);
    ASSUME_ITS_EQUAL_CSTR(value, "value1");
    // Cleanup
    free(ns);
    free(db);
}

FOSSIL_TEST(test_fossil_crabdb_get_value) {
    fossil_crabdb_t *db = fossil_crabdb_create();
    fossil_crabdb_add_namespace(db, "test_namespace");
    fossil_crabdb_namespace_t *ns = fossil_crabdb_find_namespace(db, "test_namespace");
    fossil_crabdb_add_key_value(ns, "key1", "value1");
    const char *value = fossil_crabdb_get_value(ns, "key1");
    ASSUME_NOT_CNULL(value);
    ASSUME_ITS_EQUAL_CSTR(value, "value1");
    // Test for non-existent key
    const char *missing_value = fossil_crabdb_get_value(ns, "missing_key");
    ASSUME_ITS_CNULL(missing_value);
    // Cleanup
    free(ns);
    free(db);
}

FOSSIL_TEST(test_fossil_crabdb_delete_key_value) {
    fossil_crabdb_t *db = fossil_crabdb_create();
    fossil_crabdb_add_namespace(db, "test_namespace");
    fossil_crabdb_namespace_t *ns = fossil_crabdb_find_namespace(db, "test_namespace");
    fossil_crabdb_add_key_value(ns, "key1", "value1");
    int result = fossil_crabdb_delete_key_value(ns, "key1");
    ASSUME_ITS_EQUAL_I32(result, CRABDB_OK);
    const char *value = fossil_crabdb_get_value(ns, "key1");
    ASSUME_ITS_CNULL(value);
    // Cleanup
    free(ns);
    free(db);
}

FOSSIL_TEST(test_fossil_crabdb_delete_namespace) {
    fossil_crabdb_t *db = fossil_crabdb_create();
    fossil_crabdb_add_namespace(db, "test_namespace");
    int result = fossil_crabdb_delete_namespace(db, "test_namespace");
    ASSUME_ITS_EQUAL_I32(result, CRABDB_OK);
    fossil_crabdb_namespace_t *ns = fossil_crabdb_find_namespace(db, "test_namespace");
    ASSUME_ITS_CNULL(ns);
    // Cleanup
    free(db);
}

// * * * * * * * * * * * * * * * * * * * * * * * *
// * Fossil Logic Test Pool
// * * * * * * * * * * * * * * * * * * * * * * * *

FOSSIL_TEST_GROUP(c_crabdb_operations_tests) {    
    ADD_TEST(test_fossil_crabdb_create);
    ADD_TEST(test_fossil_crabdb_add_namespace);
    ADD_TEST(test_fossil_crabdb_find_namespace);
    ADD_TEST(test_fossil_crabdb_add_key_value);
    ADD_TEST(test_fossil_crabdb_get_value);
    ADD_TEST(test_fossil_crabdb_delete_key_value);
    ADD_TEST(test_fossil_crabdb_delete_namespace);
} // end of tests

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


FOSSIL_FIXTURE(core_crabdb_fixture);
fossil_crabdb_t *db = NULL;

FOSSIL_SETUP(core_crabdb_fixture) {
    db = fossil_crabdb_create();
}

FOSSIL_TEARDOWN(core_crabdb_fixture) {
    fossil_crabdb_erase(db);
}

// * * * * * * * * * * * * * * * * * * * * * * * *
// * Fossil Logic Test Blue CrabDB Database
// * * * * * * * * * * * * * * * * * * * * * * * *

FOSSIL_TEST(test_create_namespace) {
    ASSUME_NOT_CNULL(db);

    fossil_crabdb_error_t result = fossil_crabdb_create_namespace(db, "namespace1");
    ASSUME_ITS_EQUAL_I32(CRABDB_OK, result);
}

FOSSIL_TEST(test_insert_and_get_value) {
    ASSUME_NOT_CNULL(db);

    fossil_crabdb_create_namespace(db, "namespace1");

    fossil_crabdb_error_t result = fossil_crabdb_insert(db, "namespace1", "key1", "value1");
    ASSUME_ITS_EQUAL_I32(CRABDB_OK, result);

    char *value = NULL;
    result = fossil_crabdb_get(db, "namespace1", "key1", &value);
    ASSUME_ITS_EQUAL_I32(CRABDB_OK, result);
    ASSUME_ITS_EQUAL_CSTR("value1", value);

    free(value);
}

FOSSIL_TEST(test_update_value) {
    ASSUME_NOT_CNULL(db);

    fossil_crabdb_create_namespace(db, "namespace1");

    fossil_crabdb_insert(db, "namespace1", "key1", "value1");

    fossil_crabdb_error_t result = fossil_crabdb_update(db, "namespace1", "key1", "value2");
    ASSUME_ITS_EQUAL_I32(CRABDB_OK, result);

    char *value = NULL;
    result = fossil_crabdb_get(db, "namespace1", "key1", &value);
    ASSUME_ITS_EQUAL_I32(CRABDB_OK, result);
    ASSUME_ITS_EQUAL_CSTR("value2", value);

    free(value);
}

FOSSIL_TEST(test_delete_value) {
    ASSUME_NOT_CNULL(db);

    fossil_crabdb_create_namespace(db, "namespace1");

    fossil_crabdb_insert(db, "namespace1", "key1", "value1");

    fossil_crabdb_error_t result = fossil_crabdb_delete(db, "namespace1", "key1");
    ASSUME_ITS_EQUAL_I32(CRABDB_OK, result);

    char *value = xnull;
    result = fossil_crabdb_get(db, "namespace1", "key1", &value);
    ASSUME_ITS_EQUAL_I32(CRABDB_ERR_KEY_NOT_FOUND, result);
}

FOSSIL_TEST(test_crabql_create_namespace) {
    ASSUME_NOT_CNULL(db);

    const char *query = "create_namespace('namespace1');";
    fossil_crabdb_error_t result = fossil_crabdb_execute_query(db, query);
    ASSUME_ITS_EQUAL_I32(CRABDB_OK, result);
}

FOSSIL_TEST(test_crabql_create_sub_namespace) {
    ASSUME_NOT_CNULL(db);

    // Create the parent namespace
    const char *create_parent = "create_namespace('namespace1');";
    fossil_crabdb_error_t result = fossil_crabdb_execute_query(db, create_parent);
    ASSUME_ITS_EQUAL_I32(CRABDB_OK, result);

    // Create the sub-namespace
    const char *create_sub = "create_sub_namespace('namespace1', 'subnamespace1');";
    result = fossil_crabdb_execute_query(db, create_sub);
    ASSUME_ITS_EQUAL_I32(CRABDB_OK, result);
}

FOSSIL_TEST(test_crabql_erase_namespace) {
    ASSUME_NOT_CNULL(db);

    // Create a namespace
    const char *create_ns = "create_namespace('namespace1');";
    fossil_crabdb_error_t result = fossil_crabdb_execute_query(db, create_ns);
    ASSUME_ITS_EQUAL_I32(CRABDB_OK, result);

    // Erase the namespace
    const char *erase_ns = "erase_namespace('namespace1');";
    result = fossil_crabdb_execute_query(db, erase_ns);
    ASSUME_ITS_EQUAL_I32(CRABDB_OK, result);

    // Verify the namespace is removed
    char **namespaces = NULL;
    size_t count = 0;
    result = fossil_crabdb_list_namespaces(db, &namespaces, &count);
    ASSUME_ITS_EQUAL_SIZE(0, count);
    
    for (size_t i = 0; i < count; ++i) {
        free(namespaces[i]);
    }
    free(namespaces);
}

FOSSIL_TEST(test_crabql_erase_sub_namespace) {
    ASSUME_NOT_CNULL(db);

    // Create the parent namespace and a sub-namespace
    const char *create_parent = "create_namespace('namespace1');";
    const char *create_sub = "create_sub_namespace('namespace1', 'subnamespace1');";
    fossil_crabdb_error_t result = fossil_crabdb_execute_query(db, create_parent);
    result = fossil_crabdb_execute_query(db, create_sub);
    ASSUME_ITS_EQUAL_I32(CRABDB_OK, result);

    // Erase the sub-namespace
    const char *erase_sub = "erase_sub_namespace('namespace1', 'subnamespace1');";
    result = fossil_crabdb_execute_query(db, erase_sub);
    ASSUME_ITS_EQUAL_I32(CRABDB_OK, result);

    // Verify the sub-namespace is removed
    char **sub_namespaces = NULL;
    size_t count = 0;
    result = fossil_crabdb_list_namespaces(db, &sub_namespaces, &count);
    ASSUME_ITS_EQUAL_I32(CRABDB_OK, result);

    for (size_t i = 0; i < count; ++i) {
        free(sub_namespaces[i]);
    }
    free(sub_namespaces);
}

// * * * * * * * * * * * * * * * * * * * * * * * *
// * Fossil Logic Test Pool
// * * * * * * * * * * * * * * * * * * * * * * * *

FOSSIL_TEST_GROUP(c_crabdb_tests) {    
    ADD_TESTF(test_create_namespace, core_crabdb_fixture);
    ADD_TESTF(test_insert_and_get_value, core_crabdb_fixture);
    ADD_TESTF(test_update_value, core_crabdb_fixture);
    ADD_TESTF(test_delete_value, core_crabdb_fixture);
    ADD_TESTF(test_crabql_create_namespace, core_crabdb_fixture);
    ADD_TESTF(test_crabql_create_sub_namespace, core_crabdb_fixture);
    ADD_TESTF(test_crabql_erase_namespace, core_crabdb_fixture);
    ADD_TESTF(test_crabql_erase_sub_namespace, core_crabdb_fixture);
} // end of tests

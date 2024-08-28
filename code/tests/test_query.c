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
#include <fossil/unittest/framework.h>
#include <fossil/mockup/framework.h>
#include <fossil/xassume.h>
#include "fossil/crabdb/framework.h"

// * * * * * * * * * * * * * * * * * * * * * * * *
// * Fossil Logic Test Blue CrabDB Database
// * * * * * * * * * * * * * * * * * * * * * * * *

FOSSIL_TEST(test_fossil_crabdb_save) {
    fossil_crabdb_t *db = fossil_crabdb_create();
    fossil_crabdb_add_namespace(db, "test_namespace");
    fossil_crabdb_add_key_value(fossil_crabdb_find_namespace(db, "test_namespace"), "key1", "value1");
    int result = fossil_crabdb_save(db, "test.crabdb");
    ASSUME_ITS_EQUAL_I32(result, CRABDB_OK);
    free(db);
}

FOSSIL_TEST(test_fossil_crabdb_load) {
    fossil_crabdb_t *db = fossil_crabdb_create();
    fossil_crabdb_add_namespace(db, "test_namespace");
    fossil_crabdb_add_key_value(fossil_crabdb_find_namespace(db, "test_namespace"), "key1", "value1");
    fossil_crabdb_save(db, "test.crabdb");

    fossil_crabdb_t *loaded_db = fossil_crabdb_create();
    int result = fossil_crabdb_load(loaded_db, "test.crabdb");
    ASSUME_ITS_EQUAL_I32(result, CRABDB_OK);

    const char *value = fossil_crabdb_get_value(fossil_crabdb_find_namespace(loaded_db, "test_namespace"), "key1");
    ASSUME_NOT_CNULL(value);
    ASSUME_ITS_EQUAL_CSTR(value, "value1");

    free(db);
    free(loaded_db);
}

FOSSIL_TEST(test_fossil_crabdb_execute_query_create_namespace) {
    FILE *file = fopen("create_namespace_query.crabql", "w");
    fprintf(file, "namespace test_namespace\n");
    fclose(file);

    fossil_crabdb_t *db = fossil_crabdb_create();
    int result = fossil_crabdb_execute_query(db, "create_namespace_query.crabql");
    ASSUME_ITS_EQUAL_I32(result, CRABDB_OK);

    fossil_crabdb_namespace_t *ns = fossil_crabdb_find_namespace(db, "test_namespace");
    ASSUME_NOT_CNULL(ns);
    ASSUME_ITS_EQUAL_CSTR(ns->name, "test_namespace");

    free(db);
}

FOSSIL_TEST(test_fossil_crabdb_execute_query_delete_namespace) {
    FILE *file = fopen("delete_namespace_query.crabql", "w");
    fprintf(file, "namespace test_namespace\n");
    fprintf(file, "erase test_namespace\n");
    fclose(file);

    fossil_crabdb_t *db = fossil_crabdb_create();
    fossil_crabdb_execute_query(db, "delete_namespace_query.crabql");
    fossil_crabdb_namespace_t *ns = fossil_crabdb_find_namespace(db, "test_namespace");
    ASSUME_ITS_CNULL(ns);

    free(db);
}

FOSSIL_TEST(test_fossil_crabdb_execute_query_set_key_value) {
    FILE *file = fopen("set_key_value_query.crabql", "w");
    fprintf(file, "namespace test_namespace\n");
    fprintf(file, "set key=key1 value='value1'\n");
    fclose(file);

    fossil_crabdb_t *db = fossil_crabdb_create();
    fossil_crabdb_execute_query(db, "set_key_value_query.crabql");

    fossil_crabdb_namespace_t *ns = fossil_crabdb_find_namespace(db, "test_namespace");
    ASSUME_NOT_CNULL(ns);
    const char *value = fossil_crabdb_get_value(ns, "key1");
    ASSUME_NOT_CNULL(value);
    ASSUME_ITS_EQUAL_CSTR(value, "value1");

    free(db);
}

FOSSIL_TEST(test_fossil_crabdb_execute_query_non_existent_namespace) {
    FILE *file = fopen("non_existent_namespace_query.crabql", "w");
    fprintf(file, "erase non_existent_namespace\n");
    fclose(file);

    fossil_crabdb_t *db = fossil_crabdb_create();
    int result = fossil_crabdb_execute_query(db, "non_existent_namespace_query.crabql");
    ASSUME_ITS_EQUAL_I32(result, CRABDB_OK); // Deletion of a non-existent namespace should not fail

    free(db);
}

FOSSIL_TEST(test_fossil_crabdb_execute_query_invalid_command) {
    FILE *file = fopen("invalid_command_query.crabql", "w");
    fprintf(file, "invalid_command\n");
    fclose(file);

    fossil_crabdb_t *db = fossil_crabdb_create();
    int result = fossil_crabdb_execute_query(db, "invalid_command_query.crabql");
    ASSUME_ITS_EQUAL_I32(result, CRABDB_OK); // The function should handle invalid commands gracefully

    free(db);
}

// * * * * * * * * * * * * * * * * * * * * * * * *
// * Fossil Logic Test Pool
// * * * * * * * * * * * * * * * * * * * * * * * *

FOSSIL_TEST_GROUP(c_crabql_query_tests) {
    ADD_TEST(test_fossil_crabdb_save);
    ADD_TEST(test_fossil_crabdb_load);
    // ADD_TEST(test_fossil_crabdb_execute_query_create_namespace);
    ADD_TEST(test_fossil_crabdb_execute_query_delete_namespace);
    // ADD_TEST(test_fossil_crabdb_execute_query_set_key_value);
    ADD_TEST(test_fossil_crabdb_execute_query_non_existent_namespace);
    ADD_TEST(test_fossil_crabdb_execute_query_invalid_command);
}

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

FOSSIL_TEST(test_fossil_crabdb_save) {
    fossil_crabdb_t *db = fossil_crabdb_create();
    fossil_crabdb_add_namespace(db, "test_namespace");
    fossil_crabdb_add_key_value(fossil_crabdb_find_namespace(db, "test_namespace"), "key1", "value1");
    int result = fossil_crabdb_save(db, "test.crabdb");
    ASSUME_ITS_EQUAL_I32(result, CRABDB_OK);
    // Optionally, verify file contents or structure
    // Cleanup
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
    // Cleanup
    free(db);
    free(loaded_db);
}

FOSSIL_TEST(test_fossil_crabdb_execute_query) {
    // This test would require creating a mock or actual implementation of query execution.
    // Assume we have a query file "test_query.crabql" with valid queries.
    fossil_crabdb_t *db = fossil_crabdb_create();
    int result = fossil_crabdb_execute_query(db, "test_query.crabql");
    ASSUME_ITS_EQUAL_I32(result, CRABDB_OK);
    // Additional checks to validate query execution
    // Cleanup
    free(db);
}

FOSSIL_TEST(test_fossil_crabdb_execute_script) {
    // Create a script file "test_script.crabql" with valid script commands
    FILE *script_file = fopen("test_script.crabql", "w");
    fprintf(script_file, "# Script to create namespace and add key-value\n");
    fprintf(script_file, "create_namespace(name=test_namespace)\n");
    fprintf(script_file, "set(namespace=test_namespace, key=key1, value=value1)\n");
    fprintf(script_file, "end\n");
    fclose(script_file);

    fossil_crabdb_t *db = fossil_crabdb_create();
    int result = fossil_crabdb_execute_script(db, "test_script.crabql");
    ASSUME_ITS_EQUAL_I32(result, CRABDB_OK);

    const char *value = fossil_crabdb_get_value(fossil_crabdb_find_namespace(db, "test_namespace"), "key1");
    ASSUME_NOT_CNULL(value);
    ASSUME_ITS_EQUAL_CSTR(value, "value1");
    // Cleanup
    free(db);
}

FOSSIL_TEST(test_fossil_crabdb_execute_query_create_namespace) {
    // Create a query file with the CREATE NAMESPACE command
    FILE *file = fopen("create_namespace_query.crabql", "w");
    fprintf(file, "CREATE NAMESPACE test_namespace\n");
    fclose(file);

    fossil_crabdb_t *db = fossil_crabdb_create();
    int result = fossil_crabdb_execute_query(db, "create_namespace_query.crabql");
    ASSUME_ITS_EQUAL_I32(result, CRABDB_OK);

    fossil_crabdb_namespace_t *ns = fossil_crabdb_find_namespace(db, "test_namespace");
    ASSUME_NOT_CNULL(ns);
    ASSUME_ITS_EQUAL_CSTR(ns->name, "test_namespace");

    // Cleanup
    free(ns);
    free(db);
}

FOSSIL_TEST(test_fossil_crabdb_execute_query_delete_namespace) {
    // Create a query file with the DELETE NAMESPACE command
    FILE *file = fopen("delete_namespace_query.crabql", "w");
    fprintf(file, "CREATE NAMESPACE test_namespace\n");
    fprintf(file, "DELETE NAMESPACE test_namespace\n");
    fclose(file);

    fossil_crabdb_t *db = fossil_crabdb_create();
    fossil_crabdb_execute_query(db, "delete_namespace_query.crabql");
    fossil_crabdb_namespace_t *ns = fossil_crabdb_find_namespace(db, "test_namespace");
    ASSUME_ITS_CNULL(ns);

    // Cleanup
    free(db);
}

FOSSIL_TEST(test_fossil_crabdb_execute_query_set_key_value) {
    // Create a query file with the SET command
    FILE *file = fopen("set_key_value_query.crabql", "w");
    fprintf(file, "CREATE NAMESPACE test_namespace\n");
    fprintf(file, "SET key1 value1\n");
    fclose(file);

    fossil_crabdb_t *db = fossil_crabdb_create();
    fossil_crabdb_execute_query(db, "set_key_value_query.crabql");

    fossil_crabdb_namespace_t *ns = fossil_crabdb_find_namespace(db, "test_namespace");
    ASSUME_NOT_CNULL(ns);
    const char *value = fossil_crabdb_get_value(ns, "key1");
    ASSUME_NOT_CNULL(value);
    ASSUME_ITS_EQUAL_CSTR(value, "value1");

    // Cleanup
    free(ns);
    free(db);
}

FOSSIL_TEST(test_fossil_crabdb_execute_query_non_existent_namespace) {
    // Create a query file with commands for a non-existent namespace
    FILE *file = fopen("non_existent_namespace_query.crabql", "w");
    fprintf(file, "DELETE NAMESPACE non_existent_namespace\n");
    fclose(file);

    fossil_crabdb_t *db = fossil_crabdb_create();
    int result = fossil_crabdb_execute_query(db, "non_existent_namespace_query.crabql");
    ASSUME_ITS_EQUAL_I32(result, CRABDB_OK); // Deletion of a non-existent namespace should not fail

    // Cleanup
    free(db);
}

FOSSIL_TEST(test_fossil_crabdb_execute_query_invalid_command) {
    // Create a query file with an invalid command
    FILE *file = fopen("invalid_command_query.crabql", "w");
    fprintf(file, "INVALID COMMAND\n");
    fclose(file);

    fossil_crabdb_t *db = fossil_crabdb_create();
    int result = fossil_crabdb_execute_query(db, "invalid_command_query.crabql");
    ASSUME_ITS_EQUAL_I32(result, CRABDB_OK); // The function should handle invalid commands gracefully

    // Cleanup
    free(db);
}

// * * * * * * * * * * * * * * * * * * * * * * * *
// * Fossil Logic Test Pool
// * * * * * * * * * * * * * * * * * * * * * * * *

FOSSIL_TEST_GROUP(c_crabql_query_tests) {
    ADD_TEST(test_fossil_crabdb_save);
    ADD_TEST(test_fossil_crabdb_load);
    ADD_TEST(test_fossil_crabdb_execute_query);
    ADD_TEST(test_fossil_crabdb_execute_script);
    ADD_TEST(test_fossil_crabdb_execute_query_create_namespace);
    ADD_TEST(test_fossil_crabdb_execute_query_delete_namespace);
    ADD_TEST(test_fossil_crabdb_execute_query_set_key_value);
    ADD_TEST(test_fossil_crabdb_execute_query_non_existent_namespace);
    ADD_TEST(test_fossil_crabdb_execute_query_invalid_command);
}

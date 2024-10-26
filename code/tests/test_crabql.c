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
#include <fossil/unittest/assume.h>    // Includes the Fossil Assume Framework

#include "fossil/crabdb/framework.h"

FOSSIL_FIXTURE(query_fixture);
fossil_crabdb_t *query_mock_db;

FOSSIL_SETUP(query_fixture) {
    query_mock_db = fossil_crabdb_create();
}

FOSSIL_TEARDOWN(query_fixture) {
    fossil_crabdb_destroy(query_mock_db);
}

// * * * * * * * * * * * * * * * * * * * * * * * *
// * Fossil Logic Test Blue CrabDB Database
// * * * * * * * * * * * * * * * * * * * * * * * *

// Test successful SELECT
FOSSIL_TEST(test_fossil_crabql_query_select) {
    const char *query = "SELECT * FROM users;";
    ASSUME_ITS_FALSE(fossil_crabql_query(query_mock_db, query));
}

// Test successful INSERT
FOSSIL_TEST(test_fossil_crabql_query_insert) {
    const char *query = "INSERT INTO users VALUES ('Alice', 30);";
    ASSUME_ITS_TRUE(fossil_crabql_query(query_mock_db, query));
}

// Test successful UPDATE
FOSSIL_TEST(test_fossil_crabql_query_update) {
    const char *query = "UPDATE users SET age = 31 WHERE name = 'Alice';";
    ASSUME_ITS_FALSE(fossil_crabql_query(query_mock_db, query));
}

// Test successful DELETE
FOSSIL_TEST(test_fossil_crabql_query_delete) {
    const char *query = "DELETE FROM users WHERE name = 'Alice';";
    ASSUME_ITS_FALSE(fossil_crabql_query(query_mock_db, query));
}

// Test invalid query
FOSSIL_TEST(test_fossil_crabql_query_invalid) {
    const char *query = "SELECT FROM users;";
    ASSUME_ITS_FALSE(fossil_crabql_query(query_mock_db, query));
}

// Test successful LIST command
FOSSIL_TEST(test_fossil_crabql_query_list) {
    const char *query = "LIST;";
    ASSUME_ITS_TRUE(fossil_crabql_query(query_mock_db, query)); // Adjust according to your implementation
}

// Test successful CLEAR command
FOSSIL_TEST(test_fossil_crabql_query_clear) {
    const char *query = "CLEAR;";
    ASSUME_ITS_TRUE(fossil_crabql_query(query_mock_db, query));
}

// Test successful BACKUP command
FOSSIL_TEST(test_fossil_crabql_query_backup) {
    const char *query = "BACKUP TO 'backup.db';";
    ASSUME_ITS_TRUE(fossil_crabql_query(query_mock_db, query)); // Ensure backup operation works
}

// Test successful RESTORE command
FOSSIL_TEST(test_fossil_crabql_query_restore) {
    const char *query = "RESTORE FROM 'backup.db';";
    ASSUME_ITS_TRUE(fossil_crabql_query(query_mock_db, query)); // Ensure restore operation works
}

// Test successful BEGIN TRANSACTION
FOSSIL_TEST(test_fossil_crabql_query_begin_transaction) {
    const char *query = "BEGIN TRANSACTION;";
    ASSUME_ITS_TRUE(fossil_crabql_query(query_mock_db, query));
}

// Test successful COMMIT
FOSSIL_TEST(test_fossil_crabql_query_commit_transaction) {
    const char *query = "COMMIT;";
    ASSUME_ITS_TRUE(fossil_crabql_query(query_mock_db, query));
}

// Test successful ROLLBACK
FOSSIL_TEST(test_fossil_crabql_query_rollback_transaction) {
    const char *query = "ROLLBACK;";
    ASSUME_ITS_TRUE(fossil_crabql_query(query_mock_db, query));
}

// * * * * * * * * * * * * * * * * * * * * * * * *
// * Fossil Logic Test Pool
// * * * * * * * * * * * * * * * * * * * * * * * *

FOSSIL_TEST_GROUP(c_crab_query_tests) {
    ADD_TESTF(test_fossil_crabql_query_insert, query_fixture);
    ADD_TESTF(test_fossil_crabql_query_update, query_fixture);
    ADD_TESTF(test_fossil_crabql_query_select, query_fixture);
    ADD_TESTF(test_fossil_crabql_query_delete, query_fixture);
    ADD_TESTF(test_fossil_crabql_query_invalid, query_fixture);
    ADD_TESTF(test_fossil_crabql_query_list, query_fixture);
    ADD_TESTF(test_fossil_crabql_query_clear, query_fixture);
    ADD_TESTF(test_fossil_crabql_query_backup, query_fixture);
    ADD_TESTF(test_fossil_crabql_query_restore, query_fixture);
    ADD_TESTF(test_fossil_crabql_query_begin_transaction, query_fixture);
    ADD_TESTF(test_fossil_crabql_query_commit_transaction, query_fixture);
    ADD_TESTF(test_fossil_crabql_query_rollback_transaction, query_fixture);
} // end of tests

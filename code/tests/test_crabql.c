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
#include <fossil/unittest/assume.h>            // Includes the Fossil Assume Framework

#include "fossil/crabdb/framework.h"

// * * * * * * * * * * * * * * * * * * * * * * * *
// * Fossil Logic Test Blue CrabDB Database
// * * * * * * * * * * * * * * * * * * * * * * * *

// Test cases for AST manipulation
FOSSIL_TEST(test_fossil_crabql_ast_create_node) {
    crabql_ast_node_t* node = fossil_crabql_ast_create_node(AST_NODE_TYPE_EXAMPLE, "test_value");
    ASSUME_NOT_CNULL(node);
    ASSUME_ITS_EQUAL_CSTR("test_value", node->value);
    ASSUME_ITS_EQUAL_I32(AST_NODE_TYPE_EXAMPLE, node->type);
    fossil_crabql_ast_free_node(node);  // Clean up
}

FOSSIL_TEST(test_fossil_crabql_ast_add_child) {
    crabql_ast_node_t* parent = fossil_crabql_ast_create_node(AST_NODE_TYPE_EXAMPLE, "parent");
    crabql_ast_node_t* child = fossil_crabql_ast_create_node(AST_NODE_TYPE_CHILD, "child");

    ASSUME_ITS_TRUE(fossil_crabql_ast_add_child(parent, child));
    ASSUME_ITS_EQUAL_I32(1, parent->child_count);  // Assuming child_count is a property in the node struct

    fossil_crabql_ast_free_node(parent);  // Clean up
}

FOSSIL_TEST(test_fossil_crabql_ast_free_node) {
    crabql_ast_node_t* node = fossil_crabql_ast_create_node(AST_NODE_TYPE_EXAMPLE, "test_value");
    ASSUME_NOT_CNULL(node);
    fossil_crabql_ast_free_node(node);  // Should not crash
}

// Test cases for database operations
FOSSIL_TEST(test_fossil_crabql_insert) {
    fossil_crabdb_t* db = fossil_crabql_create_database();
    bool result = fossil_crabql_insert(db, "INSERT INTO test_table (column1) VALUES ('value1')");
    ASSUME_ITS_TRUE(result);
    fossil_crabql_free_database(db);  // Clean up
}

FOSSIL_TEST(test_fossil_crabql_update) {
    fossil_crabdb_t* db = fossil_crabql_create_database();
    fossil_crabql_insert(db, "INSERT INTO test_table (column1) VALUES ('value1')");
    
    bool result = fossil_crabql_update(db, "UPDATE test_table SET column1='value2' WHERE column1='value1'");
    ASSUME_ITS_TRUE(result);
    
    // Additional validation could be done here to ensure the value was updated
    fossil_crabql_free_database(db);  // Clean up
}

FOSSIL_TEST(test_fossil_crabql_select) {
    fossil_crabdb_t* db = fossil_crabql_create_database();
    fossil_crabql_insert(db, "INSERT INTO test_table (column1) VALUES ('value1')");
    
    char result[256];
    bool success = fossil_crabql_select(db, "SELECT * FROM test_table", result, sizeof(result));
    ASSUME_ITS_TRUE(success);
    ASSUME_ITS_EQUAL_CSTR("value1", result);  // Assuming result format is direct
    
    fossil_crabql_free_database(db);  // Clean up
}

FOSSIL_TEST(test_fossil_crabql_delete) {
    fossil_crabdb_t* db = fossil_crabql_create_database();
    fossil_crabql_insert(db, "INSERT INTO test_table (column1) VALUES ('value1')");
    
    bool result = fossil_crabql_delete(db, "DELETE FROM test_table WHERE column1='value1'");
    ASSUME_ITS_TRUE(result);
    
    // Additional validation could be done here to ensure the value was deleted
    fossil_crabql_free_database(db);  // Clean up
}

// * * * * * * * * * * * * * * * * * * * * * * * *
// * Fossil Logic Test Pool
// * * * * * * * * * * * * * * * * * * * * * * * *

FOSSIL_TEST_GROUP(c_crabdb_operations_tests) {    
    ADD_TEST(test_fossil_crabql_ast_create_node);
    ADD_TEST(test_fossil_crabql_ast_add_child);
    ADD_TEST(test_fossil_crabql_ast_free_node);
    ADD_TEST(test_fossil_crabql_insert);
    ADD_TEST(test_fossil_crabql_update);
    ADD_TEST(test_fossil_crabql_select);
    ADD_TEST(test_fossil_crabql_delete);
} // end of tests

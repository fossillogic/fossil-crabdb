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
#include "fossil/crabdb/crabql.h"

crabql_ast_node_t* fossil_crabql_ast_create_node(crabql_ast_node_type_t type, const char* value) {
    crabql_ast_node_t* node = (crabql_ast_node_t*)malloc(sizeof(crabql_ast_node_t));
    if (!node) return NULL;

    node->type = type;
    node->value = value ? strdup(value) : NULL;
    node->children = NULL;
    node->child_count = 0;
    return node;
}

bool fossil_crabql_ast_add_child(crabql_ast_node_t* parent, crabql_ast_node_t* child) {
    if (!parent || !child) return false;

    crabql_ast_node_t** children = (crabql_ast_node_t**)realloc(parent->children, sizeof(crabql_ast_node_t*) * (parent->child_count + 1));
    if (!children) return false;

    parent->children = children;
    parent->children[parent->child_count++] = child;
    return true;
}

void fossil_crabql_ast_free_node(crabql_ast_node_t* node) {
    if (!node) return;

    for (size_t i = 0; i < node->child_count; i++) {
        fossil_crabql_ast_free_node(node->children[i]);
    }

    free(node->children);
    free(node);
}

crabql_ast_t* fossil_crabql_ast_create(void) {
    crabql_ast_t* ast = (crabql_ast_t*)malloc(sizeof(crabql_ast_t));
    if (!ast) return NULL;

    ast->root = NULL;
    return ast;
}

void fossil_crabql_ast_free(crabql_ast_t* ast) {
    if (!ast) return;

    fossil_crabql_ast_free_node(ast->root);
    free(ast);
}

bool fossil_crabql_insert(fossil_crabdb_t* db, const char* query) {
    if (!db || !query) return false;

    char key[FOSSIL_CRABDB_KEY_SIZE];
    char value[FOSSIL_CRABDB_VAL_SIZE];
    fossil_crabdb_type_t type;

    if (sscanf(query, "INSERT INTO %s VALUES %s", key, value) != 2) {
        return false;
    }

    if (strstr(value, "'") != NULL) {
        type = FOSSIL_CRABDB_TYPE_STRING;
    } else if (strstr(value, ".") != NULL) {
        type = FOSSIL_CRABDB_TYPE_FLOAT;
    } else {
        type = FOSSIL_CRABDB_TYPE_INT32;
    }

    return fossil_crabdb_insert(db, key, value, type);
}

bool fossil_crabql_update(fossil_crabdb_t* db, const char* query) {
    if (!db || !query) return false;

    char key[FOSSIL_CRABDB_KEY_SIZE];
    char value[FOSSIL_CRABDB_VAL_SIZE];

    if (sscanf(query, "UPDATE %s SET %s", key, value) != 2) {
        return false;
    }

    return fossil_crabdb_update(db, key, value);
}

bool fossil_crabql_select(fossil_crabdb_t* db, const char* query, char* result, size_t result_size) {
    if (!db || !query || !result) return false;

    char key[FOSSIL_CRABDB_KEY_SIZE];

    if (sscanf(query, "SELECT %s", key) != 1) {
        return false;
    }

    return fossil_crabdb_select(db, key, result, result_size);
}

bool fossil_crabql_delete(fossil_crabdb_t* db, const char* query) {
    if (!db || !query) return false;

    char key[FOSSIL_CRABDB_KEY_SIZE];

    if (sscanf(query, "DELETE FROM %s", key) != 1) {
        return false;
    }

    return fossil_crabdb_delete(db, key);
}

bool fossil_crabql_parse_query(const char* query, crabql_ast_t* ast) {
    // Initialize the AST structure
    memset(ast, 0, sizeof(crabql_ast_t));

    // Tokenize the query string and begin parsing
    if (!fossil_crabql_tokenize(query, ast)) {
        return false;  // Failed to tokenize the query
    }

    // Build the AST from tokens
    if (!fossil_crabql_build_ast(ast)) {
        return false;  // Failed to build AST from tokens
    }

    return true;
}

bool fossil_crabql_validate_query(crabql_ast_t* ast) {
    // Validate the AST structure and logical correctness
    if (ast == NULL) {
        return false;
    }

    // Ensure that the AST conforms to CrabQL syntax rules
    if (!fossil_crabql_check_syntax(ast)) {
        return false;  // Syntax error in query
    }

    // Perform semantic validation
    return fossil_crabql_check_semantics(ast);
}


bool fossil_crabql_filter(fossil_crabdb_t* db, const char* condition, char* result_buffer, size_t buffer_size) {
    // Parse the WHERE condition
    crabql_ast_t ast;
    if (!fossil_crabql_parse_query(condition, &ast)) {
        return false;
    }

    // Validate the parsed query
    if (!fossil_crabql_validate_query(&ast)) {
        return false;
    }

    // Execute the query against the database and filter results
    return fossil_crabql_execute_filter(db, &ast, result_buffer, buffer_size);
}

bool fossil_crabql_inner_join(fossil_crabdb_t* db, const char* table1, const char* table2, const char* on_condition, char* result_buffer, size_t buffer_size) {
    // Parse the ON condition
    crabql_ast_t ast;
    if (!fossil_crabql_parse_query(on_condition, &ast)) {
        return false;
    }

    // Perform the INNER JOIN on the two tables
    return fossil_crabql_execute_inner_join(db, table1, table2, &ast, result_buffer, buffer_size);
}

bool fossil_crabql_left_join(fossil_crabdb_t* db, const char* table1, const char* table2, const char* on_condition, char* result_buffer, size_t buffer_size) {
    // Parse the ON condition
    crabql_ast_t ast;
    if (!fossil_crabql_parse_query(on_condition, &ast)) {
        return false;
    }

    // Perform the LEFT JOIN on the two tables
    return fossil_crabql_execute_left_join(db, table1, table2, &ast, result_buffer, buffer_size);
}

bool fossil_crabql_execute_subquery(fossil_crabdb_t* db, const char* subquery, char* result_buffer, size_t buffer_size) {
    // Parse the subquery
    crabql_ast_t ast;
    if (!fossil_crabql_parse_query(subquery, &ast)) {
        return false;
    }

    // Execute the parsed subquery
    return fossil_crabql_execute_query(db, &ast, result_buffer, buffer_size);
}

bool fossil_crabql_create_index(fossil_crabdb_t* db, const char* table, const char* column) {
    // Create an index on the specified column
    return fossil_crabql_index_create(db, table, column);
}

bool fossil_crabql_drop_index(fossil_crabdb_t* db, const char* table, const char* column) {
    // Drop the index on the specified column
    return fossil_crabql_index_drop(db, table, column);
}

bool fossil_crabql_query_with_limit(fossil_crabdb_t* db, const char* query, unsigned int limit, unsigned int offset, char* result_buffer, size_t buffer_size) {
    // Parse the query
    crabql_ast_t ast;
    if (!fossil_crabql_parse_query(query, &ast)) {
        return false;
    }

    // Execute the query with limit and offset
    return fossil_crabql_execute_query_with_limit(db, &ast, limit, offset, result_buffer, buffer_size);
}

bool fossil_crabql_begin_transaction(fossil_crabdb_t* db) {
    return fossil_crabdb_begin_transaction(db);
}

bool fossil_crabql_commit_transaction(fossil_crabdb_t* db) {
    return fossil_crabdb_commit_transaction(db);
}

bool fossil_crabql_rollback_transaction(fossil_crabdb_t* db) {
    return fossil_crabdb_rollback_transaction(db);
}

bool fossil_crabql_list_tables(fossil_crabdb_t* db, char* table_buffer, size_t buffer_size) {
    return fossil_crabdb_get_tables(db, table_buffer, buffer_size);
}

bool fossil_crabql_list_columns(fossil_crabdb_t* db, const char* table, char* column_buffer, size_t buffer_size) {
    return fossil_crabdb_get_columns(db, table, column_buffer, buffer_size);
}

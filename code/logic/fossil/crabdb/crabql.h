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
#ifndef FOSSIL_CRABDB_QUERY_H
#define FOSSIL_CRABDB_QUERY_H

#include "crabdb.h"

#ifdef __cplusplus
extern "C" {
#endif

// Enumerate different types of AST nodes
typedef enum {
    CRABQL_AST_SELECT,
    CRABQL_AST_INSERT,
    CRABQL_AST_UPDATE,
    CRABQL_AST_DELETE,
    CRABQL_AST_WHERE,
    CRABQL_AST_CONDITION,
    CRABQL_AST_COLUMN,
    CRABQL_AST_VALUE,
    CRABQL_AST_JOIN,
    CRABQL_AST_LIMIT,
    CRABQL_AST_OFFSET,
    CRABQL_AST_ORDER_BY,
    CRABQL_AST_GROUP_BY,
    CRABQL_AST_HAVING,
    CRABQL_AST_SUBQUERY
} crabql_ast_node_type_t;

// Represents a single AST node
typedef struct crabql_ast_node {
    crabql_ast_node_type_t type; // Type of node (e.g., SELECT, INSERT, etc.)
    char* value;                 // The value (if any, like column name, table name, etc.)
    struct crabql_ast_node** children; // Child nodes (e.g., conditions or other clauses)
    size_t child_count;          // Number of child nodes
} crabql_ast_node_t;

// AST root structure
typedef struct {
    crabql_ast_node_t* root;     // The root node of the AST
} crabql_ast_t;

// Function declarations for AST manipulation

/**
 * @brief Allocates and initializes a new AST node.
 * @param type The type of the AST node.
 * @param value The value of the node, or NULL.
 * @return A pointer to the new AST node.
 */
crabql_ast_node_t* fossil_crabql_ast_create_node(crabql_ast_node_type_t type, const char* value);

/**
 * @brief Adds a child node to an AST node.
 * @param parent The parent node.
 * @param child The child node to add.
 * @return true if the child was added successfully, false otherwise.
 */
bool fossil_crabql_ast_add_child(crabql_ast_node_t* parent, crabql_ast_node_t* child);

/**
 * @brief Frees all memory associated with an AST node and its children.
 * @param node The AST node to free.
 */
void fossil_crabql_ast_free_node(crabql_ast_node_t* node);

/**
 * @brief Creates a new AST.
 * @return A pointer to the new AST.
 */
crabql_ast_t* fossil_crabql_ast_create(void);

/**
 * @brief Frees all memory associated with the AST.
 * @param ast The AST to free.
 */
void fossil_crabql_ast_free(crabql_ast_t* ast);

/* Query Operations */
/**
 * @brief Inserts a new record into the database.
 *
 * @param db Pointer to the database structure.
 * @param query SQL query string for the insert operation.
 * @return true if the insert operation is successful, false otherwise.
 */
bool fossil_crabql_insert(fossil_crabdb_t* db, const char* query);

/**
 * @brief Updates an existing record in the database.
 *
 * @param db Pointer to the database structure.
 * @param query SQL query string for the update operation.
 * @return true if the update operation is successful, false otherwise.
 */
bool fossil_crabql_update(fossil_crabdb_t* db, const char* query);

/**
 * @brief Selects records from the database.
 *
 * @param db Pointer to the database structure.
 * @param query SQL query string for the select operation.
 * @param result Buffer to store the result of the select operation.
 * @param result_size Size of the result buffer.
 * @return true if the select operation is successful, false otherwise.
 */
bool fossil_crabql_select(fossil_crabdb_t* db, const char* query, char* result, size_t result_size);

/**
 * @brief Deletes a record from the database.
 *
 * @param db Pointer to the database structure.
 * @param query SQL query string for the delete operation.
 * @return true if the delete operation is successful, false otherwise.
 */
bool fossil_crabql_delete(fossil_crabdb_t* db, const char* query);

/**
 * @brief Parses a CrabQL query into an AST.
 * @param query The query string to parse.
 * @param ast Output parameter, the parsed AST.
 * @return true if the query was parsed successfully, false otherwise.
 */
bool fossil_crabql_parse_query(const char* query, crabql_ast_t* ast);

/**
 * @brief Validates a parsed CrabQL query.
 * @param ast The AST of the query to validate.
 * @return true if the query is valid, false otherwise.
 */
bool fossil_crabql_validate_query(crabql_ast_t* ast);

/**
 * @brief Filters rows in the db based on a CrabQL WHERE condition.
 * @param db A pointer to the db to query.
 * @param condition The WHERE condition to apply.
 * @param result_buffer The buffer to store the filtered results.
 * @param buffer_size The size of the result buffer.
 * @return true if the query was successful, false otherwise.
 */
bool fossil_crabql_filter(fossil_crabdb_t* db, const char* condition, char* result_buffer, size_t buffer_size);

/**
 * @brief Performs an INNER JOIN between two tables.
 * @param db A pointer to the db.
 * @param table1 The name of the first table.
 * @param table2 The name of the second table.
 * @param on_condition The condition to join the two tables on.
 * @param result_buffer The buffer to store the join results.
 * @param buffer_size The size of the result buffer.
 * @return true if the join was successful, false otherwise.
 */
bool fossil_crabql_inner_join(fossil_crabdb_t* db, const char* table1, const char* table2, const char* on_condition, char* result_buffer, size_t buffer_size);

/**
 * @brief Performs a LEFT JOIN between two tables.
 * @param db A pointer to the db.
 * @param table1 The name of the first table.
 * @param table2 The name of the second table.
 * @param on_condition The condition to join the two tables on.
 * @param result_buffer The buffer to store the join results.
 * @param buffer_size The size of the result buffer.
 * @return true if the join was successful, false otherwise.
 */
bool fossil_crabql_left_join(fossil_crabdb_t* db, const char* table1, const char* table2, const char* on_condition, char* result_buffer, size_t buffer_size);

/**
 * @brief Executes a subquery within a query.
 * @param db A pointer to the db.
 * @param subquery The subquery string.
 * @param result_buffer The buffer to store the subquery result.
 * @param buffer_size The size of the result buffer.
 * @return true if the subquery was executed successfully, false otherwise.
 */
bool fossil_crabql_execute_subquery(fossil_crabdb_t* db, const char* subquery, char* result_buffer, size_t buffer_size);

/**
 * @brief Creates an index on a column in a table.
 * @param db A pointer to the db.
 * @param table The name of the table.
 * @param column The column to index.
 * @return true if the index was created successfully, false otherwise.
 */
bool fossil_crabql_create_index(fossil_crabdb_t* db, const char* table, const char* column);

/**
 * @brief Drops an index on a column in a table.
 * @param db A pointer to the db.
 * @param table The name of the table.
 * @param column The column to drop the index from.
 * @return true if the index was dropped successfully, false otherwise.
 */
bool fossil_crabql_drop_index(fossil_crabdb_t* db, const char* table, const char* column);

/**
 * @brief Limits the number of results returned by a query.
 * @param db A pointer to the db.
 * @param query The query string.
 * @param limit The maximum number of results to return.
 * @param offset The number of rows to skip.
 * @param result_buffer The buffer to store the results.
 * @param buffer_size The size of the result buffer.
 * @return true if the query was successful, false otherwise.
 */
bool fossil_crabql_query_with_limit(fossil_crabdb_t* db, const char* query, unsigned int limit, unsigned int offset, char* result_buffer, size_t buffer_size);

/**
 * @brief Begins a transaction in CrabQL.
 * @param db A pointer to the db.
 * @return true if the transaction began successfully, false otherwise.
 */
bool fossil_crabql_begin_transaction(fossil_crabdb_t* db);

/**
 * @brief Commits the current transaction in CrabQL.
 * @param db A pointer to the db.
 * @return true if the transaction was committed successfully, false otherwise.
 */
bool fossil_crabql_commit_transaction(fossil_crabdb_t* db);

/**
 * @brief Rolls back the current transaction in CrabQL.
 * @param db A pointer to the db.
 * @return true if the transaction was rolled back successfully, false otherwise.
 */
bool fossil_crabql_rollback_transaction(fossil_crabdb_t* db);

/**
 * @brief Retrieves the list of tables in the db.
 * @param db A pointer to the db.
 * @param table_buffer The buffer to store the list of tables.
 * @param buffer_size The size of the table buffer.
 * @return true if the query was successful, false otherwise.
 */
bool fossil_crabql_list_tables(fossil_crabdb_t* db, char* table_buffer, size_t buffer_size);

/**
 * @brief Retrieves the list of columns in a table.
 * @param db A pointer to the db.
 * @param table The name of the table.
 * @param column_buffer The buffer to store the list of columns.
 * @param buffer_size The size of the column buffer.
 * @return true if the query was successful, false otherwise.
 */
bool fossil_crabql_list_columns(fossil_crabdb_t* db, const char* table, char* column_buffer, size_t buffer_size);

#ifdef __cplusplus
}
#endif

#endif /* FOSSIL_CRABDB_FRAMEWORK_H */

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
#ifndef FOSSIL_CRABDB_DB_H
#define FOSSIL_CRABDB_DB_H

#include <stdbool.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <stdio.h>
#include <time.h>

#ifdef __cplusplus
extern "C" {
#endif

// *****************************************************************************
// Enumerations for data types and attributes
// *****************************************************************************

/**
 * @brief Enumerates the possible data types for values stored in the database.
 */
typedef enum {
    FOSSIL_CRABDB_TYPE_I8,        // 8 Bit Integer values
    FOSSIL_CRABDB_TYPE_I16,       // 16 Bit Integer values
    FOSSIL_CRABDB_TYPE_I32,       // 32 Bit Integer values
    FOSSIL_CRABDB_TYPE_I64,       // 64 Bit Integer values
    FOSSIL_CRABDB_TYPE_U8,        // Unsigned 8 Bit Integer values
    FOSSIL_CRABDB_TYPE_U16,       // Unsigned 16 Bit Integer values
    FOSSIL_CRABDB_TYPE_U32,       // Unsigned 32 Bit Integer values
    FOSSIL_CRABDB_TYPE_U64,       // Unsigned 64 Bit Integer values
    FOSSIL_CRABDB_TYPE_HEX,       // Hexadecimal values
    FOSSIL_CRABDB_TYPE_OCT,       // Octal values
    FOSSIL_CRABDB_TYPE_BIN,       // Binary values
    FOSSIL_CRABDB_TYPE_F32,       // Floating-point values
    FOSSIL_CRABDB_TYPE_F64,       // Double-precision floating-point values
    FOSSIL_CRABDB_TYPE_CSTRING,   // Null-terminated string
    FOSSIL_CRABDB_TYPE_WSTRING,   // Wide-character string
    FOSSIL_CRABDB_TYPE_CCHAR,     // Character values
    FOSSIL_CRABDB_TYPE_WCHAR,     // Wide-character values
    FOSSIL_CRABDB_TYPE_BOOL,      // Boolean values
    FOSSIL_CRABDB_TYPE_DATE,      // Date values
    FOSSIL_CRABDB_TYPE_SIZE,      // Size values
    FOSSIL_CRABDB_TYPE_ANY,       // Any type represented as a void * pointer
    FOSSIL_CRABDB_TYPE_NULL       // Null value
} fossil_crabdb_type_t;

/**
 * @brief Represents additional attributes for each database entry.
 */
typedef struct {
    bool is_primary_key;          // Indicates if this entry is a primary key
    bool is_unique;               // Indicates if this entry is unique
    bool is_nullable;             // Indicates if this entry allows null values
} fossil_crabdb_attributes_t;

// *****************************************************************************
// Database structures
// *****************************************************************************

/**
 * The following types help illustrate a database structure that takes the form
 * of a book, which is a common and easily understandable metaphor. Each entry
 * in the database is like a page in the book, and the entire database is like
 * a collection of these pages.
 * 
 * - fossil_crabdb_entry_t: Represents an entry in the database, holding a key-value pair.
 * - fossil_crabdb_page_t: Represents a node in the doubly linked list, containing an entry and pointers to the next and previous nodes.
 * - fossil_crabdb_book_t: Represents the entire database, maintaining pointers to the head and tail of the list and the number of elements.
 */

/**
 * @brief Represents an entry in the fossil_crabdb database.
 * 
 * Each entry holds a key-value pair, a type, and additional attributes.
 */
typedef struct {
    char *key;                    // Null-terminated string for the key
    char *value;                  // Null-terminated string for the value
    fossil_crabdb_type_t type;    // Type of the value
    fossil_crabdb_attributes_t attributes; // Entry attributes
} fossil_crabdb_entry_t;

/**
 * @brief Represents a node in the doubly linked list used by fossil_crabdb.
 * 
 * Each node contains an entry and pointers to the next and previous nodes.
 */
typedef struct fossil_crabdb_node {
    fossil_crabdb_entry_t entry;     // Data stored in the node
    struct fossil_crabdb_node *next; // Pointer to the next node
    struct fossil_crabdb_node *prev; // Pointer to the previous node
} fossil_crabdb_page_t;

/**
 * @brief Represents a doubly linked list-based relational database.
 * 
 * Maintains pointers to the head and tail of the list and tracks the number of entries.
 */
typedef struct {
    fossil_crabdb_page_t *head;      // Pointer to the first node
    fossil_crabdb_page_t *tail;      // Pointer to the last node
    size_t size;                     // Number of entries in the database
} fossil_crabdb_book_t;

/**
 * @brief Represents a transaction state in the fossil_crabdb database.
 */
typedef struct fossil_crabdb_transaction {
    char *name;                         // Transaction name
    fossil_crabdb_book_t snapshot;      // Snapshot of the database state
    struct fossil_crabdb_transaction *next;
} fossil_crabdb_transaction_t;

// *****************************************************************************
// Relational database operations
// *****************************************************************************

/**
 * Create a table-like structure in the database with attributes.
 * 
 * @param table_name The name of the table to create.
 * @param attributes The attributes that define the table schema.
 * @param attr_count The number of attributes in the schema.
 */
void fossil_crabdb_create_table(const char *table_name, fossil_crabdb_attributes_t *attributes, size_t attr_count);

/**
 * Insert a new row into a table.
 * 
 * @param table_name The name of the table to insert into.
 * @param entries An array of entries to insert.
 * @param entry_count The number of entries to insert.
 */
void fossil_crabdb_insert_row(const char *table_name, const fossil_crabdb_entry_t *entries, size_t entry_count);

/**
 * Query rows from a table using conditions.
 * 
 * @param table_name The name of the table to query from.
 * @param condition A callback function to filter rows.
 * @param result_count A pointer to store the number of matching rows.
 * 
 * @return An array of matching rows.
 */
fossil_crabdb_entry_t *fossil_crabdb_query(const char *table_name, bool (*condition)(const fossil_crabdb_entry_t *), size_t *result_count);

/**
 * Search for rows by key or value.
 * 
 * @param table_name The name of the table to search in.
 * @param search_term The term to search for.
 * @param search_in_keys Indicates whether to search in keys (true) or values (false).
 * @param result_count A pointer to store the number of matching rows.
 * 
 * @return An array of matching rows.
 */
fossil_crabdb_entry_t *fossil_crabdb_search(const char *table_name, const char *search_term, bool search_in_keys, size_t *result_count);

/**
 * Update rows in a table.
 * 
 * @param table_name The name of the table to update.
 * @param condition A callback function to filter rows to update.
 * @param new_values An array of updated values.
 * @param value_count The number of updated values.
 */
void fossil_crabdb_update_rows(const char *table_name, bool (*condition)(const fossil_crabdb_entry_t *), const fossil_crabdb_entry_t *new_values, size_t value_count);

/**
 * Delete rows from a table.
 * 
 * @param table_name The name of the table to delete from.
 * @param condition A callback function to filter rows to delete.
 */
void fossil_crabdb_delete_rows(const char *table_name, bool (*condition)(const fossil_crabdb_entry_t *));

/**
 * Begin a transaction.
 * 
 * @param transaction_name The name of the transaction.
 */
void fossil_crabdb_begin_transaction(const char *transaction_name);

/**
 * Commit a transaction, making all changes permanent.
 * 
 * @param transaction_name The name of the transaction to commit.
 */
void fossil_crabdb_commit_transaction(const char *transaction_name);

/**
 * Rollback a transaction, reverting all changes made during the transaction.
 * 
 * @param transaction_name The name of the transaction to rollback.
 */
void fossil_crabdb_rollback_transaction(const char *transaction_name);

/**
 * Backup the database to a file.
 * 
 * @param file_path The path to the backup file.
 */
void fossil_crabdb_backup(const char *file_path);

/**
 * Restore the database from a backup file.
 * 
 * @param file_path The path to the backup file.
 */
void fossil_crabdb_restore(const char *file_path);

// *****************************************************************************
// Utility functions for relational operations
// *****************************************************************************

/**
 * Join two tables on a specified condition.
 * 
 * @param table1 The name of the first table.
 * @param table2 The name of the second table.
 * @param join_condition A callback function defining the join condition.
 * @param result_count A pointer to store the number of resulting rows.
 * 
 * @return An array of joined rows.
 */
fossil_crabdb_entry_t *fossil_crabdb_join(const char *table1, const char *table2, bool (*join_condition)(const fossil_crabdb_entry_t *, const fossil_crabdb_entry_t *), size_t *result_count);

/**
 * Aggregate data from a table.
 * 
 * @param table_name The name of the table to aggregate from.
 * @param aggregate_function A callback function to perform the aggregation.
 * 
 * @return The result of the aggregation.
 */
void *fossil_crabdb_aggregate(const char *table_name, void *(*aggregate_function)(const fossil_crabdb_entry_t *, size_t));

/**
 * Paginate query results.
 * 
 * @param entries The array of query results.
 * @param total_entries The total number of query results.
 * @param page_size The number of results per page.
 * @param page_number The page number to fetch.
 * 
 * @return A pointer to the entries for the requested page.
 */
fossil_crabdb_entry_t *fossil_crabdb_paginate(const fossil_crabdb_entry_t *entries, size_t total_entries, size_t page_size, size_t page_number);

/**
 * Rebuild indices for optimizing search operations.
 * 
 * @param table_name The name of the table to rebuild indices for.
 */
void fossil_crabdb_rebuild_indices(const char *table_name);

#ifdef __cplusplus
}
#include <string>
#include <vector>

namespace fossil {

/**
 * @brief Represents a high-level interface to the Fossil CrabDB database.
 */
class CrabDB {
public:
    /**
     * @brief Construct a new CrabDB object
     */
    CrabDB() {
        // Initialization code if needed
    }
    
    /**
     * @brief Destroy the CrabDB object
     */
    ~CrabDB() {
        // Cleanup code if needed
    }

    /**
     * @brief Create a table-like structure in the database with attributes.
     * 
     * @param table_name The name of the table to create.
     * @param attributes The attributes that define the table schema.
     * @param attr_count The number of attributes in the schema.
     */
    void createTable(const std::string &table_name, fossil_crabdb_attributes_t *attributes, size_t attr_count) {
        fossil_crabdb_create_table(table_name.c_str(), attributes, attr_count);
    }

    /**
     * @brief Insert a new row into a table.
     * 
     * @param table_name The name of the table to insert into.
     * @param entries An array of entries to insert.
     * @param entry_count The number of entries to insert.
     */
    void insertRow(const std::string &table_name, const fossil_crabdb_entry_t *entries, size_t entry_count) {
        fossil_crabdb_insert_row(table_name.c_str(), entries, entry_count);
    }

    /**
     * @brief Query rows from a table using conditions.
     * 
     * @param table_name The name of the table to query from.
     * @param condition A callback function to filter rows.
     * 
     * @return An array of matching rows.
     */
    std::vector<fossil_crabdb_entry_t> query(const std::string &table_name, bool (*condition)(const fossil_crabdb_entry_t *)) {
        size_t result_count;
        fossil_crabdb_entry_t *results = fossil_crabdb_query(table_name.c_str(), condition, &result_count);
        return std::vector<fossil_crabdb_entry_t>(results, results + result_count);
    }

    /**
     * @brief Search for rows by key or value.
     * 
     * @param table_name The name of the table to search in.
     * @param search_term The term to search for.
     * @param search_in_keys Indicates whether to search in keys (true) or values (false).
     * 
     * @return An array of matching rows.
     */
    std::vector<fossil_crabdb_entry_t> search(const std::string &table_name, const std::string &search_term, bool search_in_keys) {
        size_t result_count;
        fossil_crabdb_entry_t *results = fossil_crabdb_search(table_name.c_str(), search_term.c_str(), search_in_keys, &result_count);
        return std::vector<fossil_crabdb_entry_t>(results, results + result_count);
    }

    /**
     * @brief Update rows in a table.
     * 
     * @param table_name The name of the table to update.
     * @param condition A callback function to filter rows to update.
     * @param new_values An array of updated values.
     * @param value_count The number of updated values.
     */
    void updateRows(const std::string &table_name, bool (*condition)(const fossil_crabdb_entry_t *), const fossil_crabdb_entry_t *new_values, size_t value_count) {
        fossil_crabdb_update_rows(table_name.c_str(), condition, new_values, value_count);
    }

    /**
     * @brief Delete rows from a table.
     * 
     * @param table_name The name of the table to delete from.
     * @param condition A callback function to filter rows to delete.
     */
    void deleteRows(const std::string &table_name, bool (*condition)(const fossil_crabdb_entry_t *)) {
        fossil_crabdb_delete_rows(table_name.c_str(), condition);
    }

    /**
     * @brief Begin a transaction.
     * 
     * @param transaction_name The name of the transaction.
     */
    void beginTransaction(const std::string &transaction_name) {
        fossil_crabdb_begin_transaction(transaction_name.c_str());
    }

    /**
     * @brief Commit a transaction, making all changes permanent.
     * 
     * @param transaction_name The name of the transaction to commit.
     */
    void commitTransaction(const std::string &transaction_name) {
        fossil_crabdb_commit_transaction(transaction_name.c_str());
    }

    /**
     * @brief Rollback a transaction, reverting all changes made during the transaction.
     * 
     * @param transaction_name The name of the transaction to rollback.
     */
    void rollbackTransaction(const std::string &transaction_name) {
        fossil_crabdb_rollback_transaction(transaction_name.c_str());
    }

    /**
     * @brief Backup the database to a file.
     * 
     * @param file_path The path to the backup file.
     */
    void backup(const std::string &file_path) {
        fossil_crabdb_backup(file_path.c_str());
    }

    /**
     * @brief Restore the database from a backup file.
     * 
     * @param file_path The path to the backup file.
     */
    void restore(const std::string &file_path) {
        fossil_crabdb_restore(file_path.c_str());
    }

    /**
     * @brief Join two tables on a specified condition.
     * 
     * @param table1 The name of the first table.
     * @param table2 The name of the second table.
     * @param join_condition A callback function defining the join condition.
     * 
     * @return An array of joined rows.
     */
    std::vector<fossil_crabdb_entry_t> join(const std::string &table1, const std::string &table2, bool (*join_condition)(const fossil_crabdb_entry_t *, const fossil_crabdb_entry_t *)) {
        size_t result_count;
        fossil_crabdb_entry_t *results = fossil_crabdb_join(table1.c_str(), table2.c_str(), join_condition, &result_count);
        return std::vector<fossil_crabdb_entry_t>(results, results + result_count);
    }

    /**
     * @brief Aggregate data from a table.
     * 
     * @param table_name The name of the table to aggregate from.
     * @param aggregate_function A callback function to perform the aggregation.
     * 
     * @return The result of the aggregation.
     */
    void *aggregate(const std::string &table_name, void *(*aggregate_function)(const fossil_crabdb_entry_t *, size_t)) {
        return fossil_crabdb_aggregate(table_name.c_str(), aggregate_function);
    }

    /**
     * @brief Paginate query results.
     * 
     * @param entries The array of query results.
     * @param total_entries The total number of query results.
     * @param page_size The number of results per page.
     * @param page_number The page number to fetch.
     * 
     * @return A vector of entries for the requested page.
     */
    std::vector<fossil_crabdb_entry_t> paginate(const std::vector<fossil_crabdb_entry_t> &entries, size_t page_size, size_t page_number) {
        return std::vector<fossil_crabdb_entry_t>(fossil_crabdb_paginate(entries.data(), entries.size(), page_size, page_number), 
                                                  fossil_crabdb_paginate(entries.data(), entries.size(), page_size, page_number) + page_size);
    }

    /**
     * @brief Rebuild indices for optimizing search operations.
     * 
     * @param table_name The name of the table to rebuild indices for.
     */
    void rebuildIndices(const std::string &table_name) {
        fossil_crabdb_rebuild_indices(table_name.c_str());
    }
};

} // namespace fossil
#endif

#endif /* FOSSIL_CRABDB_FRAMEWORK_H */

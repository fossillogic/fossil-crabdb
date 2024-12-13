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

/* Maximum sizes for keys and values */
#define FOSSIL_CRABDB_VAL_SIZE 256
#define FOSSIL_CRABDB_KEY_SIZE 1024
#define MIN_BUFFER_SIZE 256  // Example minimum size, adjust based on needs

/* Enumeration for data types */
typedef enum {
    FOSSIL_CRABDB_TYPE_INVALID = -1,
    FOSSIL_CRABDB_TYPE_INT8,
    FOSSIL_CRABDB_TYPE_INT16,
    FOSSIL_CRABDB_TYPE_INT32,
    FOSSIL_CRABDB_TYPE_INT64,
    FOSSIL_CRABDB_TYPE_UINT8,
    FOSSIL_CRABDB_TYPE_UINT16,
    FOSSIL_CRABDB_TYPE_UINT32,
    FOSSIL_CRABDB_TYPE_UINT64,
    FOSSIL_CRABDB_TYPE_OCTAL8,
    FOSSIL_CRABDB_TYPE_OCTAL16,
    FOSSIL_CRABDB_TYPE_OCTAL32,
    FOSSIL_CRABDB_TYPE_OCTAL64,
    FOSSIL_CRABDB_TYPE_HEX8,
    FOSSIL_CRABDB_TYPE_HEX16,
    FOSSIL_CRABDB_TYPE_HEX32,
    FOSSIL_CRABDB_TYPE_HEX64,
    FOSSIL_CRABDB_TYPE_BIN8,
    FOSSIL_CRABDB_TYPE_BIN16,
    FOSSIL_CRABDB_TYPE_BIN32,
    FOSSIL_CRABDB_TYPE_BIN64,
    FOSSIL_CRABDB_TYPE_FLOAT,
    FOSSIL_CRABDB_TYPE_DOUBLE,
    FOSSIL_CRABDB_TYPE_STRING,
    FOSSIL_CRABDB_TYPE_BOOL,
    FOSSIL_CRABDB_TYPE_CHAR,
    FOSSIL_CRABDB_TYPE_NULL
} fossil_crabdb_type_t;

/* Structure for CrabDB node */
typedef struct fossil_crabdb_node {
    char key[FOSSIL_CRABDB_KEY_SIZE];
    char value[FOSSIL_CRABDB_VAL_SIZE];
    fossil_crabdb_type_t type;
    time_t timestamp;  // Timestamp for the node
    unsigned int ttl;  // Time-To-Live for the node in seconds
    struct fossil_crabdb_node* prev;
    struct fossil_crabdb_node* next;
} fossil_crabdb_node_t;

typedef struct fossil_crabdb_row {
    fossil_crabdb_node_t* values; // Linked list of nodes representing columns
    struct fossil_crabdb_row* next; // Pointer to the next row
} fossil_crabdb_row_t;

typedef struct fossil_crabdb_table {
    char table_name[FOSSIL_CRABDB_KEY_SIZE]; // Name of the table
    fossil_crabdb_node_t* rows; // Pointer to the first row (linked list of nodes)
    size_t row_count; // Number of rows in the table
    struct fossil_crabdb_table* next; // For linking multiple tables
} fossil_crabdb_table_t;

/* Structure for CrabDB */
typedef struct fossil_crabdb {
    fossil_crabdb_node_t* head;  // Pointer to the first node in the deque
    fossil_crabdb_node_t* tail;  // Pointer to the last node in the deque
    fossil_crabdb_table_t* tables; // Pointer to the first table in the list

    bool in_transaction;         // Flag indicating if a transaction is active
    struct fossil_crabdb* transaction_backup; // Backup of the database state for rollback during transactions

    FILE* db_file;               // Pointer to the persistent .crabdb file
    char* file_path;             // Path to the .crabdb file
    bool logging_enabled;         // Flag indicating if logging is enabled

    size_t node_count;           // Number of nodes (key-value pairs) in the database
} fossil_crabdb_t;


/* Database Management */

/**
 * @brief Creates a new CrabDB instance.
 * @return A pointer to the newly created CrabDB instance.
 */
fossil_crabdb_t* fossil_crabdb_create(void);

/**
 * @brief Destroys the given CrabDB instance and frees all associated memory.
 * @param db A pointer to the CrabDB instance to be destroyed.
 */
void fossil_crabdb_destroy(fossil_crabdb_t* db);

/**
 * @brief Opens a CrabDB file for reading and writing.
 * @param filename The name of the file to open.
 * @return A pointer to the CrabDB instance.
 */
bool fossil_crabdb_create_table(fossil_crabdb_t* db, const char* table_name);

/**
 * @brief Closes the CrabDB file.
 * @param db A pointer to the CrabDB instance.
 */
bool fossil_crabdb_delete_table(fossil_crabdb_t* db, const char* table_name);

/**
 * @brief Checks if a table exists in the CrabDB.
 * @param db A pointer to the CrabDB instance.
 * @param table_name The name of the table to check.
 * @return true if the table exists, false otherwise.
 */
bool fossil_crabdb_table_exists(fossil_crabdb_t* db, const char* table_name);

/**
 * @brief Counts the number of keys in the CrabDB.
 * @param db A pointer to the CrabDB instance.
 * @return The number of keys in the database.
 */
size_t fossil_crabdb_count_keys(fossil_crabdb_t* db);

/* CRUD Operations */

/**
 * @brief Inserts a new key-value pair into the CrabDB.
 * @param db A pointer to the CrabDB instance.
 * @param key The key to be inserted.
 * @param value The value to be associated with the key.
 * @param type The type of the value.
 * @return true if the insertion was successful, false otherwise.
 */
bool fossil_crabdb_insert(fossil_crabdb_t* db, const char* key, const char* value, fossil_crabdb_type_t type);

/**
 * @brief Updates the value associated with the given key in the CrabDB.
 * @param db A pointer to the CrabDB instance.
 * @param key The key whose value is to be updated.
 * @param value The new value to be associated with the key.
 * @return true if the update was successful, false otherwise.
 */
bool fossil_crabdb_update(fossil_crabdb_t* db, const char* key, const char* value);

/**
 * @brief Deletes the key-value pair associated with the given key from the CrabDB.
 * @param db A pointer to the CrabDB instance.
 * @param key The key to be deleted.
 * @return true if the deletion was successful, false otherwise.
 */
bool fossil_crabdb_delete(fossil_crabdb_t* db, const char* key);

/**
 * @brief Selects the value associated with the given key from the CrabDB.
 * @param db A pointer to the CrabDB instance.
 * @param key The key whose value is to be selected.
 * @param value A buffer to store the selected value.
 * @param value_size The size of the buffer.
 * @return true if the selection was successful, false otherwise.
 */
bool fossil_crabdb_select(fossil_crabdb_t* db, const char* key, char* value, size_t value_size);

/**
 * @brief Lists all key-value pairs in the CrabDB.
 * @param db A pointer to the CrabDB instance.
 * @param list_buffer A buffer to store the list of key-value pairs.
 * @param buffer_size The size of the buffer.
 * @return true if the listing was successful, false otherwise.
 */
bool fossil_crabdb_list(fossil_crabdb_t* db, char* list_buffer, size_t buffer_size);

/**
 * @brief Clears all key-value pairs from the CrabDB.
 * @param db A pointer to the CrabDB instance.
 * @return true if the clearing was successful, false otherwise.
 */
bool fossil_crabdb_clear(fossil_crabdb_t* db);

/**
 * @brief Drops the CrabDB and deletes the associated file.
 * @param db A pointer to the CrabDB instance.
 * @return true if the CrabDB was dropped successfully, false otherwise.
 */
bool fossil_crabdb_drop(fossil_crabdb_t* db);

/**
 * @brief Inserts a new key-value pair into the CrabDB.
 * @param db A pointer to the CrabDB instance.
 * @param key The key to be inserted.
 * @param value The value to be associated with the key.
 * @param type The type of the value.
 * @return true if the insertion was successful, false otherwise.
 */
bool fossil_crabdb_insert_into_table(fossil_crabdb_t* db, const char* table_name, const char* key, const char* value, fossil_crabdb_type_t type);

/**
 * @brief Updates the value associated with the given key in the CrabDB.
 * @param db A pointer to the CrabDB instance.
 * @param key The key whose value is to be updated.
 * @param value The new value to be associated with the key.
 * @return true if the update was successful, false otherwise.
 */
bool fossil_crabdb_update_table(fossil_crabdb_t* db, const char* table_name, const char* key, const char* value);

/**
 * @brief Deletes the key-value pair associated with the given key from the CrabDB.
 * @param db A pointer to the CrabDB instance.
 * @param key The key to be deleted.
 * @return true if the deletion was successful, false otherwise.
 */
bool fossil_crabdb_delete_from_table(fossil_crabdb_t* db, const char* table_name, const char* key);

/**
 * @brief Selects the value associated with the given key from the CrabDB.
 * @param db A pointer to the CrabDB instance.
 * @param key The key whose value is to be selected.
 * @param value A buffer to store the selected value.
 * @param value_size The size of the buffer.
 * @return true if the selection was successful, false otherwise.
 */
bool fossil_crabdb_select_from_table(fossil_crabdb_t* db, const char* table_name, const char* key, char* value, size_t value_size);

/**
 * @brief Lists all key-value pairs in the CrabDB.
 * @param db A pointer to the CrabDB instance.
 * @param list_buffer A buffer to store the list of key-value pairs.
 * @param buffer_size The size of the buffer.
 * @return true if the listing was successful, false otherwise.
 */
bool fossil_crabdb_list_table(fossil_crabdb_t* db, const char* table_name, char* list_buffer, size_t buffer_size);

/**
 * @brief Clears all key-value pairs from the CrabDB.
 * @param db A pointer to the CrabDB instance.
 * @return true if the clearing was successful, false otherwise.
 */
bool fossil_crabdb_clear_table(fossil_crabdb_t* db, const char* table_name);

/**
 * @brief Drops the CrabDB and deletes the associated file.
 * @param db A pointer to the CrabDB instance.
 * @return true if the CrabDB was dropped successfully, false otherwise.
 */
bool fossil_crabdb_drop_table(fossil_crabdb_t* db, const char* table_name);

/* Database File Operations */

/**
 * @brief Backs up the CrabDB to a file.
 * @param filename The name of the file to store the backup.
 * @param db A pointer to the CrabDB instance.
 * @return true if the backup was successful, false otherwise.
 */
bool fossil_crabdb_backup(const char* filename, fossil_crabdb_t* db);

/**
 * @brief Restores the CrabDB from a backup file.
 * @param filename The name of the backup file.
 * @param db A pointer to the CrabDB instance.
 * @return true if the restoration was successful, false otherwise.
 */
bool fossil_crabdb_restore(const char* filename, fossil_crabdb_t* db);

/**
 * @brief Gets the version of the CrabDB db.
 * @param db A pointer to the db.
 * @return The current version number of the db.
 */
unsigned int fossil_crabdb_get_version(fossil_crabdb_t* db);

/**
 * @brief Restores the CrabDB db to a previous version.
 * @param db A pointer to the db.
 * @param version The version number to restore to.
 * @return true if the db was restored successfully, false otherwise.
 */
bool fossil_crabdb_restore_version(fossil_crabdb_t* db, unsigned int version);

/* Database transactions */

/**
 * @brief Begins a new transaction in the CrabDB db.
 * @param db A pointer to the db.
 * @return true if the transaction started successfully, false otherwise.
 */
bool fossil_crabdb_begin_transaction(fossil_crabdb_t* db);

/**
 * @brief Commits the current transaction in the CrabDB db.
 * @param db A pointer to the db.
 * @return true if the transaction was committed successfully, false otherwise.
 */
bool fossil_crabdb_commit_transaction(fossil_crabdb_t* db);

/**
 * @brief Rolls back the current transaction in the CrabDB db.
 * @param db A pointer to the db.
 * @return true if the transaction was rolled back successfully, false otherwise.
 */
bool fossil_crabdb_rollback_transaction(fossil_crabdb_t* db);

/**
 * @brief Begins a new transaction in the CrabDB db for a specific table.
 * @param db A pointer to the db.
 * @param table_name The name of the table to begin the transaction for.
 * @return true if the transaction started successfully, false otherwise.
 */
bool fossil_crabdb_begin_transaction_table(fossil_crabdb_t* db, const char* table_name);

/**
 * @brief Commits the current transaction in the CrabDB db for a specific table.
 * @param db A pointer to the db.
 * @param table_name The name of the table to commit the transaction for.
 * @return true if the transaction was committed successfully, false otherwise.
 */
bool fossil_crabdb_commit_transaction_table(fossil_crabdb_t* db, const char* table_name);

/**
 * @brief Rolls back the current transaction in the CrabDB db for a specific table.
 * @param db A pointer to the db.
 * @param table_name The name of the table to roll back the transaction for.
 * @return true if the transaction was rolled back successfully, false otherwise.
 */
bool fossil_crabdb_rollback_transaction_table(fossil_crabdb_t* db, const char* table_name);

/* Database Batch Operations */

/**
 * @brief Inserts multiple key-value pairs into the CrabDB db.
 * @param db A pointer to the db.
 * @param keys An array of keys to insert.
 * @param values An array of values to insert.
 * @param types An array of types for the values.
 * @param count The number of key-value pairs to insert.
 * @return true if all pairs were inserted successfully, false otherwise.
 */
bool fossil_crabdb_insert_batch(fossil_crabdb_t* db, const char** keys, const char** values, fossil_crabdb_type_t* types, size_t count);

/**
 * @brief Deletes multiple key-value pairs from the CrabDB db.
 * @param db A pointer to the db.
 * @param keys An array of keys to delete.
 * @param count The number of keys to delete.
 * @return true if all pairs were deleted successfully, false otherwise.
 */
bool fossil_crabdb_delete_batch(fossil_crabdb_t* db, const char** keys, size_t count);

/**
 * @brief Updates multiple key-value pairs in the CrabDB db.
 * @param db A pointer to the db.
 * @param keys An array of keys to update.
 * @param values An array of new values to update.
 * @param count The number of key-value pairs to update.
 * @return true if all pairs were updated successfully, false otherwise.
 */
bool fossil_crabdb_update_batch(fossil_crabdb_t* db, const char** keys, const char** values, size_t count);

/**
 * @brief Selects multiple key-value pairs from the CrabDB db.
 * @param db A pointer to the db.
 * @param keys An array of keys to select.
 * @param values A buffer to store the selected values.
 * @param value_sizes An array of sizes for the value buffers.
 * @param count The number of key-value pairs to select.
 * @return true if all pairs were selected successfully, false otherwise.
 */
bool fossil_crabdb_select_batch(fossil_crabdb_t* db, const char** keys, char** values, size_t* value_sizes, size_t count);

/**
 * @brief Inserts multiple key-value pairs into the CrabDB db in a single transaction.
 * @param db A pointer to the db.
 * @param keys An array of keys to insert.
 * @param values An array of values to insert.
 * @param types An array of types for the values.
 * @param count The number of key-value pairs to insert.
 * @return true if all pairs were inserted successfully, false otherwise.
 */
bool fossil_crabdb_list_batch(fossil_crabdb_t* db, const char** keys, char** values, size_t* value_sizes, size_t count);

/**
 * @brief Deletes multiple key-value pairs from the CrabDB db in a single transaction.
 * @param db A pointer to the db.
 * @param keys An array of keys to delete.
 * @param count The number of keys to delete.
 * @return true if all pairs were deleted successfully, false otherwise.
 */
bool fossil_crabdb_clear_batch(fossil_crabdb_t* db, const char** keys, size_t count);

/**
 * @brief Updates multiple key-value pairs in the CrabDB db in a single transaction.
 * @param db A pointer to the db.
 * @param keys An array of keys to update.
 * @param values An array of new values to update.
 * @param count The number of key-value pairs to update.
 * @return true if all pairs were updated successfully, false otherwise.
 */
bool fossil_crabdb_drop_batch(fossil_crabdb_t* db, const char** keys, size_t count);

/**
 * @brief Inserts multiple key-value pairs into the CrabDB db in a single transaction.
 * @param db A pointer to the db.
 * @param keys An array of keys to insert.
 * @param values An array of values to insert.
 * @param types An array of types for the values.
 * @param count The number of key-value pairs to insert.
 * @return true if all pairs were inserted successfully, false otherwise.
 */
bool fossil_crabdb_insert_into_table_batch(fossil_crabdb_t* db, const char* table_name, const char** keys, const char** values, fossil_crabdb_type_t* types, size_t count);

/**
 * @brief Deletes multiple key-value pairs from the CrabDB db in a single transaction.
 * @param db A pointer to the db.
 * @param keys An array of keys to delete.
 * @param count The number of keys to delete.
 * @return true if all pairs were deleted successfully, false otherwise.
 */
bool fossil_crabdb_delete_from_table_batch(fossil_crabdb_t* db, const char* table_name, const char** keys, size_t count);

/**
 * @brief Updates multiple key-value pairs in the CrabDB db in a single transaction.
 * @param db A pointer to the db.
 * @param keys An array of keys to update.
 * @param values An array of new values to update.
 * @param count The number of key-value pairs to update.
 * @return true if all pairs were updated successfully, false otherwise.
 */
bool fossil_crabdb_update_table_batch(fossil_crabdb_t* db, const char* table_name, const char** keys, const char** values, size_t count);

/**
 * @brief Selects multiple key-value pairs from the CrabDB db in a single transaction.
 * @param db A pointer to the db.
 * @param keys An array of keys to select.
 * @param values A buffer to store the selected values.
 * @param value_sizes An array of sizes for the value buffers.
 * @param count The number of key-value pairs to select.
 * @return true if all pairs were selected successfully, false otherwise.
 */
bool fossil_crabdb_select_from_table_batch(fossil_crabdb_t* db, const char* table_name, const char** keys, char** values, size_t* value_sizes, size_t count);

/**
 * @brief Lists all key-value pairs in the CrabDB db in a single transaction.
 * @param db A pointer to the db.
 * @param list_buffer A buffer to store the list of key-value pairs.
 * @param buffer_size The size of the buffer.
 * @return true if the listing was successful, false otherwise.
 */
bool fossil_crabdb_list_table_batch(fossil_crabdb_t* db, const char* table_name, const char** keys, char** values, size_t* value_sizes, size_t count);

/**
 * @brief Clears all key-value pairs from the CrabDB db in a single transaction.
 * @param db A pointer to the db.
 * @return true if the clearing was successful, false otherwise.
 */
bool fossil_crabdb_clear_table_batch(fossil_crabdb_t* db, const char* table_name, const char** keys, size_t count);

/**
 * @brief Drops the CrabDB db and deletes the associated file in a single transaction.
 * @param db A pointer to the db.
 * @return true if the db was dropped successfully, false otherwise.
 */
bool fossil_crabdb_drop_table_batch(fossil_crabdb_t* db, const char* table_name, const char** keys, size_t count);

/* Database Algorihtms */

/**
 * @brief Searches for key-value pairs in the CrabDB db that match a wildcard pattern.
 * @param db A pointer to the db to search in.
 * @param pattern The wildcard pattern to match keys.
 * @param result_buffer The buffer to store the matching key-value pairs.
 * @param buffer_size The size of the result buffer.
 * @return true if matching pairs were found, false otherwise.
 */
bool fossil_crabdb_search_by_pattern(fossil_crabdb_t* db, const char* pattern, char* result_buffer, size_t buffer_size);

/**
 * @brief Sorts the key-value pairs in the CrabDB db by key.
 * @param db A pointer to the db to sort.
 * @return true if the sorting was successful, false otherwise.
 */
bool fossil_crabdb_sort_by_key(fossil_crabdb_t* db);

/**
 * @brief Sorts the key-value pairs in the CrabDB db by value.
 * @param db A pointer to the db to sort.
 * @return true if the sorting was successful, false otherwise.
 */
bool fossil_crabdb_sort_by_value(fossil_crabdb_t* db);

/* TTL Operations */

/**
 * @brief Inserts a key-value pair into the db with a TTL (Time-To-Live).
 * @param db A pointer to the db.
 * @param key The key of the pair.
 * @param value The value of the pair.
 * @param type The type of the value.
 * @param ttl The time-to-live in seconds.
 * @return true if the pair was inserted successfully, false otherwise.
 */
bool fossil_crabdb_insert_with_ttl(fossil_crabdb_t* db, const char* key, const char* value, fossil_crabdb_type_t type, unsigned int ttl);

/**
 * @brief Cleans up expired entries in the db.
 * @param db A pointer to the db.
 * @return true if expired entries were cleaned up successfully, false otherwise.
 */
bool fossil_crabdb_cleanup_expired(fossil_crabdb_t* db);

/**
 * @brief Inserts a key-value pair into a table with a TTL (Time-To-Live).
 * @param db A pointer to the db.
 * @param table_name The name of the table.
 * @param key The key of the pair.
 * @param value The value of the pair.
 * @param type The type of the value.
 * @param ttl The time-to-live in seconds.
 * @return true if the pair was inserted successfully, false otherwise.
 */
bool fossil_crabdb_insert_with_ttl_into_table(fossil_crabdb_t* db, const char* table_name, const char* key, const char* value, fossil_crabdb_type_t type, unsigned int ttl);

/**
 * @brief Cleans up expired entries in a table.
 * @param db A pointer to the db.
 * @param table_name The name of the table.
 * @return true if expired entries were cleaned up successfully, false otherwise.
 */
bool fossil_crabdb_cleanup_expired_table(fossil_crabdb_t* db, const char* table_name);

/* Database Logging */

/**
 * @brief Enables logging for CrabDB operations.
 * @param db A pointer to the db.
 * @param log_filename The file to log operations to.
 * @return true if logging was enabled successfully, false otherwise.
 */
bool fossil_crabdb_enable_logging(fossil_crabdb_t* db, const char* log_filename);

/**
 * @brief Disables logging for CrabDB operations.
 * @param db A pointer to the db.
 * @return true if logging was disabled successfully, false otherwise.
 */
bool fossil_crabdb_disable_logging(fossil_crabdb_t* db);

/* Database Integrity Check */

/**
 * @brief Performs a data integrity check on the CrabDB db.
 * @param db A pointer to the db.
 * @return true if the db passed the integrity check, false otherwise.
 */
bool fossil_crabdb_check_integrity(fossil_crabdb_t* db);

#ifdef __cplusplus
}
#endif

#ifdef __cplusplus
#include <string>

namespace fossil {

class CrabDB {
public:
    CrabDB() {
        db_ = fossil_crabdb_create();
    }

    ~CrabDB() {
        fossil_crabdb_destroy(db_);
    }

    bool create_table(const std::string& table_name) {
        return fossil_crabdb_create_table(db_, table_name.c_str());
    }

    bool delete_table(const std::string& table_name) {
        return fossil_crabdb_delete_table(db_, table_name.c_str());
    }

    bool table_exists(const std::string& table_name) {
        return fossil_crabdb_table_exists(db_, table_name.c_str());
    }

    size_t count_keys() {
        return fossil_crabdb_count_keys(db_);
    }

    bool insert(const std::string& key, const std::string& value, fossil_crabdb_type_t type) {
        return fossil_crabdb_insert(db_, key.c_str(), value.c_str(), type);
    }

    bool update(const std::string& key, const std::string& value) {
        return fossil_crabdb_update(db_, key.c_str(), value.c_str());
    }

    bool delete_key(const std::string& key) {
        return fossil_crabdb_delete(db_, key.c_str());
    }

    bool select(const std::string& key, std::string& value) {
        char buffer[FOSSIL_CRABDB_VAL_SIZE];
        bool result = fossil_crabdb_select(db_, key.c_str(), buffer, sizeof(buffer));
        if (result) {
            value = buffer;
        }
        return result;
    }

    bool list(std::string& list_buffer) {
        char buffer[MIN_BUFFER_SIZE];
        bool result = fossil_crabdb_list(db_, buffer, sizeof(buffer));
        if (result) {
            list_buffer = buffer;
        }
        return result;
    }

    bool clear() {
        return fossil_crabdb_clear(db_);
    }

    bool drop() {
        return fossil_crabdb_drop(db_);
    }

    bool insert_into_table(const std::string& table_name, const std::string& key, const std::string& value, fossil_crabdb_type_t type) {
        return fossil_crabdb_insert_into_table(db_, table_name.c_str(), key.c_str(), value.c_str(), type);
    }

    bool update_table(const std::string& table_name, const std::string& key, const std::string& value) {
        return fossil_crabdb_update_table(db_, table_name.c_str(), key.c_str(), value.c_str());
    }

    bool delete_from_table(const std::string& table_name, const std::string& key) {
        return fossil_crabdb_delete_from_table(db_, table_name.c_str(), key.c_str());
    }

    bool select_from_table(const std::string& table_name, const std::string& key, std::string& value) {
        char buffer[FOSSIL_CRABDB_VAL_SIZE];
        bool result = fossil_crabdb_select_from_table(db_, table_name.c_str(), key.c_str(), buffer, sizeof(buffer));
        if (result) {
            value = buffer;
        }
        return result;
    }

    bool list_table(const std::string& table_name, std::string& list_buffer) {
        char buffer[MIN_BUFFER_SIZE];
        bool result = fossil_crabdb_list_table(db_, table_name.c_str(), buffer, sizeof(buffer));
        if (result) {
            list_buffer = buffer;
        }
        return result;
    }

    bool clear_table(const std::string& table_name) {
        return fossil_crabdb_clear_table(db_, table_name.c_str());
    }

    bool drop_table(const std::string& table_name) {
        return fossil_crabdb_drop_table(db_, table_name.c_str());
    }

    bool backup(const std::string& filename) {
        return fossil_crabdb_backup(filename.c_str(), db_);
    }

    bool restore(const std::string& filename) {
        return fossil_crabdb_restore(filename.c_str(), db_);
    }

    unsigned int get_version() {
        return fossil_crabdb_get_version(db_);
    }

    bool restore_version(unsigned int version) {
        return fossil_crabdb_restore_version(db_, version);
    }

    bool begin_transaction() {
        return fossil_crabdb_begin_transaction(db_);
    }

    bool commit_transaction() {
        return fossil_crabdb_commit_transaction(db_);
    }

    bool rollback_transaction() {
        return fossil_crabdb_rollback_transaction(db_);
    }

    bool begin_transaction_table(const std::string& table_name) {
        return fossil_crabdb_begin_transaction_table(db_, table_name.c_str());
    }

    bool commit_transaction_table(const std::string& table_name) {
        return fossil_crabdb_commit_transaction_table(db_, table_name.c_str());
    }

    bool rollback_transaction_table(const std::string& table_name) {
        return fossil_crabdb_rollback_transaction_table(db_, table_name.c_str());
    }

    bool insert_with_ttl(const std::string& key, const std::string& value, fossil_crabdb_type_t type, unsigned int ttl) {
        return fossil_crabdb_insert_with_ttl(db_, key.c_str(), value.c_str(), type, ttl);
    }

    bool cleanup_expired() {
        return fossil_crabdb_cleanup_expired(db_);
    }

    bool insert_with_ttl_into_table(const std::string& table_name, const std::string& key, const std::string& value, fossil_crabdb_type_t type, unsigned int ttl) {
        return fossil_crabdb_insert_with_ttl_into_table(db_, table_name.c_str(), key.c_str(), value.c_str(), type, ttl);
    }

    bool cleanup_expired_table(const std::string& table_name) {
        return fossil_crabdb_cleanup_expired_table(db_, table_name.c_str());
    }

    bool enable_logging(const std::string& log_filename) {
        return fossil_crabdb_enable_logging(db_, log_filename.c_str());
    }

    bool disable_logging() {
        return fossil_crabdb_disable_logging(db_);
    }

    bool check_integrity() {
        return fossil_crabdb_check_integrity(db_);
    }

private:
    fossil_crabdb_t* db_;
};

} // namespace fossil
#endif

#endif /* FOSSIL_CRABDB_FRAMEWORK_H */

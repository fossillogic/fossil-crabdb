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

/* Structure for CrabDB */
typedef struct fossil_crabdb {
    fossil_crabdb_node_t* head;  // Pointer to the first node in the deque
    fossil_crabdb_node_t* tail;  // Pointer to the last node in the deque

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

/* Database Utilities */

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

/* Database Versioning */

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

/* Database Serialization */

/**
 * @brief Executes a CrabQL query on the db.
 * @param db A pointer to the db.
 * @param query The CrabQL query to execute.
 * @param result_buffer The buffer to store query results.
 * @param buffer_size The size of the result buffer.
 * @return true if the query was executed successfully, false otherwise.
 */
bool fossil_crabdb_execute_crabql(fossil_crabdb_t* db, const char* query, char* result_buffer, size_t buffer_size);

#ifdef __cplusplus
}
#endif

#endif /* FOSSIL_CRABDB_FRAMEWORK_H */

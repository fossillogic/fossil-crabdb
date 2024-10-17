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
#ifndef FOSSIL_CRABDB_DATABASE_H
#define FOSSIL_CRABDB_DATABASE_H

#include "internal.h"

#ifdef __cplusplus
extern "C" {
#endif

/* ======================================================
 * CRAB DATABASE TYPES
 * ------------------------------------------------------
 */

#define MAX_KEY_SIZE 256
#define MAX_VALUE_SIZE 1024

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

typedef struct fossil_crabdb_node {
    char key[MAX_KEY_SIZE];
    char value[MAX_VALUE_SIZE];
    fossil_crabdb_type_t type;
    struct fossil_crabdb_node* prev;
    struct fossil_crabdb_node* next;
} fossil_crabdb_node_t;

typedef struct fossil_crabdb_deque_t {
    fossil_crabdb_node_t* head;
    fossil_crabdb_node_t* tail;
    fossil_crabdb_mutex_t mutex;  // Cross-platform mutex
} fossil_crabdb_deque_t;


/* ======================================================
 * CRAB DATABASE MANAGMENT
 * ------------------------------------------------------
 */

/**
 * @brief Creates a new CrabDB deque.
 * @return A pointer to the newly created deque.
 */
fossil_crabdb_deque_t* fossil_crabdb_create(void);

/**
 * @brief Destroys a CrabDB deque and frees the associated resources.
 * @param deque A pointer to the deque to destroy.
 */
void fossil_crabdb_destroy(fossil_crabdb_deque_t* deque);

/* ======================================================
 * CRAB DATABASE OPERATIONS
 * ------------------------------------------------------
 */

/**
 * @brief Inserts a key-value pair into a CrabDB deque.
 * @param deque A pointer to the deque to insert the pair into.
 * @param key The key of the pair.
 * @param value The value of the pair.
 * @param type The type of the value.
 * @return true if the pair was inserted successfully, false otherwise.
 */
bool fossil_crabdb_insert(fossil_crabdb_deque_t* deque, const char* key, const char* value, fossil_crabdb_type_t type);

/**
 * @brief Updates the value associated with a key in a CrabDB deque.
 * @param deque A pointer to the deque to update the pair in.
 * @param key The key of the pair to update.
 * @param value The new value to associate with the key.
 * @return true if the pair was updated successfully, false otherwise.
 */
bool fossil_crabdb_update(fossil_crabdb_deque_t* deque, const char* key, const char* value);

/**
 * @brief Deletes a key-value pair from a CrabDB deque.
 * @param deque A pointer to the deque to delete the pair from.
 * @param key The key of the pair to delete.
 * @return true if the pair was deleted successfully, false otherwise.
 */
bool fossil_crabdb_delete(fossil_crabdb_deque_t* deque, const char* key);

/**
 * @brief Selects the value associated with a key in a CrabDB deque.
 * @param deque A pointer to the deque to select the value from.
 * @param key The key to select the value for.
 * @param value The buffer to store the selected value.
 * @param value_size The size of the value buffer.
 * @return true if the value was selected successfully, false otherwise.
 */
bool fossil_crabdb_select(fossil_crabdb_deque_t* deque, const char* key, char* value, size_t value_size);

/**
 * @brief Lists all key-value pairs in a CrabDB deque.
 * @param deque A pointer to the deque to list the pairs from.
 * @param list_buffer The buffer to store the list of pairs.
 * @param buffer_size The size of the list buffer.
 * @return true if the pairs were listed successfully, false otherwise.
 */
bool fossil_crabdb_list(fossil_crabdb_deque_t* deque, char* list_buffer, size_t buffer_size);

/**
 * @brief Clears all key-value pairs from a CrabDB deque.
 * @param deque A pointer to the deque to clear.
 * @return true if the deque was cleared successfully, false otherwise.
 */
bool fossil_crabdb_clear(fossil_crabdb_deque_t* deque);
/**
 * @brief Shows the contents of a CrabDB deque.
 * @param deque A pointer to the deque to show.
 * @return true if the deque was shown successfully, false otherwise.
 */
bool fossil_crabdb_show(fossil_crabdb_deque_t* deque);

/**
 * @brief Drops a CrabDB deque and clears its contents.
 * @param deque A pointer to the deque to drop.
 * @return true if the deque was dropped successfully, false otherwise.
 */
bool fossil_crabdb_drop(fossil_crabdb_deque_t* deque);

/**
 * @brief Checks if a key exists in a CrabDB deque.
 * @param deque A pointer to the deque to check.
 * @param key The key to check for existence.
 * @return true if the key exists in the deque, false otherwise.
 */
bool fossil_crabdb_exist(fossil_crabdb_deque_t* deque, const char* key);

/**
 * @brief Backs up the contents of a CrabDB deque to a specified file.
 * @param filename The name of the backup file.
 * @param deque A pointer to the deque to back up.
 * @return true if the backup was created successfully, false otherwise.
 */
bool fossil_crabdb_backup(const char* filename, fossil_crabdb_deque_t* deque);

/**
 * @brief Restores the contents of a CrabDB deque from a backup file.
 * @param filename The name of the backup file.
 * @param deque A pointer to the deque to restore.
 * @return true if the deque was restored successfully, false otherwise.
 */
bool fossil_crabdb_restore(const char* filename, fossil_crabdb_deque_t* deque);

/**
 * @brief Compacts the CrabDB deque by removing deleted entries and reclaiming space.
 * @param deque A pointer to the deque to compact.
 * @return true if the deque was compacted successfully, false otherwise.
 */
bool fossil_crabdb_compact(fossil_crabdb_deque_t* deque);

/**
 * @brief Batch inserts multiple key-value pairs into a CrabDB deque.
 * @param deque A pointer to the deque to insert the pairs into.
 * @param keys An array of keys.
 * @param values An array of values corresponding to the keys.
 * @param types An array of value types corresponding to the values.
 * @param count The number of key-value pairs to insert.
 * @return true if the pairs were inserted successfully, false otherwise.
 */
bool fossil_crabdb_batch_insert(fossil_crabdb_deque_t* deque, const char keys[][MAX_KEY_SIZE], const char values[][MAX_VALUE_SIZE], fossil_crabdb_type_t types[], size_t count);

/**
 * @brief Batch deletes multiple key-value pairs from a CrabDB deque by key.
 * @param deque A pointer to the deque to delete the pairs from.
 * @param keys An array of keys to delete.
 * @param count The number of keys to delete.
 * @return true if the pairs were deleted successfully, false otherwise.
 */
bool fossil_crabdb_batch_delete(fossil_crabdb_deque_t* deque, const char keys[][MAX_KEY_SIZE], size_t count);

/* ======================================================
 * CRAB DATABASE ALGORITMS
 * ------------------------------------------------------
 */

/**
 * @brief Searches for a key-value pair in a CrabDB deque by key.
 * @param deque A pointer to the deque to search in.
 * @param key The key to search for.
 * @param value The buffer to store the value associated with the key.
 * @param value_size The size of the value buffer.
 * @return true if the key-value pair was found, false otherwise.
 */
bool fossil_crabdb_search_by_key(fossil_crabdb_deque_t* deque, const char* key, char* value, size_t value_size);

/**
 * @brief Searches for a key-value pair in a CrabDB deque by value.
 * @param deque A pointer to the deque to search in.
 * @param value The value to search for.
 * @param key_buffer The buffer to store the key associated with the value.
 * @param key_buffer_size The size of the key buffer.
 * @return true if the key-value pair was found, false otherwise.
 */
bool fossil_crabdb_search_by_value(fossil_crabdb_deque_t* deque, const char* value, char* key_buffer, size_t key_buffer_size);

/**
 * @brief Sorts the key-value pairs in a CrabDB deque by key.
 * @param deque A pointer to the deque to sort.
 * @return true if the deque was sorted successfully, false otherwise.
 */
bool fossil_crabdb_sort_by_key(fossil_crabdb_deque_t* deque);

/**
 * @brief Sorts the key-value pairs in a CrabDB deque by value.
 * @param deque A pointer to the deque to sort.
 * @return true if the deque was sorted successfully, false otherwise.
 */
bool fossil_crabdb_sort_by_value(fossil_crabdb_deque_t* deque);

/* ======================================================
 * CRAB DATABASE SERIALIZATION
 * ------------------------------------------------------
 */

/**
 * @brief Encodes the contents of a CrabDB deque and saves it to a file.
 * @param filename The name of the file to save the encoded data to.
 * @param deque A pointer to the deque to encode.
 * @return true if the deque was encoded and saved successfully, false otherwise.
 */
bool fossil_crabdb_encode(const char* filename, fossil_crabdb_deque_t* deque);

/**
 * @brief Decodes the contents of a file and loads it into a CrabDB deque.
 * @param filename The name of the file to load the data from.
 * @param deque A pointer to the deque to load the decoded data into.
 * @return true if the file was decoded and the deque was loaded successfully, false otherwise.
 */
bool fossil_crabdb_decode(const char* filename, fossil_crabdb_deque_t* deque);

/**
 * @brief Counts the number of key-value pairs in a CrabDB deque.
 * @param deque A pointer to the deque to count the pairs in.
 * @return The number of key-value pairs in the deque.
 */
size_t fossil_crabdb_count(fossil_crabdb_deque_t* deque);

/**
 * @brief Exports the contents of a CrabDB deque to a CSV file.
 * @param filename The name of the file to save the CSV data to.
 * @param deque A pointer to the deque to export.
 * @return true if the data was exported successfully, false otherwise.
 */
bool fossil_crabdb_export_csv(const char* filename, fossil_crabdb_deque_t* deque);

/**
 * @brief Imports key-value pairs from a CSV file into a CrabDB deque.
 * @param filename The name of the CSV file to load data from.
 * @param deque A pointer to the deque to import the data into.
 * @return true if the data was imported successfully, false otherwise.
 */
bool fossil_crabdb_import_csv(const char* filename, fossil_crabdb_deque_t* deque);

/* ======================================================
 * CRAB DATABASE QUERY LANGUAGE
 * ------------------------------------------------------
 */

/**
 * @brief Executes a CrabQL query on a CrabDB deque.
 * @param deque A pointer to the deque to execute the query on.
 * @param query The CrabQL query to execute.
 * @return true if the query was executed successfully, false otherwise.
 */
bool fossil_crabdb_exec(const char* filename, fossil_crabdb_deque_t* deque);

/**
 * @brief Executes commands from a CrabQL script file on a CrabDB deque.
 * @param filename The name of the script file to execute commands from.
 * @param deque A pointer to the deque to execute commands on.
 * @return true if the commands were executed successfully, false otherwise.
 */
bool fossil_crabdb_script(const char* filename, fossil_crabdb_deque_t* deque);

/**
 * @brief Executes commands from the command line on a CrabDB deque.
 * @param deque A pointer to the deque to execute commands on.
 * @return true if the commands were executed successfully, false otherwise.
 */
bool fossil_crabdb_commandline(fossil_crabdb_deque_t* deque);

#ifdef __cplusplus
}
#endif

#ifdef __cplusplus

#include <string>
#include <vector>

namespace fossil {

    class CrabDB {
    public:
        CrabDB() {
            deque = fossil_crabdb_create();
        }

        ~CrabDB() {
            fossil_crabdb_destroy(deque);
        }

        bool insert(const std::string& key, const std::string& value, fossil_crabdb_type_t type) {
            return fossil_crabdb_insert(deque, key.c_str(), value.c_str(), type);
        }

        bool update(const std::string& key, const std::string& value) {
            return fossil_crabdb_update(deque, key.c_str(), value.c_str());
        }

        bool remove(const std::string& key) {
            return fossil_crabdb_delete(deque, key.c_str());
        }

        bool select(const std::string& key, std::string& value) {
            char buffer[MAX_VALUE_SIZE];
            bool success = fossil_crabdb_select(deque, key.c_str(), buffer, sizeof(buffer));
            if (success) {
                value = buffer;
            }
            return success;
        }

        std::vector<std::pair<std::string, std::string>> list() {
            std::vector<std::pair<std::string, std::string>> pairs;
            char buffer[MAX_VALUE_SIZE];
            bool success = fossil_crabdb_list(deque, buffer, sizeof(buffer));
            if (success) {
                std::string listString(buffer);
                std::istringstream iss(listString);
                std::string line;
                while (std::getline(iss, line)) {
                    size_t pos = line.find('=');
                    if (pos != std::string::npos) {
                        std::string key = line.substr(0, pos);
                        std::string value = line.substr(pos + 1);
                        pairs.emplace_back(key, value);
                    }
                }
            }
            return pairs;
        }

        bool clear() {
            return fossil_crabdb_clear(deque);
        }

        bool show() {
            return fossil_crabdb_show(deque);
        }

        bool drop() {
            return fossil_crabdb_drop(deque);
        }

        bool exist(const std::string& key) {
            return fossil_crabdb_exist(deque, key.c_str());
        }

        size_t count() {
            return fossil_crabdb_count(deque);
        }

        bool encode(const std::string& filename) {
            return fossil_crabdb_encode(filename.c_str(), deque);
        }

        bool decode(const std::string& filename) {
            return fossil_crabdb_decode(filename.c_str(), deque);
        }

        bool exportCSV(const std::string& filename) {
            return fossil_crabdb_export_csv(filename.c_str(), deque);
        }

        bool importCSV(const std::string& filename) {
            return fossil_crabdb_import_csv(filename.c_str(), deque);
        }

        bool exec(const std::string& query) {
            return fossil_crabdb_exec(query.c_str(), deque);
        }

        bool script(const std::string& filename) {
            return fossil_crabdb_script(filename.c_str(), deque);
        }

        bool commandline() {
            return fossil_crabdb_commandline(deque);
        }

    private:
        fossil_crabdb_deque_t* deque;
    };
} // namespace fossil
#endif

#endif /* FOSSIL_CRABDB_FRAMEWORK_H */

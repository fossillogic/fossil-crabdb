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

/**
 * @brief Defines the maximum size for values in the CrabDB.
 */
enum {
    _FOSSIL_CRABDB_VAL_SIZE = 256,
    _FOSSIL_CRABDB_KEY_SIZE = 1024
};

/**
 * @brief Enumeration of possible data types in CrabDB.
 */
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

/**
 * @brief Structure representing a node in the CrabDB db.
 */
typedef struct fossil_crabdb_node {
    char key[_FOSSIL_CRABDB_KEY_SIZE];   /**< Key of the node */
    char value[_FOSSIL_CRABDB_VAL_SIZE]; /**< Value of the node */
    fossil_crabdb_type_t type;           /**< Type of the value */
    struct fossil_crabdb_node* prev;     /**< Pointer to the previous node */
    struct fossil_crabdb_node* next;     /**< Pointer to the next node */
} fossil_crabdb_node_t;

/**
 * @brief Structure representing the CrabDB db.
 */
typedef struct fossil_crabdb_t {
    fossil_crabdb_node_t* head;  /**< Pointer to the head node */
    fossil_crabdb_node_t* tail;  /**< Pointer to the tail node */
} fossil_crabdb_t;


/* ======================================================
 * CRAB DATABASE MANAGMENT
 * ------------------------------------------------------
 */

/**
 * @brief Creates a new CrabDB db.
 * @return A pointer to the newly created db.
 */
fossil_crabdb_t* fossil_crabdb_create(void);

/**
 * @brief Destroys a CrabDB db and frees the associated resources.
 * @param db A pointer to the db to destroy.
 */
void fossil_crabdb_destroy(fossil_crabdb_t* db);

/* ======================================================
 * CRAB DATABASE OPERATIONS
 * ------------------------------------------------------
 */

/**
 * @brief Inserts a key-value pair into a CrabDB db.
 * @param db A pointer to the db to insert the pair into.
 * @param key The key of the pair.
 * @param value The value of the pair.
 * @param type The type of the value.
 * @return true if the pair was inserted successfully, false otherwise.
 */
bool fossil_crabdb_insert(fossil_crabdb_t* db, const char* key, const char* value, fossil_crabdb_type_t type);

/**
 * @brief Updates the value associated with a key in a CrabDB db.
 * @param db A pointer to the db to update the pair in.
 * @param key The key of the pair to update.
 * @param value The new value to associate with the key.
 * @return true if the pair was updated successfully, false otherwise.
 */
bool fossil_crabdb_update(fossil_crabdb_t* db, const char* key, const char* value);

/**
 * @brief Deletes a key-value pair from a CrabDB db.
 * @param db A pointer to the db to delete the pair from.
 * @param key The key of the pair to delete.
 * @return true if the pair was deleted successfully, false otherwise.
 */
bool fossil_crabdb_delete(fossil_crabdb_t* db, const char* key);

/**
 * @brief Selects the value associated with a key in a CrabDB db.
 * @param db A pointer to the db to select the value from.
 * @param key The key to select the value for.
 * @param value The buffer to store the selected value.
 * @param value_size The size of the value buffer.
 * @return true if the value was selected successfully, false otherwise.
 */
bool fossil_crabdb_select(fossil_crabdb_t* db, const char* key, char* value, size_t value_size);

/**
 * @brief Lists all key-value pairs in a CrabDB db.
 * @param db A pointer to the db to list the pairs from.
 * @param list_buffer The buffer to store the list of pairs.
 * @param buffer_size The size of the list buffer.
 * @return true if the pairs were listed successfully, false otherwise.
 */
bool fossil_crabdb_list(fossil_crabdb_t* db, char* list_buffer, size_t buffer_size);

/**
 * @brief Clears all key-value pairs from a CrabDB db.
 * @param db A pointer to the db to clear.
 * @return true if the db was cleared successfully, false otherwise.
 */
bool fossil_crabdb_clear(fossil_crabdb_t* db);
/**
 * @brief Shows the contents of a CrabDB db.
 * @param db A pointer to the db to show.
 * @return true if the db was shown successfully, false otherwise.
 */
bool fossil_crabdb_show(fossil_crabdb_t* db);

/**
 * @brief Drops a CrabDB db and clears its contents.
 * @param db A pointer to the db to drop.
 * @return true if the db was dropped successfully, false otherwise.
 */
bool fossil_crabdb_drop(fossil_crabdb_t* db);

/**
 * @brief Checks if a key exists in a CrabDB db.
 * @param db A pointer to the db to check.
 * @param key The key to check for existence.
 * @return true if the key exists in the db, false otherwise.
 */
bool fossil_crabdb_exist(fossil_crabdb_t* db, const char* key);

/**
 * @brief Backs up the contents of a CrabDB db to a specified file.
 * @param filename The name of the backup file.
 * @param db A pointer to the db to back up.
 * @return true if the backup was created successfully, false otherwise.
 */
bool fossil_crabdb_backup(const char* filename, fossil_crabdb_t* db);

/**
 * @brief Restores the contents of a CrabDB db from a backup file.
 * @param filename The name of the backup file.
 * @param db A pointer to the db to restore.
 * @return true if the db was restored successfully, false otherwise.
 */
bool fossil_crabdb_restore(const char* filename, fossil_crabdb_t* db);

/**
 * @brief Compacts the CrabDB db by removing deleted entries and reclaiming space.
 * @param db A pointer to the db to compact.
 * @return true if the db was compacted successfully, false otherwise.
 */
bool fossil_crabdb_compact(fossil_crabdb_t* db);

/**
 * @brief Renames a key in a CrabDB db.
 * @param db A pointer to the db.
 * @param old_key The old key to rename.
 * @param new_key The new key name.
 * @return true if the key was renamed successfully, false otherwise.
 */
bool fossil_crabdb_rename_key(fossil_crabdb_t* db, const char* old_key, const char* new_key);

/**
 * @brief Checks for duplicate values in the CrabDB db.
 * @param db A pointer to the db.
 * @return true if duplicates are found, false otherwise.
 */
bool fossil_crabdb_check_duplicates(fossil_crabdb_t* db);

/**
 * @brief Merges another CrabDB db into the current db.
 * @param dest_db A pointer to the destination db.
 * @param src_db A pointer to the source db to merge from.
 * @return true if the databases were merged successfully, false otherwise.
 */
bool fossil_crabdb_merge(fossil_crabdb_t* dest_db, fossil_crabdb_t* src_db);

/**
 * @brief Copies the contents of a CrabDB db to a new db.
 * @param src_db A pointer to the source db.
 * @param dest_db A pointer to the destination db.
 * @return true if the db was copied successfully, false otherwise.
 */
bool fossil_crabdb_copy(fossil_crabdb_t* src_db, fossil_crabdb_t* dest_db);

/**
 * @brief Calculates the memory usage of a CrabDB db.
 * @param db A pointer to the db.
 * @return The total memory usage of the db in bytes.
 */
size_t fossil_crabdb_memory_usage(fossil_crabdb_t* db);

/**
 * @brief Iterates over key-value pairs in a CrabDB db and applies a callback function.
 * @param db A pointer to the db.
 * @param callback The function to apply to each key-value pair.
 * @return true if the iteration was successful, false otherwise.
 */
bool fossil_crabdb_iterate(fossil_crabdb_t* db, void (*callback)(const char* key, const char* value));

/**
 * @brief Batch inserts multiple key-value pairs into a CrabDB db.
 * @param db A pointer to the db to insert the pairs into.
 * @param keys An array of keys.
 * @param values An array of values corresponding to the keys.
 * @param types An array of value types corresponding to the values.
 * @param count The number of key-value pairs to insert.
 * @return true if the pairs were inserted successfully, false otherwise.
 */
bool fossil_crabdb_batch_insert(fossil_crabdb_t* db, const char keys[][_FOSSIL_CRABDB_KEY_SIZE], const char values[][_FOSSIL_CRABDB_VAL_SIZE], fossil_crabdb_type_t types[], size_t count);

/**
 * @brief Batch deletes multiple key-value pairs from a CrabDB db by key.
 * @param db A pointer to the db to delete the pairs from.
 * @param keys An array of keys to delete.
 * @param count The number of keys to delete.
 * @return true if the pairs were deleted successfully, false otherwise.
 */
bool fossil_crabdb_batch_delete(fossil_crabdb_t* db, const char keys[][_FOSSIL_CRABDB_KEY_SIZE], size_t count);

/* ======================================================
 * CRAB DATABASE ALGORITMS
 * ------------------------------------------------------
 */

/**
 * @brief Searches for a key-value pair in a CrabDB db by key.
 * @param db A pointer to the db to search in.
 * @param key The key to search for.
 * @param value The buffer to store the value associated with the key.
 * @param value_size The size of the value buffer.
 * @return true if the key-value pair was found, false otherwise.
 */
bool fossil_crabdb_search_by_key(fossil_crabdb_t* db, const char* key, char* value, size_t value_size);

/**
 * @brief Searches for a key-value pair in a CrabDB db by value.
 * @param db A pointer to the db to search in.
 * @param value The value to search for.
 * @param key_buffer The buffer to store the key associated with the value.
 * @param key_buffer_size The size of the key buffer.
 * @return true if the key-value pair was found, false otherwise.
 */
bool fossil_crabdb_search_by_value(fossil_crabdb_t* db, const char* value, char* key_buffer, size_t key_buffer_size);

/**
 * @brief Sorts the key-value pairs in a CrabDB db by key.
 * @param db A pointer to the db to sort.
 * @return true if the db was sorted successfully, false otherwise.
 */
bool fossil_crabdb_sort_by_key(fossil_crabdb_t* db);

/**
 * @brief Sorts the key-value pairs in a CrabDB db by value.
 * @param db A pointer to the db to sort.
 * @return true if the db was sorted successfully, false otherwise.
 */
bool fossil_crabdb_sort_by_value(fossil_crabdb_t* db);

/* ======================================================
 * CRAB DATABASE SERIALIZATION
 * ------------------------------------------------------
 */

/**
 * @brief Encodes the contents of a CrabDB db and saves it to a file.
 * @param filename The name of the file to save the encoded data to.
 * @param db A pointer to the db to encode.
 * @return true if the db was encoded and saved successfully, false otherwise.
 */
bool fossil_crabdb_encode(const char* filename, fossil_crabdb_t* db);

/**
 * @brief Decodes the contents of a file and loads it into a CrabDB db.
 * @param filename The name of the file to load the data from.
 * @param db A pointer to the db to load the decoded data into.
 * @return true if the file was decoded and the db was loaded successfully, false otherwise.
 */
bool fossil_crabdb_decode(const char* filename, fossil_crabdb_t* db);

/**
 * @brief Counts the number of key-value pairs in a CrabDB db.
 * @param db A pointer to the db to count the pairs in.
 * @return The number of key-value pairs in the db.
 */
size_t fossil_crabdb_count(fossil_crabdb_t* db);

/**
 * @brief Exports the contents of a CrabDB db to a CSV file.
 * @param filename The name of the file to save the CSV data to.
 * @param db A pointer to the db to export.
 * @return true if the data was exported successfully, false otherwise.
 */
bool fossil_crabdb_export_csv(const char* filename, fossil_crabdb_t* db);

/**
 * @brief Imports key-value pairs from a CSV file into a CrabDB db.
 * @param filename The name of the CSV file to load data from.
 * @param db A pointer to the db to import the data into.
 * @return true if the data was imported successfully, false otherwise.
 */
bool fossil_crabdb_import_csv(const char* filename, fossil_crabdb_t* db);

/* ======================================================
 * CRAB DATABASE COMMANDS
 * ------------------------------------------------------
 */

/**
 * @brief Executes commands from the command line on a CrabDB db.
 * @param db A pointer to the db to execute commands on.
 * @return true if the commands were executed successfully, false otherwise.
 */
bool fossil_crabdb_commandline(fossil_crabdb_t* db);

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
            db = fossil_crabdb_create();
        }

        ~CrabDB() {
            fossil_crabdb_destroy(db);
        }

        bool insert(const std::string& key, const std::string& value, fossil_crabdb_type_t type) {
            return fossil_crabdb_insert(db, key.c_str(), value.c_str(), type);
        }

        bool update(const std::string& key, const std::string& value) {
            return fossil_crabdb_update(db, key.c_str(), value.c_str());
        }

        bool remove(const std::string& key) {
            return fossil_crabdb_delete(db, key.c_str());
        }

        bool select(const std::string& key, std::string& value) {
            char buffer[_FOSSIL_CRABDB_VAL_SIZE];
            bool success = fossil_crabdb_select(db, key.c_str(), buffer, sizeof(buffer));
            if (success) {
                value = buffer;
            }
            return success;
        }

        std::vector<std::pair<std::string, std::string>> list() {
            std::vector<std::pair<std::string, std::string>> pairs;
            char buffer[_FOSSIL_CRABDB_VAL_SIZE];
            bool success = fossil_crabdb_list(db, buffer, sizeof(buffer));
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
            return fossil_crabdb_clear(db);
        }

        bool show() {
            return fossil_crabdb_show(db);
        }

        bool drop() {
            return fossil_crabdb_drop(db);
        }

        bool exist(const std::string& key) {
            return fossil_crabdb_exist(db, key.c_str());
        }

        size_t count() {
            return fossil_crabdb_count(db);
        }

        bool encode(const std::string& filename) {
            return fossil_crabdb_encode(filename.c_str(), db);
        }

        bool decode(const std::string& filename) {
            return fossil_crabdb_decode(filename.c_str(), db);
        }

        bool exportCSV(const std::string& filename) {
            return fossil_crabdb_export_csv(filename.c_str(), db);
        }

        bool importCSV(const std::string& filename) {
            return fossil_crabdb_import_csv(filename.c_str(), db);
        }

        bool exec(const std::string& query) {
            return fossil_crabdb_exec(query.c_str(), db);
        }

        bool script(const std::string& filename) {
            return fossil_crabdb_script(filename.c_str(), db);
        }

        bool commandline() {
            return fossil_crabdb_commandline(db);
        }

    private:
        fossil_crabdb_t* db;
    };
} // namespace fossil
#endif

#endif /* FOSSIL_CRABDB_FRAMEWORK_H */

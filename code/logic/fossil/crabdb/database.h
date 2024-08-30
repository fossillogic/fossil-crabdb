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

//
// DATABASE TYPES
//

enum {
    CRABDB_OK = 0,
    MAX_KEY_LENGTH = 256,
    MAX_VALUE_LENGTH = 1024,
    MAX_LINE_LENGTH = 2048,
    MAX_VARIABLES = 256,
    MAX_COMMAND_LENGTH = 1024,
    MAX_PATH_LENGTH = 1024,
    MAX_ARG_LENGTH = 256
};

// Type identifiers
typedef enum {
    TYPE_UNKNOWN,
    TYPE_U8,
    TYPE_U16,
    TYPE_U32,
    TYPE_U64,
    TYPE_I8,
    TYPE_I16,
    TYPE_I32,
    TYPE_I64,
    TYPE_H8,
    TYPE_H16,
    TYPE_H32,
    TYPE_H64,
    TYPE_O8,
    TYPE_O16,
    TYPE_O32,
    TYPE_O64,
    TYPE_F32,
    TYPE_F64,
    TYPE_CSTR,
    TYPE_BOOL,
    TYPE_CHAR
} value_type_t;

/**
 * @brief Error codes for the CrabDB database.
 */
typedef struct fossil_crabdb_keyvalue_t {
    char key[MAX_KEY_LENGTH];
    char value[MAX_VALUE_LENGTH];
    struct fossil_crabdb_keyvalue_t *prev;
    struct fossil_crabdb_keyvalue_t *next;
} fossil_crabdb_keyvalue_t;

typedef struct fossil_crabdb_namespace_t {
    char name[MAX_KEY_LENGTH];
    fossil_crabdb_keyvalue_t *key_values;
    struct fossil_crabdb_namespace_t *prev;
    struct fossil_crabdb_namespace_t *next;
    struct fossil_crabdb_namespace_t *subNamespacesHead;
} fossil_crabdb_namespace_t;

typedef struct fossil_crabdb_t {
    fossil_crabdb_namespace_t *namespaceHead;
} fossil_crabdb_t;

typedef struct fossil_crabdb_cache_entry_t {
    char key[MAX_KEY_LENGTH];
    char value[MAX_VALUE_LENGTH];
    struct fossil_crabdb_cache_entry_t *next;
} fossil_crabdb_cache_entry_t;

typedef struct fossil_crabdb_cache_t {
    fossil_crabdb_cache_entry_t *head;
} fossil_crabdb_cache_t;

//
// DATABASE OPERATIONS
//

/**
 * @brief Creates a new CrabDB database.
 * @return A pointer to the newly created database.
 */
fossil_crabdb_t* fossil_crabdb_create(void);

/**
 * @brief Adds a new namespace to the CrabDB database.
 * @param db The CrabDB database.
 * @param name The name of the namespace to add.
 * @return 0 if successful, -1 otherwise.
 */
int fossil_crabdb_add_namespace(fossil_crabdb_t *db, const char *name);

/**
 * @brief Finds a namespace in the CrabDB database.
 * @param db The CrabDB database.
 * @param name The name of the namespace to find.
 * @return A pointer to the found namespace, or NULL if not found.
 */
fossil_crabdb_namespace_t* fossil_crabdb_find_namespace(fossil_crabdb_t *db, const char *name);

/**
 * @brief Adds a key-value pair to a namespace in the CrabDB database.
 * @param ns The namespace to add the key-value pair to.
 * @param key The key of the pair.
 * @param value The value of the pair.
 * @return 0 if successful, -1 otherwise.
 */
int fossil_crabdb_add_key_value(fossil_crabdb_namespace_t *ns, const char *key, const char *value);

/**
 * @brief Gets the value associated with a key in a namespace of the CrabDB database.
 * @param ns The namespace to search for the key-value pair.
 * @param key The key to search for.
 * @return The value associated with the key, or NULL if not found.
 */
const char* fossil_crabdb_get_value(fossil_crabdb_namespace_t *ns, const char *key);

/**
 * @brief Deletes a key-value pair from a namespace in the CrabDB database.
 * @param ns The namespace to delete the key-value pair from.
 * @param key The key of the pair to delete.
 * @return 0 if successful, -1 otherwise.
 */
int fossil_crabdb_delete_key_value(fossil_crabdb_namespace_t *ns, const char *key);

/**
 * @brief Deletes a namespace from the CrabDB database.
 * @param db The CrabDB database.
 * @param name The name of the namespace to delete.
 * @return 0 if successful, -1 otherwise.
 */
int fossil_crabdb_delete_namespace(fossil_crabdb_t *db, const char *name);

/**
 * @brief Renames a namespace in the CrabDB database.
 * @param db The CrabDB database.
 * @param old_name The current name of the namespace.
 * @param new_name The new name of the namespace.
 * @return 0 if successful, -1 otherwise.
 */
int fossil_crabdb_rename_namespace(fossil_crabdb_t *db, const char *old_name, const char *new_name);

/**
 * @brief Updates the value associated with a key in a namespace of the CrabDB database.
 * @param ns The namespace to update the key-value pair in.
 * @param key The key of the pair to update.
 * @param new_value The new value to associate with the key.
 * @return 0 if successful, -1 otherwise.
 */
int fossil_crabdb_update_key_value(fossil_crabdb_namespace_t *ns, const char *key, const char *new_value);

/**
 * @brief Retrieves all keys in a namespace of the CrabDB database.
 * @param ns The namespace to retrieve keys from.
 * @param keys Array to store the retrieved keys.
 * @param max_keys The maximum number of keys to retrieve.
 * @return The number of keys retrieved, or -1 if an error occurred.
 */
int fossil_crabdb_get_all_keys(fossil_crabdb_namespace_t *ns, char **keys, size_t max_keys);

/**
 * @brief Exports the CrabDB database to a file.
 * @param db The CrabDB database.
 * @param filename The name of the file to export to.
 * @return 0 if successful, -1 otherwise.
 */
int fossil_crabdb_export(fossil_crabdb_t *db, const char *filename);

/**
 * @brief Imports a CrabDB database from a file.
 * @param db The CrabDB database to import into.
 * @param filename The name of the file to import from.
 * @return 0 if successful, -1 otherwise.
 */
int fossil_crabdb_import(fossil_crabdb_t *db, const char *filename);

//
// DATABASE CACHE
//

/**
 * @brief Creates a new CrabDB cache.
 * @return A pointer to the newly created cache.
 */
fossil_crabdb_cache_t* fossil_crabdb_create_cache(void);

/**
 * @brief Adds a key-value pair to the CrabDB cache.
 * @param cache The CrabDB cache.
 * @param key The key of the pair.
 * @param value The value of the pair.
 */
void fossil_crabdb_cache_add(fossil_crabdb_cache_t *cache, const char *key, const char *value);

/**
 * @brief Gets the value associated with a key in the CrabDB cache.
 * @param cache The CrabDB cache.
 * @param key The key to search for.
 * @return The value associated with the key, or NULL if not found.
 */
const char* fossil_crabdb_cache_get(fossil_crabdb_cache_t *cache, const char *key);

/**
 * @brief Frees the memory used by a CrabDB cache.
 * @param cache The CrabDB cache to free.
 */
void fossil_crabdb_cache_free(fossil_crabdb_cache_t *cache);

//
// DATABASE QUERY AND SCRIPTING
//

/**
 * @brief Saves the CrabDB database to a file.
 * @param db The CrabDB database.
 * @param filename The name of the file to save to.
 * @return 0 if successful, -1 otherwise.
 */
int fossil_crabdb_save(fossil_crabdb_t *db, const char *filename);

/**
 * @brief Loads a CrabDB database from a file.
 * @param db The CrabDB database.
 * @param filename The name of the file to load from.
 * @return 0 if successful, -1 otherwise.
 */
int fossil_crabdb_load(fossil_crabdb_t *db, const char *filename);

/**
 * @brief Executes a query on the CrabDB database.
 * @param db The CrabDB database.
 * @param query The query to execute.
 * @return 0 if successful, -1 otherwise.
 */
int fossil_crabdb_execute_query(fossil_crabdb_t *db, const char *query);

/**
 * @brief Executes a script on the CrabDB database.
 * @param db The CrabDB database.
 * @param script The script to execute.
 * @return 0 if successful, -1 otherwise.
 */
int fossil_crabdb_execute_script(fossil_crabdb_t *db, const char *script);

#ifdef __cplusplus
}
#endif

#ifdef __cplusplus

#include <string>
#include <vector>

namespace fossil {

    /**
     * @brief The CrabDB class provides an interface to interact with the CrabDB database.
     */
    class CrabDB {
    public:
        /**
         * @brief Constructs a new CrabDB object.
         */
        CrabDB() {
            db = fossil_crabdb_create();
        }

        /**
         * @brief Destroys the CrabDB object and frees the associated resources.
         */
        ~CrabDB() {
            fossil_crabdb_delete(db);
        }

        /**
         * @brief Adds a new namespace to the CrabDB database.
         * @param name The name of the namespace to add.
         * @return true if the namespace was added successfully, false otherwise.
         */
        bool addNamespace(const std::string& name) {
            return fossil_crabdb_add_namespace(db, name.c_str()) == CRABDB_OK;
        }

        /**
         * @brief Deletes a namespace from the CrabDB database.
         * @param name The name of the namespace to delete.
         * @return true if the namespace was deleted successfully, false otherwise.
         */
        bool deleteNamespace(const std::string& name) {
            return fossil_crabdb_delete_namespace(db, name.c_str()) == CRABDB_OK;
        }

        /**
         * @brief Renames a namespace in the CrabDB database.
         * @param oldName The current name of the namespace.
         * @param newName The new name of the namespace.
         * @return true if the namespace was renamed successfully, false otherwise.
         */
        bool renameNamespace(const std::string& oldName, const std::string& newName) {
            return fossil_crabdb_rename_namespace(db, oldName.c_str(), newName.c_str()) == CRABDB_OK;
        }

        /**
         * @brief Adds a key-value pair to a namespace in the CrabDB database.
         * @param namespaceName The name of the namespace to add the key-value pair to.
         * @param key The key of the pair.
         * @param value The value of the pair.
         * @return true if the key-value pair was added successfully, false otherwise.
         */
        bool addKeyValue(const std::string& namespaceName, const std::string& key, const std::string& value) {
            fossil_crabdb_namespace_t* ns = fossil_crabdb_find_namespace(db, namespaceName.c_str());
            if (ns == nullptr) {
                return false;
            }
            return fossil_crabdb_add_key_value(ns, key.c_str(), value.c_str()) == CRABDB_OK;
        }

        /**
         * @brief Deletes a key-value pair from a namespace in the CrabDB database.
         * @param namespaceName The name of the namespace to delete the key-value pair from.
         * @param key The key of the pair to delete.
         * @return true if the key-value pair was deleted successfully, false otherwise.
         */
        bool deleteKeyValue(const std::string& namespaceName, const std::string& key) {
            fossil_crabdb_namespace_t* ns = fossil_crabdb_find_namespace(db, namespaceName.c_str());
            if (ns == nullptr) {
                return false;
            }
            return fossil_crabdb_delete_key_value(ns, key.c_str()) == CRABDB_OK;
        }

        /**
         * @brief Updates the value associated with a key in a namespace of the CrabDB database.
         * @param namespaceName The name of the namespace to update the key-value pair in.
         * @param key The key of the pair to update.
         * @param newValue The new value to associate with the key.
         * @return true if the key-value pair was updated successfully, false otherwise.
         */
        bool updateKeyValue(const std::string& namespaceName, const std::string& key, const std::string& newValue) {
            fossil_crabdb_namespace_t* ns = fossil_crabdb_find_namespace(db, namespaceName.c_str());
            if (ns == nullptr) {
                return false;
            }
            return fossil_crabdb_update_key_value(ns, key.c_str(), newValue.c_str()) == CRABDB_OK;
        }

        /**
         * @brief Gets the value associated with a key in a namespace of the CrabDB database.
         * @param namespaceName The name of the namespace to search for the key-value pair.
         * @param key The key to search for.
         * @return The value associated with the key, or an empty string if not found.
         */
        std::string getValue(const std::string& namespaceName, const std::string& key) {
            fossil_crabdb_namespace_t* ns = fossil_crabdb_find_namespace(db, namespaceName.c_str());
            if (ns == nullptr) {
                return "";
            }
            const char* value = fossil_crabdb_get_value(ns, key.c_str());
            if (value == nullptr) {
                return "";
            }
            return value;
        }

        /**
         * @brief Retrieves all keys in a namespace of the CrabDB database.
         * @param namespaceName The name of the namespace to retrieve keys from.
         * @return A vector containing all the keys in the namespace, or an empty vector if an error occurred.
         */
        std::vector<std::string> getAllKeys(const std::string& namespaceName) {
            fossil_crabdb_namespace_t* ns = fossil_crabdb_find_namespace(db, namespaceName.c_str());
            if (ns == nullptr) {
                return {};
            }
            char** keys = nullptr;
            int numKeys = fossil_crabdb_get_all_keys(ns, keys, 0);
            if (numKeys <= 0) {
                return {};
            }
            std::vector<std::string> keyList;
            keyList.reserve(numKeys);
            for (int i = 0; i < numKeys; i++) {
                keyList.push_back(keys[i]);
            }
            return keyList;
        }

    private:
        fossil_crabdb_t* db;
    };

} // namespace fossil
#endif

#endif /* FOSSIL_CRABDB_FRAMEWORK_H */

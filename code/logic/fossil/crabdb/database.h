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

/**
 * @brief Error codes for the fossil_crabdb framework.
 */
typedef enum {
    CRABDB_OK = 0, /**< Operation completed successfully */
    CRABDB_ERR_MEM, /**< Memory allocation error */
    CRABDB_ERR_IO,            // Input/output error (e.g., file handling)
    CRABDB_ERR_NS_NOT_FOUND, /**< Namespace not found */
    CRABDB_ERR_NS_EXISTS, /**< Namespace already exists */
    CRABDB_ERR_SUB_NS_NOT_FOUND, /**< Sub-namespace not found */
    CRABDB_ERR_INVALID_ARG, /**< Invalid argument */
    CRABDB_ERR_COPY_FAILED, /**< Copy failed */
    CRABDB_ERR_KEY_ALREADY_EXISTS, /**< Key already exists */
    CRABDB_ERR_SUB_NS_EXISTS, /**< Sub-namespace already exists */
    CRABDB_ERR_BACKUP_FAILED, /**< Backup failed */
    CRABDB_ERR_RESTORE_FAILED, /**< Restore failed */
    CRABDB_ERR_DESERIALIZE_FAILED, /**< Deserialization failed */
    CRABDB_ERR_KEY_NOT_FOUND, /**< Key not found */
    CRABDB_ERR_INVALID_QUERY, /**< Invalid query */
    CRABDB_ERR_INVALID_KEY, /**< Invalid key */
    CRABDB_ERR_INVALID_VALUE, /**< Invalid value */
    CRABDB_ERR_CONCURRENT_ACCESS /**< Concurrent access error */
} fossil_crabdb_error_t;

/**
 * @brief Key-value pair structure.
 */
typedef struct fossil_crabdb_keyvalue_t {
    char *key; /**< Key of the key-value pair */
    char *value; /**< Value of the key-value pair */
    struct fossil_crabdb_keyvalue_t *next; /**< Pointer to the next key-value pair */
} fossil_crabdb_keyvalue_t;

/**
 * @brief Namespace structure.
 */
typedef struct fossil_crabdb_namespace_t {
    char *name; /**< Name of the namespace */
    struct fossil_crabdb_namespace_t *sub_namespaces; /**< Pointer to the sub-namespaces */
    size_t sub_namespace_count; /**< Number of sub-namespaces */
    struct fossil_crabdb_namespace_t *next; /**< Pointer to the next namespace */
    fossil_crabdb_keyvalue_t *data; /**< Linked list of key-value pairs */
} fossil_crabdb_namespace_t;

/**
 * @brief Database structure.
 */
typedef struct {
    fossil_crabdb_namespace_t *namespaces; /**< Pointer to the namespaces */
} fossil_crabdb_t;

//
// DATABASE OPERATIONS
//

/**
 * @brief Create a new fossil_crabdb_t database.
 * 
 * @return Pointer to the newly created fossil_crabdb_t database.
 */
fossil_crabdb_t* fossil_crabdb_create(void);

/**
 * @brief Erase the fossil_crabdb_t database.
 * 
 * @param db Pointer to the fossil_crabdb_t database to erase.
 */
void fossil_crabdb_erase(fossil_crabdb_t *db);

/**
 * @brief Insert data into a namespace.
 * 
 * @param db Pointer to the fossil_crabdb_t database.
 * @param namespace_name Name of the namespace.
 * @param key Key of the data to insert.
 * @param value Value of the data to insert.
 * @return Error code indicating the result of the operation.
 */
fossil_crabdb_error_t fossil_crabdb_insert(fossil_crabdb_t *db, const char *namespace_name, const char *key, const char *value);

/**
 * @brief Get data from a namespace.
 * 
 * @param db Pointer to the fossil_crabdb_t database.
 * @param namespace_name Name of the namespace.
 * @param key Key of the data to get.
 * @param value Pointer to store the retrieved value.
 * @return Error code indicating the result of the operation.
 */
fossil_crabdb_error_t fossil_crabdb_get(fossil_crabdb_t *db, const char *namespace_name, const char *key, char **value);

/**
 * @brief Update data in a namespace.
 * 
 * @param db Pointer to the fossil_crabdb_t database.
 * @param namespace_name Name of the namespace.
 * @param key Key of the data to update.
 * @param value New value for the data.
 * @return Error code indicating the result of the operation.
 */
fossil_crabdb_error_t fossil_crabdb_update(fossil_crabdb_t *db, const char *namespace_name, const char *key, const char *value);

/**
 * @brief Delete data from a namespace.
 * 
 * @param db Pointer to the fossil_crabdb_t database.
 * @param namespace_name Name of the namespace.
 * @param key Key of the data to delete.
 * @return Error code indicating the result of the operation.
 */
fossil_crabdb_error_t fossil_crabdb_delete(fossil_crabdb_t *db, const char *namespace_name, const char *key);

//
// DATA STORAGE
//

/**
 * @brief Serializes the database to a file.
 *
 * @param db The database to serialize.
 * @param filename The name of the file to serialize to.
 * @return CRABDB_OK on success, or an error code on failure.
 */
fossil_crabdb_error_t fossil_crabdb_serialize_to_file(fossil_crabdb_t *db, const char *filename);

/**
 * @brief Deserializes the database from a file.
 *
 * @param db The database to deserialize into.
 * @param filename The name of the file to deserialize from.
 * @return CRABDB_OK on success, or an error code on failure.
 */
fossil_crabdb_error_t fossil_crabdb_deserialize_from_file(fossil_crabdb_t *db, const char *filename);

/**
 * @brief Saves the database to a file.
 *
 * @param db The database to save.
 * @param filename The name of the file to save to.
 * @return CRABDB_OK on success, or an error code on failure.
 */
int fossil_crabdb_save_to_file(fossil_crabdb_t *db, const char *filename);

/**
 * @brief Loads the database from a file.
 *
 * @param db The database to load into.
 * @param filename The name of the file to load from.
 * @return CRABDB_OK on success, or an error code on failure.
 */
int fossil_crabdb_load_from_file(fossil_crabdb_t *db, const char *filename);

//
// DATABASE NAMESPACES
//

/**
 * @brief Create a new namespace.
 * 
 * @param db Pointer to the fossil_crabdb_t database.
 * @param namespace_name Name of the new namespace.
 * @return Error code indicating the result of the operation.
 */
fossil_crabdb_error_t fossil_crabdb_create_namespace(fossil_crabdb_t *db, const char *namespace_name);

/**
 * @brief Create a new sub-namespace.
 * 
 * @param db Pointer to the fossil_crabdb_t database.
 * @param namespace_name Name of the parent namespace.
 * @param sub_namespace_name Name of the new sub-namespace.
 * @return Error code indicating the result of the operation.
 */
fossil_crabdb_error_t fossil_crabdb_create_sub_namespace(fossil_crabdb_t *db, const char *namespace_name, const char *sub_namespace_name);

/**
 * @brief Erase a namespace.
 * 
 * @param db Pointer to the fossil_crabdb_t database.
 * @param namespace_name Name of the namespace to erase.
 * @return Error code indicating the result of the operation.
 */
fossil_crabdb_error_t fossil_crabdb_erase_namespace(fossil_crabdb_t *db, const char *namespace_name);

/**
 * @brief Erase a sub-namespace.
 * 
 * @param db Pointer to the fossil_crabdb_t database.
 * @param namespace_name Name of the parent namespace.
 * @param sub_namespace_name Name of the sub-namespace to erase.
 * @return Error code indicating the result of the operation.
 */
fossil_crabdb_error_t fossil_crabdb_erase_sub_namespace(fossil_crabdb_t *db, const char *namespace_name, const char *sub_namespace_name);

/**
 * @brief List all namespaces.
 * 
 * @param db Pointer to the fossil_crabdb_t database.
 * @param namespaces Pointer to store the list of namespace names.
 * @param count Pointer to store the number of namespaces.
 * @return Error code indicating the result of the operation.
 */
fossil_crabdb_error_t fossil_crabdb_list_namespaces(fossil_crabdb_t *db, char ***namespaces, size_t *count);

/**
 * @brief List all keys in a namespace.
 * 
 * @param db Pointer to the fossil_crabdb_t database.
 * @param namespace_name Name of the namespace.
 * @param keys Pointer to store the list of keys.
 * @param count Pointer to store the number of keys.
 * @return Error code indicating the result of the operation.
 */
fossil_crabdb_error_t fossil_crabdb_list_namespaces_keys(fossil_crabdb_t *db, const char *namespace_name, char ***keys, size_t *count);

/**
 * @brief Get statistics for a namespace.
 * 
 * @param db Pointer to the fossil_crabdb_t database.
 * @param namespace_name Name of the namespace to get statistics for.
 * @param key_count Pointer to store the number of keys.
 * @param sub_namespace_count Pointer to store the number of sub-namespaces.
 * @return Error code indicating the result of the operation.
 */
fossil_crabdb_error_t fossil_crabdb_get_namespace_stats(fossil_crabdb_t *db, const char *namespace_name, size_t *key_count, size_t *sub_namespace_count);

/**
 * @brief Create a deep copy of a namespace.
 * 
 * @param original Pointer to the original namespace to copy.
 * @return Pointer to the copied namespace.
 */
fossil_crabdb_namespace_t* fossil_crabdb_copy_namespace(const fossil_crabdb_namespace_t *original);

/**
 * @brief Rename a namespace.
 * 
 * @param db Pointer to the fossil_crabdb_t database.
 * @param old_namespace_name Current name of the namespace.
 * @param new_namespace_name New name for the namespace.
 * @return Error code indicating the result of the operation.
 */
fossil_crabdb_error_t fossil_crabdb_rename_namespace(fossil_crabdb_t *db, const char *old_namespace_name, const char *new_namespace_name);

//
// DATABASE QUERIES
//

/**
 * @brief Execute a custom query.
 * 
 * @param db Pointer to the fossil_crabdb_t database.
 * @param query Custom query to execute.
 * @return Error code indicating the result of the operation.
 */
fossil_crabdb_error_t fossil_crabdb_execute_query(fossil_crabdb_t *db, const char *query);

#ifdef __cplusplus
}
#endif

#ifdef __cplusplus
#include <string>
#include <vector>

namespace fossil {

    /**
     * @brief Wrapper function to create a new database.
     * 
     * @return Pointer to the newly created fossil_crabdb_t database.
     */
    fossil_crabdb_t* crabdb_create_database(void) {
        return fossil_crabdb_create();
    }

    /**
     * @brief Wrapper function to erase a database.
     * 
     * @param db Pointer to the fossil_crabdb_t database to erase.
     */
    void crabdb_erase_database(fossil_crabdb_t *db) {
        fossil_crabdb_erase(db);
    }

    /**
     * @brief Wrapper function to insert data into a namespace.
     * 
     * @param db Pointer to the fossil_crabdb_t database.
     * @param namespace_name Name of the namespace.
     * @param key Key of the data to insert.
     * @param value Value of the data to insert.
     * @return Error code indicating the result of the operation.
     */
    fossil_crabdb_error_t crabdb_insert_data(fossil_crabdb_t *db, const std::string& namespace_name, const std::string& key, const std::string& value) {
        return fossil_crabdb_insert(db, namespace_name.c_str(), key.c_str(), value.c_str());
    }

    /**
     * @brief Wrapper function to get data from a namespace.
     * 
     * @param db Pointer to the fossil_crabdb_t database.
     * @param namespace_name Name of the namespace.
     * @param key Key of the data to get.
     * @param value Pointer to store the retrieved value.
     * @return Error code indicating the result of the operation.
     */
    fossil_crabdb_error_t crabdb_get_data(fossil_crabdb_t *db, const std::string& namespace_name, const std::string& key, std::string& value) {
        char* retrieved_value;
        fossil_crabdb_error_t result = fossil_crabdb_get(db, namespace_name.c_str(), key.c_str(), &retrieved_value);
        if (result == FOSSIL_CRABDB_SUCCESS) {
            value = retrieved_value;
        }
        return result;
    }

    /**
     * @brief Wrapper function to update data in a namespace.
     * 
     * @param db Pointer to the fossil_crabdb_t database.
     * @param namespace_name Name of the namespace.
     * @param key Key of the data to update.
     * @param value New value for the data.
     * @return Error code indicating the result of the operation.
     */
    fossil_crabdb_error_t crabdb_update_data(fossil_crabdb_t *db, const std::string& namespace_name, const std::string& key, const std::string& value) {
        return fossil_crabdb_update(db, namespace_name.c_str(), key.c_str(), value.c_str());
    }

    /**
     * @brief Wrapper function to delete data from a namespace.
     * 
     * @param db Pointer to the fossil_crabdb_t database.
     * @param namespace_name Name of the namespace.
     * @param key Key of the data to delete.
     * @return Error code indicating the result of the operation.
     */
    fossil_crabdb_error_t crabdb_delete_data(fossil_crabdb_t *db, const std::string& namespace_name, const std::string& key) {
        return fossil_crabdb_delete(db, namespace_name.c_str(), key.c_str());
    }

    /**
     * @brief Wrapper function to execute a custom query.
     * 
     * @param db Pointer to the fossil_crabdb_t database.
     * @param query Custom query to execute.
     * @return Error code indicating the result of the operation.
     */
    fossil_crabdb_error_t crabdb_execute_query(fossil_crabdb_t *db, const std::string& query) {
        return fossil_crabdb_execute_query(db, query.c_str());
    }

    class CrabDB {
    public:
        CrabDB() {
            db = crabdb_create_database();
        }

        ~CrabDB() {
            crabdb_erase_database(db);
        }

        fossil_crabdb_error_t insertData(const std::string& namespace_name, const std::string& key, const std::string& value) {
            return crabdb_insert_data(db, namespace_name, key, value);
        }

        fossil_crabdb_error_t getData(const std::string& namespace_name, const std::string& key, std::string& value) {
            return crabdb_get_data(db, namespace_name, key, value);
        }

        fossil_crabdb_error_t updateData(const std::string& namespace_name, const std::string& key, const std::string& value) {
            return crabdb_update_data(db, namespace_name, key, value);
        }

        fossil_crabdb_error_t deleteData(const std::string& namespace_name, const std::string& key) {
            return crabdb_delete_data(db, namespace_name, key);
        }

        fossil_crabdb_error_t executeQuery(const std::string& query) {
            return crabdb_execute_query(db, query);
        }
}
#endif

#endif /* FOSSIL_CRABDB_FRAMEWORK_H */

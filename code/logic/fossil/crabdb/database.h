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

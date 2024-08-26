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
#ifndef FOSSIL_CRABDB_FRAMEWORK_H
#define FOSSIL_CRABDB_FRAMEWORK_H

#ifdef __cplusplus
namespace fossil {
#endif

#include "database.h"

/**
 * @brief Blue fossil_crabdb_t Database Library
 * 
 * This library provides functions to manage a Blue fossil_crabdb_t database, a lightweight and efficient key-value store.
 * It supports creating databases, managing namespaces, and performing CRUD (Create, Read, Update, Delete) operations on data.
 * 
 * Key Features:
 * - Create and manage databases.
 * - fossil_crabdb_namespace_t management within the database.
 * - Insert, update, search, and remove data within namespaces.
 * - Execute custom queries on the database.
 * 
 * Usage:
 * 
 * 1. Creating a database:
 *    @code
 *    fossil_crabdb_t_t* db = fossil_crabdb_create("my_database");
 *    if (db != NULL) {
 *        // Database created successfully
 *    }
 *    @endcode
 * 
 * 2. Creating a namespace:
 *    @code
 *    int32_t result = fossil_crabdb_create_namespace(db, "my_namespace");
 *    if (result == 1) {
 *        // fossil_crabdb_namespace_t created successfully
 *    } else if (result == 0) {
 *        // fossil_crabdb_namespace_t already exists
 *    } else {
 *        // fossil_crabdb_namespace_t creation failed
 *    }
 *    @endcode
 * 
 * 3. Inserting data:
 *    @code
 *    result = fossil_crabdb_insert_data(db, "my_namespace", "key1", "value1", "string");
 *    if (result == 1) {
 *        // Data inserted successfully
 *    } else if (result == 0) {
 *        // fossil_crabdb_namespace_t or key already exists
 *    } else {
 *        // Data insertion failed
 *    }
 *    @endcode
 * 
 * 4. Updating data:
 *    @code
 *    result = fossil_crabdb_update_data(db, "my_namespace", "key1", "new_value");
 *    if (result == 1) {
 *        // Data updated successfully
 *    } else if (result == 0) {
 *        // fossil_crabdb_namespace_t or key does not exist
 *    } else {
 *        // Data update failed
 *    }
 *    @endcode
 * 
 * 5. Searching for data:
 *    @code
 *    fson_value* value = fossil_crabdb_t_search_data(db, "my_namespace", "key1");
 *    if (value != NULL) {
 *        // Data found
 *    } else {
 *        // Data not found
 *    }
 *    @endcode
 * 
 * 6. Removing data:
 *    @code
 *    result = fossil_crabdb_t_remove_data(db, "my_namespace", "key1");
 *    if (result == 1) {
 *        // Data removed successfully
 *    } else if (result == 0) {
 *        // fossil_crabdb_namespace_t or key does not exist
 *    } else {
 *        // Data removal failed
 *    }
 *    @endcode
 * 
 * 7. Displaying namespace contents:
 *    @code
 *    fossil_crabdb_t_display_namespace(db, "my_namespace");
 *    @endcode
 * 
 * 8. Executing custom queries:
 *    @code
 *    fossil_crabdb_execute_query(db, "SELECT * FROM my_namespace WHERE key='key1'");
 *    @endcode
 * 
 */

#ifdef __cplusplus
} // namespae fossil
#endif

#endif /* FOSSIL_CRABDB_FRAMEWORK_H */

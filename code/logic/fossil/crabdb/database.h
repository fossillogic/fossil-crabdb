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

typedef enum {
    CRABDB_OK = 0,
    MAX_KEY_LENGTH = 256,
    MAX_VALUE_LENGTH = 1024,
    MAX_LINE_LENGTH = 2048,
    MAX_ARG_LENGTH = 256
};

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
    fossil_crabdb_keyvalue_t *keyValueHead;
    struct fossil_crabdb_namespace_t *prev;
    struct fossil_crabdb_namespace_t *next;
    struct fossil_crabdb_namespace_t *subNamespacesHead;
} fossil_crabdb_namespace_t;

typedef struct fossil_crabdb_t {
    fossil_crabdb_namespace_t *namespaceHead;
    // Add more fields for caching or other metadata if needed
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

#endif /* FOSSIL_CRABDB_FRAMEWORK_H */

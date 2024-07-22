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
#ifndef FOSSIL_CRABDB_INTERNAL_H
#define FOSSIL_CRABDB_INTERNAL_H

#include <stddef.h>
#include <stdlib.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Error codes for the crabdb framework.
 */
typedef enum {
    CRABDB_OK = 0, /**< Operation completed successfully */
    CRABDB_ERR_MEM, /**< Memory allocation error */
    CRABDB_ERR_NS_NOT_FOUND, /**< Namespace not found */
    CRABDB_ERR_NS_EXISTS, /**< Namespace already exists */
    CRABDB_ERR_SUB_NS_NOT_FOUND, /**< Sub-namespace not found */
    CRABDB_ERR_SUB_NS_EXISTS, /**< Sub-namespace already exists */
    CRABDB_ERR_KEY_NOT_FOUND, /**< Key not found */
    CRABDB_ERR_INVALID_QUERY /**< Invalid query */
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

/**
 * @brief Allocate memory.
 * 
 * @param size Size of the memory to allocate.
 * @return Pointer to the allocated memory.
 */
typedef void * crabdb_memory_t;

/**
 * @brief Allocate memory.
 * 
 * @param size Size of the memory to allocate.
 * @return Pointer to the allocated memory.
 */
crabdb_memory_t fossil_crabdb_alloc(size_t size);

/**
 * @brief Reallocate memory.
 * 
 * @param ptr Pointer to the memory to reallocate.
 * @param size Size of the memory to reallocate.
 * @return Pointer to the reallocated memory.
 */
crabdb_memory_t fossil_crabdb_realloc(crabdb_memory_t ptr, size_t size);

/**
 * @brief Free memory.
 * 
 * @param ptr Pointer to the memory to free.
 */
void fossil_crabdb_free(crabdb_memory_t ptr);

/**
 * @brief Duplicate a string.
 * 
 * @param str String to duplicate.
 * @return Pointer to the duplicated string.
 */
char* fossil_crabdb_strdup(const char* str);

#ifdef __cplusplus
}
#endif

#endif /* FOSSIL_CRABDB_FRAMEWORK_H */

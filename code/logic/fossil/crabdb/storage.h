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
#ifndef FOSSIL_CRABDB_STORAGE_H
#define FOSSIL_CRABDB_STORAGE_H

#include <stdbool.h>
#include <stddef.h>
#include <time.h>

#ifdef __cplusplus
extern "C" {
#endif

// *****************************************************************************
// BlueCrab Unified Storage API - Prototypes
// *****************************************************************************

// ===========================================================
// Enumerations
// ===========================================================

typedef enum {
    FOSSIL_STORAGE_BACKEND_MYSHELL,   /**< SQL-like interface */
    FOSSIL_STORAGE_BACKEND_NOSHELL,   /**< Raw key-value */
    FOSSIL_STORAGE_BACKEND_TIMESHELL, /**< Time-series */
    FOSSIL_STORAGE_BACKEND_CACHESHELL,/**< In-memory cache */
    FOSSIL_STORAGE_BACKEND_FILESHELL  /**< File system storage */
} fossil_bluecrab_storage_backend_t;

typedef enum {
    FOSSIL_STORAGE_OK = 0,
    FOSSIL_STORAGE_ERROR = 1,
    FOSSIL_STORAGE_NOT_FOUND = 2,
    FOSSIL_STORAGE_FULL = 3,
    FOSSIL_STORAGE_INVALID = 4
} fossil_bluecrab_storage_error_t;

// ===========================================================
// Unified Handle
// ===========================================================

typedef struct fossil_bluecrab_storage fossil_bluecrab_storage_t;

// ===========================================================
// Lifecycle
// ===========================================================

/**
 * @brief Open a unified storage handle.
 *
 * @param backend   Backend type (myshell, noshell, etc).
 * @param uri       Path, DB file, or identifier depending on backend.
 * @return          Pointer to storage handle, NULL on error.
 */
fossil_bluecrab_storage_t* fossil_bluecrab_storage_open(
    fossil_bluecrab_storage_backend_t backend,
    const char *uri);

/**
 * @brief Close a unified storage handle.
 *
 * @param handle    Storage handle.
 */
void fossil_bluecrab_storage_close(fossil_bluecrab_storage_t *handle);

// ===========================================================
// Basic Key/Value Operations
// ===========================================================

/**
 * @brief Insert or update a key-value pair.
 *
 * @param handle    Storage handle.
 * @param key       Null-terminated key.
 * @param value     Null-terminated value.
 * @return          Storage status code.
 */
fossil_bluecrab_storage_error_t fossil_bluecrab_storage_set(
    fossil_bluecrab_storage_t *handle,
    const char *key,
    const char *value);

/**
 * @brief Retrieve a value for a given key.
 *
 * @param handle    Storage handle.
 * @param key       Null-terminated key.
 * @param out_buf   Buffer to store result.
 * @param buf_size  Size of buffer.
 * @return          Storage status code.
 */
fossil_bluecrab_storage_error_t fossil_bluecrab_storage_get(
    fossil_bluecrab_storage_t *handle,
    const char *key,
    char *out_buf,
    size_t buf_size);

/**
 * @brief Delete a key-value pair.
 *
 * @param handle    Storage handle.
 * @param key       Key string.
 * @return          Storage status code.
 */
fossil_bluecrab_storage_error_t fossil_bluecrab_storage_remove(
    fossil_bluecrab_storage_t *handle,
    const char *key);

// ===========================================================
// Advanced Operations
// ===========================================================

/**
 * @brief Count the number of records stored.
 *
 * @param handle    Storage handle.
 * @param out_count Pointer to store record count.
 * @return          Storage status code.
 */
fossil_bluecrab_storage_error_t fossil_bluecrab_storage_count(
    fossil_bluecrab_storage_t *handle,
    size_t *out_count);

/**
 * @brief Check if a key exists.
 *
 * @param handle    Storage handle.
 * @param key       Key string.
 * @return          true if key exists, false otherwise.
 */
bool fossil_bluecrab_storage_exists(
    fossil_bluecrab_storage_t *handle,
    const char *key);

// ===========================================================
// Iteration
// ===========================================================

/**
 * @brief Get the first key in the backend.
 *
 * @param handle    Storage handle.
 * @param out_buf   Buffer to store key.
 * @param buf_size  Size of buffer.
 * @return          Storage status code.
 */
fossil_bluecrab_storage_error_t fossil_bluecrab_storage_first_key(
    fossil_bluecrab_storage_t *handle,
    char *out_buf,
    size_t buf_size);

/**
 * @brief Get the next key after a given one.
 *
 * @param handle    Storage handle.
 * @param prev_key  Previous key.
 * @param out_buf   Buffer for next key.
 * @param buf_size  Buffer size.
 * @return          Storage status code.
 */
fossil_bluecrab_storage_error_t fossil_bluecrab_storage_next_key(
    fossil_bluecrab_storage_t *handle,
    const char *prev_key,
    char *out_buf,
    size_t buf_size);

#ifdef __cplusplus
}
#include <string>

namespace fossil {

namespace bluecrab {



}

}

#endif

#endif /* FOSSIL_CRABDB_FRAMEWORK_H */

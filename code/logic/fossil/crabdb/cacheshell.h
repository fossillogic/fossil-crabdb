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
#ifndef FOSSIL_CRABDB_CACHESHELL_H
#define FOSSIL_CRABDB_CACHESHELL_H

#include <stdbool.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <sys/stat.h>   // for file size
#include <errno.h>
#include <stdio.h>
#include <time.h>

#ifdef __cplusplus
extern "C" {
#endif

// *****************************************************************************
// CacheShell API - Prototypes
// *****************************************************************************

// ===========================================================
// Basic Key/Value Operations
// ===========================================================

/**
 * @brief Inserts or updates a value in the cache.
 *
 * @param key       Null-terminated key string.
 * @param value     Value string to store.
 * @return          true on success, false on failure.
 */
bool fossil_bluecrab_cacheshell_set(const char *key, const char *value);

/**
 * @brief Retrieves a value from the cache.
 *
 * @param key           Null-terminated key string.
 * @param out_value     Buffer to store retrieved value.
 * @param buffer_size   Size of output buffer.
 * @return              true if key found, false otherwise.
 */
bool fossil_bluecrab_cacheshell_get(const char *key, char *out_value, size_t buffer_size);

/**
 * @brief Removes a key/value pair from the cache.
 *
 * @param key   Null-terminated key string.
 * @return      true if removed, false if key not found.
 */
bool fossil_bluecrab_cacheshell_remove(const char *key);

// ===========================================================
// Expiration / TTL (Time-to-Live)
// ===========================================================

/**
 * @brief Inserts or updates a value with expiration time.
 *
 * @param key       Key string.
 * @param value     Value string.
 * @param ttl_sec   Time-to-live in seconds.
 * @return          true on success, false on failure.
 */
bool fossil_bluecrab_cacheshell_set_with_ttl(const char *key, const char *value, unsigned int ttl_sec);

/**
 * @brief Updates the TTL of an existing key.
 *
 * @param key       Key string.
 * @param ttl_sec   New TTL in seconds.
 * @return          true if updated, false if key not found.
 */
bool fossil_bluecrab_cacheshell_expire(const char *key, unsigned int ttl_sec);

/**
 * @brief Gets the remaining TTL of a key.
 *
 * @param key       Key string.
 * @return          Remaining TTL in seconds, or -1 if not found or no TTL set.
 */
int fossil_bluecrab_cacheshell_ttl(const char *key);

// ===========================================================
// Cache Management
// ===========================================================

/**
 * @brief Clears all keys/values from the cache.
 */
void fossil_bluecrab_cacheshell_clear(void);

/**
 * @brief Returns the number of keys currently in the cache.
 *
 * @return  Key count.
 */
size_t fossil_bluecrab_cacheshell_count(void);

/**
 * @brief Checks if a key exists in the cache.
 *
 * @param key   Key string.
 * @return      true if key exists, false otherwise.
 */
bool fossil_bluecrab_cacheshell_exists(const char *key);

// ===========================================================
// Advanced / Optional Helpers
// ===========================================================

/**
 * @brief Sets a binary-safe value (arbitrary data).
 *
 * @param key       Key string.
 * @param data      Pointer to data buffer.
 * @param size      Size of data buffer.
 * @return          true on success, false on failure.
 */
bool fossil_bluecrab_cacheshell_set_binary(const char *key, const void *data, size_t size);

/**
 * @brief Retrieves a binary-safe value.
 *
 * @param key       Key string.
 * @param out_buf   Buffer to store data.
 * @param buf_size  Size of buffer.
 * @param out_size  Actual size of data returned.
 * @return          true if found, false otherwise.
 */
bool fossil_bluecrab_cacheshell_get_binary(const char *key, void *out_buf, size_t buf_size, size_t *out_size);

#ifdef __cplusplus
}
#include <string>

namespace fossil {

namespace bluecrab {



}

}

#endif

#endif /* FOSSIL_CRABDB_FRAMEWORK_H */

/**
 * -----------------------------------------------------------------------------
 * Project: Fossil Logic
 *
 * This file is part of the Fossil Logic project, which aims to develop
 * high-performance, cross-platform applications and libraries. The code
 * contained herein is licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License. You may obtain
 * a copy of the License at:
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS, WITHOUT
 * WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied. See the
 * License for the specific language governing permissions and limitations
 * under the License.
 *
 * Author: Michael Gene Brockus (Dreamer)
 * Date: 04/05/2014
 *
 * Copyright (C) 2014-2025 Fossil Logic. All rights reserved.
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

// ===========================================================
// Initialization / Lifecycle
// ===========================================================

/**
 * @brief Initializes the cache subsystem.
 *
 * @param max_entries   Optional maximum number of cache entries (0 = unlimited).
 * @return              true on success, false on failure.
 */
bool fossil_bluecrab_cacheshell_init(size_t max_entries);

/**
 * @brief Shuts down the cache subsystem and releases all resources.
 */
void fossil_bluecrab_cacheshell_shutdown(void);

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
 * @return              Pointer to the retrieved value, or NULL if not found.
 */
char *fossil_bluecrab_cacheshell_get(const char *key, size_t buffer_size);

/**
 * @brief Removes a key/value pair from the cache.
 *
 * @param key   Null-terminated key string.
 * @return      true if removed, false if key not found.
 */
bool fossil_bluecrab_cacheshell_remove(const char *key);

/**
 * @brief Checks if a key exists in the cache.
 *
 * @param key   Key string.
 * @return      true if key exists, false otherwise.
 */
bool fossil_bluecrab_cacheshell_exists(const char *key);

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

/**
 * @brief Refreshes the TTL of a key without changing the value.
 *
 * @param key   Key string.
 * @return      true if key found and refreshed, false otherwise.
 */
bool fossil_bluecrab_cacheshell_touch(const char *key);

/**
 * @brief Manually evicts all expired keys.
 *
 * @return      Number of keys evicted.
 */
size_t fossil_bluecrab_cacheshell_evict_expired(void);

// ===========================================================
// Binary-Safe Operations
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
 * Returns a pointer to the internal stored binary data for the given key, or NULL
 * if the key does not exist. The lifetime of the returned pointer is managed by
 * the cache; copy it if you need to retain it. Do not modify the pointed data.
 *
 * @param key       Key string.
 * @param out_size  (Optional) Receives size of the binary value in bytes.
 * @return          Pointer to binary data, or NULL if not found.
 */
const void *fossil_bluecrab_cacheshell_get_binary(const char *key, size_t *out_size);

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
 * @brief Retrieves approximate memory usage by the cache.
 *
 * @return  Number of bytes used by stored entries.
 */
size_t fossil_bluecrab_cacheshell_memory_usage(void);

// ===========================================================
// Introspection / Statistics
// ===========================================================

/**
 * @brief Retrieves cache hit/miss statistics.
 *
 * @param out_hits    Pointer to store hit count (nullable).
 * @param out_misses  Pointer to store miss count (nullable).
 */
void fossil_bluecrab_cacheshell_stats(size_t *out_hits, size_t *out_misses);

// ===========================================================
// Iteration
// ===========================================================

/**
 * @brief Callback type for cache iteration.
 */
typedef void (*fossil_bluecrab_cache_iter_cb)(
    const char *key,
    const void *value,
    size_t value_size,
    void *user_data
);

/**
 * @brief Iterates over all cache entries.
 *
 * @param cb         Callback invoked per entry.
 * @param user_data  Optional pointer passed to callback.
 */
void fossil_bluecrab_cacheshell_iterate(fossil_bluecrab_cache_iter_cb cb, void *user_data);

// ===========================================================
// Thread Safety
// ===========================================================

/**
 * @brief Enables or disables internal locking for thread safety.
 *
 * @param enabled  true to enable locking, false to disable.
 */
void fossil_bluecrab_cacheshell_threadsafe(bool enabled);

// ===========================================================
// Persistence (Optional)
// ===========================================================

/**
 * @brief Saves the cache contents to a file snapshot.
 *
 * @param path  File path to save to.
 * @return      true on success, false on failure.
 */
bool fossil_bluecrab_cacheshell_save(const char *path);

/**
 * @brief Loads cache state from a snapshot file.
 *
 * @param path  File path to load from.
 * @return      true on success, false on failure.
 */
bool fossil_bluecrab_cacheshell_load(const char *path);

#ifdef __cplusplus
}
#include <string>
#include <vector>
#include <functional>
#include <cstring>

namespace fossil {

    namespace bluecrab {

        /**
         * @brief High-level C++ RAII-friendly wrapper around the C CacheShell API.
         *
         * This class provides a strongly-typed, exception-safe (no-throw) interface
         * for interacting with the underlying C cache subsystem. All functions are
         * thin inline pass‑throughs (zero or near-zero overhead) that:
         *  - Accept / return C++ standard library types (std::string, std::vector, etc.).
         *  - Preserve the semantics of the C counterparts.
         *  - Avoid throwing exceptions (all error reporting via return values).
         *
         * THREAD SAFETY:
         *  The underlying subsystem may be optionally made thread-safe via
         *  threadsafe(true). Unless enabled, callers must externally synchronize.
         *
         * LIFETIME:
         *  Use CacheShell::Guard for scoped automatic init / shutdown.
         */
        class CacheShell {
        public:
            /**
             * @brief Initialize the cache subsystem.
             *
             * @param max_entries Maximum entries (0 = unlimited).
             * @return true on success, false on failure (e.g., already initialized or OOM).
             */
            static bool init(size_t max_entries = 0) {
                return fossil_bluecrab_cacheshell_init(max_entries);
            }

            /**
             * @brief Shutdown the cache subsystem. All entries are released.
             *
             * Safe to call multiple times; extra calls are ignored by the C layer.
             */
            static void shutdown() {
                fossil_bluecrab_cacheshell_shutdown();
            }

            // -----------------------------------------------------------------
            // Basic Key / Value Operations
            // -----------------------------------------------------------------

            /**
             * @brief Insert or update a UTF-8 string value.
             *
             * @param key   Cache key.
             * @param value Value to store.
             * @return true on success, false on failure.
             */
            static bool set(const std::string& key, const std::string& value) {
                return fossil_bluecrab_cacheshell_set(key.c_str(), value.c_str());
            }

            /**
             * @brief Retrieve a string value.
             *
             * @param key       Cache key.
             * @param out_value On success, replaced with the stored value.
             * @param max_len   Upper bound hint passed to C API (may truncate longer data).
             * @return true if key found, false otherwise.
             *
             * NOTE: The C API returns a pointer to an internal (or transient) buffer.
             *       We copy it immediately into out_value to ensure safety.
             */
            static bool get(const std::string& key, std::string& out_value, size_t max_len = 4096) {
                char *val = fossil_bluecrab_cacheshell_get(key.c_str(), max_len);
                if (!val)
                    return false;
                out_value.assign(val);
                return true;
            }

            /**
             * @brief Remove a key/value pair.
             * @return true if removed, false if not present.
             */
            static bool remove(const std::string& key) {
                return fossil_bluecrab_cacheshell_remove(key.c_str());
            }

            /**
             * @brief Check if a key exists.
             * @return true if exists, false otherwise.
             */
            static bool exists(const std::string& key) {
                return fossil_bluecrab_cacheshell_exists(key.c_str());
            }

            // -----------------------------------------------------------------
            // Time-To-Live (TTL) Operations
            // -----------------------------------------------------------------

            /**
             * @brief Set a value with an expiration (TTL).
             *
             * @param key      Key to set.
             * @param value    Value to store.
             * @param ttl_sec  Lifetime in seconds (0 may mean no TTL, depending on C layer).
             * @return true on success, false on failure.
             */
            static bool set_with_ttl(const std::string& key, const std::string& value, unsigned int ttl_sec) {
                return fossil_bluecrab_cacheshell_set_with_ttl(key.c_str(), value.c_str(), ttl_sec);
            }

            /**
             * @brief Update TTL of an existing key.
             *
             * @param key      Key to update.
             * @param ttl_sec  New TTL in seconds.
             * @return true if updated, false if key not found.
             */
            static bool expire(const std::string& key, unsigned int ttl_sec) {
                return fossil_bluecrab_cacheshell_expire(key.c_str(), ttl_sec);
            }

            /**
             * @brief Query remaining TTL.
             *
             * @param key Key to query.
             * @return Remaining seconds, or -1 if not found or no TTL set.
             */
            static int ttl(const std::string& key) {
                return fossil_bluecrab_cacheshell_ttl(key.c_str());
            }

            /**
             * @brief Refresh TTL without altering value (similar to touch in Unix).
             * @return true if refreshed, false otherwise.
             */
            static bool touch(const std::string& key) {
                return fossil_bluecrab_cacheshell_touch(key.c_str());
            }

            /**
             * @brief Manually evict all expired keys.
             * @return Count of evicted entries.
             */
            static size_t evict_expired() {
                return fossil_bluecrab_cacheshell_evict_expired();
            }

            // -----------------------------------------------------------------
            // Binary (Opaque Data) Operations
            // -----------------------------------------------------------------

            /**
             * @brief Store arbitrary binary data.
             *
             * @param key   Key.
             * @param data  Pointer to raw bytes.
             * @param size  Length in bytes.
             * @return true on success, false on failure.
             */
            static bool set_binary(const std::string& key, const void* data, size_t size) {
                return fossil_bluecrab_cacheshell_set_binary(key.c_str(), data, size);
            }

            /**
             * @brief Retrieve binary data into a caller-provided buffer.
             *
             * Copies up to buf_size bytes into out_buf (if non-null). If the stored
             * data is larger than buf_size the excess is silently truncated. The
             * full (original) size is still reported via out_size (if provided).
             *
             * @param key       Key.
             * @param out_buf   Destination buffer (may be nullptr to just query size).
             * @param buf_size  Capacity of destination buffer in bytes.
             * @param out_size  (Optional) receives full stored size in bytes.
             * @return true if key exists, false otherwise.
             */
            static bool get_binary(const std::string& key, void* out_buf, size_t buf_size, size_t* out_size) {
                return fossil_bluecrab_cacheshell_get_binary(key.c_str(), out_buf, buf_size, out_size);
            }

            /**
             * @brief Obtain a direct (read-only) pointer to stored binary data.
             *
             * Lifetime is managed by the cache; copy if you need to retain.
             *
             * @param key       Key.
             * @param out_size  (Optional) receives size of data.
             * @return pointer to data or nullptr if not found.
             */
            static const void* get_binary_ptr(const std::string& key, size_t* out_size = nullptr) {
                return ::fossil_bluecrab_cacheshell_get_binary(key.c_str(), out_size);
            }

        private:
            // Internal shim to emulate an extended fetch API used by get_binary_vector().
            // This creates a unified interface (key, buffer, capacity, out_size) on top
            // of the simpler C function that only returns a pointer + size.
            static bool fossil_bluecrab_cacheshell_get_binary(const char* key,
                                                              void* out_buf,
                                                              size_t buf_size,
                                                              size_t* out_size) {
                size_t stored_sz = 0;
                const void* data = ::fossil_bluecrab_cacheshell_get_binary(key, &stored_sz);
                if (!data) {
                    if (out_size) *out_size = 0;
                    return false;
                }
                if (out_size) *out_size = stored_sz;
                if (out_buf && buf_size) {
                    size_t to_copy = stored_sz < buf_size ? stored_sz : buf_size;
                    std::memcpy(out_buf, data, to_copy);
                }
                return true;
            }
        public:

            /**
             * @brief Convenience helper returning binary data in a std::vector<uint8_t>.
             *
             * Performs a two-pass retrieval:
             *  1) Query size by calling with nullptr buffer.
             *  2) Allocate vector + fetch actual bytes.
             *
             * @param key Key to fetch.
             * @param out Vector filled with data (cleared/reallocated as needed).
             * @return true on success, false if key not found.
             */
            static bool get_binary_vector(const std::string& key, std::vector<uint8_t>& out) {
                size_t sz = 0;
                if (!fossil_bluecrab_cacheshell_get_binary(key.c_str(), nullptr, 0, &sz))
                    return false;
                out.resize(sz);
                size_t got = 0;
                if (!fossil_bluecrab_cacheshell_get_binary(key.c_str(), out.data(), sz, &got))
                    return false;
                out.resize(got);
                return true;
            }

            // -----------------------------------------------------------------
            // Cache Management
            // -----------------------------------------------------------------

            /**
             * @brief Remove all entries (flush).
             */
            static void clear() {
                fossil_bluecrab_cacheshell_clear();
            }

            /**
             * @brief Number of currently stored keys.
             */
            static size_t count() {
                return fossil_bluecrab_cacheshell_count();
            }

            /**
             * @brief Approximate memory usage in bytes (implementation-defined).
             */
            static size_t memory_usage() {
                return fossil_bluecrab_cacheshell_memory_usage();
            }

            // -----------------------------------------------------------------
            // Statistics
            // -----------------------------------------------------------------

            /**
             * @brief Simple hit/miss statistics POD.
             */
            struct Stats {
            size_t hits = 0;    ///< Number of successful lookups.
            size_t misses = 0;  ///< Number of failed lookups.
            };

            /**
             * @brief Retrieve snapshot of hit/miss counters.
             *
             * @return Stats structure populated with current counters.
             */
            static Stats stats() {
                Stats s;
                fossil_bluecrab_cacheshell_stats(&s.hits, &s.misses);
                return s;
            }

            // -----------------------------------------------------------------
            // Iteration
            // -----------------------------------------------------------------

            /**
             * @brief Iterate through all entries (no guaranteed ordering).
             *
             * WARNING: Modifying the cache within the callback may invalidate
             * iteration (depends on underlying implementation). Keep callbacks brief.
             *
             * @param cb Callback invoked once per entry: (key, value_ptr, value_size).
             */
            static void iterate(const std::function<void(const std::string&, const void*, size_t)>& cb) {
                struct Trampoline {
                    static void call(const char* k, const void* v, size_t vsz, void* ud) {
                    auto* fn = static_cast<const std::function<void(const std::string&, const void*, size_t)>*>(ud);
                    (*fn)(k, v, vsz);
                    }
                };
                fossil_bluecrab_cacheshell_iterate(&Trampoline::call,
                                const_cast<void*>(reinterpret_cast<const void*>(&cb)));
            }

            // -----------------------------------------------------------------
            // Thread Safety Control
            // -----------------------------------------------------------------

            /**
             * @brief Enable or disable internal locking.
             *
             * @param enabled true to enable internal mutex protection, false to disable.
             *                When disabled, caller must ensure external synchronization.
             */
            static void threadsafe(bool enabled) {
                fossil_bluecrab_cacheshell_threadsafe(enabled);
            }

            // -----------------------------------------------------------------
            // Persistence
            // -----------------------------------------------------------------

            /**
             * @brief Save cache snapshot to a file.
             *
             * @param path Filesystem path.
             * @return true on success, false on failure (I/O error, permissions, etc.).
             */
            static bool save(const std::string& path) {
                return fossil_bluecrab_cacheshell_save(path.c_str());
            }

            /**
             * @brief Load cache snapshot from a file.
             *
             * NOTE: Existing contents may be replaced / merged depending on C API semantics.
             *
             * @param path Filesystem path.
             * @return true on success, false on failure.
             */
            static bool load(const std::string& path) {
                return fossil_bluecrab_cacheshell_load(path.c_str());
            }

        };

    } // namespace bluecrab

} // namespace fossil

#endif

#endif /* FOSSIL_CRABDB_FRAMEWORK_H */

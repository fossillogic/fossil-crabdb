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
#include "fossil/crabdb/cacheshell.h"
#include <stdlib.h>
#include <string.h>
#include <time.h>
#if defined(_WIN32) || defined(_WIN64)
#include <windows.h>
typedef CRITICAL_SECTION pthread_mutex_t;
#else
#include <pthread.h>
#endif

// ===========================================================
// Internal Types
// ===========================================================

typedef struct fossil_cache_entry_t {
    char *key;
    void *data;
    size_t size;
    time_t expiry; // 0 if no TTL
    struct fossil_cache_entry_t *next;
} fossil_cache_entry_t;

typedef struct {
    fossil_cache_entry_t **buckets;
    size_t bucket_count;
    size_t entry_count;
    size_t max_entries;
    size_t hits;
    size_t misses;
    bool locking_enabled;
    pthread_mutex_t lock;
} fossil_cache_t;

// ===========================================================
// Internal Globals
// ===========================================================

static fossil_cache_t g_cache;

// ===========================================================
// Internal Helpers
// ===========================================================

// Enhanced hash: FNV-1a with architecture-aware final mixing (avalanche).
// Keeps speed while improving distribution over classic djb2.
static size_t fossil_cache_hash(const char *key) {
#if SIZE_MAX > 0xFFFFFFFFu  // 64-bit size_t
    uint64_t h = 1469598103934665603ull;          // FNV-1a 64-bit offset
    while (*key) {
        h ^= (uint8_t)*key++;
        h *= 1099511628211ull;                   // FNV prime
    }
    // Mix (inspired by SplitMix64 / Murmur finalizers)
    h ^= h >> 32;
    h *= 0x9e3779b97f4a7c15ull;
    h ^= h >> 29;
    h *= 0x94d049bb133111ebull;
    h ^= h >> 32;
    return (size_t)h;
#else // 32-bit size_t
    uint32_t h = 2166136261u;                    // FNV-1a 32-bit offset
    while (*key) {
        h ^= (uint8_t)*key++;
        h *= 16777619u;                          // FNV prime
    }
    // Final avalanche (MurMur / Jenkins style)
    h ^= h >> 15;
    h *= 0x85ebca6bu;
    h ^= h >> 13;
    h *= 0xc2b2ae35u;
    h ^= h >> 16;
    return (size_t)h;
#endif
}

#if defined(_WIN32) || defined(_WIN64)
static void fossil_cache_lock_init(void) {
    InitializeCriticalSection(&g_cache.lock);
}

static void fossil_cache_lock_destroy(void) {
    DeleteCriticalSection(&g_cache.lock);
}

static void fossil_cache_lock(void) {
    if (g_cache.locking_enabled)
        EnterCriticalSection(&g_cache.lock);
}

static void fossil_cache_unlock(void) {
    if (g_cache.locking_enabled)
        LeaveCriticalSection(&g_cache.lock);
}
#else
static void fossil_cache_lock_init(void) {
    pthread_mutex_init(&g_cache.lock, NULL);
}

static void fossil_cache_lock_destroy(void) {
    pthread_mutex_destroy(&g_cache.lock);
}

static void fossil_cache_lock(void) {
    if (g_cache.locking_enabled)
        pthread_mutex_lock(&g_cache.lock);
}

static void fossil_cache_unlock(void) {
    if (g_cache.locking_enabled)
        pthread_mutex_unlock(&g_cache.lock);
}
#endif

static void fossil_cache_free_entry(fossil_cache_entry_t *entry) {
    if (!entry) return;
    free(entry->key);
    free(entry->data);
    free(entry);
}

static void fossil_cache_remove_internal(const char *key) {
    size_t index = fossil_cache_hash(key) % g_cache.bucket_count;
    fossil_cache_entry_t *prev = NULL, *curr = g_cache.buckets[index];
    while (curr) {
        if (strcmp(curr->key, key) == 0) {
            if (prev)
                prev->next = curr->next;
            else
                g_cache.buckets[index] = curr->next;
            fossil_cache_free_entry(curr);
            g_cache.entry_count--;
            return;
        }
        prev = curr;
        curr = curr->next;
    }
}

static fossil_cache_entry_t *fossil_cache_find(const char *key) {
    size_t index = fossil_cache_hash(key) % g_cache.bucket_count;
    fossil_cache_entry_t *entry = g_cache.buckets[index];
    time_t now = time(NULL);
    while (entry) {
        if (strcmp(entry->key, key) == 0) {
            // Check expiration
            if (entry->expiry > 0 && entry->expiry <= now) {
                fossil_cache_remove_internal(key);
                g_cache.misses++;
                return NULL;
            }
            g_cache.hits++;
            return entry;
        }
        entry = entry->next;
    }
    g_cache.misses++;
    return NULL;
}

// ===========================================================
// Initialization / Lifecycle
// ===========================================================

bool fossil_bluecrab_cacheshell_init(size_t max_entries) {
    memset(&g_cache, 0, sizeof(g_cache));
    g_cache.bucket_count = 1024; // reasonable default
    g_cache.max_entries = max_entries;
    g_cache.buckets = calloc(g_cache.bucket_count, sizeof(fossil_cache_entry_t *));
    if (!g_cache.buckets)
        return false;
    fossil_cache_lock_init();
    return true;
}

void fossil_bluecrab_cacheshell_shutdown(void) {
    fossil_cache_lock();
    for (size_t i = 0; i < g_cache.bucket_count; ++i) {
        fossil_cache_entry_t *entry = g_cache.buckets[i];
        while (entry) {
            fossil_cache_entry_t *next = entry->next;
            fossil_cache_free_entry(entry);
            entry = next;
        }
    }
    free(g_cache.buckets);
    g_cache.buckets = NULL;
    g_cache.entry_count = 0;
    fossil_cache_unlock();
    fossil_cache_lock_destroy();
}

// ===========================================================
// Basic Key/Value Operations
// ===========================================================

bool fossil_bluecrab_cacheshell_set(const char *key, const char *value) {
    return fossil_bluecrab_cacheshell_set_binary(key, value, strlen(value) + 1);
}

bool fossil_bluecrab_cacheshell_get(const char *key, char *out_value, size_t buffer_size) {
    fossil_cache_lock();
    fossil_cache_entry_t *entry = fossil_cache_find(key);
    if (!entry) {
        fossil_cache_unlock();
        return false;
    }
    strncpy(out_value, entry->data, buffer_size - 1);
    out_value[buffer_size - 1] = '\0';
    fossil_cache_unlock();
    return true;
}

bool fossil_bluecrab_cacheshell_remove(const char *key) {
    fossil_cache_lock();
    size_t before = g_cache.entry_count;
    fossil_cache_remove_internal(key);
    fossil_cache_unlock();
    return (before != g_cache.entry_count);
}

bool fossil_bluecrab_cacheshell_exists(const char *key) {
    fossil_cache_lock();
    fossil_cache_entry_t *entry = fossil_cache_find(key);
    fossil_cache_unlock();
    return entry != NULL;
}

// ===========================================================
// Expiration / TTL
// ===========================================================

bool fossil_bluecrab_cacheshell_set_with_ttl(const char *key, const char *value, unsigned int ttl_sec) {
    fossil_cache_lock();
    bool result = fossil_bluecrab_cacheshell_set_binary(key, value, strlen(value) + 1);
    if (result) {
        fossil_cache_entry_t *entry = fossil_cache_find(key);
        if (entry)
            entry->expiry = ttl_sec > 0 ? time(NULL) + ttl_sec : 0;
    }
    fossil_cache_unlock();
    return result;
}

bool fossil_bluecrab_cacheshell_expire(const char *key, unsigned int ttl_sec) {
    fossil_cache_lock();
    fossil_cache_entry_t *entry = fossil_cache_find(key);
    if (!entry) {
        fossil_cache_unlock();
        return false;
    }
    entry->expiry = ttl_sec > 0 ? time(NULL) + ttl_sec : 0;
    fossil_cache_unlock();
    return true;
}

int fossil_bluecrab_cacheshell_ttl(const char *key) {
    fossil_cache_lock();
    fossil_cache_entry_t *entry = fossil_cache_find(key);
    if (!entry) {
        fossil_cache_unlock();
        return -1;
    }
    if (entry->expiry == 0) {
        fossil_cache_unlock();
        return -1;
    }
    int ttl = (int)difftime(entry->expiry, time(NULL));
    fossil_cache_unlock();
    return ttl > 0 ? ttl : -1;
}

bool fossil_bluecrab_cacheshell_touch(const char *key) {
    fossil_cache_lock();
    fossil_cache_entry_t *entry = fossil_cache_find(key);
    if (!entry || entry->expiry == 0) {
        fossil_cache_unlock();
        return false;
    }
    time_t now = time(NULL);
    int ttl = (int)difftime(entry->expiry, now);
    entry->expiry = now + ttl;
    fossil_cache_unlock();
    return true;
}

size_t fossil_bluecrab_cacheshell_evict_expired(void) {
    fossil_cache_lock();
    size_t evicted = 0;
    time_t now = time(NULL);
    for (size_t i = 0; i < g_cache.bucket_count; ++i) {
        fossil_cache_entry_t *prev = NULL, *curr = g_cache.buckets[i];
        while (curr) {
            if (curr->expiry > 0 && curr->expiry <= now) {
                fossil_cache_entry_t *dead = curr;
                if (prev)
                    prev->next = curr->next;
                else
                    g_cache.buckets[i] = curr->next;
                curr = curr->next;
                fossil_cache_free_entry(dead);
                g_cache.entry_count--;
                evicted++;
                continue;
            }
            prev = curr;
            curr = curr->next;
        }
    }
    fossil_cache_unlock();
    return evicted;
}

// ===========================================================
// Binary-Safe Operations
// ===========================================================

bool fossil_bluecrab_cacheshell_set_binary(const char *key, const void *data, size_t size) {
    fossil_cache_lock();
    if (g_cache.max_entries && g_cache.entry_count >= g_cache.max_entries) {
        fossil_cache_unlock();
        return false;
    }

    size_t index = fossil_cache_hash(key) % g_cache.bucket_count;
    fossil_cache_entry_t *entry = g_cache.buckets[index];
    while (entry) {
        if (strcmp(entry->key, key) == 0) {
            free(entry->data);
            entry->data = malloc(size);
            if (!entry->data) {
                fossil_cache_unlock();
                return false;
            }
            memcpy(entry->data, data, size);
            entry->size = size;
            entry->expiry = 0;
            fossil_cache_unlock();
            return true;
        }
        entry = entry->next;
    }

    entry = calloc(1, sizeof(fossil_cache_entry_t));
    if (!entry) {
        fossil_cache_unlock();
        return false;
    }

    entry->key = strdup(key);
    entry->data = malloc(size);
    if (!entry->key || !entry->data) {
        fossil_cache_free_entry(entry);
        fossil_cache_unlock();
        return false;
    }

    memcpy(entry->data, data, size);
    entry->size = size;
    entry->expiry = 0;
    entry->next = g_cache.buckets[index];
    g_cache.buckets[index] = entry;
    g_cache.entry_count++;

    fossil_cache_unlock();
    return true;
}

bool fossil_bluecrab_cacheshell_get_binary(const char *key, void *out_buf, size_t buf_size, size_t *out_size) {
    fossil_cache_lock();
    fossil_cache_entry_t *entry = fossil_cache_find(key);
    if (!entry) {
        fossil_cache_unlock();
        return false;
    }

    if (out_size)
        *out_size = entry->size;
    if (out_buf && buf_size >= entry->size)
        memcpy(out_buf, entry->data, entry->size);
    fossil_cache_unlock();
    return true;
}

// ===========================================================
// Cache Management
// ===========================================================

void fossil_bluecrab_cacheshell_clear(void) {
    fossil_cache_lock();
    for (size_t i = 0; i < g_cache.bucket_count; ++i) {
        fossil_cache_entry_t *entry = g_cache.buckets[i];
        while (entry) {
            fossil_cache_entry_t *next = entry->next;
            fossil_cache_free_entry(entry);
            entry = next;
        }
        g_cache.buckets[i] = NULL;
    }
    g_cache.entry_count = 0;
    fossil_cache_unlock();
}

size_t fossil_bluecrab_cacheshell_count(void) {
    return g_cache.entry_count;
}

size_t fossil_bluecrab_cacheshell_memory_usage(void) {
    size_t total = 0;
    fossil_cache_lock();
    for (size_t i = 0; i < g_cache.bucket_count; ++i) {
        fossil_cache_entry_t *entry = g_cache.buckets[i];
        while (entry) {
            total += sizeof(*entry) + entry->size + strlen(entry->key) + 1;
            entry = entry->next;
        }
    }
    fossil_cache_unlock();
    return total;
}

// ===========================================================
// Statistics / Thread Safety
// ===========================================================

void fossil_bluecrab_cacheshell_stats(size_t *out_hits, size_t *out_misses) {
    if (out_hits) *out_hits = g_cache.hits;
    if (out_misses) *out_misses = g_cache.misses;
}

void fossil_bluecrab_cacheshell_threadsafe(bool enabled) {
    g_cache.locking_enabled = enabled;
}

// ===========================================================
// Iteration
// ===========================================================

void fossil_bluecrab_cacheshell_iterate(fossil_bluecrab_cache_iter_cb cb, void *user_data) {
    if (!cb) return;
    fossil_cache_lock();
    for (size_t i = 0; i < g_cache.bucket_count; ++i) {
        fossil_cache_entry_t *entry = g_cache.buckets[i];
        while (entry) {
            cb(entry->key, entry->data, entry->size, user_data);
            entry = entry->next;
        }
    }
    fossil_cache_unlock();
}

// ===========================================================
// Persistence (Optional)
// ===========================================================

bool fossil_bluecrab_cacheshell_save(const char *path) {
    fossil_cache_lock();
    FILE *file = fopen(path, "wb");
    if (!file) {
        fossil_cache_unlock();
        return false;
    }

    for (size_t i = 0; i < g_cache.bucket_count; ++i) {
        fossil_cache_entry_t *entry = g_cache.buckets[i];
        while (entry) {
            fwrite(entry->key, 1, strlen(entry->key) + 1, file);
            fwrite(&entry->size, sizeof(entry->size), 1, file);
            fwrite(entry->data, 1, entry->size, file);
            entry = entry->next;
        }
    }

    fclose(file);
    fossil_cache_unlock();
    return true;
}

bool fossil_bluecrab_cacheshell_load(const char *path) {
    fossil_cache_lock();
    FILE *file = fopen(path, "rb");
    if (!file) {
        fossil_cache_unlock();
        return false;
    }

    fossil_bluecrab_cacheshell_clear();

    while (true) {
        char key[256];
        if (!fgets(key, sizeof(key), file))
            break;
        size_t key_len = strlen(key);
        if (key_len > 0 && key[key_len - 1] == '\n')
            key[key_len - 1] = '\0';

        size_t size;
        if (fread(&size, sizeof(size), 1, file) != 1)
            break;

        void *data = malloc(size);
        if (!data || fread(data, 1, size, file) != size) {
            free(data);
            break;
        }

        fossil_bluecrab_cacheshell_set_binary(key, data, size);
        free(data);
    }

    fclose(file);
    fossil_cache_unlock();
    return true;
}

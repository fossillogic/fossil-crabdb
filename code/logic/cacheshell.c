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
// Lightweight pthread mutex compatibility layer for Windows
static int pthread_mutex_init(pthread_mutex_t *m, void *attr) {
    (void)attr;
    InitializeCriticalSection(m);
    return 0;
}
static int pthread_mutex_lock(pthread_mutex_t *m) {
    EnterCriticalSection(m);
    return 0;
}
static int pthread_mutex_unlock(pthread_mutex_t *m) {
    LeaveCriticalSection(m);
    return 0;
}
static int pthread_mutex_destroy(pthread_mutex_t *m) {
    DeleteCriticalSection(m);
    return 0;
}
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

static size_t fossil_cache_hash(const char *key) {
    // djb2 hash
    size_t hash = 5381;
    unsigned char c;
    while ((c = (unsigned char)*key++))
        hash = ((hash << 5) + hash) + c;
    return hash;
}

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
    pthread_mutex_init(&g_cache.lock, NULL);
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
    pthread_mutex_destroy(&g_cache.lock);
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

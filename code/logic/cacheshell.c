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

/**
 * @brief In-memory key/value cache (BlueCrab CacheShell).
 *
 * High-level:
 *   A fixed-size hash table (1024 buckets) using separate chaining stores
 *   entries (key, binary blob, size, optional expiry). Lookups hash the key,
 *   traverse the chain, validate TTL, and return the data.
 *
 * Core Features:
 *   - String & binary storage (size tracked, not null-terminated required)
 *   - Optional per-entry TTL (seconds) with lazy + bulk eviction
 *   - Optional thread safety (runtime toggle)
 *   - Basic stats (hits / misses), memory usage, count
 *   - Iteration callback over all entries
 *   - Simple persistence (key + size + raw bytes) — TTL NOT persisted
 *
 * Data Structures:
 *   fossil_cache_entry_t:
 *       +-----------+--------------+-------+---------+----------+
 *       | key (dup) | data (malloc)| size  | expiry  | *next    |
 *       +-----------+--------------+-------+---------+----------+
 *          expiry == 0 => non-expiring
 *
 *   Hash Table (array of bucket head pointers):
 *
 *       buckets[0] --> [entry] -> [entry] -> NULL
 *       buckets[1] --> NULL
 *       buckets[2] --> [entry] -> NULL
 *       ...
 *       buckets[1023] --> [entry] -> [entry] -> [entry] -> NULL
 *
 *   Example bucket chain (collision resolution via linked list):
 *
 *       +-----------+      +-----------+      +-----------+
 *       |  key=A    |----->|  key=Q    |----->|  key=Z    |-> NULL
 *       +-----------+      +-----------+      +-----------+
 *
 * Hash Function:
 *   FNV-1a (32 or 64-bit) plus extra avalanche mixing for better
 *   distribution. Final index = hash % bucket_count.
 *
 * TTL / Expiration:
 *   - When fetched:
 *       if (expiry != 0 && expiry <= now) -> removed + miss
 *   - Bulk cleanup:
 *       fossil_bluecrab_cacheshell_evict_expired()
 *
 *   Timeline (example):
 *       set_with_ttl(K, 5s) at t=10
 *       expiry = 15
 *       access at t=12 -> valid
 *       access at t=16 -> expired -> auto remove -> miss
 *
 * Thread Safety:
 *   - Disabled by default for zero overhead.
 *   - fossil_bluecrab_cacheshell_threadsafe(true) enables a global lock:
 *
 *        Public API:
 *            lock()
 *            ... mutate / read ...
 *            unlock()
 *
 *   - Windows: CRITICAL_SECTION (typedef'd to pthread_mutex_t alias)
 *   - POSIX: pthread_mutex_t
 *
 * Persistence Format (sequential stream):
 *   For each entry (non-expired at save time):
 *       key bytes including '\0'
 *       size (size_t)
 *       raw data bytes
 *   NOT stored: expiry/TTL, stats, locking flag, bucket count.
 *   On load: table cleared, entries appended (TTL defaults to 0).
 *
 * Memory Usage Calculation:
 *   sum( sizeof(entry) + entry->size + strlen(key)+1 )
 *
 * Limitations / Trade-offs:
 *   - No resizing: very large key counts per bucket degrade performance
 *   - No eviction policy (LRU/LFU); rely on max_entries or TTL + manual removal
 *   - Global singleton cache (g_cache) — not multi-instance
 *   - Persistence is endian/ABI dependent (size_t & layout)
 *
 * Example Usage:
 *
 *   if (fossil_bluecrab_cacheshell_init(10000)) {
 *       fossil_bluecrab_cacheshell_threadsafe(true);
 *       fossil_bluecrab_cacheshell_set("greet", "hello");
 *
 *       char buf[32];
 *       if (fossil_bluecrab_cacheshell_get("greet", buf, sizeof buf)) {
 *           // buf == "hello"
 *       }
 *
 *       fossil_bluecrab_cacheshell_set_with_ttl("temp", "123", 3);
 *       // ... after >3s fossil_bluecrab_cacheshell_get("temp") -> miss
 *
 *       fossil_bluecrab_cacheshell_save("dump.cache");
 *       fossil_bluecrab_cacheshell_clear();
 *       fossil_bluecrab_cacheshell_load("dump.cache");
 *
 *       fossil_bluecrab_cacheshell_shutdown();
 *   }
 *
 * ASCII Flow (lookup):
 *
 *     key -> hash -> bucket index -> traverse chain -> (match?)
 *                                      | yes
 *                                      v
 *                               check expiry
 *                                 | valid
 *                                 v
 *                               return data
 *
 * Error Conditions:
 *   - Allocation failure -> false
 *   - Exceeds max_entries -> false
 *   - Missing key lookup -> false
 *
 * Complexity (average):
 *   - set/get/remove: O(1) expected, O(n) worst (n = entries in a bucket)
 *   - evict_expired: O(total_entries)
 *   - iterate: O(total_entries)
 *
 * Safety Notes:
 *   - Caller must provide adequate buffer for fossil_bluecrab_cacheshell_get
 *   - Binary retrieval requires caller to check returned size (out_size)
 */

// ===========================================================
// Internal Types
// ===========================================================

typedef struct fossil_cache_entry_t {
    char *key;
    void *data;
    size_t size;
    time_t expiry;          // 0 if no TTL
    time_t created;         // creation timestamp
    time_t last_access;     // last access timestamp
    struct fossil_cache_entry_t *next;
} fossil_cache_entry_t;

typedef struct {
    fossil_cache_entry_t **buckets;
    size_t bucket_count;
    size_t entry_count;
    size_t max_entries;
    size_t hits;
    size_t misses;
    size_t total_bytes;        // tracked allocated bytes (optional)
    size_t expired_evictions;  // number of expired evictions (optional)
    bool locking_enabled;
    pthread_mutex_t lock;
    time_t start_time;         // cache initialization time
} fossil_cache_t;

// ===========================================================
// Internal Globals
// ===========================================================

static fossil_cache_t g_cache;

// ===========================================================
// Internal Helpers
// ===========================================================

/**
 * Custom cacheshell_strdup implementation.
 */
static char *cacheshell_strdup(const char *s) {
    if (!s) return NULL;
    size_t len = strlen(s);
    char *copy = (char *)malloc(len + 1);
    if (copy) {
        memcpy(copy, s, len + 1);
    }
    return copy;
}

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
    if (!key || !g_cache.buckets)
        return;

    size_t index = fossil_cache_hash(key) % g_cache.bucket_count;
    fossil_cache_entry_t *prev = NULL;
    fossil_cache_entry_t *curr = g_cache.buckets[index];

    while (curr) {
        if (strcmp(curr->key, key) == 0) {
            if (prev)
                prev->next = curr->next;
            else
                g_cache.buckets[index] = curr->next;

            // Account for memory usage (best-effort).
            size_t bytes = sizeof(*curr) + curr->size + strlen(curr->key) + 1;
            if (g_cache.total_bytes >= bytes)
                g_cache.total_bytes -= bytes;
            else
                g_cache.total_bytes = 0;

            fossil_cache_free_entry(curr);
            if (g_cache.entry_count > 0)
                g_cache.entry_count--;
            return;
        }
        prev = curr;
        curr = curr->next;
    }
}

static fossil_cache_entry_t *fossil_cache_find(const char *key) {
    if (!key || !g_cache.buckets)
        return NULL;

    size_t index = fossil_cache_hash(key) % g_cache.bucket_count;
    fossil_cache_entry_t *prev = NULL;
    fossil_cache_entry_t *entry = g_cache.buckets[index];
    time_t now = time(NULL);

    while (entry) {
        if (strcmp(entry->key, key) == 0) {
            // Expired?
            if (entry->expiry > 0 && entry->expiry <= now) {
                fossil_cache_entry_t *expired = entry;
                if (prev)
                    prev->next = entry->next;
                else
                    g_cache.buckets[index] = entry->next;
                entry = entry->next;
                fossil_cache_free_entry(expired);
                g_cache.entry_count--;
                g_cache.misses++;
                g_cache.expired_evictions++;
                return NULL;
            }
            if (entry->created == 0)
                entry->created = now;
            entry->last_access = now;
            g_cache.hits++;
            return entry;
        }
        prev = entry;
        entry = entry->next;
    }

    g_cache.misses++;
    return NULL;
}

// ===========================================================
// Initialization / Lifecycle
// ===========================================================

bool fossil_bluecrab_cacheshell_init(size_t max_entries) {
    if (g_cache.buckets) // already initialized
        return true;

    memset(&g_cache, 0, sizeof(g_cache));
    g_cache.bucket_count = 1024;          // fixed bucket count for now
    g_cache.max_entries  = max_entries;
    g_cache.start_time   = time(NULL);
    g_cache.locking_enabled = false;      // opt-in

    g_cache.buckets = (fossil_cache_entry_t **)calloc(
        g_cache.bucket_count, sizeof(fossil_cache_entry_t *));
    if (!g_cache.buckets)
        return false;

    fossil_cache_lock_init();
    return true;
}

void fossil_bluecrab_cacheshell_shutdown(void) {
    if (!g_cache.buckets)
        return;

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
    free(g_cache.buckets);
    g_cache.buckets = NULL;
    g_cache.entry_count = 0;
    g_cache.hits = 0;
    g_cache.misses = 0;
    g_cache.total_bytes = 0;
    g_cache.expired_evictions = 0;
    g_cache.start_time = 0;
    fossil_cache_unlock();

    fossil_cache_lock_destroy();
    g_cache.locking_enabled = false;
}

// ===========================================================
// Basic Key/Value Operations
// ===========================================================

bool fossil_bluecrab_cacheshell_set(const char *key, const char *value) {
    if (!key || !value)
        return false;
    return fossil_bluecrab_cacheshell_set_binary(key, value, strlen(value) + 1);
}

char *fossil_bluecrab_cacheshell_get(const char *key, size_t buffer_size) {
    if (!key || buffer_size == 0)
        return NULL;

    fossil_cache_lock();
    fossil_cache_entry_t *entry = fossil_cache_find(key);
    if (!entry) {
        fossil_cache_unlock();
        return NULL;
    }

    size_t data_size = entry->size; // may include '\0' (string path) or be binary
    if (data_size == 0) {
        fossil_cache_unlock();
        return NULL;
    }

    /* We always reserve one byte for a terminating '\0' to return a C-string.
       Allocate at most buffer_size bytes, but ensure space for terminator. */
    size_t alloc_size = data_size + 1;
    if (alloc_size > buffer_size)
        alloc_size = buffer_size;
    if (alloc_size == 0) { // defensive (should not happen since buffer_size>0)
        fossil_cache_unlock();
        return NULL;
    }

    char *out = (char *)malloc(alloc_size);
    if (!out) {
        fossil_cache_unlock();
        return NULL;
    }

    size_t copy_len = data_size;
    if (copy_len > alloc_size - 1)
        copy_len = alloc_size - 1;

    if (copy_len > 0)
        memcpy(out, entry->data, copy_len);
    out[copy_len] = '\0';

    fossil_cache_unlock();
    return out;
}

bool fossil_bluecrab_cacheshell_remove(const char *key) {
    if (!key)
        return false;
    fossil_cache_lock();
    size_t before = g_cache.entry_count;
    fossil_cache_remove_internal(key);
    fossil_cache_unlock();
    return (before != g_cache.entry_count);
}

bool fossil_bluecrab_cacheshell_exists(const char *key) {
    if (!key) return false;

    fossil_cache_lock();

    if (!g_cache.buckets) {
        fossil_cache_unlock();
        return false;
    }

    size_t index = fossil_cache_hash(key) % g_cache.bucket_count;
    fossil_cache_entry_t *prev = NULL;
    fossil_cache_entry_t *curr = g_cache.buckets[index];
    time_t now = time(NULL);

    while (curr) {
        if (strcmp(curr->key, key) == 0) {
            // If expired, remove it (do NOT count as miss/hit)
            if (curr->expiry > 0 && curr->expiry <= now) {
                fossil_cache_entry_t *dead = curr;
                if (prev)
                    prev->next = curr->next;
                else
                    g_cache.buckets[index] = curr->next;
                curr = curr->next;
                fossil_cache_free_entry(dead);
                if (g_cache.entry_count > 0)
                    g_cache.entry_count--;
                g_cache.expired_evictions++;
                fossil_cache_unlock();
                return false;
            }
            // Found and valid; do NOT update hits/last_access for exists()
            fossil_cache_unlock();
            return true;
        }
        prev = curr;
        curr = curr->next;
    }

    fossil_cache_unlock();
    return false;
}

// ===========================================================
// Expiration / TTL
// ===========================================================

// Avoids double-locking by calling the existing setter first (it handles locking)
// then setting TTL under a separate lock. Also initializes created/last_access.
bool fossil_bluecrab_cacheshell_set_with_ttl(const char *key, const char *value, unsigned int ttl_sec) {
    if (!key || !value) return false;
    size_t len = strlen(value) + 1;
    if (!fossil_bluecrab_cacheshell_set_binary(key, value, len))
        return false;
    if (ttl_sec == 0) return true;

    fossil_cache_lock();
    if (g_cache.buckets) {
        size_t index = fossil_cache_hash(key) % g_cache.bucket_count;
        fossil_cache_entry_t *e = g_cache.buckets[index];
        while (e) {
            if (strcmp(e->key, key) == 0) {
                time_t now = time(NULL);
                e->expiry = now + ttl_sec;
                if (e->created == 0) e->created = now;
                e->last_access = e->created;
                break;
            }
            e = e->next;
        }
    }
    fossil_cache_unlock();
    return true;
}

// Binary variant for completeness.
bool fossil_bluecrab_cacheshell_set_binary_with_ttl(const char *key, const void *data, size_t size, unsigned int ttl_sec) {
    if (!key || !data) return false;
    if (!fossil_bluecrab_cacheshell_set_binary(key, data, size))
        return false;
    if (ttl_sec == 0) return true;

    fossil_cache_lock();
    if (g_cache.buckets) {
        size_t index = fossil_cache_hash(key) % g_cache.bucket_count;
        fossil_cache_entry_t *e = g_cache.buckets[index];
        while (e) {
            if (strcmp(e->key, key) == 0) {
                time_t now = time(NULL);
                e->expiry = now + ttl_sec;
                if (e->created == 0) e->created = now;
                e->last_access = e->created;
                break;
            }
            e = e->next;
        }
    }
    fossil_cache_unlock();
    return true;
}

bool fossil_bluecrab_cacheshell_expire(const char *key, unsigned int ttl_sec) {
    if (!key) return false;

    fossil_cache_lock();
    if (!g_cache.buckets) {
        fossil_cache_unlock();
        return false;
    }

    size_t index = fossil_cache_hash(key) % g_cache.bucket_count;
    fossil_cache_entry_t *prev = NULL;
    fossil_cache_entry_t *entry = g_cache.buckets[index];
    time_t now = time(NULL);

    while (entry) {
        if (strcmp(entry->key, key) == 0) {
            // If currently expired, unlink and treat as not found (cannot extend)
            if (entry->expiry > 0 && entry->expiry <= now) {
                if (prev)
                    prev->next = entry->next;
                else
                    g_cache.buckets[index] = entry->next;
                fossil_cache_free_entry(entry);
                if (g_cache.entry_count > 0) g_cache.entry_count--;
                g_cache.expired_evictions++;
                fossil_cache_unlock();
                return false;
            }
            if (ttl_sec > 0) {
                entry->expiry = now + ttl_sec;
                if (entry->created == 0) entry->created = now;
            } else {
                entry->expiry = 0; // make non-expiring
            }
            entry->last_access = now; // treat as a maintenance touch
            fossil_cache_unlock();
            return true;
        }
        prev = entry;
        entry = entry->next;
    }

    fossil_cache_unlock();
    return false;
}

int fossil_bluecrab_cacheshell_ttl(const char *key) {
    if (!key) return -1;

    fossil_cache_lock();
    if (!g_cache.buckets) {
        fossil_cache_unlock();
        return -1;
    }

    size_t index = fossil_cache_hash(key) % g_cache.bucket_count;
    fossil_cache_entry_t *prev = NULL;
    fossil_cache_entry_t *curr = g_cache.buckets[index];
    time_t now = time(NULL);

    while (curr) {
        if (strcmp(curr->key, key) == 0) {
            // If expired, unlink and report -1 (also count eviction, but no hit/miss)
            if (curr->expiry > 0 && curr->expiry <= now) {
                if (prev)
                    prev->next = curr->next;
                else
                    g_cache.buckets[index] = curr->next;
                fossil_cache_free_entry(curr);
                if (g_cache.entry_count > 0)
                    g_cache.entry_count--;
                g_cache.expired_evictions++;
                fossil_cache_unlock();
                return -1;
            }
            // No TTL set
            if (curr->expiry == 0) {
                fossil_cache_unlock();
                return -1;
            }
            int ttl = (int)difftime(curr->expiry, now);
            fossil_cache_unlock();
            return ttl > 0 ? ttl : -1;
        }
        prev = curr;
        curr = curr->next;
    }

    fossil_cache_unlock();
    return -1;
}

bool fossil_bluecrab_cacheshell_touch(const char *key) {
    if (!key) return false;

    fossil_cache_lock();

    if (!g_cache.buckets) {
        fossil_cache_unlock();
        return false;
    }

    size_t index = fossil_cache_hash(key) % g_cache.bucket_count;
    fossil_cache_entry_t *prev = NULL;
    fossil_cache_entry_t *entry = g_cache.buckets[index];
    time_t now = time(NULL);

    while (entry) {
        if (strcmp(entry->key, key) == 0) {
            // If expired, remove and report false (do not count as miss/hit)
            if (entry->expiry > 0 && entry->expiry <= now) {
                if (prev)
                    prev->next = entry->next;
                else
                    g_cache.buckets[index] = entry->next;
                fossil_cache_free_entry(entry);
                if (g_cache.entry_count > 0) g_cache.entry_count--;
                g_cache.expired_evictions++;
                fossil_cache_unlock();
                return false;
            }

            // Update last access timestamp
            entry->last_access = now;
            if (entry->created == 0)
                entry->created = now;

            // For expiring entries, extend by the original TTL (expiry - created)
            if (entry->expiry > 0) {
                time_t original_ttl = 0;
                if (entry->created > 0 && entry->expiry > entry->created)
                    original_ttl = entry->expiry - entry->created;

                if (original_ttl > 0) {
                    entry->expiry = now + original_ttl;
                } else {
                    // Fallback: preserve remaining time (rare path if created not set)
                    time_t remaining = entry->expiry - now;
                    if (remaining > 0)
                        entry->expiry = now + remaining;
                }
            }

            fossil_cache_unlock();
            return true;
        }
        prev = entry;
        entry = entry->next;
    }

    fossil_cache_unlock();
    return false;
}

size_t fossil_bluecrab_cacheshell_evict_expired(void) {
    fossil_cache_lock();
    if (!g_cache.buckets) {
        fossil_cache_unlock();
        return 0;
    }

    size_t evicted = 0;
    time_t now = time(NULL);

    for (size_t i = 0; i < g_cache.bucket_count; ++i) {
        fossil_cache_entry_t *prev = NULL;
        fossil_cache_entry_t *curr = g_cache.buckets[i];

        while (curr) {
            if (curr->expiry > 0 && curr->expiry <= now) {
                fossil_cache_entry_t *dead = curr;

                if (prev)
                    prev->next = curr->next;
                else
                    g_cache.buckets[i] = curr->next;

                curr = curr->next;

                size_t bytes = sizeof(*dead) + dead->size + strlen(dead->key) + 1;
                if (g_cache.total_bytes >= bytes)
                    g_cache.total_bytes -= bytes;
                else
                    g_cache.total_bytes = 0;

                fossil_cache_free_entry(dead);
                if (g_cache.entry_count > 0)
                    g_cache.entry_count--;
                g_cache.expired_evictions++;
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
    if (!key || !data || size == 0)
        return false;

    fossil_cache_lock();

    if (!g_cache.buckets) {
        fossil_cache_unlock();
        return false;
    }

    size_t index = fossil_cache_hash(key) % g_cache.bucket_count;
    fossil_cache_entry_t *entry = g_cache.buckets[index];

    // Update existing entry (allowed even if at max capacity)
    while (entry) {
        if (strcmp(entry->key, key) == 0) {
            void *newblk = malloc(size);
            if (!newblk) {
                fossil_cache_unlock();
                return false;
            }
            memcpy(newblk, data, size);

            // Adjust memory usage accounting
            size_t old_bytes = sizeof(*entry) + entry->size + strlen(entry->key) + 1;
            size_t new_bytes = sizeof(*entry) + size + strlen(entry->key) + 1;
            if (new_bytes >= old_bytes)
                g_cache.total_bytes += (new_bytes - old_bytes);
            else
                g_cache.total_bytes -= (old_bytes - new_bytes);

            free(entry->data);
            entry->data = newblk;
            entry->size = size;
            entry->expiry = 0; // reset TTL on overwrite (intentional)
            time_t now = time(NULL);
            if (entry->created == 0) entry->created = now;
            entry->last_access = now;
            fossil_cache_unlock();
            return true;
        }
        entry = entry->next;
    }

    // Insertion path
    if (g_cache.max_entries && g_cache.entry_count >= g_cache.max_entries) {
        fossil_cache_unlock();
        return false;
    }

    fossil_cache_entry_t *new_entry = (fossil_cache_entry_t *)calloc(1, sizeof(fossil_cache_entry_t));
    if (!new_entry) {
        fossil_cache_unlock();
        return false;
    }

    new_entry->key = cacheshell_strdup(key);
    new_entry->data = malloc(size);
    if (!new_entry->key || !new_entry->data) {
        fossil_cache_free_entry(new_entry);
        fossil_cache_unlock();
        return false;
    }

    memcpy(new_entry->data, data, size);
    new_entry->size = size;
    new_entry->expiry = 0;
    time_t now = time(NULL);
    new_entry->created = now;
    new_entry->last_access = now;

    new_entry->next = g_cache.buckets[index];
    g_cache.buckets[index] = new_entry;
    g_cache.entry_count++;

    // Memory accounting
    g_cache.total_bytes += sizeof(*new_entry) + size + strlen(new_entry->key) + 1;

    fossil_cache_unlock();
    return true;
}

// Binary fetch (returns internal pointer, do NOT modify or free).
// Thread-safe lookup; pointer becomes invalid if the entry is later removed or updated.
const void *fossil_bluecrab_cacheshell_get_binary(const char *key, size_t *out_size) {
    if (!key)
        return NULL;

    fossil_cache_lock();
    fossil_cache_entry_t *entry = fossil_cache_find(key);
    if (!entry) {
        fossil_cache_unlock();
        return NULL;
    }

    if (out_size)
        *out_size = entry->size;

    const void *ptr = entry->data; // internal buffer
    fossil_cache_unlock();
    return ptr;
}

// ===========================================================
// Cache Management
// ===========================================================

void fossil_bluecrab_cacheshell_clear(void) {
    fossil_cache_lock();

    if (!g_cache.buckets) {
        fossil_cache_unlock();
        return;
    }

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
    g_cache.total_bytes = 0;          // reset accounted bytes
    // Do NOT reset hits/misses or expired_evictions to preserve lifetime stats.

    fossil_cache_unlock();
}

size_t fossil_bluecrab_cacheshell_count(void) {
    return g_cache.entry_count;
}

size_t fossil_bluecrab_cacheshell_memory_usage(void) {
    fossil_cache_lock();
    size_t bytes = g_cache.total_bytes; // O(1) tracked value
    fossil_cache_unlock();
    return bytes;
}

// ===========================================================
// Statistics / Thread Safety
// ===========================================================

void fossil_bluecrab_cacheshell_stats(size_t *out_hits, size_t *out_misses) {
    fossil_cache_lock();
    if (out_hits)   *out_hits   = g_cache.hits;
    if (out_misses) *out_misses = g_cache.misses;
    fossil_cache_unlock();
}

void fossil_bluecrab_cacheshell_stats_extended(
        size_t *out_hits,
        size_t *out_misses,
        size_t *out_entries,
        size_t *out_expired_evictions,
        size_t *out_memory_bytes,
        time_t *out_uptime_seconds) {

    fossil_cache_lock();
    if (out_hits)             *out_hits = g_cache.hits;
    if (out_misses)           *out_misses = g_cache.misses;
    if (out_entries)          *out_entries = g_cache.entry_count;
    if (out_expired_evictions)*out_expired_evictions = g_cache.expired_evictions;
    if (out_memory_bytes)     *out_memory_bytes = g_cache.total_bytes;
    if (out_uptime_seconds) {
        time_t now = time(NULL);
        *out_uptime_seconds = (g_cache.start_time > 0 && now >= g_cache.start_time)
                              ? (now - g_cache.start_time)
                              : 0;
    }
    fossil_cache_unlock();
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
    if (!g_cache.buckets) {
        fossil_cache_unlock();
        return;
    }

    time_t now = time(NULL);

    for (size_t i = 0; i < g_cache.bucket_count; ++i) {
        fossil_cache_entry_t *prev = NULL;
        fossil_cache_entry_t *entry = g_cache.buckets[i];

        while (entry) {
            // Remove expired inline (lazy eviction)
            if (entry->expiry > 0 && entry->expiry <= now) {
                fossil_cache_entry_t *dead = entry;
                if (prev)
                    prev->next = entry->next;
                else
                    g_cache.buckets[i] = entry->next;

                entry = entry->next;

                size_t bytes = sizeof(*dead) + dead->size + strlen(dead->key) + 1;
                if (g_cache.total_bytes >= bytes)
                    g_cache.total_bytes -= bytes;
                else
                    g_cache.total_bytes = 0;

                fossil_cache_free_entry(dead);
                if (g_cache.entry_count > 0)
                    g_cache.entry_count--;
                g_cache.expired_evictions++;
                continue;
            }

            // Invoke callback (note: lock is held; callback must not call cache APIs that lock)
            cb(entry->key, entry->data, entry->size, user_data);

            prev = entry;
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

    if (!g_cache.buckets) {
        fossil_cache_unlock();
        return false;
    }

    FILE *file = fopen(path, "wb");
    if (!file) {
        fossil_cache_unlock();
        return false;
    }

    bool ok = true;
    time_t now = time(NULL);

    for (size_t i = 0; ok && i < g_cache.bucket_count; ++i) {
        fossil_cache_entry_t *entry = g_cache.buckets[i];
        while (ok && entry) {
            // Skip expired entries (do not persist)
            if (entry->expiry > 0 && entry->expiry <= now) {
                entry = entry->next;
                continue;
            }

            // Reject keys with embedded newlines (would corrupt format)
            if (strchr(entry->key, '\n')) {
                entry = entry->next;
                continue;
            }

            if (fputs(entry->key, file) == EOF || fputc('\n', file) == EOF) {
                ok = false;
                break;
            }

            if (fwrite(&entry->size, sizeof(entry->size), 1, file) != 1) {
                ok = false;
                break;
            }

            if (entry->size > 0 &&
                fwrite(entry->data, 1, entry->size, file) != entry->size) {
                ok = false;
                break;
            }

            entry = entry->next;
        }
    }

    if (fclose(file) != 0)
        ok = false;

    fossil_cache_unlock();
    return ok;
}

bool fossil_bluecrab_cacheshell_load(const char *path) {
    FILE *file = fopen(path, "rb");
    if (!file)
        return false;

    // Use public clear (it manages its own locking)
    fossil_bluecrab_cacheshell_clear();

    bool ok = true;

    while (ok) {
        char key[256];
        if (!fgets(key, sizeof(key), file)) {
            // EOF (normal) or read error (ftell not needed) -> stop loop
            break;
        }

        // Remove trailing newline (expected from save)
        size_t klen = strlen(key);
        if (klen > 0 && key[klen - 1] == '\n')
            key[klen - 1] = '\0';

        // Read size
        size_t size = 0;
        if (fread(&size, sizeof(size), 1, file) != 1) {
            ok = false;
            break;
        }

        void *data = NULL;
        if (size > 0) {
            data = malloc(size);
            if (!data) {
                ok = false;
                break;
            }
            if (fread(data, 1, size, file) != size) {
                free(data);
                ok = false;
                break;
            }
        }

        // Insert (public API handles locking / accounting)
        if (!fossil_bluecrab_cacheshell_set_binary(key, data ? data : "", size)) {
            free(data);
            ok = false;
            break;
        }

        free(data);
    }

    fclose(file);
    return ok;
}

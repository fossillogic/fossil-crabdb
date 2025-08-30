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
#include "fossil/crabdb/cacheshell.h"

/**
 * @brief Custom strdup implementation.
 * 
 * @param src       The source string to duplicate.
 * @return          Pointer to newly allocated string, or NULL on failure.
 */
char* fossil_cacheshell_strdup(const char *src) {
    if (!src) return NULL;

    size_t len = 0;
    while (src[len] != '\0') len++;   // compute length manually

    char *dup = (char*)malloc(len + 1); // +1 for null terminator
    if (!dup) return NULL;

    for (size_t i = 0; i < len; i++) {
        dup[i] = src[i];
    }
    dup[len] = '\0';

    return dup;
}

// *****************************************************************************
// Internal Data Structures
// *****************************************************************************

typedef struct cache_entry {
    char *key;
    void *value;
    size_t value_size;
    time_t expire_at;   /**< 0 = never expires */
    bool in_use;
} cache_entry_t;

#define CACHE_TABLE_SIZE 1024  /* fixed-size for now */
static cache_entry_t cache_table[CACHE_TABLE_SIZE];

// *****************************************************************************
// Internal Helpers
// *****************************************************************************

static unsigned long cache_hash(const char *str) {
    // 64-bit variant of djb2 for stronger distribution
    unsigned long hash = 5381;
    int c;
    while ((c = *str++)) {
        hash = ((hash << 5) + hash) ^ (unsigned long)c;
    }
    return hash;
}

static cache_entry_t *find_entry(const char *key) {
    unsigned long h = cache_hash(key) % CACHE_TABLE_SIZE;
    for (size_t i = 0; i < CACHE_TABLE_SIZE; i++) {
        size_t idx = (h + i) % CACHE_TABLE_SIZE;
        if (!cache_table[idx].in_use) {
            return NULL;
        }
        if (strcmp(cache_table[idx].key, key) == 0) {
            if (cache_table[idx].expire_at != 0 && time(NULL) > cache_table[idx].expire_at) {
                // expired: cleanup
                free(cache_table[idx].key);
                free(cache_table[idx].value);
                cache_table[idx].in_use = false;
                return NULL;
            }
            return &cache_table[idx];
        }
    }
    return NULL;
}

static cache_entry_t *alloc_entry(const char *key) {
    unsigned long h = cache_hash(key) % CACHE_TABLE_SIZE;
    for (size_t i = 0; i < CACHE_TABLE_SIZE; i++) {
        size_t idx = (h + i) % CACHE_TABLE_SIZE;
        if (!cache_table[idx].in_use) {
            cache_table[idx].key = fossil_cacheshell_strdup(key);
            cache_table[idx].value = NULL;
            cache_table[idx].value_size = 0;
            cache_table[idx].expire_at = 0;
            cache_table[idx].in_use = true;
            return &cache_table[idx];
        }
    }
    return NULL; // cache full
}

// *****************************************************************************
// Public API Implementations
// *****************************************************************************

bool fossil_bluecrab_cacheshell_set(const char *key, const char *value) {
    return fossil_bluecrab_cacheshell_set_binary(key, value, strlen(value) + 1);
}

bool fossil_bluecrab_cacheshell_get(const char *key, char *out_value, size_t buffer_size) {
    cache_entry_t *e = find_entry(key);
    if (!e) return false;
    if (e->value_size > buffer_size) return false;
    memcpy(out_value, e->value, e->value_size);
    return true;
}

bool fossil_bluecrab_cacheshell_remove(const char *key) {
    cache_entry_t *e = find_entry(key);
    if (!e) return false;
    free(e->key);
    free(e->value);
    e->in_use = false;
    return true;
}

bool fossil_bluecrab_cacheshell_set_with_ttl(const char *key, const char *value, unsigned int ttl_sec) {
    if (!fossil_bluecrab_cacheshell_set(key, value)) return false;
    return fossil_bluecrab_cacheshell_expire(key, ttl_sec);
}

bool fossil_bluecrab_cacheshell_expire(const char *key, unsigned int ttl_sec) {
    cache_entry_t *e = find_entry(key);
    if (!e) return false;
    e->expire_at = time(NULL) + ttl_sec;
    return true;
}

int fossil_bluecrab_cacheshell_ttl(const char *key) {
    cache_entry_t *e = find_entry(key);
    if (!e) return -1;
    if (e->expire_at == 0) return -1;
    int remaining = (int)(e->expire_at - time(NULL));
    return remaining > 0 ? remaining : -1;
}

void fossil_bluecrab_cacheshell_clear(void) {
    for (size_t i = 0; i < CACHE_TABLE_SIZE; i++) {
        if (cache_table[i].in_use) {
            free(cache_table[i].key);
            free(cache_table[i].value);
            cache_table[i].in_use = false;
        }
    }
}

size_t fossil_bluecrab_cacheshell_count(void) {
    size_t count = 0;
    for (size_t i = 0; i < CACHE_TABLE_SIZE; i++) {
        if (cache_table[i].in_use) {
            if (cache_table[i].expire_at != 0 && time(NULL) > cache_table[i].expire_at) {
                fossil_bluecrab_cacheshell_remove(cache_table[i].key);
            } else {
                count++;
            }
        }
    }
    return count;
}

bool fossil_bluecrab_cacheshell_exists(const char *key) {
    return find_entry(key) != NULL;
}

bool fossil_bluecrab_cacheshell_set_binary(const char *key, const void *data, size_t size) {
    cache_entry_t *e = find_entry(key);
    if (!e) e = alloc_entry(key);
    if (!e) return false;

    free(e->value);
    e->value = malloc(size);
    if (!e->value) return false;
    memcpy(e->value, data, size);
    e->value_size = size;
    e->expire_at = 0;
    return true;
}

bool fossil_bluecrab_cacheshell_get_binary(const char *key, void *out_buf, size_t buf_size, size_t *out_size) {
    cache_entry_t *e = find_entry(key);
    if (!e) return false;
    if (e->value_size > buf_size) return false;
    memcpy(out_buf, e->value, e->value_size);
    if (out_size) *out_size = e->value_size;
    return true;
}

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
#include "fossil/crabdb/search.h"

char *custom_strdup(const char *str) {
    size_t len = strlen(str);
    char *copy = (char *)malloc(len + 1);
    if (!copy) {
        fprintf(stderr, "Failed to allocate memory for the string copy.\n");
        exit(EXIT_FAILURE);
    }
    strcpy(copy, str);
    return copy;
}

// *****************************************************************************
// Search operations
// *****************************************************************************

bool fossil_crabsearch_key_exists(const char *key) {
    return fossil_crabdb_key_exists(key);
}

char **fossil_crabsearch_values_by_pattern(const char *pattern, size_t *count) {
    return fossil_crabdb_search(pattern, count);
}

char **fossil_crabsearch_keys_by_pattern(const char *pattern, size_t *count) {
    return fossil_crabdb_list_keys(count);
}

void *fossil_crabsearch_value_by_key(const char *key, size_t *value_size) {
    return fossil_crabdb_get(key, value_size);
}

bool fossil_crabsearch_key_exists_case_insensitive(const char *key) {
    size_t count;
    char **keys = fossil_crabdb_list_keys(&count);
    if (!keys) {
        return false;
    }

    for (size_t i = 0; i < count; i++) {
        if (strncmp(keys[i], key, strlen(key)) == 0) {
            free(keys);
            return true;
        }
    }

    free(keys);
    return false;
}

char **fossil_crabsearch_keys_by_prefix(const char *prefix, size_t *count) {
    size_t prefix_len = strlen(prefix);
    size_t total_count = 0;
    size_t capacity = 10;
    char **matches = (char **)malloc(capacity * sizeof(char *));
    if (!matches) {
        fprintf(stderr, "Failed to allocate memory for the matches.\n");
        exit(EXIT_FAILURE);
    }

    size_t i = 0;
    size_t j = 0;
    size_t key_count;
    char **keys = fossil_crabdb_list_keys(&key_count);
    if (!keys) {
        free(matches);
        return NULL;
    }

    for (i = 0; i < key_count; i++) {
        if (strncmp(keys[i], prefix, prefix_len) == 0) {
            if (total_count >= capacity) {
                capacity *= 2;
                char **temp = (char **)realloc(matches, capacity * sizeof(char *));
                if (!temp) {
                    fprintf(stderr, "Failed to reallocate memory for the matches.\n");
                    exit(EXIT_FAILURE);
                }
                matches = temp;
            }
            matches[j] = custom_strdup(keys[i]);
            if (!matches[j]) {
                fprintf(stderr, "Failed to allocate memory for the match.\n");
                exit(EXIT_FAILURE);
            }
            j++;
            total_count++;
        }
    }

    free(keys);
    *count = total_count;
    return matches;
}

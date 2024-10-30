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
#include <ctype.h>

static bool matches_pattern(const char* key, const char* pattern) {
    // Simple wildcard matching implementation
    while (*pattern) {
        if (*pattern == '*') {
            // Skip '*' and check for the following character
            pattern++;
            if (*pattern == '\0') {
                return true; // Match if the pattern ends with '*'
            }
            while (*key && *key != *pattern) {
                key++; // Skip characters in key until a match
            }
        } else if (*key != *pattern) {
            return false; // Characters do not match
        }
        key++;
        pattern++;
    }
    return *key == '\0'; // Match only if we reached the end of the key
}

static bool is_valid_pattern(const char* pattern) {
    // Check if the pattern is non-empty and contains only valid characters
    if (!pattern || *pattern == '\0') return false;

    while (*pattern) {
        if (!isalnum(*pattern) && *pattern != '*' && *pattern != '?' && *pattern != '[' && *pattern != ']') {
            return false; // Invalid character found
        }
        pattern++;
    }
    return true; // All characters are valid
}

crabsearch_status_t fossil_crabsearch_search(fossil_crabdb_t* db, const char* pattern, char* result_buffer, size_t buffer_size, size_t* match_count, result_format_t format) {
    if (!db || !pattern || !result_buffer || !match_count) {
        return CRABSEARCH_INVALID_PARAM; // Check for valid parameters
    }

    if (!is_valid_pattern(pattern)) {
        return CRABSEARCH_INVALID_PARAM; // Check for valid parameters
    }

    size_t total_matches = 0;
    char temp_buffer[256]; // Temporary buffer for individual results
    // Reset the result buffer
    memset(result_buffer, 0, buffer_size);

    fossil_crabdb_node_t* current_node = db->head; // Start from the head of the linked list

    while (current_node != NULL) {
        const char* key = current_node->key; // Get the key
        const char* value = current_node->value; // Get the value

        if (matches_pattern(key, pattern)) {
            // If the key matches the pattern, add it to the result buffer
            size_t required_size;
            if (format == FORMAT_PLAIN_TEXT) {
                required_size = snprintf(temp_buffer, sizeof(temp_buffer), "%s: %s\n", key, value);
            } else if (format == FORMAT_JSON) {
                required_size = snprintf(temp_buffer, sizeof(temp_buffer), "{\"key\": \"%s\", \"value\": \"%s\"}\n", key, value);
            } else if (format == FORMAT_CSV) {
                // Handle CSV output
                required_size = snprintf(temp_buffer, sizeof(temp_buffer), "\"%s\",\"%s\"\n", key, value);
            } else {
                return CRABSEARCH_INVALID_PARAM; // Unsupported format
            }

            if (total_matches + required_size < buffer_size) {
                strcat(result_buffer, temp_buffer); // Append to result buffer
                total_matches++;
            } else {
                return CRABSEARCH_BUFFER_OVERFLOW; // Buffer overflow
            }
        }
        current_node = current_node->next; // Move to the next node
    }

    *match_count = total_matches; // Store the number of matches found
    return total_matches > 0 ? CRABSEARCH_SUCCESS : CRABSEARCH_NO_MATCHES;
}

crabsearch_status_t fossil_crabsearch_search_multiple(fossil_crabdb_t* db, const char** patterns, size_t num_patterns, char* result_buffer, size_t buffer_size, size_t* match_count) {
    if (!db || !patterns || num_patterns == 0 || !result_buffer || !match_count) {
        return CRABSEARCH_INVALID_PARAM; // Check for valid parameters
    }

    size_t total_matches = 0;
    // Reset the result buffer
    memset(result_buffer, 0, buffer_size);

    for (size_t i = 0; i < num_patterns; i++) {
        size_t match_count_temp;
        crabsearch_status_t status = fossil_crabsearch_search(db, patterns[i], result_buffer, buffer_size, &match_count_temp, FORMAT_PLAIN_TEXT); // Default to plain text
        if (status == CRABSEARCH_SUCCESS) {
            total_matches += match_count_temp; // Accumulate total matches
        } else if (status == CRABSEARCH_BUFFER_OVERFLOW) {
            return CRABSEARCH_BUFFER_OVERFLOW; // Buffer overflow
        }
    }

    *match_count = total_matches; // Store the total number of matches found
    return total_matches > 0 ? CRABSEARCH_SUCCESS : CRABSEARCH_NO_MATCHES;
}

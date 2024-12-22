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

extern char *custom_strdup(const char *str);

// *****************************************************************************
// Search API Functions
// *****************************************************************************

fossil_crabdb_entry_t* fossil_crabsearch_by_key(fossil_crabdb_book_t *book, const char *key) {
    if (book == NULL || key == NULL) {
        return NULL;
    }
    fossil_crabdb_page_t *current = book->head;
    while (current != NULL) {
        if (strcmp(current->entry.key, key) == 0) {
            return &current->entry;
        }
        current = current->next;
    }
    return NULL;
}

fossil_crabdb_book_t* fossil_crabsearch_by_value(fossil_crabdb_book_t *book, const char *value) {
    if (book == NULL || value == NULL) {
        return NULL;
    }
    fossil_crabdb_book_t *result = fossil_crabdb_init();
    if (result == NULL) {
        return NULL;
    }
    fossil_crabdb_page_t *current = book->head;
    while (current != NULL) {
        if (strcmp(current->entry.value, value) == 0) {
            fossil_crabdb_insert(result, custom_strdup(current->entry.key), custom_strdup(current->entry.value), current->entry.attributes);
        }
        current = current->next;
    }
    return result;
}

fossil_crabdb_book_t* fossil_crabsearch_by_predicate(fossil_crabdb_book_t *book, bool (*predicate)(fossil_crabdb_entry_t *)) {
    if (book == NULL || predicate == NULL) {
        return NULL;
    }
    fossil_crabdb_book_t *result = fossil_crabdb_init();
    if (result == NULL) {
        return NULL;
    }
    fossil_crabdb_page_t *current = book->head;
    while (current != NULL) {
        if (predicate(&current->entry)) {
            fossil_crabdb_insert(result, custom_strdup(current->entry.key), custom_strdup(current->entry.value), current->entry.attributes);
        }
        current = current->next;
    }
    return result;
}

fossil_crabdb_entry_t* fossil_crabsearch_first_by_predicate(fossil_crabdb_book_t *book, bool (*predicate)(fossil_crabdb_entry_t *)) {
    if (book == NULL || predicate == NULL) {
        return NULL;
    }
    fossil_crabdb_page_t *current = book->head;
    while (current != NULL) {
        if (predicate(&current->entry)) {
            return &current->entry;
        }
        current = current->next;
    }
    return NULL;
}

bool fossil_crabsearch_key_exists(fossil_crabdb_book_t *book, const char *key) {
    if (book == NULL || key == NULL) {
        return false;
    }
    fossil_crabdb_page_t *current = book->head;
    while (current != NULL) {
        if (strcmp(current->entry.key, key) == 0) {
            return true;
        }
        current = current->next;
    }
    return false;
}

fossil_crabdb_book_t* fossil_crabsearch_primary_keys(fossil_crabdb_book_t *book) {
    if (book == NULL) {
        return NULL;
    }
    fossil_crabdb_book_t *result = fossil_crabdb_init();
    if (result == NULL) {
        return NULL;
    }
    fossil_crabdb_page_t *current = book->head;
    while (current != NULL) {
        if (current->entry.attributes.is_primary_key) {
            fossil_crabdb_insert(result, custom_strdup(current->entry.key), custom_strdup(current->entry.value), current->entry.attributes);
        }
        current = current->next;
    }
    return result;
}

size_t fossil_crabsearch_count_by_predicate(fossil_crabdb_book_t *book, bool (*predicate)(fossil_crabdb_entry_t *)) {
    if (book == NULL || predicate == NULL) {
        return 0;
    }
    size_t count = 0;
    fossil_crabdb_page_t *current = book->head;
    while (current != NULL) {
        if (predicate(&current->entry)) {
            count++;
        }
        current = current->next;
    }
    return count;
}

// *****************************************************************************
// Search Utility Functions
// *****************************************************************************

bool fossil_crabsearch_is_non_nullable(fossil_crabdb_entry_t *entry) {
    return !entry->attributes.is_nullable;
}

bool fossil_crabsearch_is_unique(fossil_crabdb_entry_t *entry) {
    return entry->attributes.is_unique;
}

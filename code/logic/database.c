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
#include "fossil/crabdb/database.h"

static fossil_crabdb_transaction_t *active_transaction = NULL; // Current active transaction

char *custom_strdup(const char *str) {
    if (str == NULL) {
        return NULL;
    }
    size_t len = strlen(str);
    char *copy = (char *)malloc(len + 1);
    if (copy == NULL) {
        return NULL;
    }
    strcpy(copy, str);
    return copy;
}

// *****************************************************************************
// Database API Functions
// *****************************************************************************

/**
 * @brief Initializes a new empty database.
 */
fossil_crabdb_book_t* fossil_crabdb_init(void) {
    fossil_crabdb_book_t *book = (fossil_crabdb_book_t *)malloc(sizeof(fossil_crabdb_book_t));
    if (book == NULL) {
        return NULL;
    }
    book->head = NULL;
    book->tail = NULL;
    book->size = 0;
    return book;
}

/**
 * @brief Releases all resources used by the database.
 */
void fossil_crabdb_release(fossil_crabdb_book_t *book) {
    if (book == NULL) {
        return;
    }
    fossil_crabdb_page_t *current = book->head;
    while (current != NULL) {
        fossil_crabdb_page_t *next = current->next;
        free(current->entry.key);
        free(current->entry.value);
        free(current);
        current = next;
    }
    free(book);
}

/**
 * @brief Inserts a new key-value pair into the database.
 */
bool fossil_crabdb_insert(fossil_crabdb_book_t *book, const char *key, const char *value, fossil_crabdb_attributes_t attributes) {
    if (book == NULL || key == NULL || value == NULL) {
        return false;
    }
    fossil_crabdb_page_t *page = (fossil_crabdb_page_t *)malloc(sizeof(fossil_crabdb_page_t));
    if (page == NULL) {
        return false;
    }
    page->entry.key = custom_strdup(key);
    page->entry.value = custom_strdup(value);
    page->entry.attributes = attributes;
    page->next = NULL;
    if (book->head == NULL) {
        book->head = page;
        book->tail = page;
        page->prev = NULL;
    } else {
        book->tail->next = page;
        page->prev = book->tail;
        book->tail = page;
    }
    book->size++;
    return true;
}

/**
 * @brief Updates the value of an existing key.
 */
bool fossil_crabdb_update(fossil_crabdb_book_t *book, const char *key, const char *new_value) {
    if (book == NULL || key == NULL || new_value == NULL) {
        return false;
    }
    fossil_crabdb_page_t *current = book->head;
    while (current != NULL) {
        if (strcmp(current->entry.key, key) == 0) {
            free(current->entry.value);
            current->entry.value = custom_strdup(new_value);
            return true;
        }
        current = current->next;
    }
    return false;
}

/**
 * @brief Deletes an entry from the database by key.
 */
bool fossil_crabdb_delete(fossil_crabdb_book_t *book, const char *key) {
    if (book == NULL || key == NULL) {
        return false;
    }
    fossil_crabdb_page_t *current = book->head;
    while (current != NULL) {
        if (strcmp(current->entry.key, key) == 0) {
            if (current->prev != NULL) {
                current->prev->next = current->next;
            } else {
                book->head = current->next;
            }
            if (current->next != NULL) {
                current->next->prev = current->prev;
            } else {
                book->tail = current->prev;
            }
            free(current->entry.key);
            free(current->entry.value);
            free(current);
            book->size--;
            return true;
        }
        current = current->next;
    }
    return false;
}

/**
 * @brief Searches for an entry by key.
 */
fossil_crabdb_entry_t* fossil_crabdb_search(fossil_crabdb_book_t *book, const char *key) {
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

/**
 * @brief Displays all entries in the database.
 */
void fossil_crabdb_display(fossil_crabdb_book_t *book) {
    if (book == NULL) {
        return;
    }
    fossil_crabdb_page_t *current = book->head;
    while (current != NULL) {
        printf("Key: %s, Value: %s\n", current->entry.key, current->entry.value);
        current = current->next;
    }
}

/**
 * @brief Counts the number of entries in the database.
 */
size_t fossil_crabdb_size(fossil_crabdb_book_t *book) {
    if (book == NULL) {
        return 0;
    }
    return book->size;
}

/**
 * @brief Checks if the database is empty.
 */
bool fossil_crabdb_is_empty(fossil_crabdb_book_t *book) {
    return book == NULL || book->size == 0;
}

/**
 * @brief Clears all entries from the database.
 */
void fossil_crabdb_clear(fossil_crabdb_book_t *book) {
    if (book == NULL) {
        return;
    }
    fossil_crabdb_page_t *current = book->head;
    while (current != NULL) {
        fossil_crabdb_page_t *next = current->next;
        free(current->entry.key);
        free(current->entry.value);
        free(current);
        current = next;
    }
    book->head = NULL;
    book->tail = NULL;
    book->size = 0;
}

// *****************************************************************************
// Relational Operations
// *****************************************************************************

/**
 * @brief Joins two databases based on matching keys.
 */
fossil_crabdb_book_t* fossil_crabdb_join(fossil_crabdb_book_t *book1, fossil_crabdb_book_t *book2) {
    if (book1 == NULL || book2 == NULL) {
        return NULL;
    }
    fossil_crabdb_book_t *result = fossil_crabdb_init();
    if (result == NULL) {
        return NULL;
    }
    fossil_crabdb_page_t *current1 = book1->head;
    while (current1 != NULL) {
        fossil_crabdb_page_t *current2 = book2->head;
        while (current2 != NULL) {
            if (strcmp(current1->entry.key, current2->entry.key) == 0) {
                fossil_crabdb_insert(result, current1->entry.key, current1->entry.value, current1->entry.attributes);
                fossil_crabdb_insert(result, current2->entry.key, current2->entry.value, current2->entry.attributes);
            }
            current2 = current2->next;
        }
        current1 = current1->next;
    }
    return result;
}

/**
 * @brief Filters database entries based on a condition.
 */
fossil_crabdb_book_t* fossil_crabdb_filter(fossil_crabdb_book_t *book, bool (*predicate)(fossil_crabdb_entry_t *)) {
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
            fossil_crabdb_insert(result, current->entry.key, current->entry.value, current->entry.attributes);
        }
        current = current->next;
    }
    return result;
}

/**
 * @brief Sorts database entries based on a comparison function.
 */
void fossil_crabdb_sort(fossil_crabdb_book_t *book, int (*comparator)(fossil_crabdb_entry_t *, fossil_crabdb_entry_t *)) {
    if (book == NULL || comparator == NULL) {
        return;
    }
    fossil_crabdb_page_t *current = book->head;
    while (current != NULL) {
        fossil_crabdb_page_t *next = current->next;
        while (next != NULL) {
            if (comparator(&current->entry, &next->entry) > 0) {
                fossil_crabdb_entry_t temp = current->entry;
                current->entry = next->entry;
                next->entry = temp;
            }
            next = next->next;
        }
        current = current->next;
    }
}

/**
 * @brief Merges two databases into one.
 */
fossil_crabdb_book_t* fossil_crabdb_merge(fossil_crabdb_book_t *book1, fossil_crabdb_book_t *book2) {
    if (book1 == NULL || book2 == NULL) {
        return NULL;
    }
    fossil_crabdb_book_t *result = fossil_crabdb_init();
    if (result == NULL) {
        return NULL;
    }
    fossil_crabdb_page_t *current = book1->head;
    while (current != NULL) {
        fossil_crabdb_insert(result, current->entry.key, current->entry.value, current->entry.attributes);
        current = current->next;
    }
    current = book2->head;
    while (current != NULL) {
        fossil_crabdb_insert(result, current->entry.key, current->entry.value, current->entry.attributes);
        current = current->next;
    }
    return result;
}

// *****************************************************************************
// Transaction Management
// *****************************************************************************

/**
 * @brief Begins a new transaction.
 */
fossil_crabdb_transaction_t* fossil_crabdb_transaction_begin(fossil_crabdb_book_t *book, const char *name) {
    if (book == NULL || name == NULL) {
        return NULL;
    }
    fossil_crabdb_transaction_t *transaction = (fossil_crabdb_transaction_t *)malloc(sizeof(fossil_crabdb_transaction_t));
    if (transaction == NULL) {
        return NULL;
    }
    transaction->name = custom_strdup(name);
    transaction->snapshot = *book;
    transaction->next = active_transaction;
    active_transaction = transaction;
    return transaction;
}

/**
 * @brief Commits a transaction, saving changes.
 */
bool fossil_crabdb_transaction_commit(fossil_crabdb_book_t *book, fossil_crabdb_transaction_t *transaction) {
    if (book == NULL || transaction == NULL) {
        return false;
    }
    fossil_crabdb_transaction_t *current = active_transaction;
    while (current != NULL) {
        if (current == transaction) {
            active_transaction = current->next;
            fossil_crabdb_release(&current->snapshot);
            free(current->name);
            free(current);
            return true;
        }
        current = current->next;
    }
    return false;
}

/**
 * @brief Rolls back a transaction, restoring the previous state.
 */
bool fossil_crabdb_transaction_rollback(fossil_crabdb_book_t *book, fossil_crabdb_transaction_t *transaction) {
    if (book == NULL || transaction == NULL) {
        return false;
    }
    fossil_crabdb_transaction_t *current = active_transaction;
    while (current != NULL) {
        if (current == transaction) {
            active_transaction = current->next;
            *book = current->snapshot;
            fossil_crabdb_release(&current->snapshot);
            free(current->name);
            free(current);
            return true;
        }
        current = current->next;
    }
    return false;
}

/**
 * @brief Releases a transaction's resources.
 */
void fossil_crabdb_transaction_release(fossil_crabdb_transaction_t *transaction) {
    if (transaction == NULL) {
        return;
    }
    fossil_crabdb_release(&transaction->snapshot);
    free(transaction->name);
    free(transaction);
}

// *****************************************************************************
// Utility Functions
// *****************************************************************************

/**
 * @brief Dumps the database content to a file.
 */
bool fossil_crabdb_dump_to_file(fossil_crabdb_book_t *book, const char *filename) {
    if (book == NULL || filename == NULL) {
        return false;
    }
    FILE *file = fopen(filename, "w");
    if (file == NULL) {
        return false;
    }
    fossil_crabdb_page_t *current = book->head;
    while (current != NULL) {
        fprintf(file, "%s=%s\n", current->entry.key, current->entry.value);
        current = current->next;
    }
    fclose(file);
    return true;
}

/**
 * @brief Loads the database content from a file.
 */
bool fossil_crabdb_load_from_file(fossil_crabdb_book_t *book, const char *filename) {
    if (book == NULL || filename == NULL) {
        return false;
    }
    FILE *file = fopen(filename, "r");
    if (file == NULL) {
        return false;
    }
    char line[256];
    while (fgets(line, sizeof(line), file) != NULL) {
        char *key = strtok(line, "=");
        char *value = strtok(NULL, "\n");
        fossil_crabdb_insert(book, key, value, (fossil_crabdb_attributes_t){false, false, false});
    }
    fclose(file);
    return true;
}

/**
 * @brief Validates the integrity of the database.
 */
bool fossil_crabdb_validate(fossil_crabdb_book_t *book) {
    if (book == NULL) {
        return false;
    }
    fossil_crabdb_page_t *current = book->head;
    while (current != NULL) {
        if (current->entry.key == NULL || current->entry.value == NULL) {
            return false;
        }
        current = current->next;
    }
    return true;
}

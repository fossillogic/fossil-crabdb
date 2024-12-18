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
#ifndef FOSSIL_CRABDB_DB_H
#define FOSSIL_CRABDB_DB_H

#include <stdbool.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <stdio.h>
#include <time.h>

#ifdef __cplusplus
extern "C" {
#endif

// *****************************************************************************
// Enumerations for Data Types and Attributes
// *****************************************************************************

/**
 * @brief Represents additional attributes for each database entry.
 */
typedef struct {
    bool is_primary_key;          // Entry is a primary key
    bool is_unique;               // Entry is unique
    bool is_nullable;             // Entry allows null values
} fossil_crabdb_attributes_t;

// *****************************************************************************
// Database Structures
// *****************************************************************************

typedef struct {
    char *key;                            // Key
    char *value;                          // Value
    fossil_crabdb_attributes_t attributes;// Attributes
} fossil_crabdb_entry_t;

/**
 * Doubly linked list node representing a database entry.
 */
typedef struct fossil_crabdb_node {
    fossil_crabdb_entry_t entry;
    struct fossil_crabdb_node *next;
    struct fossil_crabdb_node *prev;
} fossil_crabdb_page_t;

/**
 * Represents the database as a doubly linked list (a "book").
 */
typedef struct {
    fossil_crabdb_page_t *head;
    fossil_crabdb_page_t *tail;
    size_t size;
} fossil_crabdb_book_t;

/**
 * Represents a transaction.
 */
typedef struct fossil_crabdb_transaction {
    char *name;                              // Transaction name
    fossil_crabdb_book_t snapshot;           // Database snapshot
    struct fossil_crabdb_transaction *next;  // Next transaction
} fossil_crabdb_transaction_t;

// *****************************************************************************
// Database API Functions
// *****************************************************************************

/**
 * @brief Initializes a new empty database.
 */
fossil_crabdb_book_t* fossil_crabdb_init(void);

/**
 * @brief Releases all resources used by the database.
 */
void fossil_crabdb_release(fossil_crabdb_book_t *book);

/**
 * @brief Inserts a new key-value pair into the database.
 */
bool fossil_crabdb_insert(fossil_crabdb_book_t *book, const char *key, const char *value, fossil_crabdb_attributes_t attributes);

/**
 * @brief Updates the value of an existing key.
 */
bool fossil_crabdb_update(fossil_crabdb_book_t *book, const char *key, const char *new_value);

/**
 * @brief Deletes an entry from the database by key.
 */
bool fossil_crabdb_delete(fossil_crabdb_book_t *book, const char *key);

/**
 * @brief Searches for an entry by key.
 */
fossil_crabdb_entry_t* fossil_crabdb_search(fossil_crabdb_book_t *book, const char *key);

/**
 * @brief Displays all entries in the database.
 */
void fossil_crabdb_display(fossil_crabdb_book_t *book);

/**
 * @brief Counts the number of entries in the database.
 */
size_t fossil_crabdb_size(fossil_crabdb_book_t *book);

/**
 * @brief Checks if the database is empty.
 */
bool fossil_crabdb_is_empty(fossil_crabdb_book_t *book);

/**
 * @brief Clears all entries from the database.
 */
void fossil_crabdb_clear(fossil_crabdb_book_t *book);

// *****************************************************************************
// Relational Operations
// *****************************************************************************

/**
 * @brief Joins two databases based on matching keys.
 */
fossil_crabdb_book_t* fossil_crabdb_join(fossil_crabdb_book_t *book1, fossil_crabdb_book_t *book2);

/**
 * @brief Filters database entries based on a condition.
 */
fossil_crabdb_book_t* fossil_crabdb_filter(fossil_crabdb_book_t *book, bool (*predicate)(fossil_crabdb_entry_t *));

/**
 * @brief Sorts database entries based on a comparison function.
 */
void fossil_crabdb_sort(fossil_crabdb_book_t *book, int (*comparator)(fossil_crabdb_entry_t *, fossil_crabdb_entry_t *));

/**
 * @brief Merges two databases into one.
 */
fossil_crabdb_book_t* fossil_crabdb_merge(fossil_crabdb_book_t *book1, fossil_crabdb_book_t *book2);

// *****************************************************************************
// Transaction Management
// *****************************************************************************

/**
 * @brief Begins a new transaction.
 */
fossil_crabdb_transaction_t* fossil_crabdb_transaction_begin(fossil_crabdb_book_t *book, const char *name);

/**
 * @brief Commits a transaction, saving changes.
 */
bool fossil_crabdb_transaction_commit(fossil_crabdb_book_t *book, fossil_crabdb_transaction_t *transaction);

/**
 * @brief Rolls back a transaction, restoring the previous state.
 */
bool fossil_crabdb_transaction_rollback(fossil_crabdb_book_t *book, fossil_crabdb_transaction_t *transaction);

/**
 * @brief Releases a transaction's resources.
 */
void fossil_crabdb_transaction_release(fossil_crabdb_transaction_t *transaction);

// *****************************************************************************
// Utility Functions
// *****************************************************************************

/**
 * @brief Dumps the database content to a file.
 */
bool fossil_crabdb_dump_to_file(fossil_crabdb_book_t *book, const char *filename);

/**
 * @brief Loads the database content from a file.
 */
bool fossil_crabdb_load_from_file(fossil_crabdb_book_t *book, const char *filename);

/**
 * @brief Validates the integrity of the database.
 */
bool fossil_crabdb_validate(fossil_crabdb_book_t *book);

#ifdef __cplusplus
}

#include <string>
#include <vector>

namespace fossil {

/**
 * @brief Represents a high-level interface to the Fossil CrabDB database.
 */
class CrabDB
{
public:
    CrabDB() {
        book = fossil_crabdb_init();
        if (!book) {
            throw std::runtime_error("Failed to initialize the database.");
        }
    }

    ~CrabDB() {
        fossil_crabdb_release(book);
    }

    void insert(const std::string &key, const std::string &value, fossil_crabdb_attributes_t attributes) {
        if (!fossil_crabdb_insert(book, key.c_str(), value.c_str(), attributes)) {
            throw std::runtime_error("Failed to insert entry.");
        }
    }

    void update(const std::string &key, const std::string &new_value) {
        if (!fossil_crabdb_update(book, key.c_str(), new_value.c_str())) {
            throw std::runtime_error("Failed to update entry.");
        }
    }

    void remove(const std::string &key) {
        if (!fossil_crabdb_delete(book, key.c_str())) {
            throw std::runtime_error("Failed to delete entry.");
        }
    }

    std::string search(const std::string &key) {
        fossil_crabdb_entry_t *entry = fossil_crabdb_search(book, key.c_str());
        if (!entry) {
            throw std::runtime_error("Entry not found.");
        }
        return std::string(entry->value);
    }

    void display() {
        fossil_crabdb_display(book);
    }

    size_t size() {
        return fossil_crabdb_size(book);
    }

    bool isEmpty() {
        return fossil_crabdb_is_empty(book);
    }

    void clear() {
        fossil_crabdb_clear(book);
    }

    void dumpToFile(const std::string &filename) {
        if (!fossil_crabdb_dump_to_file(book, filename.c_str())) {
            throw std::runtime_error("Failed to dump database to file.");
        }
    }

    void loadFromFile(const std::string &filename) {
        if (!fossil_crabdb_load_from_file(book, filename.c_str())) {
            throw std::runtime_error("Failed to load database from file.");
        }
    }

    bool validate() {
        return fossil_crabdb_validate(book);
    }

private:
    fossil_crabdb_book_t *book;
};

} // namespace fossil
#endif

#endif /* FOSSIL_CRABDB_FRAMEWORK_H */

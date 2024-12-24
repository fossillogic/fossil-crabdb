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
 * @brief Enumerates sorting orders for fossil_crabdb_sort operations.
 */
typedef enum {
    FOSSIL_CRABDB_SORT_ASCENDING,     // Sort in ascending order
    FOSSIL_CRABDB_SORT_DESCENDING     // Sort in descending order
} fossil_crabdb_sort_order_t;

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
 * @brief Merges two databases into one.
 */
fossil_crabdb_book_t* fossil_crabdb_merge(fossil_crabdb_book_t *book1, fossil_crabdb_book_t *book2);

/**
 * @brief Sorts the database by key.
 *
 * @param book          Pointer to the database (fossil_crabdb_book_t).
 * @param order         Sorting order (ascending or descending).
 * @return              0 on success, non-zero on error.
 */
int fossil_crabdb_sort(fossil_crabdb_book_t *book, fossil_crabdb_sort_order_t order);

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

/**
 * @brief Executes a query on the database.
 */
bool fossil_crabdb_execute_query(fossil_crabdb_book_t *book, const char *query);

// *****************************************************************************
// Search API Functions
// *****************************************************************************

/**
 * @brief Finds an entry by key in the database.
 *
 * @param book  Pointer to the database (fossil_crabdb_book_t).
 * @param key   Key to search for.
 * @return      Pointer to the entry if found, NULL otherwise.
 */
fossil_crabdb_entry_t* fossil_crabsearch_by_key(fossil_crabdb_book_t *book, const char *key);

/**
 * @brief Finds all entries with a specific value in the database.
 *
 * @param book      Pointer to the database (fossil_crabdb_book_t).
 * @param value     Value to search for.
 * @return          A new database containing all matching entries.
 */
fossil_crabdb_book_t* fossil_crabsearch_by_value(fossil_crabdb_book_t *book, const char *value);

/**
 * @brief Finds all entries matching a predicate.
 *
 * @param book          Pointer to the database (fossil_crabdb_book_t).
 * @param predicate     Function pointer to the predicate.
 * @return              A new database containing all matching entries.
 */
fossil_crabdb_book_t* fossil_crabsearch_by_predicate(fossil_crabdb_book_t *book, bool (*predicate)(fossil_crabdb_entry_t *));

/**
 * @brief Finds the first entry that matches a predicate.
 *
 * @param book          Pointer to the database (fossil_crabdb_book_t).
 * @param predicate     Function pointer to the predicate.
 * @return              Pointer to the first matching entry, or NULL if none found.
 */
fossil_crabdb_entry_t* fossil_crabsearch_first_by_predicate(fossil_crabdb_book_t *book, bool (*predicate)(fossil_crabdb_entry_t *));

/**
 * @brief Checks if a key exists in the database.
 *
 * @param book  Pointer to the database (fossil_crabdb_book_t).
 * @param key   Key to search for.
 * @return      True if the key exists, false otherwise.
 */
bool fossil_crabsearch_key_exists(fossil_crabdb_book_t *book, const char *key);

/**
 * @brief Finds all primary key entries in the database.
 *
 * @param book  Pointer to the database (fossil_crabdb_book_t).
 * @return      A new database containing all primary key entries.
 */
fossil_crabdb_book_t* fossil_crabsearch_primary_keys(fossil_crabdb_book_t *book);

/**
 * @brief Counts the entries that match a predicate.
 *
 * @param book          Pointer to the database (fossil_crabdb_book_t).
 * @param predicate     Function pointer to the predicate.
 * @return              Number of matching entries.
 */
size_t fossil_crabsearch_count_by_predicate(fossil_crabdb_book_t *book, bool (*predicate)(fossil_crabdb_entry_t *));

// *****************************************************************************
// Search Utility Functions
// *****************************************************************************

/**
 * @brief Example predicate to find non-nullable entries.
 *
 * @param entry  Pointer to a database entry (fossil_crabdb_entry_t).
 * @return       True if the entry is non-nullable, false otherwise.
 */
bool fossil_crabsearch_is_non_nullable(fossil_crabdb_entry_t *entry);

/**
 * @brief Example predicate to find unique entries.
 *
 * @param entry  Pointer to a database entry (fossil_crabdb_entry_t).
 * @return       True if the entry is unique, false otherwise.
 */
bool fossil_crabsearch_is_unique(fossil_crabdb_entry_t *entry);

// *****************************************************************************
// INI Storage API
// *****************************************************************************

/**
 * @brief Saves the database content to an INI file.
 *
 * @param book          Pointer to the database (fossil_crabdb_book_t).
 * @param filename      Path to the output INI file.
 * @return              true on success, false on failure.
 */
bool fossil_crabstore_save_to_ini(const fossil_crabdb_book_t *book, const char *filename);

/**
 * @brief Loads the database content from an INI file.
 *
 * @param book          Pointer to the database (fossil_crabdb_book_t).
 * @param filename      Path to the input INI file.
 * @return              true on success, false on failure.
 */
bool fossil_crabstore_load_from_ini(fossil_crabdb_book_t *book, const char *filename);

// *****************************************************************************
// CSV Storage API
// *****************************************************************************

/**
 * @brief Saves the database content to a CSV file.
 *
 * @param book          Pointer to the database (fossil_crabdb_book_t).
 * @param filename      Path to the output CSV file.
 * @return              true on success, false on failure.
 */
bool fossil_crabstore_save_to_csv(const fossil_crabdb_book_t *book, const char *filename);

/**
 * @brief Loads the database content from a CSV file.
 *
 * @param book          Pointer to the database (fossil_crabdb_book_t).
 * @param filename      Path to the input CSV file.
 * @return              true on success, false on failure.
 */
bool fossil_crabstore_load_from_csv(fossil_crabdb_book_t *book, const char *filename);

// *****************************************************************************
// Utility Functions for Storage
// *****************************************************************************

/**
 * @brief Validates the structure of an INI file before loading.
 *
 * @param filename      Path to the INI file.
 * @return              true if valid, false otherwise.
 */
bool fossil_crabstore_validate_ini(const char *filename);

/**
 * @brief Validates the structure of a CSV file before loading.
 *
 * @param filename      Path to the CSV file.
 * @return              true if valid, false otherwise.
 */
bool fossil_crabstore_validate_csv(const char *filename);

#ifdef __cplusplus
}

#include <string>
#include <stdexcept>
#include <vector>

namespace fossil {

//
class CrabStore {
public:
    static bool saveToIni(const CrabDB &db, const std::string &filename) {
        return fossil_crabstore_save_to_ini(db.book, filename.c_str());
    }

    static bool loadFromIni(CrabDB &db, const std::string &filename) {
        return fossil_crabstore_load_from_ini(db.book, filename.c_str());
    }

    static bool saveToCsv(const CrabDB &db, const std::string &filename) {
        return fossil_crabstore_save_to_csv(db.book, filename.c_str());
    }

    static bool loadFromCsv(CrabDB &db, const std::string &filename) {
        return fossil_crabstore_load_from_csv(db.book, filename.c_str());
    }

    static bool validateIni(const std::string &filename) {
        return fossil_crabstore_validate_ini(filename.c_str());
    }

    static bool validateCsv(const std::string &filename) {
        return fossil_crabstore_validate_csv(filename.c_str());
    }
};

class CrabSearch {
public:
    static fossil_crabdb_entry_t* byKey(CrabDB &db, const std::string &key) {
        return fossil_crabsearch_by_key(db.book, key.c_str());
    }

    static CrabDB byValue(CrabDB &db, const std::string &value) {
        CrabDB result;
        result.book = fossil_crabsearch_by_value(db.book, value.c_str());
        return result;
    }

    static CrabDB byPredicate(CrabDB &db, bool (*predicate)(fossil_crabdb_entry_t *)) {
        CrabDB result;
        result.book = fossil_crabsearch_by_predicate(db.book, predicate);
        return result;
    }

    static fossil_crabdb_entry_t* firstByPredicate(CrabDB &db, bool (*predicate)(fossil_crabdb_entry_t *)) {
        return fossil_crabsearch_first_by_predicate(db.book, predicate);
    }

    static bool keyExists(CrabDB &db, const std::string &key) {
        return fossil_crabsearch_key_exists(db.book, key.c_str());
    }

    static CrabDB primaryKeys(CrabDB &db) {
        CrabDB result;
        result.book = fossil_crabsearch_primary_keys(db.book);
        return result;
    }

    static size_t countByPredicate(CrabDB &db, bool (*predicate)(fossil_crabdb_entry_t *)) {
        return fossil_crabsearch_count_by_predicate(db.book, predicate);
    }
};

class CrabDB {
public:
    CrabDB() {
        book = fossil_crabdb_init();
        if (!book) {
            throw std::runtime_error("Failed to initialize database");
        }
    }

    ~CrabDB() {
        fossil_crabdb_release(book);
    }

    bool insert(const std::string &key, const std::string &value, fossil_crabdb_attributes_t attributes) {
        return fossil_crabdb_insert(book, key.c_str(), value.c_str(), attributes);
    }

    bool update(const std::string &key, const std::string &new_value) {
        return fossil_crabdb_update(book, key.c_str(), new_value.c_str());
    }

    bool remove(const std::string &key) {
        return fossil_crabdb_delete(book, key.c_str());
    }

    fossil_crabdb_entry_t* search(const std::string &key) {
        return fossil_crabdb_search(book, key.c_str());
    }

    void display() {
        fossil_crabdb_display(book);
    }

    size_t size() const {
        return fossil_crabdb_size(book);
    }

    bool isEmpty() const {
        return fossil_crabdb_is_empty(book);
    }

    void clear() {
        fossil_crabdb_clear(book);
    }

    bool dumpToFile(const std::string &filename) {
        return fossil_crabdb_dump_to_file(book, filename.c_str());
    }

    bool loadFromFile(const std::string &filename) {
        return fossil_crabdb_load_from_file(book, filename.c_str());
    }

    bool validate() const {
        return fossil_crabdb_validate(book);
    }

    bool executeQuery(const std::string &query) {
        return fossil_crabdb_execute_query(book, query.c_str());
    }

private:
    fossil_crabdb_book_t *book;
};

} // namespace fossil
#endif

#endif /* FOSSIL_CRABDB_FRAMEWORK_H */

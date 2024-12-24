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
#include <fossil/test/framework.h>

#include "fossil/crabdb/framework.h"

// * * * * * * * * * * * * * * * * * * * * * * * *
// * Fossil Logic Test Utilities
// * * * * * * * * * * * * * * * * * * * * * * * *
// Setup steps for things like test fixtures and
// mock objects are set here.
// * * * * * * * * * * * * * * * * * * * * * * * *

FOSSIL_TEST_SUITE(cpp_crabdb_fixture);

FOSSIL_SETUP(cpp_crabdb_fixture) {
    // Setup the test fixture
}

FOSSIL_TEARDOWN(cpp_crabdb_fixture) {
    // Teardown the test fixture
}

// * * * * * * * * * * * * * * * * * * * * * * * *
// * Fossil Logic Test Blue CrabDB Database
// * * * * * * * * * * * * * * * * * * * * * * * *

// Test case for initializing a new database
FOSSIL_TEST_CASE(cpp_test_crabdb_init) {
    fossil_crabdb_book_t *book = fossil_crabdb_init();
    ASSUME_NOT_CNULL(book);
    ASSUME_ITS_TRUE(fossil_crabdb_is_empty(book));
    fossil_crabdb_release(book);
}

// Test case for inserting a new key-value pair
FOSSIL_TEST_CASE(cpp_test_crabdb_insert) {
    fossil_crabdb_book_t *book = fossil_crabdb_init();
    bool result = fossil_crabdb_insert(book, const_cast<char *>("key1"), const_cast<char *>("value1"), (fossil_crabdb_attributes_t){false, false, false});
    ASSUME_ITS_TRUE(result);
    ASSUME_ITS_FALSE(fossil_crabdb_is_empty(book));
    fossil_crabdb_release(book);
}

// Test case for updating an existing key
FOSSIL_TEST_CASE(cpp_test_crabdb_update) {
    fossil_crabdb_book_t *book = fossil_crabdb_init();
    fossil_crabdb_insert(book, const_cast<char *>("key1"), const_cast<char *>("value1"), (fossil_crabdb_attributes_t){false, false, false});
    bool result = fossil_crabdb_update(book, const_cast<char *>("key1"), const_cast<char *>("new_value1"));
    ASSUME_ITS_TRUE(result);
    fossil_crabdb_entry_t *entry = fossil_crabdb_search(book, const_cast<char *>("key1"));
    ASSUME_NOT_CNULL(entry);
    ASSUME_ITS_TRUE(strcmp(entry->value, "new_value1") == 0);
    fossil_crabdb_release(book);
}

// Test case for deleting an entry by key
FOSSIL_TEST_CASE(cpp_test_crabdb_delete) {
    fossil_crabdb_book_t *book = fossil_crabdb_init();
    fossil_crabdb_insert(book, const_cast<char *>("key1"), const_cast<char *>("value1"), (fossil_crabdb_attributes_t){false, false, false});
    bool result = fossil_crabdb_delete(book, const_cast<char *>("key1"));
    ASSUME_ITS_TRUE(result);
    ASSUME_ITS_TRUE(fossil_crabdb_is_empty(book));
    fossil_crabdb_release(book);
}

// Test case for searching an entry by key
FOSSIL_TEST_CASE(cpp_test_crabdb_search) {
    fossil_crabdb_book_t *book = fossil_crabdb_init();
    fossil_crabdb_insert(book, const_cast<char *>("key1"), const_cast<char *>("value1"), (fossil_crabdb_attributes_t){false, false, false});
    fossil_crabdb_entry_t *entry = fossil_crabdb_search(book, const_cast<char *>("key1"));
    ASSUME_NOT_CNULL(entry);
    ASSUME_ITS_TRUE(strcmp(entry->value, "value1") == 0);
    fossil_crabdb_release(book);
}

// Test case for clearing all entries from the database
FOSSIL_TEST_CASE(cpp_test_crabdb_clear) {
    fossil_crabdb_book_t *book = fossil_crabdb_init();
    fossil_crabdb_insert(book, const_cast<char *>("key1"), const_cast<char *>("value1"), (fossil_crabdb_attributes_t){false, false, false});
    fossil_crabdb_clear(book);
    ASSUME_ITS_TRUE(fossil_crabdb_is_empty(book));
    fossil_crabdb_release(book);
}

// Test case for joining two databases
FOSSIL_TEST_CASE(cpp_test_crabdb_join) {
    fossil_crabdb_book_t *book1 = fossil_crabdb_init();
    fossil_crabdb_book_t *book2 = fossil_crabdb_init();
    fossil_crabdb_insert(book1, const_cast<char *>("key1"), const_cast<char *>("value1"), (fossil_crabdb_attributes_t){false, false, false});
    fossil_crabdb_insert(book2, const_cast<char *>("key1"), const_cast<char *>("value2"), (fossil_crabdb_attributes_t){false, false, false});
    fossil_crabdb_book_t *result = fossil_crabdb_join(book1, book2);
    ASSUME_NOT_CNULL(result);
    ASSUME_ITS_TRUE(fossil_crabdb_size(result) == 2);
    fossil_crabdb_release(book1);
    fossil_crabdb_release(book2);
    fossil_crabdb_release(result);
}

// Test case for filtering database entries
static bool filter_key1(fossil_crabdb_entry_t *entry) {
    return strcmp(entry->key, "key1") == 0;
}

FOSSIL_TEST_CASE(cpp_test_crabdb_filter) {
    fossil_crabdb_book_t *book = fossil_crabdb_init();
    fossil_crabdb_insert(book, const_cast<char *>("key1"), const_cast<char *>("value1"), (fossil_crabdb_attributes_t){false, false, false});
    fossil_crabdb_insert(book, const_cast<char *>("key2"), const_cast<char *>("value2"), (fossil_crabdb_attributes_t){false, false, false});
    fossil_crabdb_book_t *result = fossil_crabdb_filter(book, filter_key1);
    ASSUME_NOT_CNULL(result);
    ASSUME_ITS_TRUE(fossil_crabdb_size(result) == 1);
    fossil_crabdb_release(book);
    fossil_crabdb_release(result);
}

// Test case for merging two databases
FOSSIL_TEST_CASE(cpp_test_crabdb_merge) {
    fossil_crabdb_book_t *book1 = fossil_crabdb_init();
    fossil_crabdb_book_t *book2 = fossil_crabdb_init();
    fossil_crabdb_insert(book1, const_cast<char *>("key1"), const_cast<char *>("value1"), (fossil_crabdb_attributes_t){false, false, false});
    fossil_crabdb_insert(book2, const_cast<char *>("key2"), const_cast<char *>("value2"), (fossil_crabdb_attributes_t){false, false, false});
    fossil_crabdb_book_t *result = fossil_crabdb_merge(book1, book2);
    ASSUME_NOT_CNULL(result);
    ASSUME_ITS_TRUE(fossil_crabdb_size(result) == 2);
    fossil_crabdb_release(book1);
    fossil_crabdb_release(book2);
    fossil_crabdb_release(result);
}

// Test case for validating the integrity of the database
FOSSIL_TEST_CASE(cpp_test_crabdb_validate) {
    fossil_crabdb_book_t *book = fossil_crabdb_init();
    fossil_crabdb_insert(book, const_cast<char *>("key1"), const_cast<char *>("value1"), (fossil_crabdb_attributes_t){false, false, false});
    bool result = fossil_crabdb_validate(book);
    ASSUME_ITS_TRUE(result);
    fossil_crabdb_release(book);
}

// Test case for sorting the database in ascending order
FOSSIL_TEST_CASE(cpp_test_crabdb_sort_ascending) {
    fossil_crabdb_book_t *book = fossil_crabdb_init();
    fossil_crabdb_insert(book, const_cast<char *>("key3"), const_cast<char *>("value3"), (fossil_crabdb_attributes_t){false, false, false});
    fossil_crabdb_insert(book, const_cast<char *>("key1"), const_cast<char *>("value1"), (fossil_crabdb_attributes_t){false, false, false});
    fossil_crabdb_insert(book, const_cast<char *>("key2"), const_cast<char *>("value2"), (fossil_crabdb_attributes_t){false, false, false});
    int result = fossil_crabdb_sort(book, FOSSIL_CRABDB_SORT_ASCENDING);
    ASSUME_ITS_TRUE(result == 0);
    fossil_crabdb_page_t *current = book->head;
    ASSUME_ITS_TRUE(strcmp(current->entry.key, "key1") == 0);
    fossil_crabdb_release(book);
}

// Test case for sorting the database in descending order
FOSSIL_TEST_CASE(cpp_test_crabdb_sort_descending) {
    fossil_crabdb_book_t *book = fossil_crabdb_init();
    fossil_crabdb_insert(book, const_cast<char *>("key1"), const_cast<char *>("value1"), (fossil_crabdb_attributes_t){false, false, false});
    fossil_crabdb_insert(book, const_cast<char *>("key3"), const_cast<char *>("value3"), (fossil_crabdb_attributes_t){false, false, false});
    fossil_crabdb_insert(book, const_cast<char *>("key2"), const_cast<char *>("value2"), (fossil_crabdb_attributes_t){false, false, false});
    int result = fossil_crabdb_sort(book, FOSSIL_CRABDB_SORT_DESCENDING);
    ASSUME_ITS_TRUE(result == 0);
    fossil_crabdb_page_t *current = book->head;
    ASSUME_ITS_TRUE(strcmp(current->entry.key, "key3") == 0);
    fossil_crabdb_release(book);
}

// Test case for saving the database content to an INI file
FOSSIL_TEST_CASE(cpp_test_crabstore_save_to_ini) {
    fossil_crabdb_book_t *book = fossil_crabdb_init();
    fossil_crabdb_insert(book, const_cast<char *>("key1"), const_cast<char *>("value1"), (fossil_crabdb_attributes_t){false, false, false});
    bool result = fossil_crabstore_save_to_ini(book, const_cast<char *>("test_save.ini"));
    ASSUME_ITS_TRUE(result);
    fossil_crabdb_release(book);
}

// Test case for loading the database content from an INI file
FOSSIL_TEST_CASE(cpp_test_crabstore_load_from_ini) {
    fossil_crabdb_book_t *book = fossil_crabdb_init();
    bool result = fossil_crabstore_load_from_ini(book, const_cast<char *>("test_save.ini"));
    ASSUME_ITS_TRUE(result);
    fossil_crabdb_entry_t *entry = fossil_crabdb_search(book, const_cast<char *>("key1"));
    ASSUME_NOT_CNULL(entry);
    ASSUME_ITS_TRUE(strcmp(entry->value, "value1") == 0);
    fossil_crabdb_release(book);
}

// Test case for saving the database content to a CSV file
FOSSIL_TEST_CASE(cpp_test_crabstore_save_to_csv) {
    fossil_crabdb_book_t *book = fossil_crabdb_init();
    fossil_crabdb_insert(book, const_cast<char *>("key1"), const_cast<char *>("value1"), (fossil_crabdb_attributes_t){false, false, false});
    bool result = fossil_crabstore_save_to_csv(book, const_cast<char *>("test_save.csv"));
    ASSUME_ITS_TRUE(result);
    fossil_crabdb_release(book);
}

// Test case for loading the database content from a CSV file
FOSSIL_TEST_CASE(cpp_test_crabstore_load_from_csv) {
    fossil_crabdb_book_t *book = fossil_crabdb_init();
    bool result = fossil_crabstore_load_from_csv(book, const_cast<char *>("test_save.csv"));
    ASSUME_ITS_TRUE(result);
    fossil_crabdb_entry_t *entry = fossil_crabdb_search(book, const_cast<char *>("key1"));
    ASSUME_NOT_CNULL(entry);
    ASSUME_ITS_TRUE(strcmp(entry->value, "value1") == 0);
    fossil_crabdb_release(book);
}

// Test case for validating the structure of an INI file
FOSSIL_TEST_CASE(cpp_test_crabstore_validate_ini) {
    bool result = fossil_crabstore_validate_ini(const_cast<char *>("test_save.ini"));
    ASSUME_ITS_TRUE(result);
}

// Test case for validating the structure of a CSV file
FOSSIL_TEST_CASE(cpp_test_crabstore_validate_csv) {
    bool result = fossil_crabstore_validate_csv(const_cast<char *>("test_save.csv"));
    ASSUME_ITS_TRUE(result);
}

// Test case for searching an entry by value
FOSSIL_TEST_CASE(cpp_test_crabsearch_by_value) {
    fossil_crabdb_book_t *book = fossil_crabdb_init();
    fossil_crabdb_insert(book, const_cast<char *>("key1"), const_cast<char *>("value1"), (fossil_crabdb_attributes_t){false, false, false});
    fossil_crabdb_insert(book, const_cast<char *>("key2"), const_cast<char *>("value1"), (fossil_crabdb_attributes_t){false, false, false});
    fossil_crabdb_book_t *result = fossil_crabsearch_by_value(book, const_cast<char *>("value1"));
    ASSUME_NOT_CNULL(result);
    ASSUME_ITS_TRUE(fossil_crabdb_size(result) == 2);
    fossil_crabdb_release(book);
    fossil_crabdb_release(result);
}

// Test case for searching entries by predicate
static bool predicate_key1(fossil_crabdb_entry_t *entry) {
    return strcmp(entry->key, "key1") == 0;
}

FOSSIL_TEST_CASE(cpp_test_crabsearch_by_predicate) {
    fossil_crabdb_book_t *book = fossil_crabdb_init();
    fossil_crabdb_insert(book, const_cast<char *>("key1"), const_cast<char *>("value1"), (fossil_crabdb_attributes_t){false, false, false});
    fossil_crabdb_insert(book, const_cast<char *>("key2"), const_cast<char *>("value2"), (fossil_crabdb_attributes_t){false, false, false});
    fossil_crabdb_book_t *result = fossil_crabsearch_by_predicate(book, predicate_key1);
    ASSUME_NOT_CNULL(result);
    ASSUME_ITS_TRUE(fossil_crabdb_size(result) == 1);
    fossil_crabdb_release(book);
    fossil_crabdb_release(result);
}

// Test case for searching the first entry by predicate
FOSSIL_TEST_CASE(cpp_test_crabsearch_first_by_predicate) {
    fossil_crabdb_book_t *book = fossil_crabdb_init();
    fossil_crabdb_insert(book, const_cast<char *>("key1"), const_cast<char *>("value1"), (fossil_crabdb_attributes_t){false, false, false});
    fossil_crabdb_insert(book, const_cast<char *>("key2"), const_cast<char *>("value2"), (fossil_crabdb_attributes_t){false, false, false});
    fossil_crabdb_entry_t *entry = fossil_crabsearch_first_by_predicate(book, predicate_key1);
    ASSUME_NOT_CNULL(entry);
    ASSUME_ITS_TRUE(strcmp(entry->key, "key1") == 0);
    fossil_crabdb_release(book);
}

// Test case for checking if a key exists
FOSSIL_TEST_CASE(cpp_test_crabsearch_key_exists) {
    fossil_crabdb_book_t *book = fossil_crabdb_init();
    fossil_crabdb_insert(book, const_cast<char *>("key1"), const_cast<char *>("value1"), (fossil_crabdb_attributes_t){false, false, false});
    bool exists = fossil_crabsearch_key_exists(book, const_cast<char *>("key1"));
    ASSUME_ITS_TRUE(exists);
    exists = fossil_crabsearch_key_exists(book, const_cast<char *>("key2"));
    ASSUME_ITS_FALSE(exists);
    fossil_crabdb_release(book);
}

// Test case for finding all primary key entries
FOSSIL_TEST_CASE(cpp_test_crabsearch_primary_keys) {
    fossil_crabdb_book_t *book = fossil_crabdb_init();
    fossil_crabdb_insert(book, const_cast<char *>("key1"), const_cast<char *>("value1"), (fossil_crabdb_attributes_t){true, false, false});
    fossil_crabdb_insert(book, const_cast<char *>("key2"), const_cast<char *>("value2"), (fossil_crabdb_attributes_t){false, false, false});
    fossil_crabdb_book_t *result = fossil_crabsearch_primary_keys(book);
    ASSUME_NOT_CNULL(result);
    ASSUME_ITS_TRUE(fossil_crabdb_size(result) == 1);
    fossil_crabdb_release(book);
    fossil_crabdb_release(result);
}

// Test case for counting entries by predicate
FOSSIL_TEST_CASE(cpp_test_crabsearch_count_by_predicate) {
    fossil_crabdb_book_t *book = fossil_crabdb_init();
    fossil_crabdb_insert(book, const_cast<char *>("key1"), const_cast<char *>("value1"), (fossil_crabdb_attributes_t){false, false, false});
    fossil_crabdb_insert(book, const_cast<char *>("key2"), const_cast<char *>("value1"), (fossil_crabdb_attributes_t){false, false, false});
    size_t count = fossil_crabsearch_count_by_predicate(book, predicate_key1);
    ASSUME_ITS_TRUE(count == 1);
    fossil_crabdb_release(book);
}

// Test case for finding non-nullable entries
FOSSIL_TEST_CASE(cpp_test_crabsearch_is_non_nullable) {
    fossil_crabdb_book_t *book = fossil_crabdb_init();
    fossil_crabdb_insert(book, const_cast<char *>("key1"), const_cast<char *>("value1"), (fossil_crabdb_attributes_t){false, true, false});
    fossil_crabdb_insert(book, const_cast<char *>("key2"), const_cast<char *>("value2"), (fossil_crabdb_attributes_t){false, false, false});
    fossil_crabdb_book_t *result = fossil_crabsearch_by_predicate(book, fossil_crabsearch_is_non_nullable);
    ASSUME_NOT_CNULL(result);
    ASSUME_ITS_TRUE(fossil_crabdb_size(result) == 1);
    fossil_crabdb_release(book);
    fossil_crabdb_release(result);
}

// Test case for finding unique entries
FOSSIL_TEST_CASE(cpp_test_crabsearch_is_unique) {
    fossil_crabdb_book_t *book = fossil_crabdb_init();
    fossil_crabdb_insert(book, const_cast<char *>("key1"), const_cast<char *>("value1"), (fossil_crabdb_attributes_t){false, false, true});
    fossil_crabdb_insert(book, const_cast<char *>("key2"), const_cast<char *>("value2"), (fossil_crabdb_attributes_t){false, false, false});
    fossil_crabdb_book_t *result = fossil_crabsearch_by_predicate(book, fossil_crabsearch_is_unique);
    ASSUME_NOT_CNULL(result);
    ASSUME_ITS_TRUE(fossil_crabdb_size(result) == 1);
    fossil_crabdb_release(book);
    fossil_crabdb_release(result);
}

// Test case for dumping database content to a file
FOSSIL_TEST_CASE(cpp_test_crabdb_dump_to_file) {
    fossil_crabdb_book_t *book = fossil_crabdb_init();
    fossil_crabdb_insert(book, const_cast<char *>("key1"), const_cast<char *>("value1"), (fossil_crabdb_attributes_t){false, false, false});
    bool result = fossil_crabdb_dump_to_file(book, const_cast<char *>("test_dump.txt"));
    ASSUME_ITS_TRUE(result);
    fossil_crabdb_release(book);
}

// Test case for loading database content from a file
FOSSIL_TEST_CASE(cpp_test_crabdb_load_from_file) {
    fossil_crabdb_book_t *book = fossil_crabdb_init();
    bool result = fossil_crabdb_load_from_file(book, const_cast<char *>("test_dump.txt"));
    ASSUME_ITS_TRUE(result);
    fossil_crabdb_entry_t *entry = fossil_crabdb_search(book, const_cast<char *>("key1"));
    ASSUME_NOT_CNULL(entry);
    ASSUME_ITS_TRUE(strcmp(entry->value, "value1") == 0);
    fossil_crabdb_release(book);
}

// Test case for executing an insert query
FOSSIL_TEST_CASE(cpp_test_crabdb_execute_insert_query) {
    fossil_crabdb_book_t *book = fossil_crabdb_init();
    bool result = fossil_crabdb_execute_query(book, const_cast<char *>("insert('key1', 'value1', primary_key: false, unique: false);"));
    ASSUME_ITS_TRUE(result);
    fossil_crabdb_entry_t *entry = fossil_crabdb_search(book, const_cast<char *>("key1"));
    ASSUME_NOT_CNULL(entry);
    ASSUME_ITS_TRUE(strcmp(entry->value, "value1") == 0);
    fossil_crabdb_release(book);
}

// Test case for executing an update query
FOSSIL_TEST_CASE(cpp_test_crabdb_execute_update_query) {
    fossil_crabdb_book_t *book = fossil_crabdb_init();
    fossil_crabdb_insert(book, const_cast<char *>("key1"), const_cast<char *>("value1"), (fossil_crabdb_attributes_t){false, false, false});
    bool result = fossil_crabdb_execute_query(book, const_cast<char *>("update('key1', new_value: 'new_value1');"));
    ASSUME_ITS_TRUE(result);
    fossil_crabdb_entry_t *entry = fossil_crabdb_search(book, const_cast<char *>("key1"));
    ASSUME_NOT_CNULL(entry);
    ASSUME_ITS_TRUE(strcmp(entry->value, "new_value1") == 0);
    fossil_crabdb_release(book);
}

// Test case for executing a delete query
FOSSIL_TEST_CASE(cpp_test_crabdb_execute_delete_query) {
    fossil_crabdb_book_t *book = fossil_crabdb_init();
    fossil_crabdb_insert(book, const_cast<char *>("key1"), const_cast<char *>("value1"), (fossil_crabdb_attributes_t){false, false, false});
    bool result = fossil_crabdb_execute_query(book, const_cast<char *>("delete('key1');"));
    ASSUME_ITS_TRUE(result);
    ASSUME_ITS_TRUE(fossil_crabdb_is_empty(book));
    fossil_crabdb_release(book);
}

// Test case for executing a select query
FOSSIL_TEST_CASE(cpp_test_crabdb_execute_select_query) {
    fossil_crabdb_book_t *book = fossil_crabdb_init();
    fossil_crabdb_insert(book, const_cast<char *>("key1"), const_cast<char *>("value1"), (fossil_crabdb_attributes_t){false, false, false});
    bool result = fossil_crabdb_execute_query(book, const_cast<char *>("select('key1');"));
    ASSUME_ITS_TRUE(result);
    fossil_crabdb_release(book);
}

// Test case for executing a sort query in ascending order
FOSSIL_TEST_CASE(cpp_test_crabdb_execute_sort_ascending_query) {
    fossil_crabdb_book_t *book = fossil_crabdb_init();
    fossil_crabdb_insert(book, const_cast<char *>("key3"), const_cast<char *>("value3"), (fossil_crabdb_attributes_t){false, false, false});
    fossil_crabdb_insert(book, const_cast<char *>("key1"), const_cast<char *>("value1"), (fossil_crabdb_attributes_t){false, false, false});
    fossil_crabdb_insert(book, const_cast<char *>("key2"), const_cast<char *>("value2"), (fossil_crabdb_attributes_t){false, false, false});
    bool result = fossil_crabdb_execute_query(book, const_cast<char *>("sort(order: 'ascending');"));
    ASSUME_ITS_TRUE(result);
    fossil_crabdb_page_t *current = book->head;
    ASSUME_ITS_TRUE(strcmp(current->entry.key, "key1") == 0);
    fossil_crabdb_release(book);
}

// Test case for executing a sort query in descending order
FOSSIL_TEST_CASE(cpp_test_crabdb_execute_sort_descending_query) {
    fossil_crabdb_book_t *book = fossil_crabdb_init();
    fossil_crabdb_insert(book, const_cast<char *>("key1"), const_cast<char *>("value1"), (fossil_crabdb_attributes_t){false, false, false});
    fossil_crabdb_insert(book, const_cast<char *>("key3"), const_cast<char *>("value3"), (fossil_crabdb_attributes_t){false, false, false});
    fossil_crabdb_insert(book, const_cast<char *>("key2"), const_cast<char *>("value2"), (fossil_crabdb_attributes_t){false, false, false});
    bool result = fossil_crabdb_execute_query(book, const_cast<char *>("sort(order: 'descending');"));
    ASSUME_ITS_TRUE(result);
    fossil_crabdb_page_t *current = book->head;
    ASSUME_ITS_TRUE(strcmp(current->entry.key, "key3") == 0);
    fossil_crabdb_release(book);
}

// Test case for executing a begin transaction query
FOSSIL_TEST_CASE(cpp_test_crabdb_execute_begin_transaction_query) {
    fossil_crabdb_book_t *book = fossil_crabdb_init();
    bool result = fossil_crabdb_execute_query(book, const_cast<char *>("begin_transaction('test_transaction');"));
    ASSUME_ITS_TRUE(result);
    fossil_crabdb_release(book);
}

// Test case for executing a commit transaction query
FOSSIL_TEST_CASE(cpp_test_crabdb_execute_commit_transaction_query) {
    fossil_crabdb_book_t *book = fossil_crabdb_init();
    fossil_crabdb_transaction_t *transaction = fossil_crabdb_transaction_begin(book, const_cast<char *>("test_transaction"));
    bool result = fossil_crabdb_execute_query(book, const_cast<char *>("commit_transaction('test_transaction');"));
    ASSUME_ITS_TRUE(result);
    fossil_crabdb_release(book);
}

// Test case for executing a rollback transaction query
FOSSIL_TEST_CASE(cpp_test_crabdb_execute_rollback_transaction_query) {
    fossil_crabdb_book_t *book = fossil_crabdb_init();
    fossil_crabdb_transaction_t *transaction = fossil_crabdb_transaction_begin(book, const_cast<char *>("test_transaction"));
    bool result = fossil_crabdb_execute_query(book, const_cast<char *>("rollback_transaction('test_transaction');"));
    ASSUME_ITS_TRUE(result);
    fossil_crabdb_release(book);
}

// Test case for saving the database content to an INI file using CrabStore
FOSSIL_TEST_CASE(cpp_test_crabstore_save_to_ini_class) {
    fossil::CrabDB db;
    db.insert("key1", "value1", {false, false, false});
    bool result = fossil::CrabStore::saveToIni(db, "test_save_class.ini");
    ASSUME_ITS_TRUE(result);
}

// Test case for loading the database content from an INI file using CrabStore
FOSSIL_TEST_CASE(cpp_test_crabstore_load_from_ini_class) {
    fossil::CrabDB db;
    bool result = fossil::CrabStore::loadFromIni(db, "test_save_class.ini");
    ASSUME_ITS_TRUE(result);
    fossil_crabdb_entry_t *entry = db.search("key1");
    ASSUME_NOT_CNULL(entry);
    ASSUME_ITS_TRUE(strcmp(entry->value, "value1") == 0);
}

// Test case for saving the database content to a CSV file using CrabStore
FOSSIL_TEST_CASE(cpp_test_crabstore_save_to_csv_class) {
    fossil::CrabDB db;
    db.insert("key1", "value1", {false, false, false});
    bool result = fossil::CrabStore::saveToCsv(db, "test_save_class.csv");
    ASSUME_ITS_TRUE(result);
}

// Test case for loading the database content from a CSV file using CrabStore
FOSSIL_TEST_CASE(cpp_test_crabstore_load_from_csv_class) {
    fossil::CrabDB db;
    bool result = fossil::CrabStore::loadFromCsv(db, "test_save_class.csv");
    ASSUME_ITS_TRUE(result);
    fossil_crabdb_entry_t *entry = db.search("key1");
    ASSUME_NOT_CNULL(entry);
    ASSUME_ITS_TRUE(strcmp(entry->value, "value1") == 0);
}

// Test case for validating the structure of an INI file using CrabStore
FOSSIL_TEST_CASE(cpp_test_crabstore_validate_ini_class) {
    bool result = fossil::CrabStore::validateIni("test_save_class.ini");
    ASSUME_ITS_TRUE(result);
}

// Test case for validating the structure of a CSV file using CrabStore
FOSSIL_TEST_CASE(cpp_test_crabstore_validate_csv_class) {
    bool result = fossil::CrabStore::validateCsv("test_save_class.csv");
    ASSUME_ITS_TRUE(result);
}

// Test case for searching an entry by key using CrabSearch
FOSSIL_TEST_CASE(cpp_test_crabsearch_by_key_class) {
    fossil::CrabDB db;
    db.insert("key1", "value1", {false, false, false});
    fossil_crabdb_entry_t *entry = fossil::CrabSearch::byKey(db, "key1");
    ASSUME_NOT_CNULL(entry);
    ASSUME_ITS_TRUE(strcmp(entry->value, "value1") == 0);
}

// Test case for searching entries by value using CrabSearch
FOSSIL_TEST_CASE(cpp_test_crabsearch_by_value_class) {
    fossil::CrabDB db;
    db.insert("key1", "value1", {false, false, false});
    db.insert("key2", "value1", {false, false, false});
    fossil::CrabDB result = fossil::CrabSearch::byValue(db, "value1");
    ASSUME_ITS_TRUE(result.size() == 2);
}

// Test case for searching entries by predicate using CrabSearch
FOSSIL_TEST_CASE(cpp_test_crabsearch_by_predicate_class) {
    fossil::CrabDB db;
    db.insert("key1", "value1", {false, false, false});
    db.insert("key2", "value2", {false, false, false});
    fossil::CrabDB result = fossil::CrabSearch::byPredicate(db, predicate_key1);
    ASSUME_ITS_TRUE(result.size() == 1);
}

// Test case for searching the first entry by predicate using CrabSearch
FOSSIL_TEST_CASE(cpp_test_crabsearch_first_by_predicate_class) {
    fossil::CrabDB db;
    db.insert("key1", "value1", {false, false, false});
    db.insert("key2", "value2", {false, false, false});
    fossil_crabdb_entry_t *entry = fossil::CrabSearch::firstByPredicate(db, predicate_key1);
    ASSUME_NOT_CNULL(entry);
    ASSUME_ITS_TRUE(strcmp(entry->key, "key1") == 0);
}

// Test case for checking if a key exists using CrabSearch
FOSSIL_TEST_CASE(cpp_test_crabsearch_key_exists_class) {
    fossil::CrabDB db;
    db.insert("key1", "value1", {false, false, false});
    bool exists = fossil::CrabSearch::keyExists(db, "key1");
    ASSUME_ITS_TRUE(exists);
    exists = fossil::CrabSearch::keyExists(db, "key2");
    ASSUME_ITS_FALSE(exists);
}

// Test case for finding all primary key entries using CrabSearch
FOSSIL_TEST_CASE(cpp_test_crabsearch_primary_keys_class) {
    fossil::CrabDB db;
    db.insert("key1", "value1", {true, false, false});
    db.insert("key2", "value2", {false, false, false});
    fossil::CrabDB result = fossil::CrabSearch::primaryKeys(db);
    ASSUME_ITS_TRUE(result.size() == 1);
}

// Test case for counting entries by predicate using CrabSearch
FOSSIL_TEST_CASE(cpp_test_crabsearch_count_by_predicate_class) {
    fossil::CrabDB db;
    db.insert("key1", "value1", {false, false, false});
    db.insert("key2", "value1", {false, false, false});
    size_t count = fossil::CrabSearch::countByPredicate(db, predicate_key1);
    ASSUME_ITS_TRUE(count == 1);
}

// Test case for inserting a new key-value pair using CrabDB class
FOSSIL_TEST_CASE(cpp_test_crabdb_insert_class) {
    fossil::CrabDB db;
    bool result = db.insert("key1", "value1", {false, false, false});
    ASSUME_ITS_TRUE(result);
    ASSUME_ITS_FALSE(db.isEmpty());
}

// Test case for updating an existing key using CrabDB class
FOSSIL_TEST_CASE(cpp_test_crabdb_update_class) {
    fossil::CrabDB db;
    db.insert("key1", "value1", {false, false, false});
    bool result = db.update("key1", "new_value1");
    ASSUME_ITS_TRUE(result);
    fossil_crabdb_entry_t *entry = db.search("key1");
    ASSUME_NOT_CNULL(entry);
    ASSUME_ITS_TRUE(strcmp(entry->value, "new_value1") == 0);
}

// Test case for deleting an entry by key using CrabDB class
FOSSIL_TEST_CASE(cpp_test_crabdb_delete_class) {
    fossil::CrabDB db;
    db.insert("key1", "value1", {false, false, false});
    bool result = db.remove("key1");
    ASSUME_ITS_TRUE(result);
    ASSUME_ITS_TRUE(db.isEmpty());
}

// Test case for searching an entry by key using CrabDB class
FOSSIL_TEST_CASE(cpp_test_crabdb_search_class) {
    fossil::CrabDB db;
    db.insert("key1", "value1", {false, false, false});
    fossil_crabdb_entry_t *entry = db.search("key1");
    ASSUME_NOT_CNULL(entry);
    ASSUME_ITS_TRUE(strcmp(entry->value, "value1") == 0);
}

// Test case for clearing all entries from the database using CrabDB class
FOSSIL_TEST_CASE(cpp_test_crabdb_clear_class) {
    fossil::CrabDB db;
    db.insert("key1", "value1", {false, false, false});
    db.clear();
    ASSUME_ITS_TRUE(db.isEmpty());
}

// Test case for dumping database content to a file using CrabDB class
FOSSIL_TEST_CASE(cpp_test_crabdb_dump_to_file_class) {
    fossil::CrabDB db;
    db.insert("key1", "value1", {false, false, false});
    bool result = db.dumpToFile("test_dump_class.txt");
    ASSUME_ITS_TRUE(result);
}

// Test case for loading database content from a file using CrabDB class
FOSSIL_TEST_CASE(cpp_test_crabdb_load_from_file_class) {
    fossil::CrabDB db;
    bool result = db.loadFromFile("test_dump_class.txt");
    ASSUME_ITS_TRUE(result);
    fossil_crabdb_entry_t *entry = db.search("key1");
    ASSUME_NOT_CNULL(entry);
    ASSUME_ITS_TRUE(strcmp(entry->value, "value1") == 0);
}

// Test case for validating the integrity of the database using CrabDB class
FOSSIL_TEST_CASE(cpp_test_crabdb_validate_class) {
    fossil::CrabDB db;
    db.insert("key1", "value1", {false, false, false});
    bool result = db.validate();
    ASSUME_ITS_TRUE(result);
}

// Test case for executing an insert query using CrabDB class
FOSSIL_TEST_CASE(cpp_test_crabdb_execute_insert_query_class) {
    fossil::CrabDB db;
    bool result = db.executeQuery("insert('key1', 'value1', primary_key: false, unique: false);");
    ASSUME_ITS_TRUE(result);
    fossil_crabdb_entry_t *entry = db.search("key1");
    ASSUME_NOT_CNULL(entry);
    ASSUME_ITS_TRUE(strcmp(entry->value, "value1") == 0);
}

// Test case for executing an update query using CrabDB class
FOSSIL_TEST_CASE(cpp_test_crabdb_execute_update_query_class) {
    fossil::CrabDB db;
    db.insert("key1", "value1", {false, false, false});
    bool result = db.executeQuery("update('key1', new_value: 'new_value1');");
    ASSUME_ITS_TRUE(result);
    fossil_crabdb_entry_t *entry = db.search("key1");
    ASSUME_NOT_CNULL(entry);
    ASSUME_ITS_TRUE(strcmp(entry->value, "new_value1") == 0);
}

// Test case for executing a delete query using CrabDB class
FOSSIL_TEST_CASE(cpp_test_crabdb_execute_delete_query_class) {
    fossil::CrabDB db;
    db.insert("key1", "value1", {false, false, false});
    bool result = db.executeQuery("delete('key1');");
    ASSUME_ITS_TRUE(result);
    ASSUME_ITS_TRUE(db.isEmpty());
}

// Test case for executing a select query using CrabDB class
FOSSIL_TEST_CASE(cpp_test_crabdb_execute_select_query_class) {
    fossil::CrabDB db;
    db.insert("key1", "value1", {false, false, false});
    bool result = db.executeQuery("select('key1');");
    ASSUME_ITS_TRUE(result);
}

// Test case for executing a sort query in ascending order using CrabDB class
FOSSIL_TEST_CASE(cpp_test_crabdb_execute_sort_ascending_query_class) {
    fossil::CrabDB db;
    db.insert("key3", "value3", {false, false, false});
    db.insert("key1", "value1", {false, false, false});
    db.insert("key2", "value2", {false, false, false});
    bool result = db.executeQuery("sort(order: 'ascending');");
    ASSUME_ITS_TRUE(result);
    fossil_crabdb_page_t *current = db.search("key1");
    ASSUME_ITS_TRUE(strcmp(current->entry.key, "key1") == 0);
}

// Test case for executing a sort query in descending order using CrabDB class
FOSSIL_TEST_CASE(cpp_test_crabdb_execute_sort_descending_query_class) {
    fossil::CrabDB db;
    db.insert("key1", "value1", {false, false, false});
    db.insert("key3", "value3", {false, false, false});
    db.insert("key2", "value2", {false, false, false});
    bool result = db.executeQuery("sort(order: 'descending');");
    ASSUME_ITS_TRUE(result);
    fossil_crabdb_page_t *current = db.search("key3");
    ASSUME_ITS_TRUE(strcmp(current->entry.key, "key3") == 0);
}

// Test case for executing a begin transaction query using CrabDB class
FOSSIL_TEST_CASE(cpp_test_crabdb_execute_begin_transaction_query_class) {
    fossil::CrabDB db;
    bool result = db.executeQuery("begin_transaction('test_transaction');");
    ASSUME_ITS_TRUE(result);
}

// Test case for executing a commit transaction query using CrabDB class
FOSSIL_TEST_CASE(cpp_test_crabdb_execute_commit_transaction_query_class) {
    fossil::CrabDB db;
    db.executeQuery("begin_transaction('test_transaction');");
    bool result = db.executeQuery("commit_transaction('test_transaction');");
    ASSUME_ITS_TRUE(result);
}

// Test case for executing a rollback transaction query using CrabDB class
FOSSIL_TEST_CASE(cpp_test_crabdb_execute_rollback_transaction_query_class) {
    fossil::CrabDB db;
    db.executeQuery("begin_transaction('test_transaction');");
    bool result = db.executeQuery("rollback_transaction('test_transaction');");
    ASSUME_ITS_TRUE(result);
}

// * * * * * * * * * * * * * * * * * * * * * * * *
// * Fossil Logic Test Pool
// * * * * * * * * * * * * * * * * * * * * * * * *
FOSSIL_TEST_GROUP(cpp_crab_database_tests) {    
    FOSSIL_TEST_ADD(cpp_crabdb_fixture, cpp_test_crabdb_init);
    FOSSIL_TEST_ADD(cpp_crabdb_fixture, cpp_test_crabdb_insert);
    FOSSIL_TEST_ADD(cpp_crabdb_fixture, cpp_test_crabdb_update);
    FOSSIL_TEST_ADD(cpp_crabdb_fixture, cpp_test_crabdb_delete);
    FOSSIL_TEST_ADD(cpp_crabdb_fixture, cpp_test_crabdb_search);
    FOSSIL_TEST_ADD(cpp_crabdb_fixture, cpp_test_crabdb_clear);
    FOSSIL_TEST_ADD(cpp_crabdb_fixture, cpp_test_crabdb_join);
    FOSSIL_TEST_ADD(cpp_crabdb_fixture, cpp_test_crabdb_filter);
    FOSSIL_TEST_ADD(cpp_crabdb_fixture, cpp_test_crabdb_merge);
    FOSSIL_TEST_ADD(cpp_crabdb_fixture, cpp_test_crabdb_validate);
    FOSSIL_TEST_ADD(cpp_crabdb_fixture, cpp_test_crabdb_sort_ascending);
    FOSSIL_TEST_ADD(cpp_crabdb_fixture, cpp_test_crabdb_sort_descending);
    FOSSIL_TEST_ADD(cpp_crabdb_fixture, cpp_test_crabstore_save_to_ini);
    FOSSIL_TEST_ADD(cpp_crabdb_fixture, cpp_test_crabstore_load_from_ini);
    FOSSIL_TEST_ADD(cpp_crabdb_fixture, cpp_test_crabstore_save_to_csv);
    FOSSIL_TEST_ADD(cpp_crabdb_fixture, cpp_test_crabstore_load_from_csv);
    FOSSIL_TEST_ADD(cpp_crabdb_fixture, cpp_test_crabstore_validate_ini);
    FOSSIL_TEST_ADD(cpp_crabdb_fixture, cpp_test_crabstore_validate_csv);
    FOSSIL_TEST_ADD(cpp_crabdb_fixture, cpp_test_crabsearch_by_value);
    FOSSIL_TEST_ADD(cpp_crabdb_fixture, cpp_test_crabsearch_by_predicate);
    FOSSIL_TEST_ADD(cpp_crabdb_fixture, cpp_test_crabsearch_first_by_predicate);
    FOSSIL_TEST_ADD(cpp_crabdb_fixture, cpp_test_crabsearch_key_exists);
    FOSSIL_TEST_ADD(cpp_crabdb_fixture, cpp_test_crabsearch_primary_keys);
    FOSSIL_TEST_ADD(cpp_crabdb_fixture, cpp_test_crabsearch_count_by_predicate);
    FOSSIL_TEST_ADD(cpp_crabdb_fixture, cpp_test_crabsearch_is_non_nullable);
    FOSSIL_TEST_ADD(cpp_crabdb_fixture, cpp_test_crabsearch_is_unique);
    FOSSIL_TEST_ADD(cpp_crabdb_fixture, cpp_test_crabdb_dump_to_file);
    FOSSIL_TEST_ADD(cpp_crabdb_fixture, cpp_test_crabdb_load_from_file);
    FOSSIL_TEST_ADD(cpp_crabdb_fixture, cpp_test_crabdb_execute_insert_query);
    FOSSIL_TEST_ADD(cpp_crabdb_fixture, cpp_test_crabdb_execute_update_query);
    FOSSIL_TEST_ADD(cpp_crabdb_fixture, cpp_test_crabdb_execute_delete_query);
    FOSSIL_TEST_ADD(cpp_crabdb_fixture, cpp_test_crabdb_execute_select_query);
    FOSSIL_TEST_ADD(cpp_crabdb_fixture, cpp_test_crabdb_execute_sort_ascending_query);
    FOSSIL_TEST_ADD(cpp_crabdb_fixture, cpp_test_crabdb_execute_sort_descending_query);
    FOSSIL_TEST_ADD(cpp_crabdb_fixture, cpp_test_crabdb_execute_begin_transaction_query);
    FOSSIL_TEST_ADD(cpp_crabdb_fixture, cpp_test_crabdb_execute_commit_transaction_query);
    FOSSIL_TEST_ADD(cpp_crabdb_fixture, cpp_test_crabdb_execute_rollback_transaction_query);

    FOSSIL_TEST_ADD(cpp_crabdb_fixture, cpp_test_crabstore_save_to_ini_class);
    FOSSIL_TEST_ADD(cpp_crabdb_fixture, cpp_test_crabstore_load_from_ini_class);
    FOSSIL_TEST_ADD(cpp_crabdb_fixture, cpp_test_crabstore_save_to_csv_class);
    FOSSIL_TEST_ADD(cpp_crabdb_fixture, cpp_test_crabstore_load_from_csv_class);
    FOSSIL_TEST_ADD(cpp_crabdb_fixture, cpp_test_crabstore_validate_ini_class);
    FOSSIL_TEST_ADD(cpp_crabdb_fixture, cpp_test_crabstore_validate_csv_class);
    FOSSIL_TEST_ADD(cpp_crabdb_fixture, cpp_test_crabsearch_by_key_class);
    FOSSIL_TEST_ADD(cpp_crabdb_fixture, cpp_test_crabsearch_by_value_class);
    FOSSIL_TEST_ADD(cpp_crabdb_fixture, cpp_test_crabsearch_by_predicate_class);
    FOSSIL_TEST_ADD(cpp_crabdb_fixture, cpp_test_crabsearch_first_by_predicate_class);
    FOSSIL_TEST_ADD(cpp_crabdb_fixture, cpp_test_crabsearch_key_exists_class);
    FOSSIL_TEST_ADD(cpp_crabdb_fixture, cpp_test_crabsearch_primary_keys_class);
    FOSSIL_TEST_ADD(cpp_crabdb_fixture, cpp_test_crabsearch_count_by_predicate_class);
    FOSSIL_TEST_ADD(cpp_crabdb_fixture, cpp_test_crabdb_insert_class);
    FOSSIL_TEST_ADD(cpp_crabdb_fixture, cpp_test_crabdb_update_class);
    FOSSIL_TEST_ADD(cpp_crabdb_fixture, cpp_test_crabdb_delete_class);
    FOSSIL_TEST_ADD(cpp_crabdb_fixture, cpp_test_crabdb_search_class);
    FOSSIL_TEST_ADD(cpp_crabdb_fixture, cpp_test_crabdb_clear_class);
    FOSSIL_TEST_ADD(cpp_crabdb_fixture, cpp_test_crabdb_dump_to_file_class);
    FOSSIL_TEST_ADD(cpp_crabdb_fixture, cpp_test_crabdb_load_from_file_class);
    FOSSIL_TEST_ADD(cpp_crabdb_fixture, cpp_test_crabdb_validate_class);
    FOSSIL_TEST_ADD(cpp_crabdb_fixture, cpp_test_crabdb_execute_insert_query_class);
    FOSSIL_TEST_ADD(cpp_crabdb_fixture, cpp_test_crabdb_execute_update_query_class);
    FOSSIL_TEST_ADD(cpp_crabdb_fixture, cpp_test_crabdb_execute_delete_query_class);
    FOSSIL_TEST_ADD(cpp_crabdb_fixture, cpp_test_crabdb_execute_select_query_class);
    FOSSIL_TEST_ADD(cpp_crabdb_fixture, cpp_test_crabdb_execute_sort_ascending_query_class);
    FOSSIL_TEST_ADD(cpp_crabdb_fixture, cpp_test_crabdb_execute_sort_descending_query_class);
    FOSSIL_TEST_ADD(cpp_crabdb_fixture, cpp_test_crabdb_execute_begin_transaction_query_class);
    FOSSIL_TEST_ADD(cpp_crabdb_fixture, cpp_test_crabdb_execute_commit_transaction_query_class);
    FOSSIL_TEST_ADD(cpp_crabdb_fixture, cpp_test_crabdb_execute_rollback_transaction_query_class);

    FOSSIL_TEST_REGISTER(cpp_crabdb_fixture);
} // end of tests

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

FOSSIL_TEST_SUITE(c_crabdb_fixture);

FOSSIL_SETUP(c_crabdb_fixture) {
    // Setup the test fixture
}

FOSSIL_TEARDOWN(c_crabdb_fixture) {
    // Teardown the test fixture
}

// * * * * * * * * * * * * * * * * * * * * * * * *
// * Fossil Logic Test Blue CrabDB Database
// * * * * * * * * * * * * * * * * * * * * * * * *

// Test case for initializing a new database
FOSSIL_TEST_CASE(c_test_crabdb_init) {
    fossil_crabdb_book_t *book = fossil_crabdb_init();
    ASSUME_NOT_CNULL(book);
    ASSUME_ITS_TRUE(fossil_crabdb_is_empty(book));
    fossil_crabdb_release(book);
}

// Test case for inserting a new key-value pair
FOSSIL_TEST_CASE(c_test_crabdb_insert) {
    fossil_crabdb_book_t *book = fossil_crabdb_init();
    bool result = fossil_crabdb_insert(book, "key1", "value1", (fossil_crabdb_attributes_t){false, false, false});
    ASSUME_ITS_TRUE(result);
    ASSUME_ITS_FALSE(fossil_crabdb_is_empty(book));
    fossil_crabdb_release(book);
}

// Test case for updating an existing key
FOSSIL_TEST_CASE(c_test_crabdb_update) {
    fossil_crabdb_book_t *book = fossil_crabdb_init();
    fossil_crabdb_insert(book, "key1", "value1", (fossil_crabdb_attributes_t){false, false, false});
    bool result = fossil_crabdb_update(book, "key1", "new_value1");
    ASSUME_ITS_TRUE(result);
    fossil_crabdb_entry_t *entry = fossil_crabdb_search(book, "key1");
    ASSUME_NOT_CNULL(entry);
    ASSUME_ITS_TRUE(strcmp(entry->value, "new_value1") == 0);
    fossil_crabdb_release(book);
}

// Test case for deleting an entry by key
FOSSIL_TEST_CASE(c_test_crabdb_delete) {
    fossil_crabdb_book_t *book = fossil_crabdb_init();
    fossil_crabdb_insert(book, "key1", "value1", (fossil_crabdb_attributes_t){false, false, false});
    bool result = fossil_crabdb_delete(book, "key1");
    ASSUME_ITS_TRUE(result);
    ASSUME_ITS_TRUE(fossil_crabdb_is_empty(book));
    fossil_crabdb_release(book);
}

// Test case for searching an entry by key
FOSSIL_TEST_CASE(c_test_crabdb_search) {
    fossil_crabdb_book_t *book = fossil_crabdb_init();
    fossil_crabdb_insert(book, "key1", "value1", (fossil_crabdb_attributes_t){false, false, false});
    fossil_crabdb_entry_t *entry = fossil_crabdb_search(book, "key1");
    ASSUME_NOT_CNULL(entry);
    ASSUME_ITS_TRUE(strcmp(entry->value, "value1") == 0);
    fossil_crabdb_release(book);
}

// Test case for clearing all entries from the database
FOSSIL_TEST_CASE(c_test_crabdb_clear) {
    fossil_crabdb_book_t *book = fossil_crabdb_init();
    fossil_crabdb_insert(book, "key1", "value1", (fossil_crabdb_attributes_t){false, false, false});
    fossil_crabdb_clear(book);
    ASSUME_ITS_TRUE(fossil_crabdb_is_empty(book));
    fossil_crabdb_release(book);
}

// Test case for joining two databases
FOSSIL_TEST_CASE(c_test_crabdb_join) {
    fossil_crabdb_book_t *book1 = fossil_crabdb_init();
    fossil_crabdb_book_t *book2 = fossil_crabdb_init();
    fossil_crabdb_insert(book1, "key1", "value1", (fossil_crabdb_attributes_t){false, false, false});
    fossil_crabdb_insert(book2, "key1", "value2", (fossil_crabdb_attributes_t){false, false, false});
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

FOSSIL_TEST_CASE(c_test_crabdb_filter) {
    fossil_crabdb_book_t *book = fossil_crabdb_init();
    fossil_crabdb_insert(book, "key1", "value1", (fossil_crabdb_attributes_t){false, false, false});
    fossil_crabdb_insert(book, "key2", "value2", (fossil_crabdb_attributes_t){false, false, false});
    fossil_crabdb_book_t *result = fossil_crabdb_filter(book, filter_key1);
    ASSUME_NOT_CNULL(result);
    ASSUME_ITS_TRUE(fossil_crabdb_size(result) == 1);
    fossil_crabdb_release(book);
    fossil_crabdb_release(result);
}

// Test case for merging two databases
FOSSIL_TEST_CASE(c_test_crabdb_merge) {
    fossil_crabdb_book_t *book1 = fossil_crabdb_init();
    fossil_crabdb_book_t *book2 = fossil_crabdb_init();
    fossil_crabdb_insert(book1, "key1", "value1", (fossil_crabdb_attributes_t){false, false, false});
    fossil_crabdb_insert(book2, "key2", "value2", (fossil_crabdb_attributes_t){false, false, false});
    fossil_crabdb_book_t *result = fossil_crabdb_merge(book1, book2);
    ASSUME_NOT_CNULL(result);
    ASSUME_ITS_TRUE(fossil_crabdb_size(result) == 2);
    fossil_crabdb_release(book1);
    fossil_crabdb_release(book2);
    fossil_crabdb_release(result);
}

// Test case for validating the integrity of the database
FOSSIL_TEST_CASE(c_test_crabdb_validate) {
    fossil_crabdb_book_t *book = fossil_crabdb_init();
    fossil_crabdb_insert(book, "key1", "value1", (fossil_crabdb_attributes_t){false, false, false});
    bool result = fossil_crabdb_validate(book);
    ASSUME_ITS_TRUE(result);
    fossil_crabdb_release(book);
}

// Test case for sorting the database in ascending order
FOSSIL_TEST_CASE(c_test_crabdb_sort_ascending) {
    fossil_crabdb_book_t *book = fossil_crabdb_init();
    fossil_crabdb_insert(book, "key3", "value3", (fossil_crabdb_attributes_t){false, false, false});
    fossil_crabdb_insert(book, "key1", "value1", (fossil_crabdb_attributes_t){false, false, false});
    fossil_crabdb_insert(book, "key2", "value2", (fossil_crabdb_attributes_t){false, false, false});
    int result = fossil_crabdb_sort(book, FOSSIL_CRABDB_SORT_ASCENDING);
    ASSUME_ITS_TRUE(result == 0);
    fossil_crabdb_page_t *current = book->head;
    ASSUME_ITS_TRUE(strcmp(current->entry.key, "key1") == 0);
    fossil_crabdb_release(book);
}

// Test case for sorting the database in descending order
FOSSIL_TEST_CASE(c_test_crabdb_sort_descending) {
    fossil_crabdb_book_t *book = fossil_crabdb_init();
    fossil_crabdb_insert(book, "key1", "value1", (fossil_crabdb_attributes_t){false, false, false});
    fossil_crabdb_insert(book, "key3", "value3", (fossil_crabdb_attributes_t){false, false, false});
    fossil_crabdb_insert(book, "key2", "value2", (fossil_crabdb_attributes_t){false, false, false});
    int result = fossil_crabdb_sort(book, FOSSIL_CRABDB_SORT_DESCENDING);
    ASSUME_ITS_TRUE(result == 0);
    fossil_crabdb_page_t *current = book->head;
    ASSUME_ITS_TRUE(strcmp(current->entry.key, "key3") == 0);
    fossil_crabdb_release(book);
}

// Test case for dumping database content to a file
FOSSIL_TEST_CASE(c_test_crabdb_dump_to_file) {
    fossil_crabdb_book_t *book = fossil_crabdb_init();
    fossil_crabdb_insert(book, "key1", "value1", (fossil_crabdb_attributes_t){false, false, false});
    bool result = fossil_crabdb_dump_to_file(book, "test_dump.txt");
    ASSUME_ITS_TRUE(result);
    fossil_crabdb_release(book);
}

// Test case for loading database content from a file
FOSSIL_TEST_CASE(c_test_crabdb_load_from_file) {
    fossil_crabdb_book_t *book = fossil_crabdb_init();
    bool result = fossil_crabdb_load_from_file(book, "test_dump.txt");
    ASSUME_ITS_TRUE(result);
    fossil_crabdb_entry_t *entry = fossil_crabdb_search(book, "key1");
    ASSUME_NOT_CNULL(entry);
    ASSUME_ITS_TRUE(strcmp(entry->value, "value1") == 0);
    fossil_crabdb_release(book);
}

// Test case for executing an insert query
FOSSIL_TEST_CASE(c_test_crabdb_execute_insert_query) {
    fossil_crabdb_book_t *book = fossil_crabdb_init();
    bool result = fossil_crabdb_execute_query(book, "insert('key1', 'value1', primary_key: false, unique: false);");
    ASSUME_ITS_TRUE(result);
    fossil_crabdb_entry_t *entry = fossil_crabdb_search(book, "key1");
    ASSUME_NOT_CNULL(entry);
    ASSUME_ITS_TRUE(strcmp(entry->value, "value1") == 0);
    fossil_crabdb_release(book);
}

// Test case for executing an update query
FOSSIL_TEST_CASE(c_test_crabdb_execute_update_query) {
    fossil_crabdb_book_t *book = fossil_crabdb_init();
    fossil_crabdb_insert(book, "key1", "value1", (fossil_crabdb_attributes_t){false, false, false});
    bool result = fossil_crabdb_execute_query(book, "update('key1', new_value: 'new_value1');");
    ASSUME_ITS_TRUE(result);
    fossil_crabdb_entry_t *entry = fossil_crabdb_search(book, "key1");
    ASSUME_NOT_CNULL(entry);
    ASSUME_ITS_TRUE(strcmp(entry->value, "new_value1") == 0);
    fossil_crabdb_release(book);
}

// Test case for executing a delete query
FOSSIL_TEST_CASE(c_test_crabdb_execute_delete_query) {
    fossil_crabdb_book_t *book = fossil_crabdb_init();
    fossil_crabdb_insert(book, "key1", "value1", (fossil_crabdb_attributes_t){false, false, false});
    bool result = fossil_crabdb_execute_query(book, "delete('key1');");
    ASSUME_ITS_TRUE(result);
    ASSUME_ITS_TRUE(fossil_crabdb_is_empty(book));
    fossil_crabdb_release(book);
}

// Test case for executing a select query
FOSSIL_TEST_CASE(c_test_crabdb_execute_select_query) {
    fossil_crabdb_book_t *book = fossil_crabdb_init();
    fossil_crabdb_insert(book, "key1", "value1", (fossil_crabdb_attributes_t){false, false, false});
    bool result = fossil_crabdb_execute_query(book, "select('key1');");
    ASSUME_ITS_TRUE(result);
    fossil_crabdb_release(book);
}

// Test case for executing a sort query in ascending order
FOSSIL_TEST_CASE(c_test_crabdb_execute_sort_ascending_query) {
    fossil_crabdb_book_t *book = fossil_crabdb_init();
    fossil_crabdb_insert(book, "key3", "value3", (fossil_crabdb_attributes_t){false, false, false});
    fossil_crabdb_insert(book, "key1", "value1", (fossil_crabdb_attributes_t){false, false, false});
    fossil_crabdb_insert(book, "key2", "value2", (fossil_crabdb_attributes_t){false, false, false});
    bool result = fossil_crabdb_execute_query(book, "sort(order: 'ascending');");
    ASSUME_ITS_TRUE(result);
    fossil_crabdb_page_t *current = book->head;
    ASSUME_ITS_TRUE(strcmp(current->entry.key, "key1") == 0);
    fossil_crabdb_release(book);
}

// Test case for executing a sort query in descending order
FOSSIL_TEST_CASE(c_test_crabdb_execute_sort_descending_query) {
    fossil_crabdb_book_t *book = fossil_crabdb_init();
    fossil_crabdb_insert(book, "key1", "value1", (fossil_crabdb_attributes_t){false, false, false});
    fossil_crabdb_insert(book, "key3", "value3", (fossil_crabdb_attributes_t){false, false, false});
    fossil_crabdb_insert(book, "key2", "value2", (fossil_crabdb_attributes_t){false, false, false});
    bool result = fossil_crabdb_execute_query(book, "sort(order: 'descending');");
    ASSUME_ITS_TRUE(result);
    fossil_crabdb_page_t *current = book->head;
    ASSUME_ITS_TRUE(strcmp(current->entry.key, "key3") == 0);
    fossil_crabdb_release(book);
}

// Test case for executing a begin transaction query
FOSSIL_TEST_CASE(c_test_crabdb_execute_begin_transaction_query) {
    fossil_crabdb_book_t *book = fossil_crabdb_init();
    bool result = fossil_crabdb_execute_query(book, "begin_transaction('test_transaction');");
    ASSUME_ITS_TRUE(result);
    fossil_crabdb_release(book);
}

// * * * * * * * * * * * * * * * * * * * * * * * *
// * Fossil Logic Test Pool
// * * * * * * * * * * * * * * * * * * * * * * * *
FOSSIL_TEST_GROUP(c_crab_database_tests) {    
    FOSSIL_TEST_ADD(c_crabdb_fixture, c_test_crabdb_init);
    FOSSIL_TEST_ADD(c_crabdb_fixture, c_test_crabdb_insert);
    FOSSIL_TEST_ADD(c_crabdb_fixture, c_test_crabdb_update);
    FOSSIL_TEST_ADD(c_crabdb_fixture, c_test_crabdb_delete);
    FOSSIL_TEST_ADD(c_crabdb_fixture, c_test_crabdb_search);
    FOSSIL_TEST_ADD(c_crabdb_fixture, c_test_crabdb_clear);
    FOSSIL_TEST_ADD(c_crabdb_fixture, c_test_crabdb_join);
    FOSSIL_TEST_ADD(c_crabdb_fixture, c_test_crabdb_filter);
    FOSSIL_TEST_ADD(c_crabdb_fixture, c_test_crabdb_merge);
    FOSSIL_TEST_ADD(c_crabdb_fixture, c_test_crabdb_validate);
    FOSSIL_TEST_ADD(c_crabdb_fixture, c_test_crabdb_sort_ascending);
    FOSSIL_TEST_ADD(c_crabdb_fixture, c_test_crabdb_sort_descending);
    FOSSIL_TEST_ADD(c_crabdb_fixture, c_test_crabdb_dump_to_file);
    FOSSIL_TEST_ADD(c_crabdb_fixture, c_test_crabdb_load_from_file);
    FOSSIL_TEST_ADD(c_crabdb_fixture, c_test_crabdb_execute_insert_query);
    FOSSIL_TEST_ADD(c_crabdb_fixture, c_test_crabdb_execute_update_query);
    FOSSIL_TEST_ADD(c_crabdb_fixture, c_test_crabdb_execute_delete_query);
    FOSSIL_TEST_ADD(c_crabdb_fixture, c_test_crabdb_execute_select_query);
    FOSSIL_TEST_ADD(c_crabdb_fixture, c_test_crabdb_execute_sort_ascending_query);
    FOSSIL_TEST_ADD(c_crabdb_fixture, c_test_crabdb_execute_sort_descending_query);
    FOSSIL_TEST_ADD(c_crabdb_fixture, c_test_crabdb_execute_begin_transaction_query);

    FOSSIL_TEST_REGISTER(c_crabdb_fixture);
} // end of tests

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
    bool result = fossil_crabdb_insert(book, const_cast<char *>("key1"), const_cast<char *>("value1"), {false, false, false});
    ASSUME_ITS_TRUE(result);
    ASSUME_ITS_FALSE(fossil_crabdb_is_empty(book));
    fossil_crabdb_release(book);
}

// Test case for updating an existing key
FOSSIL_TEST_CASE(cpp_test_crabdb_update) {
    fossil_crabdb_book_t *book = fossil_crabdb_init();
    fossil_crabdb_insert(book, const_cast<char *>("key1"), const_cast<char *>("value1"), {false, false, false});
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
    fossil_crabdb_insert(book, const_cast<char *>("key1"), const_cast<char *>("value1"), {false, false, false});
    bool result = fossil_crabdb_delete(book, const_cast<char *>("key1"));
    ASSUME_ITS_TRUE(result);
    ASSUME_ITS_TRUE(fossil_crabdb_is_empty(book));
    fossil_crabdb_release(book);
}

// Test case for searching an entry by key
FOSSIL_TEST_CASE(cpp_test_crabdb_search) {
    fossil_crabdb_book_t *book = fossil_crabdb_init();
    fossil_crabdb_insert(book, const_cast<char *>("key1"), const_cast<char *>("value1"), {false, false, false});
    fossil_crabdb_entry_t *entry = fossil_crabdb_search(book, const_cast<char *>("key1"));
    ASSUME_NOT_CNULL(entry);
    ASSUME_ITS_TRUE(strcmp(entry->value, "value1") == 0);
    fossil_crabdb_release(book);
}

// Test case for clearing all entries from the database
FOSSIL_TEST_CASE(cpp_test_crabdb_clear) {
    fossil_crabdb_book_t *book = fossil_crabdb_init();
    fossil_crabdb_insert(book, const_cast<char *>("key1"), const_cast<char *>("value1"), {false, false, false});
    fossil_crabdb_clear(book);
    ASSUME_ITS_TRUE(fossil_crabdb_is_empty(book));
    fossil_crabdb_release(book);
}

// Test case for joining two databases
FOSSIL_TEST_CASE(cpp_test_crabdb_join) {
    fossil_crabdb_book_t *book1 = fossil_crabdb_init();
    fossil_crabdb_book_t *book2 = fossil_crabdb_init();
    fossil_crabdb_insert(book1, const_cast<char *>("key1"), const_cast<char *>("value1"), {false, false, false});
    fossil_crabdb_insert(book2, const_cast<char *>("key1"), const_cast<char *>("value2"), {false, false, false});
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
    fossil_crabdb_insert(book, const_cast<char *>("key1"), const_cast<char *>("value1"), {false, false, false});
    fossil_crabdb_insert(book, const_cast<char *>("key2"), const_cast<char *>("value2"), {false, false, false});
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
    fossil_crabdb_insert(book1, const_cast<char *>("key1"), const_cast<char *>("value1"), {false, false, false});
    fossil_crabdb_insert(book2, const_cast<char *>("key2"), const_cast<char *>("value2"), {false, false, false});
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
    fossil_crabdb_insert(book, const_cast<char *>("key1"), const_cast<char *>("value1"), {false, false, false});
    bool result = fossil_crabdb_validate(book);
    ASSUME_ITS_TRUE(result);
    fossil_crabdb_release(book);
}

// Test case for sorting the database in ascending order
FOSSIL_TEST_CASE(cpp_test_crabdb_sort_ascending) {
    fossil_crabdb_book_t *book = fossil_crabdb_init();
    fossil_crabdb_insert(book, const_cast<char *>("key3"), const_cast<char *>("value3"), {false, false, false});
    fossil_crabdb_insert(book, const_cast<char *>("key1"), const_cast<char *>("value1"), {false, false, false});
    fossil_crabdb_insert(book, const_cast<char *>("key2"), const_cast<char *>("value2"), {false, false, false});
    int result = fossil_crabdb_sort(book, FOSSIL_CRABDB_SORT_ASCENDING);
    ASSUME_ITS_TRUE(result == 0);
    fossil_crabdb_page_t *current = book->head;
    ASSUME_ITS_TRUE(strcmp(current->entry.key, "key1") == 0);
    fossil_crabdb_release(book);
}

// Test case for sorting the database in descending order
FOSSIL_TEST_CASE(cpp_test_crabdb_sort_descending) {
    fossil_crabdb_book_t *book = fossil_crabdb_init();
    fossil_crabdb_insert(book, const_cast<char *>("key1"), const_cast<char *>("value1"), {false, false, false});
    fossil_crabdb_insert(book, const_cast<char *>("key3"), const_cast<char *>("value3"), {false, false, false});
    fossil_crabdb_insert(book, const_cast<char *>("key2"), const_cast<char *>("value2"), {false, false, false});
    int result = fossil_crabdb_sort(book, FOSSIL_CRABDB_SORT_DESCENDING);
    ASSUME_ITS_TRUE(result == 0);
    fossil_crabdb_page_t *current = book->head;
    ASSUME_ITS_TRUE(strcmp(current->entry.key, "key3") == 0);
    fossil_crabdb_release(book);
}

// Test case for inserting a new key-value pair using CrabDB class
FOSSIL_TEST_CASE(cpp_test_crabdb_class_insert) {
    fossil::CrabDB db;
    bool result = db.insert("key1", "value1", {false, false, false});
    ASSUME_ITS_TRUE(result);
    ASSUME_ITS_FALSE(db.isEmpty());
}

// Test case for updating an existing key using CrabDB class
FOSSIL_TEST_CASE(cpp_test_crabdb_class_update) {
    fossil::CrabDB db;
    db.insert("key1", "value1", {false, false, false});
    bool result = db.update("key1", "new_value1");
    ASSUME_ITS_TRUE(result);
    fossil_crabdb_entry_t *entry = db.search("key1");
    ASSUME_NOT_CNULL(entry);
    ASSUME_ITS_TRUE(strcmp(entry->value, "new_value1") == 0);
}

// Test case for deleting an entry by key using CrabDB class
FOSSIL_TEST_CASE(cpp_test_crabdb_class_delete) {
    fossil::CrabDB db;
    db.insert("key1", "value1", {false, false, false});
    bool result = db.remove("key1");
    ASSUME_ITS_TRUE(result);
    ASSUME_ITS_TRUE(db.isEmpty());
}

// Test case for searching an entry by key using CrabDB class
FOSSIL_TEST_CASE(cpp_test_crabdb_class_search) {
    fossil::CrabDB db;
    db.insert("key1", "value1", {false, false, false});
    fossil_crabdb_entry_t *entry = db.search("key1");
    ASSUME_NOT_CNULL(entry);
    ASSUME_ITS_TRUE(strcmp(entry->value, "value1") == 0);
}

// Test case for clearing all entries from the database using CrabDB class
FOSSIL_TEST_CASE(cpp_test_crabdb_class_clear) {
    fossil::CrabDB db;
    db.insert("key1", "value1", {false, false, false});
    db.clear();
    ASSUME_ITS_TRUE(db.isEmpty());
}

// Test case for validating the integrity of the database using CrabDB class
FOSSIL_TEST_CASE(cpp_test_crabdb_class_validate) {
    fossil::CrabDB db;
    db.insert("key1", "value1", {false, false, false});
    bool result = db.validate();
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
    FOSSIL_TEST_ADD(cpp_crabdb_fixture, cpp_test_crabdb_class_insert);
    FOSSIL_TEST_ADD(cpp_crabdb_fixture, cpp_test_crabdb_class_update);
    FOSSIL_TEST_ADD(cpp_crabdb_fixture, cpp_test_crabdb_class_delete);
    FOSSIL_TEST_ADD(cpp_crabdb_fixture, cpp_test_crabdb_class_search);
    FOSSIL_TEST_ADD(cpp_crabdb_fixture, cpp_test_crabdb_class_clear);
    FOSSIL_TEST_ADD(cpp_crabdb_fixture, cpp_test_crabdb_class_validate);

    FOSSIL_TEST_REGISTER(cpp_crabdb_fixture);
} // end of tests

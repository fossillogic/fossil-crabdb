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

/**
 * The following types help illustrate a database structure that takes the form
 * of a book, which is a common and easily understandable metaphor. Each entry
 * in the database is like a page in the book, and the entire database is like
 * a collection of these pages.
 * 
 * - fossil_crabdb_entry_t: Represents an entry in the database, holding a key-value pair.
 * - fossil_crabdb_page_t: Represents a node in the doubly linked list, containing an entry and pointers to the next and previous nodes.
 * - fossil_crabdb_book_t: Represents the entire database, maintaining pointers to the head and tail of the list and the number of elements.
 */

/**
 * @brief Represents an entry in the fossil_crabdb database.
 * 
 * This structure holds a key-value pair where the key is a null-terminated string
 * and the value is a pointer to the data with its size specified.
 */
typedef struct {
    char *key;                // Null-terminated string for the key.
    void *value;              // Pointer to the value.
    size_t value_size;        // Size of the value in bytes.
} fossil_crabdb_entry_t;

/**
 * @brief Represents a node in the doubly linked list used by fossil_crabdb.
 * 
 * Each node contains an entry and pointers to the next and previous nodes
 * in the list, allowing traversal in both directions.
 */
typedef struct fossil_crabdb_node {
    fossil_crabdb_entry_t entry;     // Data stored in the node.
    struct fossil_crabdb_node *next; // Pointer to the next node.
    struct fossil_crabdb_node *prev; // Pointer to the previous node.
} fossil_crabdb_page_t;

/**
 * @brief Represents a deque (double-ended queue) in the fossil_crabdb database.
 * 
 * This structure maintains pointers to the head and tail of the doubly linked list
 * and keeps track of the number of elements in the deque.
 */
typedef struct {
    fossil_crabdb_page_t *head;      // Pointer to the first node.
    fossil_crabdb_page_t *tail;      // Pointer to the last node.
    size_t size;                     // Number of elements in the deque.
} fossil_crabdb_book_t;

// *****************************************************************************
// Create and destroy
// *****************************************************************************

/**
 * Initialize the database.
 *
 * @param filename The name of the file to open.
 * 
 * @note The file must have a .crabdb extension.
 * 
 * @warning This function must be called before any other database functions.
 */
void fossil_crabdb_open(const char *filename);

/**
 * Cleanup resources and close the database.
 * 
 * This function will free all memory allocated for the database.
 * 
 * @note This function must be called before the program exits.
 * 
 * @warning Do not call this function if the database is still in use.
 */
void fossil_crabdb_close(void);

// *****************************************************************************
// classic CRUD operations
// *****************************************************************************

/**
 * Create a key-value pair.
 *
 * @param key The key to store the value under.
 * @param value The value to store.
 * @param value_size The size of the value in bytes.
 */
void fossil_crabdb_create(const char *key, const void *value, size_t value_size);

/**
 * Read a value by key.
 *
 * @param key The key to search for.
 * @param value_size A pointer to store the size of the value.
 * 
 * @return A pointer to the value.
 */
void *fossil_crabdb_read(const char *key, size_t *value_size);

/**
 * Update a key-value pair.
 *
 * @param key The key to update.
 * @param new_value The new value to store.
 * @param value_size The size of the new value in bytes.
 */
void fossil_crabdb_update(const char *key, const void *new_value, size_t value_size);

/**
 * Delete a key-value pair.
 *
 * @param key The key to delete.
 */
void fossil_crabdb_delete(const char *key);

// *****************************************************************************
// batch CRUD operations
// *****************************************************************************

/**
 * Batch create key-value pairs.
 *
 * @param entries An array of entries to create.
 * @param count The number of entries in the array.
 */
void fossil_crabdb_batch_create(const fossil_crabdb_entry_t *entries, size_t count);

/**
 * Batch read values by key.
 *
 * @param keys An array of keys to search for.
 * @param count The number of keys in the array.
 * @param values A pointer to store the array of values.
 * @param value_sizes A pointer to store the array of value sizes.
 */
void fossil_crabdb_batch_read(const char **keys, size_t count, void ***values, size_t **value_sizes);

/**
 * Batch update key-value pairs.
 *
 * @param entries An array of entries to update.
 * @param count The number of entries in the array.
 */
void fossil_crabdb_batch_update(const fossil_crabdb_entry_t *entries, size_t count);

/**
 * Batch delete key-value pairs.
 *
 * @param keys An array of keys to delete.
 * @param count The number of keys in the array.
 */
void fossil_crabdb_batch_delete(const char **keys, size_t count);

// *****************************************************************************
// Algorithic operations
// *****************************************************************************

/**
 * Search for entries by a partial key match.
 *
 * @param partial_key The partial key to search for.
 * @param count A pointer to store the number of matching entries.
 * 
 * @return An array of matching entries.
 */
fossil_crabdb_entry_t *fossil_crabdb_search(const char *partial_key, size_t *count);

/**
 * Sort the database entries by key.
 * 
 * This function will sort the entries in ascending order based on their keys.
 */
void fossil_crabdb_sort(void);

/**
 * Filter entries by a custom predicate function.
 *
 * @param predicate The predicate function to apply to each entry.
 * @param count A pointer to store the number of matching entries.
 * 
 * @return An array of entries that match the predicate.
 */
fossil_crabdb_entry_t *fossil_crabdb_filter(bool (*predicate)(const fossil_crabdb_entry_t *), size_t *count);

// *****************************************************************************
// Utility declarations
// *****************************************************************************

/**
 * Count the number of entries in the database.
 * 
 * @return The number of entries in the database.
 */
size_t fossil_crabdb_count(void);

/**
 * Get a value by key.
 *
 * @param key The key to search for.
 * @param value_size A pointer to store the size of the value.
 * 
 * @return A pointer to the value.
 */
void *fossil_crabdb_get(const char *key, size_t *value_size);

/**
 * Check if a key exists in the database.
 *
 * @param key The key to search for.
 * 
 * @return True if the key exists, false otherwise.
 */
bool fossil_crabdb_key_exists(const char *key);

/**
 * Get a list of all keys in the database.
 *
 * @param count A pointer to store the number of keys.
 * 
 * @return An array of keys.
 */
char **fossil_crabdb_list_keys(size_t *count);

/**
 * Clear all entries in the database.
 * 
 * This function will free all memory allocated for the database.
 * 
 * @warning This function will delete all data in the database.
 * 
 * @note This function is useful for testing and cleanup.
 */
void fossil_crabdb_clear(void);

// *****************************************************************************
// encode and decoding
// *****************************************************************************

/**
 * Encode data to the CrabDB format.
 *
 * @param key The key to encode.
 * @param value The value to encode.
 * @param value_size The size of the value in bytes.
 * @param file The file to write the encoded data to.
 */
void fossil_crabdb_encode(const char *key, const void *value, size_t value_size, FILE *file);

/**
 * Decode data from the CrabDB format.
 *
 * @param file The file to read the encoded data from.
 * @param key A pointer to store the decoded key.
 * @param value A pointer to store the decoded value.
 * @param value_size A pointer to store the size of the decoded value.
 */
void fossil_crabdb_decode(FILE *file, char *key, void **value, size_t *value_size);

#ifdef __cplusplus
}
#endif

#endif /* FOSSIL_CRABDB_FRAMEWORK_H */

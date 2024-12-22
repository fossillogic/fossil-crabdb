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
#ifndef FOSSIL_CRABDB_QUERY_H
#define FOSSIL_CRABDB_QUERY_H

#include "database.h"

#ifdef __cplusplus
extern "C" {
#endif

// *****************************************************************************
// Query Language API
// *****************************************************************************

/**
 * @brief Executes a query string on the database.
 *
 * @param book          Pointer to the database (fossil_crabdb_book_t).
 * @param query         Query string to execute.
 * @return              0 on success, non-zero on error.
 */
int fossil_crabql_execute(fossil_crabdb_book_t *book, const char *query);

/**
 * @brief Parses and validates a query string.
 *
 * @param query         Query string to parse.
 * @return              true if valid, false otherwise.
 */
bool fossil_crabql_validate(const char *query);

/**
 * @brief Selects entries from the database matching a condition.
 *
 * @param book          Pointer to the database (fossil_crabdb_book_t).
 * @param condition     Predicate function to filter entries.
 * @return              A new book containing the filtered entries.
 */
fossil_crabdb_book_t* fossil_crabql_select(fossil_crabdb_book_t *book, bool (*condition)(fossil_crabdb_entry_t *));

/**
 * @brief Inserts a new entry into the database using a query string.
 *
 * @param book          Pointer to the database (fossil_crabdb_book_t).
 * @param query         Query string in the format "INSERT key=value".
 * @return              0 on success, non-zero on error.
 */
int fossil_crabql_insert(fossil_crabdb_book_t *book, const char *query);

/**
 * @brief Updates existing entries in the database using a query string.
 *
 * @param book          Pointer to the database (fossil_crabdb_book_t).
 * @param query         Query string in the format "UPDATE key=value WHERE condition".
 * @return              Number of updated entries, or -1 on error.
 */
int fossil_crabql_update(fossil_crabdb_book_t *book, const char *query);

/**
 * @brief Deletes entries from the database using a query string.
 *
 * @param book          Pointer to the database (fossil_crabdb_book_t).
 * @param query         Query string in the format "DELETE WHERE condition".
 * @return              Number of deleted entries, or -1 on error.
 */
int fossil_crabql_delete(fossil_crabdb_book_t *book, const char *query);

/**
 * @brief Sorts the database using a query string.
 *
 * @param book          Pointer to the database (fossil_crabdb_book_t).
 * @param query         Query string in the format "SORT BY key ASC|DESC".
 * @return              0 on success, non-zero on error.
 */
int fossil_crabql_sort(fossil_crabdb_book_t *book, const char *query);

/**
 * @brief Joins two databases using a query string.
 *
 * @param book1         Pointer to the first database (fossil_crabdb_book_t).
 * @param book2         Pointer to the second database (fossil_crabdb_book_t).
 * @param query         Query string in the format "JOIN ON key".
 * @return              A new book containing the joined entries.
 */
fossil_crabdb_book_t* fossil_crabql_join(fossil_crabdb_book_t *book1, fossil_crabdb_book_t *book2, const char *query);

/**
 * @brief Prints the query results.
 *
 * @param book          Pointer to the database (fossil_crabdb_book_t).
 */
void fossil_crabql_print_results(fossil_crabdb_book_t *book);

/**
 * @brief Exports query results to a file.
 *
 * @param book          Pointer to the database (fossil_crabdb_book_t).
 * @param filename      Name of the output file.
 * @return              0 on success, non-zero on error.
 */
int fossil_crabql_export(fossil_crabdb_book_t *book, const char *filename);

#ifdef __cplusplus
}
#endif

#endif // FOSSIL_CRABDB_QUERY_H

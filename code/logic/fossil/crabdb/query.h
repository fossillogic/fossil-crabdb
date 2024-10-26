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

/**
 * Executes a query in the CrabQL language.
 * 
 * @param db The database context.
 * @param query The query string to be executed.
 * @return true if the query was executed successfully, false otherwise.
 */
bool fossil_crabql_query(fossil_crabdb_t *db, const char *query);

/**
 * Tokenizes a CrabQL query string into individual tokens.
 * 
 * @param db The database context.
 * @param query The query string to be tokenized.
 * @param num_tokens Pointer to an integer where the number of tokens will be stored.
 * @return An array of strings, each representing a token.
 */
char **fossil_crabql_tokenize(fossil_crabdb_t *db, const char *query, int *num_tokens);

/**
 * Parses and executes a tokenized CrabQL query.
 * 
 * @param db The database context.
 * @param tokens An array of strings representing the tokens.
 * @param num_tokens The number of tokens in the array.
 * @return true if the query was parsed and executed successfully, false otherwise.
 */
bool fossil_crabql_parse_and_execute(fossil_crabdb_t *db, char **tokens, int num_tokens);

/**
 * Executes a SELECT statement in CrabQL.
 * 
 * @param db The database context.
 * @param tokens An array of strings representing the tokens.
 * @param num_tokens The number of tokens in the array.
 * @return true if the SELECT statement was executed successfully, false otherwise.
 */
bool fossil_crabql_execute_select(fossil_crabdb_t *db, char **tokens, int num_tokens);

/**
 * Executes an INSERT statement in CrabQL.
 * 
 * @param db The database context.
 * @param tokens An array of strings representing the tokens.
 * @param num_tokens The number of tokens in the array.
 * @return true if the INSERT statement was executed successfully, false otherwise.
 */
bool fossil_crabql_execute_insert(fossil_crabdb_t *db, char **tokens, int num_tokens);

/**
 * Executes an UPDATE statement in CrabQL.
 * 
 * @param db The database context.
 * @param tokens An array of strings representing the tokens.
 * @param num_tokens The number of tokens in the array.
 * @return true if the UPDATE statement was executed successfully, false otherwise.
 */
bool fossil_crabql_execute_update(fossil_crabdb_t *db, char **tokens, int num_tokens);

/**
 * Executes a DELETE statement in CrabQL.
 * 
 * @param db The database context.
 * @param tokens An array of strings representing the tokens.
 * @param num_tokens The number of tokens in the array.
 * @return true if the DELETE statement was executed successfully, false otherwise.
 */
bool fossil_crabql_execute_delete(fossil_crabdb_t *db, char **tokens, int num_tokens);

/**
 * Logs an error message.
 * 
 * @param message The error message to be logged.
 */
void fossil_crabql_log_error(const char *message);

#ifdef __cplusplus
}
#endif

#endif // FOSSIL_CRABDB_QUERY_H

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
// Query Language Operations
// *****************************************************************************

/**
 * Parse and execute a query to insert or update a key-value pair in the database.
 *
 * @param query The query string to execute (e.g., "INSERT INTO table (key, value) VALUES ('key1', 'value1')").
 * 
 * @return 0 if the operation was successful, or an error code if something went wrong.
 */
void fossil_crabql_execute(const char *query);

#ifdef __cplusplus
}
#endif

#endif // FOSSIL_CRABDB_QUERY_H

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
#ifndef FOSSIL_CRABDB_DATASTORE_H
#define FOSSIL_CRABDB_DATASTORE_H

#include "internal.h"

#include <stddef.h>
#include <stdlib.h>
#include <string.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Serializes the database to a file.
 *
 * @param db The database to serialize.
 * @param filename The name of the file to serialize to.
 * @return CRABDB_OK on success, or an error code on failure.
 */
fossil_crabdb_error_t fossil_crabdb_serialize_to_file(fossil_crabdb_t *db, const char *filename);

/**
 * @brief Deserializes the database from a file.
 *
 * @param db The database to deserialize into.
 * @param filename The name of the file to deserialize from.
 * @return CRABDB_OK on success, or an error code on failure.
 */
fossil_crabdb_error_t fossil_crabdb_deserialize_from_file(fossil_crabdb_t *db, const char *filename);

/**
 * @brief Saves the database to a file.
 *
 * @param db The database to save.
 * @param filename The name of the file to save to.
 * @return CRABDB_OK on success, or an error code on failure.
 */
int fossil_crabdb_save_to_file(fossil_crabdb_t *db, const char *filename);

/**
 * @brief Loads the database from a file.
 *
 * @param db The database to load into.
 * @param filename The name of the file to load from.
 * @return CRABDB_OK on success, or an error code on failure.
 */
int fossil_crabdb_load_from_file(fossil_crabdb_t *db, const char *filename);

#ifdef __cplusplus
}
#endif

#endif /* FOSSIL_CRABDB_FRAMEWORK_H */

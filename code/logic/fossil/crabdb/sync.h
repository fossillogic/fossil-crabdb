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
#ifndef FOSSIL_CRABDB_SYNC_H
#define FOSSIL_CRABDB_SYNC_H

#include "database.h"

#ifdef _WIN32
#include <windows.h>
#else
#include <pthread.h>
#include <errno.h>
#endif

#ifdef __cplusplus
extern "C" {
#endif

typedef struct fossil_crabsync {
    // Mutex for synchronizing access to the database
#ifdef _WIN32
    HANDLE mutex;  // Windows-specific mutex
#else
    pthread_mutex_t mutex;  // POSIX mutex
#endif
} fossil_crabsync_t;

/**
 * @brief Initializes the CrabSync library.
 * 
 * @return true if successful, false otherwise.
 */
bool fossil_crabsync_initialize(fossil_crabsync_t *sync);

/**
 * @brief Destroys the CrabSync library, cleaning up resources.
 * 
 * @param sync A pointer to the CrabSync instance to destroy.
 */
void fossil_crabsync_destroy(fossil_crabsync_t *sync);

/**
 * @brief Locks the synchronization mutex.
 * 
 * @param sync A pointer to the CrabSync instance.
 */
void fossil_crabsync_lock(fossil_crabsync_t *sync);

/**
 * @brief Unlocks the synchronization mutex.
 * 
 * @param sync A pointer to the CrabSync instance.
 */
void fossil_crabsync_unlock(fossil_crabsync_t *sync);

/**
 * @brief Synchronizes the CrabDB data across instances.
 * 
 * @param db A pointer to the CrabDB instance to synchronize.
 */
void fossil_crabsync_synchronize(fossil_crabdb_t *db);

/**
 * @brief Handles errors during synchronization.
 * 
 * @param error_code The error code to handle.
 */
void fossil_crabsync_handle_error(int error_code);

#ifdef __cplusplus
}
#endif

#endif // FOSSIL_CRABDB_QUERY_H

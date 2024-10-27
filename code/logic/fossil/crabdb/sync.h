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

/* Maximum sizes for sync metadata */
#define FOSSIL_CRABSYNC_METADATA_SIZE 512
#define FOSSIL_CRABSYNC_QUEUE_SIZE 100

/* Enumeration for sync statuses */
typedef enum {
    FOSSIL_CRABSYNC_STATUS_PENDING,
    FOSSIL_CRABSYNC_STATUS_SUCCESS,
    FOSSIL_CRABSYNC_STATUS_FAILED,
    FOSSIL_CRABSYNC_STATUS_IN_PROGRESS
} fossil_crabsync_status_t;

/* Structure for synchronization metadata */
typedef struct fossil_crabsync_metadata {
    char db_key[FOSSIL_CRABDB_KEY_SIZE];
    char db_value[FOSSIL_CRABDB_VAL_SIZE];
    fossil_crabdb_type_t type;
    time_t sync_timestamp;    // Timestamp for sync operation
    fossil_crabsync_status_t status; // Current status of the sync operation
} fossil_crabsync_metadata_t;

/* Structure for CrabSync queue */
typedef struct fossil_crabsync_queue {
    fossil_crabsync_metadata_t entries[FOSSIL_CRABSYNC_QUEUE_SIZE];
    size_t front;  // Index of the front of the queue
    size_t rear;   // Index of the rear of the queue
    size_t count;  // Current count of entries
} fossil_crabsync_queue_t;

/* Structure for CrabSync */
typedef struct fossil_crabsync {
    fossil_crabsync_queue_t queue; // Queue for pending sync operations
    fossil_crabdb_t* source_db;    // Source database to sync from
    fossil_crabdb_t* target_db;    // Target database to sync to
    bool syncing_enabled;           // Flag to indicate if syncing is enabled
    #ifdef _WIN32
        CRITICAL_SECTION sync_mutex;              // Mutex for thread safety on Windows
    #else
        pthread_mutex_t sync_mutex;     // Mutex for thread safety on POSIX
    #endif
} fossil_crabsync_t;

/* Allocate memory for a new CrabSync instance */

/**
 * @brief Creates a new CrabSync instance.
 * @param source_db A pointer to the source CrabDB instance.
 * @param target_db A pointer to the target CrabDB instance.
 * @return A pointer to the newly created CrabSync instance.
 */
fossil_crabsync_t* fossil_crabsync_create(fossil_crabdb_t* source_db, fossil_crabdb_t* target_db);

/**
 * @brief Destroys the given CrabSync instance and frees all associated memory.
 * @param sync A pointer to the CrabSync instance to be destroyed.
 */
void fossil_crabsync_destroy(fossil_crabsync_t* sync);

/* CrabSync Operations */

/**
 * @brief Adds a new sync operation to the queue.
 * @param sync A pointer to the CrabSync instance.
 * @param key The key to sync.
 * @param value The value to sync.
 * @param type The type of the value.
 * @return true if the sync operation was added successfully, false otherwise.
 */
bool fossil_crabsync_add(fossil_crabsync_t* sync, const char* key, const char* value, fossil_crabdb_type_t type);

/**
 * @brief Processes the next sync operation in the queue.
 * @param sync A pointer to the CrabSync instance.
 * @return true if the sync operation was processed successfully, false otherwise.
 */
bool fossil_crabsync_process_next(fossil_crabsync_t* sync);

/**
 * @brief Syncs all pending operations in the queue.
 * @param sync A pointer to the CrabSync instance.
 * @return true if all operations were synced successfully, false otherwise.
 */
bool fossil_crabsync_sync_all(fossil_crabsync_t* sync);

/* CrabSync Queue Operations */

/**
 * @brief Initializes the sync queue.
 * @param queue A pointer to the sync queue.
 */
void fossil_crabsync_queue_init(fossil_crabsync_queue_t* queue);

/**
 * @brief Adds an entry to the sync queue.
 * @param queue A pointer to the sync queue.
 * @param entry The metadata entry to add.
 * @return true if the entry was added successfully, false otherwise.
 */
bool fossil_crabsync_queue_enqueue(fossil_crabsync_queue_t* queue, const fossil_crabsync_metadata_t* entry);

/**
 * @brief Removes an entry from the sync queue.
 * @param queue A pointer to the sync queue.
 * @param entry A pointer to store the removed entry.
 * @return true if the entry was removed successfully, false otherwise.
 */
bool fossil_crabsync_queue_dequeue(fossil_crabsync_queue_t* queue, fossil_crabsync_metadata_t* entry);

#ifdef __cplusplus
}
#endif

#endif // FOSSIL_CRABDB_QUERY_H

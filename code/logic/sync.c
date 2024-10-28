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
#include "fossil/crabdb/sync.h"

fossil_crabsync_t* fossil_crabsync_create(fossil_crabdb_t* source_db, fossil_crabdb_t* target_db) {
    if (!source_db || !target_db) return NULL;

    /* Allocate memory for the CrabSync instance */
    fossil_crabsync_t* sync = (fossil_crabsync_t*)malloc(sizeof(fossil_crabsync_t));
    if (!sync) {
        return NULL;
    }

    /* Initialize the CrabSync queue */
    fossil_crabsync_queue_init(&sync->queue);

    /* Set the source and target databases */
    sync->source_db = source_db;
    sync->target_db = target_db;

    /* Enable syncing by default */
    sync->syncing_enabled = true;

#ifdef _WIN32
    /* Initialize the sync mutex for Windows */
    InitializeCriticalSection(&sync->sync_mutex);
#else
    /* Initialize the sync mutex for POSIX */
    pthread_mutex_init(&sync->sync_mutex, NULL);
#endif

    return sync;
}

void fossil_crabsync_destroy(fossil_crabsync_t* sync) {
    if (!sync) return;

    /* Free the CrabSync queue */
    fossil_crabsync_queue_init(&sync->queue);

#ifdef _WIN32
    /* Destroy the sync mutex for Windows */
    DeleteCriticalSection(&sync->sync_mutex);
#else
    /* Destroy the sync mutex for POSIX */
    pthread_mutex_destroy(&sync->sync_mutex);
#endif

    /* Free the CrabSync instance */
    free(sync);
}

/* CrabSync Operations */

bool fossil_crabsync_add(fossil_crabsync_t* sync, const char* key, const char* value, fossil_crabdb_type_t type) {
    if (!sync || !key || !value) return false;

#ifdef _WIN32
    /* Lock the sync mutex for Windows */
    EnterCriticalSection(&sync->sync_mutex);
#else
    /* Lock the sync mutex for POSIX */
    if (pthread_mutex_lock(&sync->sync_mutex) != 0) {
        return false;
    }
#endif

    /* Create a new metadata entry */
    fossil_crabsync_metadata_t entry;
    strncpy(entry.db_key, key, FOSSIL_CRABDB_KEY_SIZE);
    strncpy(entry.db_value, value, FOSSIL_CRABDB_VAL_SIZE);
    entry.type = type;
    entry.sync_timestamp = time(NULL);
    entry.status = FOSSIL_CRABSYNC_STATUS_PENDING;

    /* Add the entry to the queue */
    bool success = fossil_crabsync_queue_enqueue(&sync->queue, &entry);

#ifdef _WIN32
    /* Unlock the sync mutex for Windows */
    LeaveCriticalSection(&sync->sync_mutex);
#else
    /* Unlock the sync mutex for POSIX */
    pthread_mutex_unlock(&sync->sync_mutex);
#endif

    return success;
}

bool fossil_crabsync_process_next(fossil_crabsync_t* sync) {
    if (!sync) return false;

    fossil_crabsync_metadata_t entry;
    if (!fossil_crabsync_queue_dequeue(&sync->queue, &entry)) {
        return false;
    }

#ifdef _WIN32
    /* Lock the sync mutex for Windows */
    EnterCriticalSection(&sync->sync_mutex);
#else
    /* Lock the sync mutex for POSIX */
    if (pthread_mutex_lock(&sync->sync_mutex) != 0) {
        return false;
    }
#endif

    /* Perform the sync operation */
    bool success = fossil_crabdb_insert(sync->target_db, entry.db_key, entry.db_value, entry.type);

    /* Update the sync status */
    entry.status = success ? FOSSIL_CRABSYNC_STATUS_SUCCESS : FOSSIL_CRABSYNC_STATUS_FAILED;

#ifdef _WIN32
    /* Unlock the sync mutex for Windows */
    LeaveCriticalSection(&sync->sync_mutex);
#else
    /* Unlock the sync mutex for POSIX */
    pthread_mutex_unlock(&sync->sync_mutex);
#endif

    return success;
}

bool fossil_crabsync_sync_all(fossil_crabsync_t* sync) {
    if (!sync) return false;

    while (sync->queue.count > 0) {
        if (!fossil_crabsync_process_next(sync)) {
            return false;
        }
    }

    return true;
}

/* CrabSync Queue Operations */

void fossil_crabsync_queue_init(fossil_crabsync_queue_t* queue) {
    if (!queue) return;

    queue->front = 0;
    queue->rear = 0;
    queue->count = 0;
}

bool fossil_crabsync_queue_enqueue(fossil_crabsync_queue_t* queue, const fossil_crabsync_metadata_t* entry) {
    if (!queue || !entry) return false;

    if (queue->count == FOSSIL_CRABSYNC_QUEUE_SIZE) {
        return false;
    }

    queue->entries[queue->rear] = *entry;
    queue->rear = (queue->rear + 1) % FOSSIL_CRABSYNC_QUEUE_SIZE;
    queue->count++;

    return true;
}

bool fossil_crabsync_queue_dequeue(fossil_crabsync_queue_t* queue, fossil_crabsync_metadata_t* entry) {
    if (!queue || !entry) return false;

    if (queue->count == 0) {
        return false;
    }

    *entry = queue->entries[queue->front];
    queue->front = (queue->front + 1) % FOSSIL_CRABSYNC_QUEUE_SIZE;
    queue->count--;

    return true;
}

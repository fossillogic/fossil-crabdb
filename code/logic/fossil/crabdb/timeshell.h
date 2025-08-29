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
#ifndef FOSSIL_CRABDB_TIMESHELL_H
#define FOSSIL_CRABDB_TIMESHELL_H

#include <stdbool.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <sys/stat.h>   // for file size
#include <errno.h>
#include <stdio.h>
#include <time.h>

#ifdef __cplusplus
extern "C" {
#endif

// *****************************************************************************
// Enumerations for Error Codes
// *****************************************************************************
typedef enum {
    FOSSIL_TIMESHELL_ERROR_SUCCESS = 0,
    FOSSIL_TIMESHELL_ERROR_INVALID_FILE,
    FOSSIL_TIMESHELL_ERROR_FILE_NOT_FOUND,
    FOSSIL_TIMESHELL_ERROR_IO,
    FOSSIL_TIMESHELL_ERROR_INVALID_INTERVAL,
    FOSSIL_TIMESHELL_ERROR_NOT_FOUND,
    FOSSIL_TIMESHELL_ERROR_ALREADY_EXISTS,
    FOSSIL_TIMESHELL_ERROR_BACKUP_FAILED,
    FOSSIL_TIMESHELL_ERROR_RESTORE_FAILED,
    FOSSIL_TIMESHELL_ERROR_UNKNOWN
} fossil_timeshell_error_t;

// *****************************************************************************
// Time Interval Structure
// *****************************************************************************
typedef struct {
    uint64_t start_time;   // Unix timestamp in seconds
    uint64_t end_time;     // Unix timestamp in seconds
} fossil_timeshell_interval_t;

// *****************************************************************************
// TimeShell CRUD Functions
// *****************************************************************************

/**
 * @brief Inserts a new time interval into the database.
 * 
 * @param file_name     Database file name (.fdb enforced)
 * @param interval      Pointer to interval to insert
 * @return              FOSSIL_TIMESHELL_ERROR_SUCCESS on success, otherwise error code
 */
fossil_timeshell_error_t fossil_bluecrab_timeshell_insert(const char *file_name, const fossil_timeshell_interval_t *interval);

/**
 * @brief Finds intervals that overlap with a given interval.
 * 
 * @param file_name     Database file name
 * @param query         Interval to query
 * @param results       Buffer to store matching intervals
 * @param max_results   Maximum number of intervals to return
 * @param out_count     Number of intervals found
 * @return              FOSSIL_TIMESHELL_ERROR_SUCCESS on success, otherwise error code
 */
fossil_timeshell_error_t fossil_bluecrab_timeshell_find(const char *file_name, const fossil_timeshell_interval_t *query,
                                                        fossil_timeshell_interval_t *results, size_t max_results, size_t *out_count);

/**
 * @brief Updates an existing interval based on exact start/end match.
 * 
 * @param file_name     Database file name
 * @param old_interval  Interval to match
 * @param new_interval  Interval to replace with
 * @return              FOSSIL_TIMESHELL_ERROR_SUCCESS on success, otherwise error code
 */
fossil_timeshell_error_t fossil_bluecrab_timeshell_update(const char *file_name, const fossil_timeshell_interval_t *old_interval,
                                                          const fossil_timeshell_interval_t *new_interval);

/**
 * @brief Removes a specific interval.
 * 
 * @param file_name     Database file name
 * @param interval      Interval to remove
 * @return              FOSSIL_TIMESHELL_ERROR_SUCCESS on success, otherwise error code
 */
fossil_timeshell_error_t fossil_bluecrab_timeshell_remove(const char *file_name, const fossil_timeshell_interval_t *interval);

// *****************************************************************************
// Database Management
// *****************************************************************************

fossil_timeshell_error_t fossil_bluecrab_timeshell_create_database(const char *file_name);
fossil_timeshell_error_t fossil_bluecrab_timeshell_open_database(const char *file_name);
fossil_timeshell_error_t fossil_bluecrab_timeshell_delete_database(const char *file_name);

// *****************************************************************************
// Backup / Restore
// *****************************************************************************

fossil_timeshell_error_t fossil_bluecrab_timeshell_backup_database(const char *source_file, const char *backup_file);
fossil_timeshell_error_t fossil_bluecrab_timeshell_restore_database(const char *backup_file, const char *destination_file);

// *****************************************************************************
// Metadata
// *****************************************************************************

/**
 * @brief Counts the number of intervals in the database.
 */
fossil_timeshell_error_t fossil_bluecrab_timeshell_count_intervals(const char *file_name, size_t *count);

/**
 * @brief Gets the size of the database file in bytes.
 */
fossil_timeshell_error_t fossil_bluecrab_timeshell_get_file_size(const char *file_name, size_t *size_bytes);

/**
 * @brief Validates the file extension (.fdb)
 */
bool fossil_bluecrab_timeshell_validate_extension(const char *file_name);

/**
 * @brief Validates a time interval (start < end)
 */
bool fossil_bluecrab_timeshell_validate_interval(const fossil_timeshell_interval_t *interval);

#ifdef __cplusplus
}
#include <string>

namespace fossil {

namespace bluecrab {



}

}

#endif

#endif /* FOSSIL_CRABDB_FRAMEWORK_H */

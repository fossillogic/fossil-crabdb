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
    FOSSIL_TIMESHELL_ERROR_SUCCESS = 0,       /**< Operation completed successfully */
    FOSSIL_TIMESHELL_ERROR_INVALID_FILE,      /**< File name or format is invalid */
    FOSSIL_TIMESHELL_ERROR_FILE_NOT_FOUND,    /**< Database file not found */
    FOSSIL_TIMESHELL_ERROR_IO,                /**< Input/output error during file operation */
    FOSSIL_TIMESHELL_ERROR_INVALID_INTERVAL,  /**< Interval data is invalid */
    FOSSIL_TIMESHELL_ERROR_NOT_FOUND,         /**< Requested interval not found */
    FOSSIL_TIMESHELL_ERROR_ALREADY_EXISTS,    /**< Interval or database already exists */
    FOSSIL_TIMESHELL_ERROR_BACKUP_FAILED,     /**< Database backup failed */
    FOSSIL_TIMESHELL_ERROR_RESTORE_FAILED,    /**< Database restore failed */
    FOSSIL_TIMESHELL_ERROR_LOCKED,            /**< Database is locked and cannot be accessed */
    FOSSIL_TIMESHELL_ERROR_CORRUPTED,         /**< Database file is corrupted or hash mismatch */
    FOSSIL_TIMESHELL_ERROR_MEMORY,            /**< Memory allocation failed */
    FOSSIL_TIMESHELL_ERROR_UNSUPPORTED,       /**< Operation not supported */
    FOSSIL_TIMESHELL_ERROR_UNKNOWN            /**< Unknown or unexpected error */
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
 * @param file_name     Database file name (.crabdb enforced)
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
 * @brief Validates the file extension (.crabdb)
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

        /**
         * @class TimeShell
         * @brief C++ wrapper for Fossil BlueCrab TimeShell C API.
         * 
         * Provides static methods for CRUD operations, database management,
         * backup/restore, and metadata queries on time interval databases.
         */
        class TimeShell {
        public:
            /**
             * @brief Inserts a new time interval into the database.
             * @param file_name Database file name (.crabdb enforced)
             * @param interval Pointer to interval to insert
             * @return Error code (see fossil_timeshell_error_t)
             */
            static fossil_timeshell_error_t insert(const std::string& file_name, const fossil_timeshell_interval_t* interval) {
                return fossil_bluecrab_timeshell_insert(file_name.c_str(), interval);
            }

            /**
             * @brief Finds intervals that overlap with a given interval.
             * @param file_name Database file name
             * @param query Interval to query
             * @param results Buffer to store matching intervals
             * @param max_results Maximum number of intervals to return
             * @param out_count Number of intervals found
             * @return Error code (see fossil_timeshell_error_t)
             */
            static fossil_timeshell_error_t find(const std::string& file_name, const fossil_timeshell_interval_t* query,
                                                fossil_timeshell_interval_t* results, size_t max_results, size_t* out_count) {
                return fossil_bluecrab_timeshell_find(file_name.c_str(), query, results, max_results, out_count);
            }

            /**
             * @brief Updates an existing interval based on exact start/end match.
             * @param file_name Database file name
             * @param old_interval Interval to match
             * @param new_interval Interval to replace with
             * @return Error code (see fossil_timeshell_error_t)
             */
            static fossil_timeshell_error_t update(const std::string& file_name, const fossil_timeshell_interval_t* old_interval,
                                                const fossil_timeshell_interval_t* new_interval) {
                return fossil_bluecrab_timeshell_update(file_name.c_str(), old_interval, new_interval);
            }

            /**
             * @brief Removes a specific interval.
             * @param file_name Database file name
             * @param interval Interval to remove
             * @return Error code (see fossil_timeshell_error_t)
             */
            static fossil_timeshell_error_t remove(const std::string& file_name, const fossil_timeshell_interval_t* interval) {
                return fossil_bluecrab_timeshell_remove(file_name.c_str(), interval);
            }

            /**
             * @brief Creates a new database file.
             * @param file_name Database file name (.crabdb enforced)
             * @return Error code (see fossil_timeshell_error_t)
             */
            static fossil_timeshell_error_t create_database(const std::string& file_name) {
                return fossil_bluecrab_timeshell_create_database(file_name.c_str());
            }

            /**
             * @brief Opens an existing database file.
             * @param file_name Database file name
             * @return Error code (see fossil_timeshell_error_t)
             */
            static fossil_timeshell_error_t open_database(const std::string& file_name) {
                return fossil_bluecrab_timeshell_open_database(file_name.c_str());
            }

            /**
             * @brief Deletes a database file.
             * @param file_name Database file name
             * @return Error code (see fossil_timeshell_error_t)
             */
            static fossil_timeshell_error_t delete_database(const std::string& file_name) {
                return fossil_bluecrab_timeshell_delete_database(file_name.c_str());
            }

            /**
             * @brief Creates a backup of the database file.
             * @param source_file Source database file name
             * @param backup_file Backup file name
             * @return Error code (see fossil_timeshell_error_t)
             */
            static fossil_timeshell_error_t backup_database(const std::string& source_file, const std::string& backup_file) {
                return fossil_bluecrab_timeshell_backup_database(source_file.c_str(), backup_file.c_str());
            }

            /**
             * @brief Restores the database from a backup file.
             * @param backup_file Backup file name
             * @param destination_file Destination database file name
             * @return Error code (see fossil_timeshell_error_t)
             */
            static fossil_timeshell_error_t restore_database(const std::string& backup_file, const std::string& destination_file) {
                return fossil_bluecrab_timeshell_restore_database(backup_file.c_str(), destination_file.c_str());
            }

            /**
             * @brief Counts the number of intervals in the database.
             * @param file_name Database file name
             * @param count Pointer to store the interval count
             * @return Error code (see fossil_timeshell_error_t)
             */
            static fossil_timeshell_error_t count_intervals(const std::string& file_name, size_t* count) {
                return fossil_bluecrab_timeshell_count_intervals(file_name.c_str(), count);
            }

            /**
             * @brief Gets the size of the database file in bytes.
             * @param file_name Database file name
             * @param size_bytes Pointer to store the file size in bytes
             * @return Error code (see fossil_timeshell_error_t)
             */
            static fossil_timeshell_error_t get_file_size(const std::string& file_name, size_t* size_bytes) {
                return fossil_bluecrab_timeshell_get_file_size(file_name.c_str(), size_bytes);
            }

            /**
             * @brief Validates the file extension (.crabdb).
             * @param file_name Database file name
             * @return True if extension is valid, false otherwise
             */
            static bool validate_extension(const std::string& file_name) {
                return fossil_bluecrab_timeshell_validate_extension(file_name.c_str());
            }

            /**
             * @brief Validates a time interval (start < end).
             * @param interval Pointer to interval to validate
             * @return True if interval is valid, false otherwise
             */
            static bool validate_interval(const fossil_timeshell_interval_t* interval) {
                return fossil_bluecrab_timeshell_validate_interval(interval);
            }
        }; // class TimeShell

    } // namespace crabdb

} // namespace fossil

#endif

#endif /* FOSSIL_CRABDB_FRAMEWORK_H */

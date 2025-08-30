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
#ifndef FOSSIL_CRABDB_MYSHELL_H
#define FOSSIL_CRABDB_MYSHELL_H

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
// Enumerations for Data Types and Attributes
// *****************************************************************************

// ===========================================================
// MyShell Error Codes
// ===========================================================
typedef enum {
    FOSSIL_MYSHELL_ERROR_SUCCESS = 0,
    FOSSIL_MYSHELL_ERROR_INVALID_FILE,
    FOSSIL_MYSHELL_ERROR_FILE_NOT_FOUND,
    FOSSIL_MYSHELL_ERROR_IO,
    FOSSIL_MYSHELL_ERROR_INVALID_QUERY,
    FOSSIL_MYSHELL_ERROR_CONCURRENCY,
    FOSSIL_MYSHELL_ERROR_NOT_FOUND,
    FOSSIL_MYSHELL_ERROR_PERMISSION_DENIED,
    FOSSIL_MYSHELL_ERROR_CORRUPTED,
    FOSSIL_MYSHELL_ERROR_OUT_OF_MEMORY,
    FOSSIL_MYSHELL_ERROR_UNSUPPORTED,
    FOSSIL_MYSHELL_ERROR_LOCKED,
    FOSSIL_MYSHELL_ERROR_TIMEOUT,
    FOSSIL_MYSHELL_ERROR_ALREADY_EXISTS,
    FOSSIL_MYSHELL_ERROR_BACKUP_FAILED,
    FOSSIL_MYSHELL_ERROR_RESTORE_FAILED,
    FOSSIL_MYSHELL_ERROR_UNKNOWN
} fossil_bluecrab_myshell_error_t;

// ===========================================================
// MyShell Functions
// ===========================================================

// CRUD operations

/**
 * @brief Creates a new record in the database.
 * 
 * @param file_name     The name of the database file.
 * @param key           The key of the record.
 * @param value         The value of the record.
 * @return              0 on success, non-zero on error.
 */
fossil_bluecrab_myshell_error_t fossil_bluecrab_myshell_create_record(const char *file_name, const char *key, const char *value);

/**
 * @brief Reads a record from the database.
 * 
 * @param file_name     The name of the database file.
 * @param key           The key of the record to read.
 * @param value         The buffer to store the value of the record.
 * @param buffer_size   The size of the buffer.
 * @return              0 on success, non-zero on error.
 */
fossil_bluecrab_myshell_error_t fossil_bluecrab_myshell_read_record(const char *file_name, const char *key, char *value, size_t buffer_size);

/**
 * @brief Updates the value of a record in the database.
 * 
 * @param file_name     The name of the database file.
 * @param key           The key of the record to update.
 * @param new_value     The new value to set.
 * @return              0 on success, non-zero on error.
 */
fossil_bluecrab_myshell_error_t fossil_bluecrab_myshell_update_record(const char *file_name, const char *key, const char *new_value);

/**
 * @brief Deletes a record from the database.
 * 
 * @param file_name     The name of the database file.
 * @param key           The key of the record to delete.
 * @return              0 on success, non-zero on error.
 */
fossil_bluecrab_myshell_error_t fossil_bluecrab_myshell_delete_record(const char *file_name, const char *key);

// Database management

/**
 * @brief Creates a new database file.
 * 
 * @param file_name     The name of the database file.
 * @return              0 on success, non-zero on error.
 */
fossil_bluecrab_myshell_error_t fossil_bluecrab_myshell_create_database(const char *file_name);

/**
 * @brief Opens an existing database file.
 * 
 * @param file_name     The name of the database file.
 * @return              0 on success, non-zero on error.
 */
fossil_bluecrab_myshell_error_t fossil_bluecrab_myshell_open_database(const char *file_name);

/**
 * @brief Deletes a database file.
 * 
 * @param file_name     The name of the database file.
 * @return              0 on success, non-zero on error.
 */
fossil_bluecrab_myshell_error_t fossil_bluecrab_myshell_delete_database(const char *file_name);

// Backup and Restore

/**
 * @brief Backs up a database file.
 * 
 * @param source_file   The name of the source database file.
 * @param backup_file   The name of the backup file.
 * @return              0 on success, non-zero on error.
 */
fossil_bluecrab_myshell_error_t fossil_bluecrab_myshell_backup_database(const char *source_file, const char *backup_file);

/**
 * @brief Restores a database file from a backup.
 * 
 * @param backup_file       The name of the backup file.
 * @param destination_file  The name of the destination database file.
 * @return                  0 on success, non-zero on error.
 */
fossil_bluecrab_myshell_error_t fossil_bluecrab_myshell_restore_database(const char *backup_file, const char *destination_file);

// Query and data validation

/**
 * @brief Verifies the integrity of a .fdb database file by checking all record hashes.
 * 
 * @param file_name     The name of the database file.
 * @return              FOSSIL_MYSHELL_ERROR_SUCCESS if all records pass,
 *                      FOSSIL_MYSHELL_ERROR_CORRUPTED if any record fails,
 *                      other error codes on I/O or invalid file.
 */
fossil_bluecrab_myshell_error_t fossil_bluecrab_myshell_verify_database(const char *file_name);

/**
 * @brief Validates the file extension of a database file.
 * 
 * @param file_name     The name of the database file.
 * @return              True if the file extension is valid, false otherwise.
 */
bool fossil_bluecrab_myshell_validate_extension(const char *file_name);

/**
 * @brief Validates a data string.
 * 
 * @param data          The data string to validate.
 * @return              True if the data is valid, false otherwise.
 */
bool fossil_bluecrab_myshell_validate_data(const char *data);

/**
 * @brief Closes an opened database file.
 * 
 * @param file_name     The name of the database file.
 * @return              0 on success, non-zero on error.
 */
fossil_bluecrab_myshell_error_t fossil_bluecrab_myshell_close_database(const char *file_name);

/**
 * @brief Checks if a database file is currently open.
 * 
 * @param file_name     The name of the database file.
 * @return              True if open, false otherwise.
 */
bool fossil_bluecrab_myshell_is_open(const char *file_name);

/**
 * @brief Gets the first key in the database.
 * 
 * @param file_name     The name of the database file.
 * @param key_buffer    Buffer to store the key.
 * @param buffer_size   Size of the buffer.
 * @return              0 on success, non-zero on error.
 */
fossil_bluecrab_myshell_error_t fossil_bluecrab_myshell_first_key(const char *file_name, char *key_buffer, size_t buffer_size);

/**
 * @brief Gets the next key in the database (iteration).
 * 
 * @param file_name     The name of the database file.
 * @param prev_key      The previous key from iteration.
 * @param key_buffer    Buffer to store the next key.
 * @param buffer_size   Size of the buffer.
 * @return              0 on success, non-zero on error, or EOF if no more keys.
 */
fossil_bluecrab_myshell_error_t fossil_bluecrab_myshell_next_key(const char *file_name, const char *prev_key, char *key_buffer, size_t buffer_size);

/**
 * @brief Gets the total number of records in the database.
 * 
 * @param file_name     The name of the database file.
 * @param count         Pointer to store the record count.
 * @return              0 on success, non-zero on error.
 */
fossil_bluecrab_myshell_error_t fossil_bluecrab_myshell_count_records(const char *file_name, size_t *count);

/**
 * @brief Gets the size of the database file in bytes.
 * 
 * @param file_name     The name of the database file.
 * @param size_bytes    Pointer to store the file size.
 * @return              0 on success, non-zero on error.
 */
fossil_bluecrab_myshell_error_t fossil_bluecrab_myshell_get_file_size(const char *file_name, size_t *size_bytes);

/**
 * @brief Converts an error code to a human-readable string.
 * 
 * @param error_code    The error code.
 * @return              A string describing the error.
 */
const char* fossil_bluecrab_myshell_error_string(fossil_bluecrab_myshell_error_t error_code);

#ifdef __cplusplus
}
#include <string>

namespace fossil {

    namespace bluecrab {

        /**
         * @brief C++ wrapper class for MyShell database operations.
         * 
         * Provides static methods for CRUD, database management, backup/restore,
         * validation, and iteration, mapping to the C API.
         */
        class MyShell {
        public:
            /**
             * @brief Creates a new record in the database.
             * @param file_name Name of the database file.
             * @param key Key of the record.
             * @param value Value of the record.
             * @return Error code (0 on success).
             */
            static fossil_bluecrab_myshell_error_t create_record(const std::string& file_name, const std::string& key, const std::string& value) {
                return fossil_bluecrab_myshell_create_record(file_name.c_str(), key.c_str(), value.c_str());
            }

            /**
             * @brief Reads a record from the database.
             * @param file_name Name of the database file.
             * @param key Key of the record to read.
             * @param value Output parameter to store the value.
             * @return Error code (0 on success).
             */
            static fossil_bluecrab_myshell_error_t read_record(const std::string& file_name, const std::string& key, std::string& value) {
                char buffer[4096] = {0};
                fossil_bluecrab_myshell_error_t err = fossil_bluecrab_myshell_read_record(file_name.c_str(), key.c_str(), buffer, sizeof(buffer));
                if (err == FOSSIL_MYSHELL_ERROR_SUCCESS) {
                    value = buffer;
                }
                return err;
            }

            /**
             * @brief Updates the value of a record in the database.
             * @param file_name Name of the database file.
             * @param key Key of the record to update.
             * @param new_value New value to set.
             * @return Error code (0 on success).
             */
            static fossil_bluecrab_myshell_error_t update_record(const std::string& file_name, const std::string& key, const std::string& new_value) {
                return fossil_bluecrab_myshell_update_record(file_name.c_str(), key.c_str(), new_value.c_str());
            }

            /**
             * @brief Deletes a record from the database.
             * @param file_name Name of the database file.
             * @param key Key of the record to delete.
             * @return Error code (0 on success).
             */
            static fossil_bluecrab_myshell_error_t delete_record(const std::string& file_name, const std::string& key) {
                return fossil_bluecrab_myshell_delete_record(file_name.c_str(), key.c_str());
            }

            /**
             * @brief Creates a new database file.
             * @param file_name Name of the database file.
             * @return Error code (0 on success).
             */
            static fossil_bluecrab_myshell_error_t create_database(const std::string& file_name) {
                return fossil_bluecrab_myshell_create_database(file_name.c_str());
            }

            /**
             * @brief Opens an existing database file.
             * @param file_name Name of the database file.
             * @return Error code (0 on success).
             */
            static fossil_bluecrab_myshell_error_t open_database(const std::string& file_name) {
                return fossil_bluecrab_myshell_open_database(file_name.c_str());
            }

            /**
             * @brief Deletes a database file.
             * @param file_name Name of the database file.
             * @return Error code (0 on success).
             */
            static fossil_bluecrab_myshell_error_t delete_database(const std::string& file_name) {
                return fossil_bluecrab_myshell_delete_database(file_name.c_str());
            }

            /**
             * @brief Backs up a database file.
             * @param source_file Name of the source database file.
             * @param backup_file Name of the backup file.
             * @return Error code (0 on success).
             */
            static fossil_bluecrab_myshell_error_t backup_database(const std::string& source_file, const std::string& backup_file) {
                return fossil_bluecrab_myshell_backup_database(source_file.c_str(), backup_file.c_str());
            }

            /**
             * @brief Restores a database file from a backup.
             * @param backup_file Name of the backup file.
             * @param destination_file Name of the destination database file.
             * @return Error code (0 on success).
             */
            static fossil_bluecrab_myshell_error_t restore_database(const std::string& backup_file, const std::string& destination_file) {
                return fossil_bluecrab_myshell_restore_database(backup_file.c_str(), destination_file.c_str());
            }

            /**
             * @brief Verifies the integrity of a database file.
             * @param file_name Name of the database file.
             * @return Error code (0 on success, FOSSIL_MYSHELL_ERROR_CORRUPTED if failed).
             */
            static fossil_bluecrab_myshell_error_t verify_database(const std::string& file_name) {
                return fossil_bluecrab_myshell_verify_database(file_name.c_str());
            }

            /**
             * @brief Validates the file extension of a database file.
             * @param file_name Name of the database file.
             * @return True if the extension is valid, false otherwise.
             */
            static bool validate_extension(const std::string& file_name) {
                return fossil_bluecrab_myshell_validate_extension(file_name.c_str());
            }

            /**
             * @brief Validates a data string.
             * @param data Data string to validate.
             * @return True if the data is valid, false otherwise.
             */
            static bool validate_data(const std::string& data) {
                return fossil_bluecrab_myshell_validate_data(data.c_str());
            }

            /**
             * @brief Closes an opened database file.
             * @param file_name Name of the database file.
             * @return Error code (0 on success).
             */
            static fossil_bluecrab_myshell_error_t close_database(const std::string& file_name) {
                return fossil_bluecrab_myshell_close_database(file_name.c_str());
            }

            /**
             * @brief Checks if a database file is currently open.
             * @param file_name Name of the database file.
             * @return True if open, false otherwise.
             */
            static bool is_open(const std::string& file_name) {
                return fossil_bluecrab_myshell_is_open(file_name.c_str());
            }

            /**
             * @brief Gets the first key in the database.
             * @param file_name Name of the database file.
             * @param key Output parameter to store the key.
             * @return Error code (0 on success).
             */
            static fossil_bluecrab_myshell_error_t first_key(const std::string& file_name, std::string& key) {
                char buffer[4096] = {0};
                fossil_bluecrab_myshell_error_t err = fossil_bluecrab_myshell_first_key(file_name.c_str(), buffer, sizeof(buffer));
                if (err == FOSSIL_MYSHELL_ERROR_SUCCESS) {
                    key = buffer;
                }
                return err;
            }

            /**
             * @brief Gets the next key in the database (iteration).
             * @param file_name Name of the database file.
             * @param prev_key Previous key from iteration.
             * @param key Output parameter to store the next key.
             * @return Error code (0 on success, or EOF if no more keys).
             */
            static fossil_bluecrab_myshell_error_t next_key(const std::string& file_name, const std::string& prev_key, std::string& key) {
                char buffer[4096] = {0};
                fossil_bluecrab_myshell_error_t err = fossil_bluecrab_myshell_next_key(file_name.c_str(), prev_key.c_str(), buffer, sizeof(buffer));
                if (err == FOSSIL_MYSHELL_ERROR_SUCCESS) {
                    key = buffer;
                }
                return err;
            }

            /**
             * @brief Gets the total number of records in the database.
             * @param file_name Name of the database file.
             * @param count Output parameter to store the record count.
             * @return Error code (0 on success).
             */
            static fossil_bluecrab_myshell_error_t count_records(const std::string& file_name, size_t& count) {
                return fossil_bluecrab_myshell_count_records(file_name.c_str(), &count);
            }

            /**
             * @brief Gets the size of the database file in bytes.
             * @param file_name Name of the database file.
             * @param size_bytes Output parameter to store the file size.
             * @return Error code (0 on success).
             */
            static fossil_bluecrab_myshell_error_t get_file_size(const std::string& file_name, size_t& size_bytes) {
                return fossil_bluecrab_myshell_get_file_size(file_name.c_str(), &size_bytes);
            }

            /**
             * @brief Converts an error code to a human-readable string.
             * @param error_code The error code.
             * @return String describing the error.
             */
            static std::string error_string(fossil_bluecrab_myshell_error_t error_code) {
                return fossil_bluecrab_myshell_error_string(error_code);
            }
        };

    } // namespace bluecrab

} // namespace fossil

#endif

#endif /* FOSSIL_CRABDB_FRAMEWORK_H */

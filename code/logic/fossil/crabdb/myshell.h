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
    FOSSIL_MYSHELL_ERROR_ALREADY_EXISTS,
    FOSSIL_MYSHELL_ERROR_BACKUP_FAILED,
    FOSSIL_MYSHELL_ERROR_RESTORE_FAILED,
    FOSSIL_MYSHELL_ERROR_UNKNOWN
} fossil_myshell_error_t;

// *****************************************************************************
// Database API Functions
// *****************************************************************************

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
fossil_myshell_error_t fossil_myshell_create_record(const char *file_name, const char *key, const char *value);

/**
 * @brief Reads a record from the database.
 * 
 * @param file_name     The name of the database file.
 * @param key           The key of the record to read.
 * @param value         The buffer to store the value of the record.
 * @param buffer_size   The size of the buffer.
 * @return              0 on success, non-zero on error.
 */
fossil_myshell_error_t fossil_myshell_read_record(const char *file_name, const char *key, char *value, size_t buffer_size);

/**
 * @brief Updates the value of a record in the database.
 * 
 * @param file_name     The name of the database file.
 * @param key           The key of the record to update.
 * @param new_value     The new value to set.
 * @return              0 on success, non-zero on error.
 */
fossil_myshell_error_t fossil_myshell_update_record(const char *file_name, const char *key, const char *new_value);

/**
 * @brief Deletes a record from the database.
 * 
 * @param file_name     The name of the database file.
 * @param key           The key of the record to delete.
 * @return              0 on success, non-zero on error.
 */
fossil_myshell_error_t fossil_myshell_delete_record(const char *file_name, const char *key);

// Database management

/**
 * @brief Creates a new database file.
 * 
 * @param file_name     The name of the database file.
 * @return              0 on success, non-zero on error.
 */
fossil_myshell_error_t fossil_myshell_create_database(const char *file_name);

/**
 * @brief Opens an existing database file.
 * 
 * @param file_name     The name of the database file.
 * @return              0 on success, non-zero on error.
 */
fossil_myshell_error_t fossil_myshell_open_database(const char *file_name);

/**
 * @brief Closes an open database file.
 * 
 * @param file_name     The name of the database file.
 * @return              0 on success, non-zero on error.
 */
fossil_myshell_error_t fossil_myshell_close_database(const char *file_name);

/**
 * @brief Deletes a database file.
 * 
 * @param file_name     The name of the database file.
 * @return              0 on success, non-zero on error.
 */
fossil_myshell_error_t fossil_myshell_delete_database(const char *file_name);

// Backup and Restore

/**
 * @brief Backs up a database file.
 * 
 * @param source_file   The name of the source database file.
 * @param backup_file   The name of the backup file.
 * @return              0 on success, non-zero on error.
 */
fossil_myshell_error_t fossil_myshell_backup_database(const char *source_file, const char *backup_file);

/**
 * @brief Restores a database file from a backup.
 * 
 * @param backup_file       The name of the backup file.
 * @param destination_file  The name of the destination database file.
 * @return                  0 on success, non-zero on error.
 */
fossil_myshell_error_t fossil_myshell_restore_database(const char *backup_file, const char *destination_file);

// Query and data validation

/**
 * @brief Validates the file extension of a database file.
 * 
 * @param file_name     The name of the database file.
 * @return              True if the file extension is valid, false otherwise.
 */
bool fossil_myshell_validate_extension(const char *file_name);

/**
 * @brief Validates a data string.
 * 
 * @param data          The data string to validate.
 * @return              True if the data is valid, false otherwise.
 */
bool fossil_myshell_validate_data(const char *data);

#ifdef __cplusplus
}
#include <string>

namespace fossil {

/**
 * @class MyShell
 * @brief A class that provides an interface to interact with the Fossil MyShell database.
 * 
 * This class encapsulates the functionality of the Fossil MyShell, providing methods to
 * create, read, update, and delete records in the database. It also provides methods to
 * manage the database, enable/disable concurrency, backup/restore the database, and perform
 * sorting, aggregation, and filtering operations.
 * 
 * @note The class manages the lifecycle of the database, ensuring proper initialization
 * and release of resources.
 */
class MyShell {
public:
    // CRUD operations

    /**
     * @brief Creates a new record in the database.
     * 
     * @param fileName      The name of the database file.
     * @param key           The key of the record.
     * @param value         The value of the record.
     * @return              0 on success, non-zero on error.
     */
    static fossil_myshell_error_t createRecord(const std::string &fileName, const std::string &key, const std::string &value) {
        return fossil_myshell_create_record(fileName.c_str(), key.c_str(), value.c_str());
    }

    /**
     * @brief Reads a record from the database.
     * 
     * @param fileName      The name of the database file.
     * @param key           The key of the record to read.
     * @param value         The buffer to store the value of the record.
     * @return              0 on success, non-zero on error.
     */
    static fossil_myshell_error_t readRecord(const std::string &fileName, const std::string &key, std::string &value) {
        char buffer[1024];
        fossil_myshell_error_t result = fossil_myshell_read_record(fileName.c_str(), key.c_str(), buffer, sizeof(buffer));
        if (result == FOSSIL_MYSHELL_ERROR_SUCCESS) {
            value = buffer;
        }
        return result;
    }

    /**
     * @brief Updates the value of a record in the database.
     * 
     * @param fileName      The name of the database file.
     * @param key           The key of the record to update.
     * @param newValue      The new value to set.
     * @return              0 on success, non-zero on error.
     */
    static fossil_myshell_error_t updateRecord(const std::string &fileName, const std::string &key, const std::string &newValue) {
        return fossil_myshell_update_record(fileName.c_str(), key.c_str(), newValue.c_str());
    }

    /**
     * @brief Deletes a record from the database.
     * 
     * @param fileName      The name of the database file.
     * @param key           The key of the record to delete.
     * @return              0 on success, non-zero on error.
     */
    static fossil_myshell_error_t deleteRecord(const std::string &fileName, const std::string &key) {
        return fossil_myshell_delete_record(fileName.c_str(), key.c_str());
    }

    // Database management
    /**
     * @brief Creates a new database file.
     * 
     * @param fileName      The name of the database file.
     * @return              0 on success, non-zero on error.
     */
    static fossil_myshell_error_t createDatabase(const std::string &fileName) {
        return fossil_myshell_create_database(fileName.c_str());
    }

    /**
     * @brief Opens an existing database file.
     * 
     * @param fileName      The name of the database file.
     * @return              0 on success, non-zero on error.
     */
    static fossil_myshell_error_t openDatabase(const std::string &fileName) {
        return fossil_myshell_open_database(fileName.c_str());
    }

    /**
     * @brief Closes an open database file.
     * 
     * @param fileName      The name of the database file.
     * @return              0 on success, non-zero on error.
     */
    static fossil_myshell_error_t closeDatabase(const std::string &fileName) {
        return fossil_myshell_close_database(fileName.c_str());
    }

    /**
     * @brief Deletes a database file.
     * 
     * @param fileName      The name of the database file.
     * @return              0 on success, non-zero on error.
     */
    static fossil_myshell_error_t deleteDatabase(const std::string &fileName) {
        return fossil_myshell_delete_database(fileName.c_str());
    }

    // Backup and Restore
    /**
     * @brief Backs up a database file.
     * 
     * @param sourceFile    The name of the source database file.
     * @param backupFile    The name of the backup file.
     * @return              0 on success, non-zero on error.
     */
    static fossil_myshell_error_t backupDatabase(const std::string &sourceFile, const std::string &backupFile) {
        return fossil_myshell_backup_database(sourceFile.c_str(), backupFile.c_str());
    }

    /**
     * @brief Restores a database file from a backup.
     * 
     * @param backupFile        The name of the backup file.
     * @param destinationFile   The name of the destination database file.
     * @return                  0 on success, non-zero on error.
     */
    static fossil_myshell_error_t restoreDatabase(const std::string &backupFile, const std::string &destinationFile) {
        return fossil_myshell_restore_database(backupFile.c_str(), destinationFile.c_str());
    }

    // Query and data validation
    /**
     * @brief Validates the file extension of a database file.
     * 
     * @param fileName      The name of the database file.
     * @return              True if the file extension is valid, false otherwise.
     */
    static bool validateExtension(const std::string &fileName) {
        return fossil_myshell_validate_extension(fileName.c_str());
    }

    /**
     * @brief Validates a data string.
     * 
     * @param data          The data string to validate.
     * @return              True if the data is valid, false otherwise.
     */
    static bool validateData(const std::string &data) {
        return fossil_myshell_validate_data(data.c_str());
    }
};

}

#endif

#endif /* FOSSIL_CRABDB_FRAMEWORK_H */

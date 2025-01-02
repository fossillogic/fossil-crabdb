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
#ifndef FOSSIL_CRABDB_NOSHELL_H
#define FOSSIL_CRABDB_NOSHELL_H

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
// NoShell Error Codes
// ===========================================================
typedef enum {
    FOSSIL_NOSHELL_ERROR_SUCCESS = 0,
    FOSSIL_NOSHELL_ERROR_INVALID_FILE,
    FOSSIL_NOSHELL_ERROR_FILE_NOT_FOUND,
    FOSSIL_NOSHELL_ERROR_IO,
    FOSSIL_NOSHELL_ERROR_INVALID_QUERY,
    FOSSIL_NOSHELL_ERROR_CONCURRENCY,
    FOSSIL_NOSHELL_ERROR_NOT_FOUND,
    FOSSIL_NOSHELL_ERROR_ALREADY_EXISTS,
    FOSSIL_NOSHELL_ERROR_BACKUP_FAILED,
    FOSSIL_NOSHELL_ERROR_RESTORE_FAILED,
    FOSSIL_NOSHELL_ERROR_UNKNOWN
} fossil_noshell_error_t;

// *****************************************************************************
// Database API Functions
// *****************************************************************************

// ===========================================================
// NoShell Functions
// ===========================================================

// CRUD operations

/**
 * @brief Inserts a new document into the database.
 * 
 * @param file_name     The name of the database file.
 * @param document      The document to insert.
 * @return              0 on success, non-zero on error.
 */
fossil_noshell_error_t fossil_noshell_insert(const char *file_name, const char *document);

/**
 * @brief Finds a document in the database based on a query.
 * 
 * @param file_name     The name of the database file.
 * @param query         The query to search for.
 * @param result        The buffer to store the result.
 * @return              0 on success, non-zero on error.
 */
fossil_noshell_error_t fossil_noshell_find(const char *file_name, const char *query, char *result, size_t buffer_size);

/**
 * @brief Updates a document in the database based on a query.
 * 
 * @param file_name     The name of the database file.
 * @param query         The query to search for.
 * @param new_document  The new document to update.
 * @return              0 on success, non-zero on error.
 */
fossil_noshell_error_t fossil_noshell_update(const char *file_name, const char *query, const char *new_document);

/**
 * @brief Removes a document from the database based on a query.
 * 
 * @param file_name     The name of the database file.
 * @param query         The query to search for.
 * @return              0 on success, non-zero on error.
 */
fossil_noshell_error_t fossil_noshell_remove(const char *file_name, const char *query);

// Database management

/**
 * @brief Creates a new database file.
 * 
 * @param file_name     The name of the database file.
 * @return              0 on success, non-zero on error.
 */
fossil_noshell_error_t fossil_noshell_create_database(const char *file_name);

/**
 * @brief Opens an existing database file.
 * 
 * @param file_name     The name of the database file.
 * @return              0 on success, non-zero on error.
 */
fossil_noshell_error_t fossil_noshell_open_database(const char *file_name);

/**
 * @brief Deletes a database file.
 * 
 * @param file_name     The name of the database file.
 * @return              0 on success, non-zero on error.
 */
fossil_noshell_error_t fossil_noshell_delete_database(const char *file_name);

// Backup and Restore

/**
 * @brief Backs up a database file.
 * 
 * @param source_file   The name of the source database file.
 * @param backup_file   The name of the backup file.
 * @return              0 on success, non-zero on error.
 */
fossil_noshell_error_t fossil_noshell_backup_database(const char *source_file, const char *backup_file);

/**
 * @brief Restores a database file from a backup.
 * 
 * @param backup_file       The name of the backup file.
 * @param destination_file  The name of the destination database file.
 * @return                  0 on success, non-zero on error.
 */
fossil_noshell_error_t fossil_noshell_restore_database(const char *backup_file, const char *destination_file);

// Query and data validation

/**
 * @brief Validates the file extension of a database file.
 * 
 * @param file_name      The name of the database file.
 * @return              True if the file extension is valid, false otherwise.
 */
bool fossil_noshell_validate_extension(const char *file_name);

/**
 * @brief Validates a document string.
 * 
 * @param document      The document string to validate.
 * @return              True if the document is valid, false otherwise.
 */
bool fossil_noshell_validate_document(const char *document);

#ifdef __cplusplus
}
#include <string>

namespace fossil {

//
class NoShell {
public:
    // CRUD operations
    /**
     * @brief Inserts a new document into the database.
     * 
     * @param file_name     The name of the database file.
     * @param document      The document to insert.
     * @return              0 on success, non-zero on error.
     */
    static fossil_noshell_error_t insert(const std::string &file_name, const std::string &document) {
        return fossil_noshell_insert(file_name.c_str(), document.c_str());
    }

    /**
     * @brief Finds a document in the database based on a query.
     * 
     * @param file_name     The name of the database file.
     * @param query         The query to search for.
     * @param result        The buffer to store the result.
     * @return              0 on success, non-zero on error.
     */
    static fossil_noshell_error_t find(const std::string &file_name, const std::string &query, std::string &result) {
        char buffer[1024];
        fossil_noshell_error_t error = fossil_noshell_find(file_name.c_str(), query.c_str(), buffer, sizeof(buffer));
        if (error == FOSSIL_NOSHELL_ERROR_SUCCESS) {
            result = buffer;
        }
        return error;
    }

    /**
     * @brief Updates a document in the database based on a query.
     * 
     * @param file_name     The name of the database file.
     * @param query         The query to search for.
     * @param new_document  The new document to update.
     * @return              0 on success, non-zero on error.
     */
    static fossil_noshell_error_t update(const std::string &file_name, const std::string &query, const std::string &new_document) {
        return fossil_noshell_update(file_name.c_str(), query.c_str(), new_document.c_str());
    }

    /**
     * @brief Removes a document from the database based on a query.
     * 
     * @param file_name     The name of the database file.
     * @param query         The query to search for.
     * @return              0 on success, non-zero on error.
     */
    static fossil_noshell_error_t remove(const std::string &file_name, const std::string &query) {
        return fossil_noshell_remove(file_name.c_str(), query.c_str());
    }

    // Database management
    /**
     * @brief Creates a new database file.
     * 
     * @param file_name     The name of the database file.
     * @return              0 on success, non-zero on error.
     */
    static fossil_noshell_error_t createDatabase(const std::string &file_name) {
        return fossil_noshell_create_database(file_name.c_str());
    }

    /**
     * @brief Opens an existing database file.
     * 
     * @param file_name     The name of the database file.
     * @return              0 on success, non-zero on error.
     */
    static fossil_noshell_error_t openDatabase(const std::string &file_name) {
        return fossil_noshell_open_database(file_name.c_str());
    }

    /**
     * @brief Deletes a database file.
     * 
     * @param file_name     The name of the database file.
     * @return              0 on success, non-zero on error.
     */
    static fossil_noshell_error_t deleteDatabase(const std::string &file_name) {
        return fossil_noshell_delete_database(file_name.c_str());
    }

    // Backup and Restore
    /**
     * @brief Backs up a database file.
     * 
     * @param source_file   The name of the source database file.
     * @param backup_file   The name of the backup file.
     * @return              0 on success, non-zero on error.
     */
    static fossil_noshell_error_t backupDatabase(const std::string &source_file, const std::string &backup_file) {
        return fossil_noshell_backup_database(source_file.c_str(), backup_file.c_str());
    }

    /**
     * @brief Restores a database file from a backup.
     * 
     * @param backup_file       The name of the backup file.
     * @param destination_file  The name of the destination database file.
     * @return                  0 on success, non-zero on error.
     */
    static fossil_noshell_error_t restoreDatabase(const std::string &backup_file, const std::string &destination_file) {
        return fossil_noshell_restore_database(backup_file.c_str(), destination_file.c_str());
    }

    // Query and data validation
    /**
     * @brief Validates the file extension of a database file.
     * 
     * @param file_name      The name of the database file.
     * @return              True if the file extension is valid, false otherwise.
     */
    static bool validateExtension(const std::string &file_name) {
        return fossil_noshell_validate_extension(file_name.c_str());
    }

    /**
     * @brief Validates a document string.
     * 
     * @param document      The document string to validate.
     * @return              True if the document is valid, false otherwise.
     */
    static bool validateDocument(const std::string &document) {
        return fossil_noshell_validate_document(document.c_str());
    }
};

}

#endif

#endif /* FOSSIL_CRABDB_FRAMEWORK_H */

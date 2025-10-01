/**
 * -----------------------------------------------------------------------------
 * Project: Fossil Logic
 *
 * This file is part of the Fossil Logic project, which aims to develop
 * high-performance, cross-platform applications and libraries. The code
 * contained herein is licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License. You may obtain
 * a copy of the License at:
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS, WITHOUT
 * WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied. See the
 * License for the specific language governing permissions and limitations
 * under the License.
 *
 * Author: Michael Gene Brockus (Dreamer)
 * Date: 04/05/2014
 *
 * Copyright (C) 2014-2025 Fossil Logic. All rights reserved.
 * -----------------------------------------------------------------------------
 */
#ifndef FOSSIL_CRABDB_NOSHELL_H
#define FOSSIL_CRABDB_NOSHELL_H

#include <inttypes.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <sys/stat.h>   // for file size
#include <errno.h>
#include <stdio.h>
#include <strings.h>
#include <ctype.h>
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
    FOSSIL_NOSHELL_ERROR_PERMISSION_DENIED,
    FOSSIL_NOSHELL_ERROR_CORRUPTED,
    FOSSIL_NOSHELL_ERROR_OUT_OF_MEMORY,
    FOSSIL_NOSHELL_ERROR_UNSUPPORTED,
    FOSSIL_NOSHELL_ERROR_LOCKED,
    FOSSIL_NOSHELL_ERROR_TIMEOUT,
    FOSSIL_NOSHELL_ERROR_ALREADY_EXISTS,
    FOSSIL_NOSHELL_ERROR_BACKUP_FAILED,
    FOSSIL_NOSHELL_ERROR_PARSE_FAILED,
    FOSSIL_NOSHELL_ERROR_RESTORE_FAILED,
    FOSSIL_NOSHELL_ERROR_UNKNOWN
} fossil_bluecrab_noshell_error_t;

// ============================================================================
// FSON v2 compatible value representation (local to NoShell)
// ============================================================================
typedef enum {
    NOSHELL_FSON_TYPE_NULL = 0,
    NOSHELL_FSON_TYPE_BOOL,

    // Scalars
    NOSHELL_FSON_TYPE_I8,
    NOSHELL_FSON_TYPE_I16,
    NOSHELL_FSON_TYPE_I32,
    NOSHELL_FSON_TYPE_I64,
    NOSHELL_FSON_TYPE_U8,
    NOSHELL_FSON_TYPE_U16,
    NOSHELL_FSON_TYPE_U32,
    NOSHELL_FSON_TYPE_U64,
    NOSHELL_FSON_TYPE_F32,
    NOSHELL_FSON_TYPE_F64,

    // Literals
    NOSHELL_FSON_TYPE_OCT,
    NOSHELL_FSON_TYPE_HEX,
    NOSHELL_FSON_TYPE_BIN,

    // Strings
    NOSHELL_FSON_TYPE_CHAR,
    NOSHELL_FSON_TYPE_CSTR,

    // Composite
    NOSHELL_FSON_TYPE_ARRAY,
    NOSHELL_FSON_TYPE_OBJECT,

    // v2 Additions
    NOSHELL_FSON_TYPE_ENUM,
    NOSHELL_FSON_TYPE_DATETIME,
    NOSHELL_FSON_TYPE_DURATION
} fossil_bluecrab_noshell_fson_type_t;

typedef struct {
    fossil_bluecrab_noshell_fson_type_t type;
    union {
        // Base
        bool b;

        // Signed
        int8_t   i8;
        int16_t  i16;
        int32_t  i32;
        int64_t  i64;

        // Unsigned
        uint8_t  u8;
        uint16_t u16;
        uint32_t u32;
        uint64_t u64;

        // Floating
        float    f32;
        double   f64;

        // Numeric literals (store as string repr)
        char    *oct;   // "0755"
        char    *hex;   // "0xFF"
        char    *bin;   // "0b1010"

        // Strings / chars
        char     c;
        char    *cstr;

        // Composite containers (stored as serialized text blobs)
        char    *array;   // e.g. "[1,2,3]"
        char    *object;  // e.g. "{key:val}"

        // v2 extras
        char    *enum_symbol;  // "RED", "GREEN"
        char    *datetime;     // ISO 8601 "2025-09-30T12:00:00Z"
        char    *duration;     // "30s", "1h", "5d"
    } as;
} fossil_bluecrab_noshell_fson_value_t;

// ===========================================================
// Document CRUD Operations
// ===========================================================

/**
 * @brief Inserts a new document into the database.
 * 
 * @param file_name     The database file name (.crabdb enforced).
 * @param document      The document string to insert.
 * @return              FOSSIL_NOSHELL_ERROR_SUCCESS on success, otherwise error code.
 */
fossil_bluecrab_noshell_error_t fossil_bluecrab_noshell_insert(const char *file_name, const char *document);

/**
 * @brief Inserts a document and returns a unique internal ID.
 * 
 * @param file_name     The database file name (.crabdb enforced).
 * @param document      The document string to insert.
 * @param out_id        Buffer to store generated document ID.
 * @param id_size       Size of the buffer.
 * @return              FOSSIL_NOSHELL_ERROR_SUCCESS on success, otherwise error code.
 */
fossil_bluecrab_noshell_error_t fossil_bluecrab_noshell_insert_with_id(const char *file_name, const char *document, char *out_id, size_t id_size);

/**
 * @brief Finds a document based on a query string.
 * 
 * @param file_name     The database file name.
 * @param query         The query string to search.
 * @param result        Buffer to store the matching document.
 * @param buffer_size   Size of the buffer.
 * @return              FOSSIL_NOSHELL_ERROR_SUCCESS on success, otherwise error code.
 */
fossil_bluecrab_noshell_error_t fossil_bluecrab_noshell_find(const char *file_name, const char *query, char *result, size_t buffer_size);

/**
 * @brief Finds documents using a callback filter function.
 * 
 * @param file_name     The database file name.
 * @param cb            Callback function to evaluate each document.
 * @param userdata      Optional user data passed to the callback.
 * @return              FOSSIL_NOSHELL_ERROR_SUCCESS on success, otherwise error code.
 */
fossil_bluecrab_noshell_error_t fossil_bluecrab_noshell_find_cb(const char *file_name, bool (*cb)(const char *document, void *userdata), void *userdata);

/**
 * @brief Updates a document in the database based on a query string.
 * 
 * @param file_name     The database file name.
 * @param query         Query string to locate document(s).
 * @param new_document  New document content to replace matching documents.
 * @return              FOSSIL_NOSHELL_ERROR_SUCCESS on success, otherwise error code.
 */
fossil_bluecrab_noshell_error_t fossil_bluecrab_noshell_update(const char *file_name, const char *query, const char *new_document);

/**
 * @brief Removes a document from the database based on a query string.
 * 
 * @param file_name     The database file name.
 * @param query         Query string to locate document(s) to remove.
 * @return              FOSSIL_NOSHELL_ERROR_SUCCESS on success, otherwise error code.
 */
fossil_bluecrab_noshell_error_t fossil_bluecrab_noshell_remove(const char *file_name, const char *query);

// ===========================================================
// Database Management
// ===========================================================

/**
 * @brief Creates a new database file.
 * 
 * @param file_name     The database file name (.crabdb enforced).
 * @return              FOSSIL_NOSHELL_ERROR_SUCCESS on success, otherwise error code.
 */
fossil_bluecrab_noshell_error_t fossil_bluecrab_noshell_create_database(const char *file_name);

/**
 * @brief Opens an existing database file.
 * 
 * @param file_name     The database file name.
 * @return              FOSSIL_NOSHELL_ERROR_SUCCESS on success, otherwise error code.
 */
fossil_bluecrab_noshell_error_t fossil_bluecrab_noshell_open_database(const char *file_name);

/**
 * @brief Deletes a database file.
 * 
 * @param file_name     The database file name.
 * @return              FOSSIL_NOSHELL_ERROR_SUCCESS on success, otherwise error code.
 */
fossil_bluecrab_noshell_error_t fossil_bluecrab_noshell_delete_database(const char *file_name);

/**
 * @brief Locks the database file for exclusive access.
 * 
 * @param file_name     The database file name.
 * @return              FOSSIL_NOSHELL_ERROR_SUCCESS on success, otherwise error code.
 */
fossil_bluecrab_noshell_error_t fossil_bluecrab_noshell_lock_database(const char *file_name);

/**
 * @brief Unlocks the database file.
 * 
 * @param file_name     The database file name.
 * @return              FOSSIL_NOSHELL_ERROR_SUCCESS on success, otherwise error code.
 */
fossil_bluecrab_noshell_error_t fossil_bluecrab_noshell_unlock_database(const char *file_name);

/**
 * @brief Checks if a database file is currently locked.
 * 
 * @param file_name     The database file name.
 * @return              true if locked, false otherwise.
 */
bool fossil_bluecrab_noshell_is_locked(const char *file_name);

// ===========================================================
// Backup, Restore, and Verification
// ===========================================================

/**
 * @brief Backs up a database file.
 * 
 * @param source_file   The source database file.
 * @param backup_file   The backup file path.
 * @return              FOSSIL_NOSHELL_ERROR_SUCCESS on success, otherwise error code.
 */
fossil_bluecrab_noshell_error_t fossil_bluecrab_noshell_backup_database(const char *source_file, const char *backup_file);

/**
 * @brief Restores a database file from a backup.
 * 
 * @param backup_file       The backup file path.
 * @param destination_file  The destination database file.
 * @return                  FOSSIL_NOSHELL_ERROR_SUCCESS on success, otherwise error code.
 */
fossil_bluecrab_noshell_error_t fossil_bluecrab_noshell_restore_database(const char *backup_file, const char *destination_file);

/**
 * @brief Verifies the integrity of all documents using internal 64-bit hashes.
 * 
 * @param file_name     The database file name.
 * @return              FOSSIL_NOSHELL_ERROR_SUCCESS if all documents valid,
 *                      FOSSIL_NOSHELL_ERROR_CORRUPTED if any mismatch.
 */
fossil_bluecrab_noshell_error_t fossil_bluecrab_noshell_verify_database(const char *file_name);

// ===========================================================
// Iteration Helpers
// ===========================================================

/**
 * @brief Gets the first document ID in the database.
 * 
 * @param file_name     The database file name.
 * @param id_buffer     Buffer to store the first document ID.
 * @param buffer_size   Size of the buffer.
 * @return              FOSSIL_NOSHELL_ERROR_SUCCESS on success, otherwise error code.
 */
fossil_bluecrab_noshell_error_t fossil_bluecrab_noshell_first_document(const char *file_name, char *id_buffer, size_t buffer_size);

/**
 * @brief Gets the next document ID after a previous one.
 * 
 * @param file_name     The database file name.
 * @param prev_id       The previous document ID.
 * @param id_buffer     Buffer to store the next document ID.
 * @param buffer_size   Size of the buffer.
 * @return              FOSSIL_NOSHELL_ERROR_SUCCESS on success, FOSSIL_NOSHELL_ERROR_NOT_FOUND if no more documents.
 */
fossil_bluecrab_noshell_error_t fossil_bluecrab_noshell_next_document(const char *file_name, const char *prev_id, char *id_buffer, size_t buffer_size);

// ===========================================================
// Metadata Helpers
// ===========================================================

/**
 * @brief Counts the number of documents in the database.
 * 
 * @param file_name     The database file name.
 * @param count         Pointer to store document count.
 * @return              FOSSIL_NOSHELL_ERROR_SUCCESS on success, otherwise error code.
 */
fossil_bluecrab_noshell_error_t fossil_bluecrab_noshell_count_documents(const char *file_name, size_t *count);

/**
 * @brief Gets the size of the database file in bytes.
 * 
 * @param file_name     The database file name.
 * @param size_bytes    Pointer to store the file size.
 * @return              FOSSIL_NOSHELL_ERROR_SUCCESS on success, otherwise error code.
 */
fossil_bluecrab_noshell_error_t fossil_bluecrab_noshell_get_file_size(const char *file_name, size_t *size_bytes);

// ===========================================================
// Validation Helpers
// ===========================================================

/**
 * @brief Validates the file extension of a database file.
 * 
 * @param file_name     The database file name.
 * @return              true if the extension is valid (.crabdb), false otherwise.
 */
bool fossil_bluecrab_noshell_validate_extension(const char *file_name);

/**
 * @brief Validates a document string.
 * 
 * @param document      The document string to validate.
 * @return              true if valid, false otherwise.
 */
bool fossil_bluecrab_noshell_validate_document(const char *document);

#ifdef __cplusplus
}
#include <string>

namespace fossil {

    namespace bluecrab {

        /**
         * @brief C++ wrapper class for NoShell database operations.
         * 
         * Provides static methods that wrap the C API for easier use in C++ code.
         */
        class NoShell {
        public:
            /**
             * @brief Inserts a new document into the database.
             * @param file_name The database file name (.crabdb enforced).
             * @param document The document string to insert.
             * @return FOSSIL_NOSHELL_ERROR_SUCCESS on success, otherwise error code.
             */
            static fossil_bluecrab_noshell_error_t insert(const std::string& file_name, const std::string& document) {
                return fossil_bluecrab_noshell_insert(file_name.c_str(), document.c_str());
            }

            /**
             * @brief Inserts a document and returns a unique internal ID.
             * @param file_name The database file name (.crabdb enforced).
             * @param document The document string to insert.
             * @param out_id Reference to a string to store the generated document ID.
             * @return FOSSIL_NOSHELL_ERROR_SUCCESS on success, otherwise error code.
             */
            static fossil_bluecrab_noshell_error_t insert_with_id(const std::string& file_name, const std::string& document, std::string& out_id) {
                char buffer[128] = {0};
                fossil_bluecrab_noshell_error_t err = fossil_bluecrab_noshell_insert_with_id(file_name.c_str(), document.c_str(), buffer, sizeof(buffer));
                if (err == FOSSIL_NOSHELL_ERROR_SUCCESS) {
                    out_id = buffer;
                }
                return err;
            }

            /**
             * @brief Finds a document based on a query string.
             * @param file_name The database file name.
             * @param query The query string to search.
             * @param result Reference to a string to store the matching document.
             * @return FOSSIL_NOSHELL_ERROR_SUCCESS on success, otherwise error code.
             */
            static fossil_bluecrab_noshell_error_t find(const std::string& file_name, const std::string& query, std::string& result) {
                char buffer[4096] = {0};
                fossil_bluecrab_noshell_error_t err = fossil_bluecrab_noshell_find(file_name.c_str(), query.c_str(), buffer, sizeof(buffer));
                if (err == FOSSIL_NOSHELL_ERROR_SUCCESS) {
                    result = buffer;
                }
                return err;
            }

            /**
             * @brief Updates a document in the database based on a query string.
             * @param file_name The database file name.
             * @param query Query string to locate document(s).
             * @param new_document New document content to replace matching documents.
             * @return FOSSIL_NOSHELL_ERROR_SUCCESS on success, otherwise error code.
             */
            static fossil_bluecrab_noshell_error_t update(const std::string& file_name, const std::string& query, const std::string& new_document) {
                return fossil_bluecrab_noshell_update(file_name.c_str(), query.c_str(), new_document.c_str());
            }

            /**
             * @brief Removes a document from the database based on a query string.
             * @param file_name The database file name.
             * @param query Query string to locate document(s) to remove.
             * @return FOSSIL_NOSHELL_ERROR_SUCCESS on success, otherwise error code.
             */
            static fossil_bluecrab_noshell_error_t remove(const std::string& file_name, const std::string& query) {
                return fossil_bluecrab_noshell_remove(file_name.c_str(), query.c_str());
            }

            /**
             * @brief Creates a new database file.
             * @param file_name The database file name (.crabdb enforced).
             * @return FOSSIL_NOSHELL_ERROR_SUCCESS on success, otherwise error code.
             */
            static fossil_bluecrab_noshell_error_t create_database(const std::string& file_name) {
                return fossil_bluecrab_noshell_create_database(file_name.c_str());
            }

            /**
             * @brief Opens an existing database file.
             * @param file_name The database file name.
             * @return FOSSIL_NOSHELL_ERROR_SUCCESS on success, otherwise error code.
             */
            static fossil_bluecrab_noshell_error_t open_database(const std::string& file_name) {
                return fossil_bluecrab_noshell_open_database(file_name.c_str());
            }

            /**
             * @brief Deletes a database file.
             * @param file_name The database file name.
             * @return FOSSIL_NOSHELL_ERROR_SUCCESS on success, otherwise error code.
             */
            static fossil_bluecrab_noshell_error_t delete_database(const std::string& file_name) {
                return fossil_bluecrab_noshell_delete_database(file_name.c_str());
            }

            /**
             * @brief Locks the database file for exclusive access.
             * @param file_name The database file name.
             * @return FOSSIL_NOSHELL_ERROR_SUCCESS on success, otherwise error code.
             */
            static fossil_bluecrab_noshell_error_t lock_database(const std::string& file_name) {
                return fossil_bluecrab_noshell_lock_database(file_name.c_str());
            }

            /**
             * @brief Unlocks the database file.
             * @param file_name The database file name.
             * @return FOSSIL_NOSHELL_ERROR_SUCCESS on success, otherwise error code.
             */
            static fossil_bluecrab_noshell_error_t unlock_database(const std::string& file_name) {
                return fossil_bluecrab_noshell_unlock_database(file_name.c_str());
            }

            /**
             * @brief Checks if a database file is currently locked.
             * @param file_name The database file name.
             * @return true if locked, false otherwise.
             */
            static bool is_locked(const std::string& file_name) {
                return fossil_bluecrab_noshell_is_locked(file_name.c_str());
            }

            /**
             * @brief Backs up a database file.
             * @param source_file The source database file.
             * @param backup_file The backup file path.
             * @return FOSSIL_NOSHELL_ERROR_SUCCESS on success, otherwise error code.
             */
            static fossil_bluecrab_noshell_error_t backup_database(const std::string& source_file, const std::string& backup_file) {
                return fossil_bluecrab_noshell_backup_database(source_file.c_str(), backup_file.c_str());
            }

            /**
             * @brief Restores a database file from a backup.
             * @param backup_file The backup file path.
             * @param destination_file The destination database file.
             * @return FOSSIL_NOSHELL_ERROR_SUCCESS on success, otherwise error code.
             */
            static fossil_bluecrab_noshell_error_t restore_database(const std::string& backup_file, const std::string& destination_file) {
                return fossil_bluecrab_noshell_restore_database(backup_file.c_str(), destination_file.c_str());
            }

            /**
             * @brief Verifies the integrity of all documents using internal 64-bit hashes.
             * @param file_name The database file name.
             * @return FOSSIL_NOSHELL_ERROR_SUCCESS if all documents valid, error code otherwise.
             */
            static fossil_bluecrab_noshell_error_t verify_database(const std::string& file_name) {
                return fossil_bluecrab_noshell_verify_database(file_name.c_str());
            }

            /**
             * @brief Gets the first document ID in the database.
             * @param file_name The database file name.
             * @param id Reference to a string to store the first document ID.
             * @return FOSSIL_NOSHELL_ERROR_SUCCESS on success, otherwise error code.
             */
            static fossil_bluecrab_noshell_error_t first_document(const std::string& file_name, std::string& id) {
                char buffer[128] = {0};
                fossil_bluecrab_noshell_error_t err = fossil_bluecrab_noshell_first_document(file_name.c_str(), buffer, sizeof(buffer));
                if (err == FOSSIL_NOSHELL_ERROR_SUCCESS) {
                    id = buffer;
                }
                return err;
            }

            /**
             * @brief Gets the next document ID after a previous one.
             * @param file_name The database file name.
             * @param prev_id The previous document ID.
             * @param id Reference to a string to store the next document ID.
             * @return FOSSIL_NOSHELL_ERROR_SUCCESS on success, FOSSIL_NOSHELL_ERROR_NOT_FOUND if no more documents.
             */
            static fossil_bluecrab_noshell_error_t next_document(const std::string& file_name, const std::string& prev_id, std::string& id) {
                char buffer[128] = {0};
                fossil_bluecrab_noshell_error_t err = fossil_bluecrab_noshell_next_document(file_name.c_str(), prev_id.c_str(), buffer, sizeof(buffer));
                if (err == FOSSIL_NOSHELL_ERROR_SUCCESS) {
                    id = buffer;
                }
                return err;
            }

            /**
             * @brief Counts the number of documents in the database.
             * @param file_name The database file name.
             * @param count Reference to a size_t to store document count.
             * @return FOSSIL_NOSHELL_ERROR_SUCCESS on success, otherwise error code.
             */
            static fossil_bluecrab_noshell_error_t count_documents(const std::string& file_name, size_t& count) {
                return fossil_bluecrab_noshell_count_documents(file_name.c_str(), &count);
            }

            /**
             * @brief Gets the size of the database file in bytes.
             * @param file_name The database file name.
             * @param size_bytes Reference to a size_t to store the file size.
             * @return FOSSIL_NOSHELL_ERROR_SUCCESS on success, otherwise error code.
             */
            static fossil_bluecrab_noshell_error_t get_file_size(const std::string& file_name, size_t& size_bytes) {
                return fossil_bluecrab_noshell_get_file_size(file_name.c_str(), &size_bytes);
            }

            /**
             * @brief Validates the file extension of a database file.
             * @param file_name The database file name.
             * @return true if the extension is valid (.crabdb), false otherwise.
             */
            static bool validate_extension(const std::string& file_name) {
                return fossil_bluecrab_noshell_validate_extension(file_name.c_str());
            }

            /**
             * @brief Validates a document string.
             * @param document The document string to validate.
             * @return true if valid, false otherwise.
             */
            static bool validate_document(const std::string& document) {
                return fossil_bluecrab_noshell_validate_document(document.c_str());
            }
        };

    } // namespace bluecrab

} // namespace fossil

#endif

#endif /* FOSSIL_CRABDB_FRAMEWORK_H */

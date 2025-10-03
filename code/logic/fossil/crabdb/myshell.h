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
#ifndef FOSSIL_CRABDB_MYSHELL_H
#define FOSSIL_CRABDB_MYSHELL_H

#include <inttypes.h>
#include <stdbool.h>
#include <strings.h>
#include <string.h>
#include <stddef.h>
#include <stdlib.h>
#include <stdint.h>
#include <sys/stat.h>   // for file size
#include <errno.h>
#include <stdio.h>
#include <ctype.h>
#include <time.h>

#ifdef __cplusplus
extern "C" {
#endif

// *****************************************************************************
// Enumerations for Data Types and Attributes
// *****************************************************************************

/**
 * ===========================================================
 * MyShell Error Codes
 * ===========================================================
 * Enumerates all possible error codes returned by MyShell API functions.
 * These codes represent various failure and success states encountered
 * during database operations such as file handling, queries, concurrency,
 * permissions, backup/restore, and parsing.
 */
typedef enum {
    FOSSIL_MYSHELL_ERROR_SUCCESS = 0,          /**< Operation completed successfully. */
    FOSSIL_MYSHELL_ERROR_INVALID_FILE,         /**< The specified file is invalid or corrupt. */
    FOSSIL_MYSHELL_ERROR_FILE_NOT_FOUND,       /**< The requested file was not found. */
    FOSSIL_MYSHELL_ERROR_IO,                   /**< Input/output error occurred during operation. */
    FOSSIL_MYSHELL_ERROR_INVALID_QUERY,        /**< The query provided is invalid or malformed. */
    FOSSIL_MYSHELL_ERROR_CONCURRENCY,          /**< Concurrency conflict detected (e.g., locked resource). */
    FOSSIL_MYSHELL_ERROR_NOT_FOUND,            /**< Requested key or record not found in database. */
    FOSSIL_MYSHELL_ERROR_PERMISSION_DENIED,    /**< Operation denied due to insufficient permissions. */
    FOSSIL_MYSHELL_ERROR_CORRUPTED,            /**< Database or file is corrupted. */
    FOSSIL_MYSHELL_ERROR_OUT_OF_MEMORY,        /**< Memory allocation failed. */
    FOSSIL_MYSHELL_ERROR_UNSUPPORTED,          /**< Operation or feature is not supported. */
    FOSSIL_MYSHELL_ERROR_LOCKED,               /**< Resource is locked and cannot be accessed. */
    FOSSIL_MYSHELL_ERROR_TIMEOUT,              /**< Operation timed out. */
    FOSSIL_MYSHELL_ERROR_ALREADY_EXISTS,       /**< Resource already exists (e.g., duplicate key). */
    FOSSIL_MYSHELL_ERROR_BACKUP_FAILED,        /**< Backup operation failed. */
    FOSSIL_MYSHELL_ERROR_PARSE_FAILED,         /**< Parsing of input or file failed. */
    FOSSIL_MYSHELL_ERROR_RESTORE_FAILED,       /**< Restore operation failed. */
    FOSSIL_MYSHELL_ERROR_LOCK_FAILED,          /**< Failed to acquire or release lock. */
    FOSSIL_MYSHELL_ERROR_SCHEMA_MISMATCH,      /**< Schema or format mismatch between versions. */
    FOSSIL_MYSHELL_ERROR_VERSION_UNSUPPORTED,  /**< Database created with unsupported version. */
    FOSSIL_MYSHELL_ERROR_INDEX_CORRUPTED,      /**< Index structure corrupted or unreadable. */
    FOSSIL_MYSHELL_ERROR_INTEGRITY,            /**< Data integrity check failed (hash mismatch). */
    FOSSIL_MYSHELL_ERROR_TRANSACTION_FAILED,   /**< Transaction aborted or rolled back. */
    FOSSIL_MYSHELL_ERROR_CAPACITY_EXCEEDED,    /**< Reached maximum size or record capacity. */
    FOSSIL_MYSHELL_ERROR_CONFIG_INVALID,       /**< Invalid configuration or options. */
    FOSSIL_MYSHELL_ERROR_UNKNOWN               /**< Unknown or unspecified error occurred. */
} fossil_bluecrab_myshell_error_t;

/**
 * -------------------------------
 * Simple, Git-like Public API
 * -------------------------------
 * Core database handle structure for MyShell. This struct encapsulates
 * all metadata, state, and resources required for database operations,
 * including file management, branch/commit tracking, error state, and
 * optional caching and locking mechanisms.
 */
typedef struct fossil_bluecrab_myshell_t {
    char    *path;                /**< Path to the database file. */
    int      flags;               /**< Flags for database options/state. */
    FILE    *file;                /**< File handle for the database. */
    size_t   file_size;           /**< Cached file size for quick access. */
    time_t   last_modified;       /**< Last modified timestamp of the file. */
    char    *branch;              /**< Current branch name. */
    uint64_t commit_head;         /**< Current commit head hash. */
    bool     is_open;             /**< Indicates if the DB is currently open. */
    void    *cache;               /**< Pointer to cache structure (if any). */
    void    *lock;                /**< Pointer to lock/mutex (if any). */
    int      error_code;          /**< Last error code encountered. */

    /* Git-like chain fields for commit/branch management */
    uint64_t prev_commit_hash;    /**< Previous commit hash (for chain). */
    uint64_t next_commit_hash;    /**< Next commit hash (for chain, if applicable). */
    char    *author;              /**< Author of the commit. */
    char    *commit_message;      /**< Commit message. */
    time_t   commit_timestamp;    /**< Commit timestamp. */
    char    *parent_branch;       /**< Parent branch name (if any). */
    uint64_t merge_commit_hash;   /**< Merge commit hash (if merge). */
} fossil_bluecrab_myshell_t;

/**
 * o-Open/create/close
 * Opens an existing database file, creates a new database file, or closes a database handle.
 * Time Complexity: O(1) for handle allocation, O(n) for file scan (n = file size).
 * @param path Path to the database file.
 * @param err Output parameter for error code.
 * @return Pointer to fossil_bluecrab_myshell_t database handle, or NULL on failure.
 */
fossil_bluecrab_myshell_t *fossil_myshell_open(const char *path, fossil_bluecrab_myshell_error_t *err);

/**
 * o-Create
 * Creates a new database file at the specified path.
 * Time Complexity: O(1) for file creation.
 * @param path Path to the new database file.
 * @param err Output parameter for error code.
 * @return Pointer to fossil_bluecrab_myshell_t database handle, or NULL on failure.
 */
fossil_bluecrab_myshell_t *fossil_myshell_create(const char *path, fossil_bluecrab_myshell_error_t *err);

/**
 * o-Close
 * Closes the database handle and releases resources.
 * Time Complexity: O(1).
 * @param db Pointer to fossil_bluecrab_myshell_t database handle.
 */
void fossil_myshell_close(fossil_bluecrab_myshell_t *db);

/**
 * o-Record CRUD (key/value, git-like chain)
 * Inserts or updates a key/value record in the database.
 * Time Complexity: O(1) for append, O(n) for update (n = number of records).
 * @param db Database handle.
 * @param key Key string.
 * @param value Value string.
 * @return Error code.
 */
fossil_bluecrab_myshell_error_t fossil_myshell_put(fossil_bluecrab_myshell_t *db, const char *key, const char *value);

/**
 * o-Record CRUD (key/value, git-like chain)
 * Retrieves the value for a given key from the database.
 * Time Complexity: O(n) (n = number of records).
 * @param db Database handle.
 * @param key Key string.
 * @param out_value Output buffer for value.
 * @param out_size Size of output buffer.
 * @return Error code.
 */
fossil_bluecrab_myshell_error_t fossil_myshell_get(fossil_bluecrab_myshell_t *db, const char *key, char *out_value, size_t out_size);

/**
 * o-Record CRUD (key/value, git-like chain)
 * Deletes a key/value record from the database.
 * Time Complexity: O(n) (n = number of records).
 * @param db Database handle.
 * @param key Key string.
 * @return Error code.
 */
fossil_bluecrab_myshell_error_t fossil_myshell_del(fossil_bluecrab_myshell_t *db, const char *key);

/**
 * o-Commit/branch
 * Commits the current changes to the database with a message.
 * Time Complexity: O(1) for metadata update.
 * @param db Database handle.
 * @param message Commit message.
 * @return Error code.
 */
fossil_bluecrab_myshell_error_t fossil_myshell_commit(fossil_bluecrab_myshell_t *db, const char *message);

/**
 * o-Commit/branch
 * Creates a new branch in the database.
 * Time Complexity: O(1).
 * @param db Database handle.
 * @param branch_name Name of the new branch.
 * @return Error code.
 */
fossil_bluecrab_myshell_error_t fossil_myshell_branch(fossil_bluecrab_myshell_t *db, const char *branch_name);

/**
 * o-Commit/branch
 * Checks out a branch or commit in the database.
 * Time Complexity: O(1) for branch, O(n) for commit scan (n = number of commits).
 * @param db Database handle.
 * @param branch_or_commit Branch name or commit hash.
 * @return Error code.
 */
fossil_bluecrab_myshell_error_t fossil_myshell_checkout(fossil_bluecrab_myshell_t *db, const char *branch_or_commit);

/**
 * o-History iteration
 * Callback type for commit log iteration.
 * Time Complexity: O(1) per callback.
 * @param commit_hash Commit hash string.
 * @param message Commit message string.
 * @param user User data pointer.
 * @return True to continue iteration, false to stop.
 */
typedef bool (*fossil_myshell_commit_cb)(const char *commit_hash, const char *message, void *user);

/**
 * o-History iteration
 * Iterates over the commit log, invoking the callback for each commit.
 * Time Complexity: O(n) (n = number of commits).
 * @param db Database handle.
 * @param cb Callback function.
 * @param user User data pointer.
 * @return Error code.
 */
fossil_bluecrab_myshell_error_t fossil_myshell_log(fossil_bluecrab_myshell_t *db, fossil_myshell_commit_cb cb, void *user);

/**
 * o-Backup/restore
 * Creates a backup of the database file.
 * Time Complexity: O(n) (n = file size).
 * @param db Database handle.
 * @param backup_path Path to backup file.
 * @return Error code.
 */
fossil_bluecrab_myshell_error_t fossil_myshell_backup(fossil_bluecrab_myshell_t *db, const char *backup_path);

/**
 * o-Backup/restore
 * Restores a database file from a backup.
 * Time Complexity: O(n) (n = file size).
 * @param backup_path Path to backup file.
 * @param target_path Path to restore target file.
 * @return Error code.
 */
fossil_bluecrab_myshell_error_t fossil_myshell_restore(const char *backup_path, const char *target_path);

/**
 * o-Utility
 * Converts an error code to a human-readable string.
 * Time Complexity: O(1).
 * @param err Error code.
 * @return Error string.
 */
const char *fossil_myshell_errstr(fossil_bluecrab_myshell_error_t err);

/**
 * Validates database integrity (hash chain, file size, corruption).
 * Time Complexity: O(n) (n = number of records/commits).
 * @param db Database handle.
 * @return Error code.
 */
fossil_bluecrab_myshell_error_t fossil_myshell_check_integrity(fossil_bluecrab_myshell_t *db);

#ifdef __cplusplus
}
#include <string>

namespace fossil {

    namespace bluecrab {

        /**
         * o-MyShell C++ RAII Wrapper for Database Operations
         *
         * o-Overview:
         *   - Provides a modern C++ interface for MyShell database management.
         *   - Encapsulates resource management, error handling, and exposes high-level methods for CRUD,
         *     commit/branch, backup/restore, and utility functions, mapping directly to the underlying C API.
         *
         * o-Resource Management:
         *   - Non-copyable, movable. Ensures single ownership of database handle.
         *   - Destructor and close() guarantee resource release.
         *
         * o-Error Handling:
         *   - All operations return fossil_bluecrab_myshell_error_t error codes.
         *   - Use errstr() to obtain error descriptions.
         *
         * o-Thread Safety:
         *   - Not thread-safe. External synchronization required for concurrent access.
         */
        class MyShell {
        public:
            /**
             * o-Non-copyable, movable
             *   - Prevents copying, allows move semantics for single ownership.
             */
            MyShell(const MyShell&) = delete;
            MyShell& operator=(const MyShell&) = delete;
            MyShell(MyShell&& other) noexcept : db_(other.db_) { other.db_ = nullptr; }
            MyShell& operator=(MyShell&& other) noexcept {
            if (this != &other) {
                close();
                db_ = other.db_;
                other.db_ = nullptr;
            }
            return *this;
            }

            /**
             * o-Open
             *   - Opens an existing database file.
             *   - Time Complexity: O(1) for handle allocation, O(n) for file scan (n = file size).
             * @param path Path to the database file.
             * @param[out] err Output error code.
             */
            explicit MyShell(const std::string& path, fossil_bluecrab_myshell_error_t& err) {
            db_ = fossil_myshell_open(path.c_str(), &err);
            }

            /**
             * o-Create
             *   - Creates a new database file.
             *   - Time Complexity: O(1) for file creation.
             * @param path Path to the new database file.
             * @param[out] err Output error code.
             */
            static MyShell create(const std::string& path, fossil_bluecrab_myshell_error_t& err) {
            MyShell shell;
            shell.db_ = fossil_myshell_create(path.c_str(), &err);
            return shell;
            }

            /**
             * o-Close
             *   - Closes the database handle and releases resources.
             *   - Time Complexity: O(1).
             */
            ~MyShell() { close(); }

            /**
             * o-Close
             *   - Explicitly closes the database handle.
             */
            void close() {
            if (db_) {
                fossil_myshell_close(db_);
                db_ = nullptr;
            }
            }

            /**
             * o-Record CRUD (put)
             *   - Inserts or updates a key/value record in the database.
             *   - Time Complexity: O(1) for append, O(n) for update (n = number of records).
             * @param key Key string.
             * @param value Value string.
             * @return Error code.
             */
            fossil_bluecrab_myshell_error_t put(const std::string& key, const std::string& value) {
                return fossil_myshell_put(db_, key.c_str(), value.c_str());
            }

            /**
             * o-Record CRUD (get)
             *   - Retrieves the value for a given key from the database.
             *   - Time Complexity: O(n) (n = number of records).
             * @param key Key string.
             * @param out_value Output string for value.
             * @return Error code.
             */
            fossil_bluecrab_myshell_error_t get(const std::string& key, std::string& out_value) {
                char buffer[4096] = {0};
                fossil_bluecrab_myshell_error_t err = fossil_myshell_get(db_, key.c_str(), buffer, sizeof(buffer));
                if (err == FOSSIL_MYSHELL_ERROR_SUCCESS) {
                    out_value = buffer;
                }
                return err;
            }

            /**
             * o-Record CRUD (del)
             *   - Deletes a key/value record from the database.
             *   - Time Complexity: O(n) (n = number of records).
             * @param key Key string.
             * @return Error code.
             */
            fossil_bluecrab_myshell_error_t del(const std::string& key) {
                return fossil_myshell_del(db_, key.c_str());
            }

            /**
             * o-Commit
             *   - Commits the current changes to the database with a message.
             *   - Time Complexity: O(1) for metadata update.
             * @param message Commit message.
             * @return Error code.
             */
            fossil_bluecrab_myshell_error_t commit(const std::string& message) {
                return fossil_myshell_commit(db_, message.c_str());
            }

            /**
             * o-Branch
             *   - Creates a new branch in the database.
             *   - Time Complexity: O(1).
             * @param branch_name Name of the new branch.
             * @return Error code.
             */
            fossil_bluecrab_myshell_error_t branch(const std::string& branch_name) {
                return fossil_myshell_branch(db_, branch_name.c_str());
            }

            /**
             * o-Checkout
             *   - Checks out a branch or commit in the database.
             *   - Time Complexity: O(1) for branch, O(n) for commit scan (n = number of commits).
             * @param branch_or_commit Branch name or commit hash.
             * @return Error code.
             */
            fossil_bluecrab_myshell_error_t checkout(const std::string& branch_or_commit) {
                return fossil_myshell_checkout(db_, branch_or_commit.c_str());
            }

            /**
             * o-History iteration (log)
             *   - Iterates over the commit log, invoking the callback for each commit.
             *   - Time Complexity: O(n) (n = number of commits).
             * @param cb Callback function.
             * @param user User data pointer.
             * @return Error code.
             */
            fossil_bluecrab_myshell_error_t log(fossil_myshell_commit_cb cb, void* user) {
                return fossil_myshell_log(db_, cb, user);
            }

            /**
             * o-Backup
             *   - Creates a backup of the database file.
             *   - Time Complexity: O(n) (n = file size).
             * @param backup_path Path to backup file.
             * @return Error code.
             */
            fossil_bluecrab_myshell_error_t backup(const std::string& backup_path) {
                return fossil_myshell_backup(db_, backup_path.c_str());
            }

            /**
             * o-Restore
             *   - Restores a database file from a backup.
             *   - Time Complexity: O(n) (n = file size).
             *   - Static utility, does not require open handle.
             * @param backup_path Path to backup file.
             * @param target_path Path to restore target file.
             * @return Error code.
             */
            static fossil_bluecrab_myshell_error_t restore(const std::string& backup_path, const std::string& target_path) {
                return fossil_myshell_restore(backup_path.c_str(), target_path.c_str());
            }

            /**
             * o-Utility (errstr)
             *   - Converts an error code to a human-readable string.
             *   - Time Complexity: O(1).
             * @param err Error code.
             * @return Error string.
             */
            static const char* errstr(fossil_bluecrab_myshell_error_t err) {
                return fossil_myshell_errstr(err);
            }

            /**
             * o-Utility (check_integrity)
             *   - Validates database integrity (hash chain, file size, corruption).
             *   - Time Complexity: O(n) (n = number of records/commits).
             * @return Error code.
             */
            fossil_bluecrab_myshell_error_t check_integrity() {
                return fossil_myshell_check_integrity(db_);
            }

            /**
             * o-Utility (is_open)
             *   - Returns true if the database is open.
             */
            bool is_open() const { return db_ != nullptr; }

            /**
             * o-Utility (handle)
             *   - Returns the underlying C handle for advanced operations.
             */
            fossil_bluecrab_myshell_t* handle() const { return db_; }

        private:
            /**
             * o-Private default constructor
             *   - Used internally for static create().
             */
            MyShell() : db_(nullptr) {}
            fossil_bluecrab_myshell_t* db_;
        };

    } // namespace bluecrab

} // namespace fossil

#endif

#endif /* FOSSIL_CRABDB_FRAMEWORK_H */

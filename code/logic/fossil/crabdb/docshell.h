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
#ifndef FOSSIL_CRABDB_CACHESHELL_H
#define FOSSIL_CRABDB_CACHESHELL_H

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
// Type definitions
// *****************************************************************************

typedef struct fossil_bluecrab_docshell_t fossil_bluecrab_docshell_t;
typedef struct fossil_bluecrab_document_t fossil_bluecrab_document_t;

// Error codes
enum {
    FOSSIL_BLUECRAB_DOCSHELL_SUCCESS = 0,
    FOSSIL_BLUECRAB_DOCSHELL_FAILURE = -1,
    FOSSIL_BLUECRAB_DOCSHELL_ERROR_INVALID_ARGUMENT = -2,
    FOSSIL_BLUECRAB_DOCSHELL_ERROR_MEMORY_ALLOCATION = -3,
    FOSSIL_BLUECRAB_DOCSHELL_ERROR_NOT_FOUND = -4,
    FOSSIL_BLUECRAB_DOCSHELL_ERROR_IO = -5,
};

// Document structure (opaque key-value object)
struct fossil_bluecrab_document_t {
    char *id;       // Unique document ID
    char *data;     // JSON/FSON text payload
};

// *****************************************************************************
// DocShell API - Prototypes
// *****************************************************************************

// *****************************************************************************
// Lifecycle functions
// *****************************************************************************

/**
 * Create a new document store in memory.
 */
fossil_bluecrab_docshell_t *fossil_bluecrab_docshell_create(void);

/**
 * Open an existing document store from disk (if persistence is enabled).
 */
fossil_bluecrab_docshell_t *fossil_bluecrab_docshell_open(const char *path);

/**
 * Close the document store, freeing resources.
 */
int fossil_bluecrab_docshell_close(fossil_bluecrab_docshell_t *store);

// *****************************************************************************
// CRUD operations
// *****************************************************************************

/**
 * Insert a document. If `id` is NULL, an ID is generated automatically.
 */
int fossil_bluecrab_docshell_insert(fossil_bluecrab_docshell_t *store,
                                    const char *id,
                                    const char *json_or_fson);

/**
 * Find a document by ID. Returns NULL if not found.
 */
const fossil_bluecrab_document_t *fossil_bluecrab_docshell_find(
    fossil_bluecrab_docshell_t *store,
    const char *id);

/**
 * Update an existing document by ID.
 */
int fossil_bluecrab_docshell_update(fossil_bluecrab_docshell_t *store,
                                    const char *id,
                                    const char *json_or_fson);

/**
 * Patch a document (merge fields instead of replacing entire doc).
 */
int fossil_bluecrab_docshell_patch(fossil_bluecrab_docshell_t *store,
                                   const char *id,
                                   const char *json_patch);

/**
 * Delete a document by ID.
 */
int fossil_bluecrab_docshell_delete(fossil_bluecrab_docshell_t *store,
                                    const char *id);

// *****************************************************************************
// Query and Iteration
// *****************************************************************************

/**
 * Query documents by field match (very simple key/value filter).
 */
int fossil_bluecrab_docshell_query(fossil_bluecrab_docshell_t *store,
                                   const char *field,
                                   const char *value,
                                   fossil_bluecrab_document_t ***results,
                                   size_t *count);

/**
 * Iterate over all documents (returns NULL when done).
 */
const fossil_bluecrab_document_t *fossil_bluecrab_docshell_next(
    fossil_bluecrab_docshell_t *store,
    void **iterator_state);

// *****************************************************************************
// Backup & Restore
// *****************************************************************************

/**
 * Backup the entire document store to a file.
 */
int fossil_bluecrab_docshell_backup(fossil_bluecrab_docshell_t *store,
                                    const char *backup_path);

/**
 * Restore a document store from a backup file.
 */
int fossil_bluecrab_docshell_restore(fossil_bluecrab_docshell_t *store,
                                     const char *backup_path);

// *****************************************************************************
// Memory Management
// *****************************************************************************

/**
 * Free memory allocated for a query result set.
 */
void fossil_bluecrab_docshell_free_results(fossil_bluecrab_document_t **results,
                                           size_t count);

#ifdef __cplusplus
}
#include <string>
#include <vector>

namespace fossil {
    namespace bluecrab {

        /**
         * @brief C++ wrapper class for the DocShell C API.
         *
         * Provides static methods for interacting with the document store using
         * std::string and C++ containers, mapping directly to the underlying C functions.
         */
        class DocShell {
        public:
            /**
             * @brief Creates a new in-memory document store.
             *
             * @return  A pointer to the newly created document store.
             */
            static fossil_bluecrab_docshell_t* create() {
                return fossil_bluecrab_docshell_create();
            }

            /**
             * @brief Opens a document store from disk.
             *
             * @param path Path to the persistent storage file.
             * @return     Pointer to the opened document store.
             */
            static fossil_bluecrab_docshell_t* open(const std::string& path) {
                return fossil_bluecrab_docshell_open(path.c_str());
            }

            /**
             * @brief Closes the document store and frees resources.
             *
             * @param store Pointer to the store.
             * @return      true on success, false on error.
             */
            static bool close(fossil_bluecrab_docshell_t* store) {
                return fossil_bluecrab_docshell_close(store) == FOSSIL_BLUECRAB_DOCSHELL_SUCCESS;
            }

            /**
             * @brief Inserts a new document into the store.
             *
             * @param store        Pointer to the store.
             * @param id           Document ID (optional, pass empty string to auto-generate).
             * @param json_or_fson Document payload.
             * @return             true on success, false on failure.
             */
            static bool insert(fossil_bluecrab_docshell_t* store,
                               const std::string& id,
                               const std::string& json_or_fson)
            {
                const char* id_ptr = id.empty() ? nullptr : id.c_str();
                return fossil_bluecrab_docshell_insert(store, id_ptr, json_or_fson.c_str())
                       == FOSSIL_BLUECRAB_DOCSHELL_SUCCESS;
            }

            /**
             * @brief Finds a document by ID.
             *
             * @param store Pointer to the store.
             * @param id    Document ID.
             * @param out   Output string to hold the document data.
             * @return      true if found, false if not found.
             */
            static bool find(fossil_bluecrab_docshell_t* store,
                             const std::string& id,
                             std::string& out)
            {
                const fossil_bluecrab_document_t* doc =
                    fossil_bluecrab_docshell_find(store, id.c_str());
                if (!doc) return false;
                out.assign(doc->data);
                return true;
            }

            /**
             * @brief Updates a document by replacing its data.
             */
            static bool update(fossil_bluecrab_docshell_t* store,
                               const std::string& id,
                               const std::string& json_or_fson)
            {
                return fossil_bluecrab_docshell_update(store, id.c_str(), json_or_fson.c_str())
                       == FOSSIL_BLUECRAB_DOCSHELL_SUCCESS;
            }

            /**
             * @brief Applies a patch to a document.
             *
             * @param store      Pointer to store.
             * @param id         Document ID.
             * @param json_patch Patch data in JSON or FSON.
             * @return           true on success, false on failure.
             */
            static bool patch(fossil_bluecrab_docshell_t* store,
                              const std::string& id,
                              const std::string& json_patch)
            {
                return fossil_bluecrab_docshell_patch(store, id.c_str(), json_patch.c_str())
                       == FOSSIL_BLUECRAB_DOCSHELL_SUCCESS;
            }

            /**
             * @brief Deletes a document by ID.
             */
            static bool remove(fossil_bluecrab_docshell_t* store, const std::string& id) {
                return fossil_bluecrab_docshell_delete(store, id.c_str())
                       == FOSSIL_BLUECRAB_DOCSHELL_SUCCESS;
            }

            /**
             * @brief Queries documents by field/value match.
             *
             * @param store  Pointer to store.
             * @param field  Field name to match.
             * @param value  Expected value.
             * @return       Vector of document strings (payloads). Empty if none found.
             */
            static std::vector<std::string> query(fossil_bluecrab_docshell_t* store,
                                                 const std::string& field,
                                                 const std::string& value)
            {
                fossil_bluecrab_document_t** results = nullptr;
                size_t count = 0;
                std::vector<std::string> docs;
                if (fossil_bluecrab_docshell_query(store, field.c_str(), value.c_str(), &results, &count)
                    == FOSSIL_BLUECRAB_DOCSHELL_SUCCESS)
                {
                    docs.reserve(count);
                    for (size_t i = 0; i < count; ++i) {
                        docs.emplace_back(results[i]->data);
                    }
                    fossil_bluecrab_docshell_free_results(results, count);
                }
                return docs;
            }

            /**
             * @brief Iterates over all documents in the store.
             *
             * @param store Pointer to store.
             * @return      Vector of all document payloads.
             */
            static std::vector<std::string> all(fossil_bluecrab_docshell_t* store) {
                std::vector<std::string> docs;
                void* iter = nullptr;
                const fossil_bluecrab_document_t* doc;
                while ((doc = fossil_bluecrab_docshell_next(store, &iter)) != nullptr) {
                    docs.emplace_back(doc->data);
                }
                free(iter);
                return docs;
            }

            /**
             * @brief Backs up the entire document store.
             */
            static bool backup(fossil_bluecrab_docshell_t* store, const std::string& path) {
                return fossil_bluecrab_docshell_backup(store, path.c_str())
                       == FOSSIL_BLUECRAB_DOCSHELL_SUCCESS;
            }

            /**
             * @brief Restores a document store from a backup.
             */
            static bool restore(fossil_bluecrab_docshell_t* store, const std::string& path) {
                return fossil_bluecrab_docshell_restore(store, path.c_str())
                       == FOSSIL_BLUECRAB_DOCSHELL_SUCCESS;
            }
        };

    } // namespace bluecrab

} // namespace fossil

#endif

#endif /* FOSSIL_CRABDB_FRAMEWORK_H */

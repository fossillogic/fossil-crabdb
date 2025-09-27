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
#include "fossil/crabdb/docshell.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

// *****************************************************************************
// Internal structures
// *****************************************************************************

struct fossil_bluecrab_docshell_t {
    fossil_bluecrab_document_t **documents;
    size_t count;
    size_t capacity;
    char *storage_path; // optional, for persistence
};

// *****************************************************************************
// Internal helpers
// *****************************************************************************

static fossil_bluecrab_document_t *docshell_find_internal(
    fossil_bluecrab_docshell_t *store, const char *id, size_t *index_out) {
    if (!store || !id) return NULL;
    for (size_t i = 0; i < store->count; ++i) {
        if (strcmp(store->documents[i]->id, id) == 0) {
            if (index_out) *index_out = i;
            return store->documents[i];
        }
    }
    return NULL;
}

static int docshell_ensure_capacity(fossil_bluecrab_docshell_t *store) {
    if (store->count < store->capacity) return FOSSIL_BLUECRAB_DOCSHELL_SUCCESS;
    size_t new_capacity = (store->capacity == 0) ? 8 : store->capacity * 2;
    void *new_block = realloc(store->documents, new_capacity * sizeof(*store->documents));
    if (!new_block) return FOSSIL_BLUECRAB_DOCSHELL_ERROR_MEMORY_ALLOCATION;
    store->documents = new_block;
    store->capacity = new_capacity;
    return FOSSIL_BLUECRAB_DOCSHELL_SUCCESS;
}

// *****************************************************************************
// Lifecycle
// *****************************************************************************

fossil_bluecrab_docshell_t *fossil_bluecrab_docshell_create(void) {
    fossil_bluecrab_docshell_t *store = calloc(1, sizeof(*store));
    if (!store) return NULL;
    return store;
}

fossil_bluecrab_docshell_t *fossil_bluecrab_docshell_open(const char *path) {
    fossil_bluecrab_docshell_t *store = fossil_bluecrab_docshell_create();
    if (!store) return NULL;
    if (path) {
        store->storage_path = strdup(path);
        // TODO: load existing documents from file
    }
    return store;
}

int fossil_bluecrab_docshell_close(fossil_bluecrab_docshell_t *store) {
    if (!store) return FOSSIL_BLUECRAB_DOCSHELL_ERROR_INVALID_ARGUMENT;
    for (size_t i = 0; i < store->count; ++i) {
        free(store->documents[i]->id);
        free(store->documents[i]->data);
        free(store->documents[i]);
    }
    free(store->documents);
    free(store->storage_path);
    free(store);
    return FOSSIL_BLUECRAB_DOCSHELL_SUCCESS;
}

// *****************************************************************************
// CRUD
// *****************************************************************************

int fossil_bluecrab_docshell_insert(fossil_bluecrab_docshell_t *store,
                                    const char *id,
                                    const char *json_or_fson) {
    if (!store || !json_or_fson)
        return FOSSIL_BLUECRAB_DOCSHELL_ERROR_INVALID_ARGUMENT;

    if (id && docshell_find_internal(store, id, NULL))
        return FOSSIL_BLUECRAB_DOCSHELL_FAILURE; // duplicate ID

    if (docshell_ensure_capacity(store) != FOSSIL_BLUECRAB_DOCSHELL_SUCCESS)
        return FOSSIL_BLUECRAB_DOCSHELL_ERROR_MEMORY_ALLOCATION;

    fossil_bluecrab_document_t *doc = calloc(1, sizeof(*doc));
    if (!doc) return FOSSIL_BLUECRAB_DOCSHELL_ERROR_MEMORY_ALLOCATION;

    doc->id = id ? strdup(id) : NULL; // TODO: auto-generate ID if NULL
    doc->data = strdup(json_or_fson);
    store->documents[store->count++] = doc;
    return FOSSIL_BLUECRAB_DOCSHELL_SUCCESS;
}

const fossil_bluecrab_document_t *fossil_bluecrab_docshell_find(
    fossil_bluecrab_docshell_t *store,
    const char *id) {
    return docshell_find_internal(store, id, NULL);
}

int fossil_bluecrab_docshell_update(fossil_bluecrab_docshell_t *store,
                                    const char *id,
                                    const char *json_or_fson) {
    size_t idx;
    fossil_bluecrab_document_t *doc = docshell_find_internal(store, id, &idx);
    if (!doc) return FOSSIL_BLUECRAB_DOCSHELL_ERROR_NOT_FOUND;

    char *new_data = strdup(json_or_fson);
    if (!new_data) return FOSSIL_BLUECRAB_DOCSHELL_ERROR_MEMORY_ALLOCATION;

    free(doc->data);
    doc->data = new_data;
    return FOSSIL_BLUECRAB_DOCSHELL_SUCCESS;
}

int fossil_bluecrab_docshell_patch(fossil_bluecrab_docshell_t *store,
                                   const char *id,
                                   const char *json_patch) {
    // TODO: implement minimal merge or JSON Patch application
    (void)store; (void)id; (void)json_patch;
    return FOSSIL_BLUECRAB_DOCSHELL_FAILURE;
}

int fossil_bluecrab_docshell_delete(fossil_bluecrab_docshell_t *store,
                                    const char *id) {
    size_t idx;
    fossil_bluecrab_document_t *doc = docshell_find_internal(store, id, &idx);
    if (!doc) return FOSSIL_BLUECRAB_DOCSHELL_ERROR_NOT_FOUND;

    free(doc->id);
    free(doc->data);
    free(doc);

    // shift remaining documents left
    for (size_t i = idx; i < store->count - 1; ++i)
        store->documents[i] = store->documents[i + 1];

    store->count--;
    return FOSSIL_BLUECRAB_DOCSHELL_SUCCESS;
}

// *****************************************************************************
// Query & Iteration
// *****************************************************************************

int fossil_bluecrab_docshell_query(fossil_bluecrab_docshell_t *store,
                                   const char *field,
                                   const char *value,
                                   fossil_bluecrab_document_t ***results,
                                   size_t *count) {
    // TODO: naive scan for now, parse `data` as JSON/FSON and match field
    (void)field; (void)value;
    *results = NULL;
    *count = 0;
    return FOSSIL_BLUECRAB_DOCSHELL_FAILURE;
}

const fossil_bluecrab_document_t *fossil_bluecrab_docshell_next(
    fossil_bluecrab_docshell_t *store,
    void **iterator_state) {
    if (!store || !iterator_state) return NULL;
    size_t *index = (size_t *)(*iterator_state);
    if (!index) {
        index = calloc(1, sizeof(size_t));
        if (!index) return NULL;
        *iterator_state = index;
    }
    if (*index >= store->count) return NULL;
    return store->documents[(*index)++];
}

// *****************************************************************************
// Backup & Restore
// *****************************************************************************

int fossil_bluecrab_docshell_backup(fossil_bluecrab_docshell_t *store,
                                    const char *backup_path) {
    if (!store || !backup_path)
        return FOSSIL_BLUECRAB_DOCSHELL_ERROR_INVALID_ARGUMENT;

    FILE *f = fopen(backup_path, "w");
    if (!f) return FOSSIL_BLUECRAB_DOCSHELL_ERROR_IO;

    for (size_t i = 0; i < store->count; ++i) {
        fprintf(f, "%s:%s\n", store->documents[i]->id, store->documents[i]->data);
    }
    fclose(f);
    return FOSSIL_BLUECRAB_DOCSHELL_SUCCESS;
}

int fossil_bluecrab_docshell_restore(fossil_bluecrab_docshell_t *store,
                                     const char *backup_path) {
    if (!store || !backup_path)
        return FOSSIL_BLUECRAB_DOCSHELL_ERROR_INVALID_ARGUMENT;

    FILE *f = fopen(backup_path, "r");
    if (!f) return FOSSIL_BLUECRAB_DOCSHELL_ERROR_IO;

    char line[4096];
    while (fgets(line, sizeof(line), f)) {
        char *sep = strchr(line, ':');
        if (!sep) continue;
        *sep = '\0';
        char *id = line;
        char *data = sep + 1;
        data[strcspn(data, "\n")] = '\0';
        fossil_bluecrab_docshell_insert(store, id, data);
    }
    fclose(f);
    return FOSSIL_BLUECRAB_DOCSHELL_SUCCESS;
}

// *****************************************************************************
// Memory Management
// *****************************************************************************

void fossil_bluecrab_docshell_free_results(fossil_bluecrab_document_t **results,
                                           size_t count) {
    if (!results) return;
    for (size_t i = 0; i < count; ++i) {
        free(results[i]); // assume results are copies
    }
    free(results);
}

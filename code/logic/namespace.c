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
#include "fossil/crabdb/internal.h"
#include "fossil/crabdb/namespace.h"

#include <string.h>
#include <stdlib.h>

/**
 * @brief Create a new namespace.
 * 
 * @param db Pointer to the fossil_crabdb_t database.
 * @param namespace_name Name of the new namespace.
 * @return Error code indicating the result of the operation.
 */
fossil_crabdb_error_t fossil_crabdb_create_namespace(fossil_crabdb_t *db, const char *namespace_name) {
    if (!db || !namespace_name) return CRABDB_ERR_MEM;

    fossil_crabdb_namespace_t *current = db->namespaces;
    while (current) {
        if (strcmp(current->name, namespace_name) == 0) {
            return CRABDB_ERR_NS_EXISTS;
        }
        current = current->next;
    }

    fossil_crabdb_namespace_t *new_namespace = fossil_crabdb_alloc(sizeof(fossil_crabdb_namespace_t));
    if (!new_namespace) return CRABDB_ERR_MEM;

    new_namespace->name = fossil_crabdb_strdup(namespace_name);
    new_namespace->sub_namespaces = NULL;
    new_namespace->sub_namespace_count = 0;
    new_namespace->next = db->namespaces;
    new_namespace->data = NULL;
    db->namespaces = new_namespace;

    return CRABDB_OK;
} // end of fun

/**
 * @brief Create a new sub-namespace.
 * 
 * @param db Pointer to the fossil_crabdb_t database.
 * @param namespace_name Name of the parent namespace.
 * @param sub_namespace_name Name of the new sub-namespace.
 * @return Error code indicating the result of the operation.
 */
fossil_crabdb_error_t fossil_crabdb_create_sub_namespace(fossil_crabdb_t *db, const char *namespace_name, const char *sub_namespace_name) {
    if (!db || !namespace_name || !sub_namespace_name) return CRABDB_ERR_MEM;

    fossil_crabdb_namespace_t *current = db->namespaces;
    while (current) {
        if (strcmp(current->name, namespace_name) == 0) {
            for (size_t i = 0; i < current->sub_namespace_count; i++) {
                if (strcmp(current->sub_namespaces[i].name, sub_namespace_name) == 0) {
                    return CRABDB_ERR_SUB_NS_EXISTS;
                }
            }

            current->sub_namespaces = (fossil_crabdb_namespace_t*)fossil_crabdb_realloc(current->sub_namespaces, sizeof(fossil_crabdb_namespace_t) * (current->sub_namespace_count + 1));
            if (!current->sub_namespaces) return CRABDB_ERR_MEM;

            current->sub_namespaces[current->sub_namespace_count].name = fossil_crabdb_strdup(sub_namespace_name);
            current->sub_namespaces[current->sub_namespace_count].sub_namespaces = NULL;
            current->sub_namespaces[current->sub_namespace_count].sub_namespace_count = 0;
            current->sub_namespaces[current->sub_namespace_count].next = NULL;
            current->sub_namespaces[current->sub_namespace_count].data = NULL;

            current->sub_namespace_count++;
            return CRABDB_OK;
        }
        current = current->next;
    }

    return CRABDB_ERR_NS_NOT_FOUND;
} // end of fun

/**
 * @brief Erase a namespace.
 * 
 * @param db Pointer to the fossil_crabdb_t database.
 * @param namespace_name Name of the namespace to erase.
 * @return Error code indicating the result of the operation.
 */
fossil_crabdb_error_t fossil_crabdb_erase_namespace(fossil_crabdb_t *db, const char *namespace_name) {
    if (!db || !namespace_name) return CRABDB_ERR_MEM;

    fossil_crabdb_namespace_t *prev = NULL;
    fossil_crabdb_namespace_t *current = db->namespaces;

    while (current) {
        if (strcmp(current->name, namespace_name) == 0) {
            if (prev) {
                prev->next = current->next;
            } else {
                db->namespaces = current->next;
            }

            fossil_crabdb_free(current->name);

            for (size_t i = 0; i < current->sub_namespace_count; i++) {
                fossil_crabdb_free(current->sub_namespaces[i].name);
            }
            fossil_crabdb_free(current->sub_namespaces);

            fossil_crabdb_keyvalue_t *kv = current->data;
            while (kv) {
                fossil_crabdb_keyvalue_t *kv_next = kv->next;
                fossil_crabdb_free(kv->key);
                fossil_crabdb_free(kv->value);
                fossil_crabdb_free(kv);
                kv = kv_next;
            }

            fossil_crabdb_free(current);
            return CRABDB_OK;
        }
        prev = current;
        current = current->next;
    }

    return CRABDB_ERR_NS_NOT_FOUND;
} // end of fun

/**
 * @brief Erase a sub-namespace.
 * 
 * @param db Pointer to the fossil_crabdb_t database.
 * @param namespace_name Name of the parent namespace.
 * @param sub_namespace_name Name of the sub-namespace to erase.
 * @return Error code indicating the result of the operation.
 */
fossil_crabdb_error_t fossil_crabdb_erase_sub_namespace(fossil_crabdb_t *db, const char *namespace_name, const char *sub_namespace_name) {
    if (!db || !namespace_name || !sub_namespace_name) return CRABDB_ERR_MEM;

    fossil_crabdb_namespace_t *current = db->namespaces;
    while (current) {
        if (strcmp(current->name, namespace_name) == 0) {
            for (size_t i = 0; i < current->sub_namespace_count; i++) {
                if (strcmp(current->sub_namespaces[i].name, sub_namespace_name) == 0) {
                    fossil_crabdb_free(current->sub_namespaces[i].name);

                    for (size_t j = i; j < current->sub_namespace_count - 1; j++) {
                        current->sub_namespaces[j] = current->sub_namespaces[j + 1];
                    }

                    current->sub_namespaces = (fossil_crabdb_namespace_t*)fossil_crabdb_realloc(current->sub_namespaces, sizeof(fossil_crabdb_namespace_t) * (current->sub_namespace_count - 1));
                    current->sub_namespace_count--;

                    return CRABDB_OK;
                }
            }
        }
        current = current->next;
    }

    return CRABDB_ERR_SUB_NS_NOT_FOUND;
} // end of fun

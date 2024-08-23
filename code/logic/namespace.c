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

/**
 * @brief Create a deep copy of a namespace.
 * 
 * @param original Pointer to the original namespace to copy.
 * @return Pointer to the copied namespace.
 */
fossil_crabdb_namespace_t* fossil_crabdb_copy_namespace(const fossil_crabdb_namespace_t *original) {
    if (!original) return NULL;

    fossil_crabdb_namespace_t *copy = fossil_crabdb_alloc(sizeof(fossil_crabdb_namespace_t));
    if (!copy) return NULL;

    copy->name = fossil_crabdb_strdup(original->name);
    copy->sub_namespace_count = original->sub_namespace_count;
    copy->sub_namespaces = (fossil_crabdb_namespace_t *)fossil_crabdb_alloc(sizeof(fossil_crabdb_namespace_t) * original->sub_namespace_count);

    for (size_t i = 0; i < original->sub_namespace_count; i++) {
        copy->sub_namespaces[i] = *fossil_crabdb_copy_namespace(&original->sub_namespaces[i]);
    }

    // Deep copy of key-value pairs
    copy->data = NULL;
    fossil_crabdb_keyvalue_t *kv_current = original->data;
    while (kv_current) {
        fossil_crabdb_keyvalue_t *kv_copy = fossil_crabdb_alloc(sizeof(fossil_crabdb_keyvalue_t));
        kv_copy->key = fossil_crabdb_strdup(kv_current->key);
        kv_copy->value = fossil_crabdb_strdup(kv_current->value);
        kv_copy->next = copy->data;
        copy->data = kv_copy;
        kv_current = kv_current->next;
    }

    return copy;
}

/**
 * @brief List all namespaces.
 * 
 * @param db Pointer to the fossil_crabdb_t database.
 * @param namespaces Pointer to store the list of namespace names.
 * @param count Pointer to store the number of namespaces.
 * @return Error code indicating the result of the operation.
 */
fossil_crabdb_error_t fossil_crabdb_list_namespaces(fossil_crabdb_t *db, char ***namespaces, size_t *count) {
    if (!db || !namespaces || !count) return CRABDB_ERR_MEM;

    size_t ns_count = 0;
    fossil_crabdb_namespace_t *current = db->namespaces;

    // First, count the number of namespaces
    while (current) {
        ns_count++;
        current = current->next;
    }

    // Allocate memory for the array of namespace names
    *namespaces = (char **)fossil_crabdb_alloc(sizeof(char *) * ns_count);
    if (!*namespaces) return CRABDB_ERR_MEM;

    // Populate the array with namespace names
    current = db->namespaces;
    for (size_t i = 0; i < ns_count; i++) {
        (*namespaces)[i] = fossil_crabdb_strdup(current->name);
        if (!(*namespaces)[i]) return CRABDB_ERR_MEM; // Handle memory allocation failure
        current = current->next;
    }

    *count = ns_count;
    return CRABDB_OK;
}

/**
 * @brief List all keys in a namespace.
 * 
 * @param db Pointer to the fossil_crabdb_t database.
 * @param namespace_name Name of the namespace.
 * @param keys Pointer to store the list of keys.
 * @param count Pointer to store the number of keys.
 * @return Error code indicating the result of the operation.
 */
fossil_crabdb_error_t fossil_crabdb_list_keys(fossil_crabdb_t *db, const char *namespace_name, char ***keys, size_t *count) {
    if (!db || !namespace_name || !keys || !count) return CRABDB_ERR_MEM;

    fossil_crabdb_namespace_t *current = db->namespaces;
    while (current) {
        if (strcmp(current->name, namespace_name) == 0) {
            size_t key_count = 0;
            fossil_crabdb_keyvalue_t *kv_current = current->data;

            // First, count the number of keys
            while (kv_current) {
                key_count++;
                kv_current = kv_current->next;
            }

            // Allocate memory for the array of keys
            *keys = (char **)fossil_crabdb_alloc(sizeof(char *) * key_count);
            if (!*keys) return CRABDB_ERR_MEM;

            // Populate the array with keys
            kv_current = current->data;
            for (size_t i = 0; i < key_count; i++) {
                (*keys)[i] = fossil_crabdb_strdup(kv_current->key);
                if (!(*keys)[i]) return CRABDB_ERR_MEM; // Handle memory allocation failure
                kv_current = kv_current->next;
            }

            *count = key_count;
            return CRABDB_OK;
        }
        current = current->next;
    }

    return CRABDB_ERR_NS_NOT_FOUND;
}

/**
 * @brief Get statistics for a namespace.
 * 
 * @param db Pointer to the fossil_crabdb_t database.
 * @param namespace_name Name of the namespace to get statistics for.
 * @param key_count Pointer to store the number of keys.
 * @param sub_namespace_count Pointer to store the number of sub-namespaces.
 * @return Error code indicating the result of the operation.
 */
fossil_crabdb_error_t fossil_crabdb_get_namespace_stats(fossil_crabdb_t *db, const char *namespace_name, size_t *key_count, size_t *sub_namespace_count) {
    if (!db || !namespace_name || !key_count || !sub_namespace_count) return CRABDB_ERR_MEM;

    fossil_crabdb_namespace_t *current = db->namespaces;
    while (current) {
        if (strcmp(current->name, namespace_name) == 0) {
            *key_count = 0;
            *sub_namespace_count = current->sub_namespace_count;

            fossil_crabdb_keyvalue_t *kv_current = current->data;
            while (kv_current) {
                (*key_count)++;
                kv_current = kv_current->next;
            }

            return CRABDB_OK;
        }
        current = current->next;
    }

    return CRABDB_ERR_NS_NOT_FOUND;
}

/**
 * @brief Rename a namespace.
 * 
 * @param db Pointer to the fossil_crabdb_t database.
 * @param old_namespace_name Current name of the namespace.
 * @param new_namespace_name New name for the namespace.
 * @return Error code indicating the result of the operation.
 */
fossil_crabdb_error_t fossil_crabdb_rename_namespace(fossil_crabdb_t *db, const char *old_namespace_name, const char *new_namespace_name) {
    if (!db || !old_namespace_name || !new_namespace_name) return CRABDB_ERR_MEM;

    fossil_crabdb_namespace_t *current = db->namespaces;
    while (current) {
        if (strcmp(current->name, old_namespace_name) == 0) {
            fossil_crabdb_free(current->name);
            current->name = fossil_crabdb_strdup(new_namespace_name);
            return CRABDB_OK;
        }
        current = current->next;
    }

    return CRABDB_ERR_NS_NOT_FOUND;
}

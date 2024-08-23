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
#ifndef FOSSIL_CRABDB_NAMESPACE_H
#define FOSSIL_CRABDB_NAMESPACE_H

#include "internal.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Create a new namespace.
 * 
 * @param db Pointer to the fossil_crabdb_t database.
 * @param namespace_name Name of the new namespace.
 * @return Error code indicating the result of the operation.
 */
fossil_crabdb_error_t fossil_crabdb_create_namespace(fossil_crabdb_t *db, const char *namespace_name);

/**
 * @brief Create a new sub-namespace.
 * 
 * @param db Pointer to the fossil_crabdb_t database.
 * @param namespace_name Name of the parent namespace.
 * @param sub_namespace_name Name of the new sub-namespace.
 * @return Error code indicating the result of the operation.
 */
fossil_crabdb_error_t fossil_crabdb_create_sub_namespace(fossil_crabdb_t *db, const char *namespace_name, const char *sub_namespace_name);

/**
 * @brief Erase a namespace.
 * 
 * @param db Pointer to the fossil_crabdb_t database.
 * @param namespace_name Name of the namespace to erase.
 * @return Error code indicating the result of the operation.
 */
fossil_crabdb_error_t fossil_crabdb_erase_namespace(fossil_crabdb_t *db, const char *namespace_name);

/**
 * @brief Erase a sub-namespace.
 * 
 * @param db Pointer to the fossil_crabdb_t database.
 * @param namespace_name Name of the parent namespace.
 * @param sub_namespace_name Name of the sub-namespace to erase.
 * @return Error code indicating the result of the operation.
 */
fossil_crabdb_error_t fossil_crabdb_erase_sub_namespace(fossil_crabdb_t *db, const char *namespace_name, const char *sub_namespace_name);

/**
 * @brief List all namespaces.
 * 
 * @param db Pointer to the fossil_crabdb_t database.
 * @param namespaces Pointer to store the list of namespace names.
 * @param count Pointer to store the number of namespaces.
 * @return Error code indicating the result of the operation.
 */
fossil_crabdb_error_t fossil_crabdb_list_namespaces(fossil_crabdb_t *db, char ***namespaces, size_t *count);

/**
 * @brief List all keys in a namespace.
 * 
 * @param db Pointer to the fossil_crabdb_t database.
 * @param namespace_name Name of the namespace.
 * @param keys Pointer to store the list of keys.
 * @param count Pointer to store the number of keys.
 * @return Error code indicating the result of the operation.
 */
fossil_crabdb_error_t fossil_crabdb_list_keys(fossil_crabdb_t *db, const char *namespace_name, char ***keys, size_t *count);

/**
 * @brief Get statistics for a namespace.
 * 
 * @param db Pointer to the fossil_crabdb_t database.
 * @param namespace_name Name of the namespace to get statistics for.
 * @param key_count Pointer to store the number of keys.
 * @param sub_namespace_count Pointer to store the number of sub-namespaces.
 * @return Error code indicating the result of the operation.
 */
fossil_crabdb_error_t fossil_crabdb_get_namespace_stats(fossil_crabdb_t *db, const char *namespace_name, size_t *key_count, size_t *sub_namespace_count);

/**
 * @brief Create a deep copy of a namespace.
 * 
 * @param original Pointer to the original namespace to copy.
 * @return Pointer to the copied namespace.
 */
fossil_crabdb_namespace_t* fossil_crabdb_copy_namespace(const fossil_crabdb_namespace_t *original);

/**
 * @brief Rename a namespace.
 * 
 * @param db Pointer to the fossil_crabdb_t database.
 * @param old_namespace_name Current name of the namespace.
 * @param new_namespace_name New name for the namespace.
 * @return Error code indicating the result of the operation.
 */
fossil_crabdb_error_t fossil_crabdb_rename_namespace(fossil_crabdb_t *db, const char *old_namespace_name, const char *new_namespace_name);

#ifdef __cplusplus
}
#endif

#ifdef __cplusplus
namespace fossil {

    /**
     * @brief Wrapper function to create a new namespace.
     * 
     * @param db Pointer to the fossil_crabdb_t database.
     * @param namespace_name Name of the new namespace.
     * @return Error code indicating the result of the operation.
     */
    fossil_crabdb_error_t create_namespace(fossil_crabdb_t *db, const char *namespace_name) {
        return fossil_crabdb_create_namespace(db, namespace_name);
    }

    /**
     * @brief Wrapper function to create a new sub-namespace.
     * 
     * @param db Pointer to the fossil_crabdb_t database.
     * @param namespace_name Name of the parent namespace.
     * @param sub_namespace_name Name of the new sub-namespace.
     * @return Error code indicating the result of the operation.
     */
    fossil_crabdb_error_t create_sub_namespace(fossil_crabdb_t *db, const char *namespace_name, const char *sub_namespace_name) {
        return fossil_crabdb_create_sub_namespace(db, namespace_name, sub_namespace_name);
    }

    /**
     * @brief Wrapper function to erase a namespace.
     * 
     * @param db Pointer to the fossil_crabdb_t database.
     * @param namespace_name Name of the namespace to erase.
     * @return Error code indicating the result of the operation.
     */
    fossil_crabdb_error_t erase_namespace(fossil_crabdb_t *db, const char *namespace_name) {
        return fossil_crabdb_erase_namespace(db, namespace_name);
    }

    /**
     * @brief Wrapper function to erase a sub-namespace.
     * 
     * @param db Pointer to the fossil_crabdb_t database.
     * @param namespace_name Name of the parent namespace.
     * @param sub_namespace_name Name of the sub-namespace to erase.
     * @return Error code indicating the result of the operation.
     */
    fossil_crabdb_error_t erase_sub_namespace(fossil_crabdb_t *db, const char *namespace_name, const char *sub_namespace_name) {
        return fossil_crabdb_erase_sub_namespace(db, namespace_name, sub_namespace_name);
    }

    class CrabNamespace {
    public:
        CrabNamespace(fossil_crabdb_t *db, const char *namespace_name) {
            fossil_crabdb_create_namespace(db, namespace_name);
        }

        fossil_crabdb_error_t create_sub_namespace(fossil_crabdb_t *db, const char *namespace_name, const char *sub_namespace_name) {
            return fossil_crabdb_create_sub_namespace(db, namespace_name, sub_namespace_name);
        }

        fossil_crabdb_error_t erase_namespace(fossil_crabdb_t *db, const char *namespace_name) {
            return fossil_crabdb_erase_namespace(db, namespace_name);
        }

        fossil_crabdb_error_t erase_sub_namespace(fossil_crabdb_t *db, const char *namespace_name, const char *sub_namespace_name) {
            return fossil_crabdb_erase_sub_namespace(db, namespace_name, sub_namespace_name);
        }
    };

}

#endif

#endif /* FOSSIL_CRABDB_FRAMEWORK_H */

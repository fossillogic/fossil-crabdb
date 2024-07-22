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

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
#ifndef FOSSIL_CRABDB_STORAGE_H
#define FOSSIL_CRABDB_STORAGE_H

#include "database.h"

#ifdef __cplusplus
extern "C" {
#endif

// *****************************************************************************
// INI Storage API
// *****************************************************************************

/**
 * @brief Saves the database content to an INI file.
 *
 * @param book          Pointer to the database (fossil_crabdb_book_t).
 * @param filename      Path to the output INI file.
 * @return              true on success, false on failure.
 */
bool fossil_crabstore_save_to_ini(const fossil_crabdb_book_t *book, const char *filename);

/**
 * @brief Loads the database content from an INI file.
 *
 * @param book          Pointer to the database (fossil_crabdb_book_t).
 * @param filename      Path to the input INI file.
 * @return              true on success, false on failure.
 */
bool fossil_crabstore_load_from_ini(fossil_crabdb_book_t *book, const char *filename);

// *****************************************************************************
// CSV Storage API
// *****************************************************************************

/**
 * @brief Saves the database content to a CSV file.
 *
 * @param book          Pointer to the database (fossil_crabdb_book_t).
 * @param filename      Path to the output CSV file.
 * @return              true on success, false on failure.
 */
bool fossil_crabstore_save_to_csv(const fossil_crabdb_book_t *book, const char *filename);

/**
 * @brief Loads the database content from a CSV file.
 *
 * @param book          Pointer to the database (fossil_crabdb_book_t).
 * @param filename      Path to the input CSV file.
 * @return              true on success, false on failure.
 */
bool fossil_crabstore_load_from_csv(fossil_crabdb_book_t *book, const char *filename);

// *****************************************************************************
// Utility Functions for Storage
// *****************************************************************************

/**
 * @brief Validates the structure of an INI file before loading.
 *
 * @param filename      Path to the INI file.
 * @return              true if valid, false otherwise.
 */
bool fossil_crabstore_validate_ini(const char *filename);

/**
 * @brief Validates the structure of a CSV file before loading.
 *
 * @param filename      Path to the CSV file.
 * @return              true if valid, false otherwise.
 */
bool fossil_crabstore_validate_csv(const char *filename);

#ifdef __cplusplus
}

#include <string>

namespace fossil {
class CrabDBStore {
public:
    /**
     * @brief Saves the database content to an INI file.
     *
     * @param book          Reference to the database (fossil_crabdb_book_t).
     * @param filename      Path to the output INI file.
     * @return              true on success, false on failure.
     */
    static bool saveToIni(const fossil_crabdb_book_t &book, const std::string &filename) {
        return fossil_crabstore_save_to_ini(&book, filename.c_str());
    }

    /**
     * @brief Loads the database content from an INI file.
     *
     * @param book          Reference to the database (fossil_crabdb_book_t).
     * @param filename      Path to the input INI file.
     * @return              true on success, false on failure.
     */
    static bool loadFromIni(fossil_crabdb_book_t &book, const std::string &filename) {
        return fossil_crabstore_load_from_ini(&book, filename.c_str());
    }

    /**
     * @brief Saves the database content to a CSV file.
     *
     * @param book          Reference to the database (fossil_crabdb_book_t).
     * @param filename      Path to the output CSV file.
     * @return              true on success, false on failure.
     */
    static bool saveToCsv(const fossil_crabdb_book_t &book, const std::string &filename) {
        return fossil_crabstore_save_to_csv(&book, filename.c_str());
    }

    /**
     * @brief Loads the database content from a CSV file.
     *
     * @param book          Reference to the database (fossil_crabdb_book_t).
     * @param filename      Path to the input CSV file.
     * @return              true on success, false on failure.
     */
    static bool loadFromCsv(fossil_crabdb_book_t &book, const std::string &filename) {
        return fossil_crabstore_load_from_csv(&book, filename.c_str());
    }

    /**
     * @brief Validates the structure of an INI file before loading.
     *
     * @param filename      Path to the INI file.
     * @return              true if valid, false otherwise.
     */
    static bool validateIni(const std::string &filename) {
        return fossil_crabstore_validate_ini(filename.c_str());
    }

    /**
     * @brief Validates the structure of a CSV file before loading.
     *
     * @param filename      Path to the CSV file.
     * @return              true if valid, false otherwise.
     */
    static bool validateCsv(const std::string &filename) {
        return fossil_crabstore_validate_csv(filename.c_str());
    }
};


} // namespace fossil

#endif

#endif // FOSSIL_CRABDB_QUERY_H

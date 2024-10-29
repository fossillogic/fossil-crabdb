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

#ifndef FOSSIL_CRABSCI_H
#define FOSSIL_CRABSCI_H

#include "database.h"
#include "search.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Calculate the mean of the given data.
 * 
 * @param data Pointer to the array of data.
 * @param length Number of elements in the data array.
 * @return The mean of the data.
 */
double fossil_crabsci_mean(const double* data, size_t length);

/**
 * @brief Calculate the median of the given data.
 * 
 * @param data Pointer to the array of data. The array will be modified.
 * @param length Number of elements in the data array.
 * @return The median of the data.
 */
double fossil_crabsci_median(double* data, size_t length);

/**
 * @brief Calculate the standard deviation of the given data.
 * 
 * @param data Pointer to the array of data.
 * @param length Number of elements in the data array.
 * @return The standard deviation of the data.
 */
double fossil_crabsci_standard_deviation(const double* data, size_t length);

/**
 * @brief Analyze data from CrabDB based on a given pattern.
 * 
 * @param db Pointer to the CrabDB instance.
 * @param pattern Pattern to search for in the database.
 * @param results Pointer to the array where results will be stored.
 * @param result_count Pointer to the variable where the number of results will be stored.
 * @return Status of the search operation.
 */
crabsearch_status_t fossil_crabsci_analyze_data(fossil_crabdb_t* db, const char* pattern, double* results, size_t* result_count);

#ifdef __cplusplus
}
#endif

#endif // FOSSIL_CRABSCI_H

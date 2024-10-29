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
#include "fossil/crabdb/sci.h"
#include <math.h>

// Function to calculate the mean of an array of doubles
double fossil_crabsci_mean(const double* data, size_t length) {
    if (length == 0) return NAN; // Return NaN if no data
    double sum = 0.0;
    for (size_t i = 0; i < length; i++) {
        sum += data[i];
    }
    return sum / length;
}

// Function to calculate the median of an array of doubles
double fossil_crabsci_median(double* data, size_t length) {
    if (length == 0) return NAN; // Return NaN if no data
    // Sort the array (simple bubble sort for demonstration; consider a more efficient sort for production)
    for (size_t i = 0; i < length - 1; i++) {
        for (size_t j = 0; j < length - i - 1; j++) {
            if (data[j] > data[j + 1]) {
                double temp = data[j];
                data[j] = data[j + 1];
                data[j + 1] = temp;
            }
        }
    }
    // Return the median value
    if (length % 2 == 0) {
        return (data[length / 2 - 1] + data[length / 2]) / 2.0; // Average of two middle values
    } else {
        return data[length / 2]; // Middle value
    }
}

// Function to calculate the standard deviation of an array of doubles
double fossil_crabsci_standard_deviation(const double* data, size_t length) {
    if (length == 0) return NAN; // Return NaN if no data
    double mean = fossil_crabsci_mean(data, length);
    double sum_squared_diff = 0.0;
    for (size_t i = 0; i < length; i++) {
        sum_squared_diff += (data[i] - mean) * (data[i] - mean);
    }
    return sqrt(sum_squared_diff / length); // Sample standard deviation
}

// Function to retrieve data from the CrabDB and perform analysis
crabsearch_status_t fossil_crabsci_analyze_data(fossil_crabdb_t* db, const char* pattern, double* results, size_t* result_count) {
    if (!db || !pattern || !results || !result_count) {
        return CRABSEARCH_INVALID_PARAM; // Check for valid parameters
    }

    char result_buffer[1024];
    size_t match_count = 0;

    // Use existing search function to retrieve data
    crabsearch_status_t status = fossil_crabsearch_search(db, pattern, result_buffer, sizeof(result_buffer), &match_count, FORMAT_PLAIN_TEXT);
    if (status != CRABSEARCH_SUCCESS) {
        return status; // Return if search fails
    }

    // Assume result_buffer is parsed into a double array `data` (you'll need a parser for this)
    double data[100]; // Example: assume we have space for 100 double values
    size_t data_count = 0;

    // Parse result_buffer into data array and count valid entries (to be implemented)

    // Perform statistical calculations
    double mean = fossil_crabsci_mean(data, data_count);
    double median = fossil_crabsci_median(data, data_count);
    double stddev = fossil_crabsci_standard_deviation(data, data_count);

    // Store results
    results[0] = mean;
    results[1] = median;
    results[2] = stddev;
    *result_count = 3; // Number of results

    return CRABSEARCH_SUCCESS;
}

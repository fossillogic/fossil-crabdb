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

#include <fossil/unittest/framework.h> // Includes the Fossil Unit Test Framework
#include <fossil/mockup/framework.h>   // Includes the Fossil Mockup Framework
#include <fossil/unittest/assume.h>    // Includes the Fossil Assume Framework

#include "fossil/crabdb/framework.h"   // Includes the CrabDB Framework

FOSSIL_FIXTURE(sci_fixture);
fossil_crabdb_t *sci_mock_db;

FOSSIL_SETUP(sci_fixture) {
    sci_mock_db = fossil_crabdb_create();
}

FOSSIL_TEARDOWN(sci_fixture) {
    fossil_crabdb_destroy(sci_mock_db);
}

// Test mean calculation
FOSSIL_TEST(test_fossil_crabsci_calculate_mean) {
    double data[] = {1.0, 2.0, 3.0, 4.0, 5.0};
    size_t length = sizeof(data) / sizeof(data[0]);
    double mean = fossil_crabsci_mean(data, length);
    ASSUME_ITS_TRUE(mean == 3.0); // Check if the mean is correct
}

// Test median calculation
FOSSIL_TEST(test_fossil_crabsci_calculate_median) {
    double data[] = {1.0, 3.0, 2.0, 5.0, 4.0};
    size_t length = sizeof(data) / sizeof(data[0]);
    double median = fossil_crabsci_median(data, length);
    ASSUME_ITS_TRUE(median == 3.0); // Check if the median is correct
}

// Test standard deviation calculation
FOSSIL_TEST(test_fossil_crabsci_calculate_standard_deviation) {
    double data[] = {1.0, 2.0, 3.0, 4.0, 5.0};
    size_t length = sizeof(data) / sizeof(data[0]);
    double stddev = fossil_crabsci_standard_deviation(data, length);
    ASSUME_ITS_TRUE(stddev == 1.41421356); // Check if stddev is correct
}

// Test data analysis from CrabDB
FOSSIL_TEST(test_fossil_crabsci_analyze_data) {
    double results[10];
    size_t result_count = sizeof(results) / sizeof(results[0]);
    crabsearch_status_t status = fossil_crabsci_analyze_data(sci_mock_db, "SELECT * FROM training_data;", results, &result_count);
    ASSUME_ITS_TRUE(status == CRABSEARCH_SUCCESS);
    ASSUME_ITS_TRUE(result_count > 0); // Ensure results are returned
}

// Test invalid parameters for mean calculation
FOSSIL_TEST(test_fossil_crabsci_calculate_mean_invalid_params) {
    double mean = fossil_crabsci_mean(NULL, 5);
    ASSUME_ITS_TRUE(mean == 0.0); // Assuming the function returns 0.0 on invalid input
}

// Test invalid parameters for median calculation
FOSSIL_TEST(test_fossil_crabsci_calculate_median_invalid_params) {
    double median = fossil_crabsci_median(NULL, 5);
    ASSUME_ITS_TRUE(median == 0.0); // Assuming the function returns 0.0 on invalid input
}

// Test invalid parameters for standard deviation calculation
FOSSIL_TEST(test_fossil_crabsci_calculate_standard_deviation_invalid_params) {
    double stddev = fossil_crabsci_standard_deviation(NULL, 5);
    ASSUME_ITS_TRUE(stddev == 0.0); // Assuming the function returns 0.0 on invalid input
}

// Test invalid parameters for data analysis
FOSSIL_TEST(test_fossil_crabsci_analyze_data_invalid_params) {
    double results[10];
    size_t result_count = sizeof(results) / sizeof(results[0]);
    crabsearch_status_t status = fossil_crabsci_analyze_data(NULL, "SELECT * FROM training_data;", results, &result_count);
    ASSUME_ITS_TRUE(status == CRABSEARCH_INVALID_PARAM);
}

// * * * * * * * * * * * * * * * * * * * * * * * *
// * Fossil Logic Test Pool
// * * * * * * * * * * * * * * * * * * * * * * * *

FOSSIL_TEST_GROUP(c_crab_sci_tests) {
    ADD_TESTF(test_fossil_crabsci_calculate_mean, sci_fixture);
    ADD_TESTF(test_fossil_crabsci_calculate_median, sci_fixture);
    ADD_TESTF(test_fossil_crabsci_calculate_standard_deviation, sci_fixture);
    ADD_TESTF(test_fossil_crabsci_analyze_data, sci_fixture);
    ADD_TESTF(test_fossil_crabsci_calculate_mean_invalid_params, sci_fixture);
    ADD_TESTF(test_fossil_crabsci_calculate_median_invalid_params, sci_fixture);
    ADD_TESTF(test_fossil_crabsci_calculate_standard_deviation_invalid_params, sci_fixture);
    ADD_TESTF(test_fossil_crabsci_analyze_data_invalid_params, sci_fixture);
} // end of tests

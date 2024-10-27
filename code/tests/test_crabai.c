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

#include "fossil/crabdb/framework.h"

FOSSIL_FIXTURE(ai_fixture);
fossil_crabdb_t *ai_mock_db;

FOSSIL_SETUP(ai_fixture) {
    ai_mock_db = fossil_crabdb_create();
}

FOSSIL_TEARDOWN(ai_fixture) {
    fossil_crabdb_destroy(ai_mock_db);
}

// Test model training
FOSSIL_TEST(test_fossil_crabai_train_model) {
    const char *model_name = "test_model";
    const char *data_query = "SELECT * FROM training_data;";
    crabai_model_status_t status = fossil_crabai_train_model(ai_mock_db, model_name, data_query);
    ASSUME_ITS_TRUE(status == CRABAI_MODEL_SUCCESS);
}

// Test prediction
FOSSIL_TEST(test_fossil_crabai_predict) {
    const char *model_name = "test_model";
    const char *input_data = "test_input";
    char output_buffer[256];
    crabai_model_status_t status = fossil_crabai_predict(ai_mock_db, model_name, input_data, output_buffer, sizeof(output_buffer));
    ASSUME_ITS_TRUE(status == CRABAI_MODEL_SUCCESS);
    ASSUME_NOT_CNULL(output_buffer[0]); // Ensure output is not empty
}

// Test evaluation of the model
FOSSIL_TEST(test_fossil_crabai_evaluate_model) {
    const char *model_name = "test_model";
    const char *evaluation_data_query = "SELECT * FROM evaluation_data;";
    double performance_metric = 0.0;
    crabai_model_status_t status = fossil_crabai_evaluate_model(ai_mock_db, model_name, evaluation_data_query, &performance_metric);
    ASSUME_ITS_TRUE(status == CRABAI_MODEL_SUCCESS);
}

// Test getting insights
FOSSIL_TEST(test_fossil_crabai_get_insights) {
    const char *model_name = "test_model";
    const char *insight_query = "SELECT insights FROM model_insights;";
    char insight_buffer[256];
    crabai_model_status_t status = fossil_crabai_get_insights(ai_mock_db, model_name, insight_query, insight_buffer, sizeof(insight_buffer));
    ASSUME_ITS_TRUE(status == CRABAI_MODEL_SUCCESS);
    ASSUME_NOT_CNULL(insight_buffer[0]); // Ensure insights are returned
}

// Test invalid model training parameters
FOSSIL_TEST(test_fossil_crabai_train_model_invalid_params) {
    crabai_model_status_t status = fossil_crabai_train_model(NULL, "test_model", "SELECT * FROM training_data;");
    ASSUME_ITS_TRUE(status == CRABAI_MODEL_INVALID_PARAM);
}

// Test prediction with invalid parameters
FOSSIL_TEST(test_fossil_crabai_predict_invalid_params) {
    char output_buffer[256];
    crabai_model_status_t status = fossil_crabai_predict(NULL, "test_model", "test_input", output_buffer, sizeof(output_buffer));
    ASSUME_ITS_TRUE(status == CRABAI_MODEL_INVALID_PARAM);
}

// Test evaluation with invalid parameters
FOSSIL_TEST(test_fossil_crabai_evaluate_model_invalid_params) {
    double performance_metric = 0.0;
    crabai_model_status_t status = fossil_crabai_evaluate_model(NULL, "test_model", "SELECT * FROM evaluation_data;", &performance_metric);
    ASSUME_ITS_TRUE(status == CRABAI_MODEL_INVALID_PARAM);
}

// Test insights with invalid parameters
FOSSIL_TEST(test_fossil_crabai_get_insights_invalid_params) {
    char insight_buffer[256];
    crabai_model_status_t status = fossil_crabai_get_insights(NULL, "test_model", "SELECT insights FROM model_insights;", insight_buffer, sizeof(insight_buffer));
    ASSUME_ITS_TRUE(status == CRABAI_MODEL_INVALID_PARAM);
}

// * * * * * * * * * * * * * * * * * * * * * * * *
// * Fossil Logic Test Pool
// * * * * * * * * * * * * * * * * * * * * * * * *

FOSSIL_TEST_GROUP(c_crab_ai_tests) {
    ADD_TESTF(test_fossil_crabai_train_model, ai_fixture);
    ADD_TESTF(test_fossil_crabai_predict, ai_fixture);
    ADD_TESTF(test_fossil_crabai_evaluate_model, ai_fixture);
    ADD_TESTF(test_fossil_crabai_get_insights, ai_fixture);
    ADD_TESTF(test_fossil_crabai_train_model_invalid_params, ai_fixture);
    ADD_TESTF(test_fossil_crabai_predict_invalid_params, ai_fixture);
    ADD_TESTF(test_fossil_crabai_evaluate_model_invalid_params, ai_fixture);
    ADD_TESTF(test_fossil_crabai_get_insights_invalid_params, ai_fixture);
} // end of tests

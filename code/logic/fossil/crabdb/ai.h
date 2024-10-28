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
#ifndef FOSSIL_CRABDB_AI_H
#define FOSSIL_CRABDB_AI_H

#include "database.h"

#ifdef __cplusplus
extern "C" {
#endif

// AI Model Status Enum
typedef enum {
    CRABAI_MODEL_SUCCESS,
    CRABAI_MODEL_FAILURE,
    CRABAI_MODEL_INVALID_PARAM,
    CRABAI_MODEL_NOT_READY
} crabai_model_status_t;

/** 
 * @brief Trains an AI model on the database.
 * 
 * @param db A pointer to the database to train the model on.
 * @param model_name The name of the model to train.
 * @param data_query The query to fetch the training data.
 * @return crabai_model_status_t Status code indicating success or error.
 */
crabai_model_status_t fossil_crabai_train_model(fossil_crabdb_t *db, const char *model_name, const char *data_query);

/** 
 * @brief Makes predictions using the trained model.
 * 
 * @param db A pointer to the database to use for predictions.
 * @param model_name The name of the model to use.
 * @param input_data The input data for the prediction.
 * @param output_buffer The buffer to store the prediction output.
 * @param buffer_size The size of the output buffer.
 * @return crabai_model_status_t Status code indicating success or error.
 */
crabai_model_status_t fossil_crabai_predict(fossil_crabdb_t *db, const char *model_name, const char *input_data, char *output_buffer, size_t buffer_size);

/** 
 * @brief Evaluates the performance of a trained model.
 * 
 * @param db A pointer to the database to use for evaluation.
 * @param model_name The name of the model to evaluate.
 * @param evaluation_data_query The query to fetch the evaluation data.
 * @param performance_metric Pointer to store the performance metric.
 * @return crabai_model_status_t Status code indicating success or error.
 */
crabai_model_status_t fossil_crabai_evaluate_model(fossil_crabdb_t *db, const char *model_name, const char *evaluation_data_query, double *performance_metric);

/** 
 * @brief Gets insights based on predictive analysis.
 * 
 * @param db A pointer to the database to use for insights.
 * @param model_name The name of the model to get insights from.
 * @param insight_query The query to fetch the insights.
 * @param insight_buffer The buffer to store the insights.
 * @param buffer_size The size of the insights buffer.
 * @return crabai_model_status_t Status code indicating success or error.
 */
crabai_model_status_t fossil_crabai_get_insights(fossil_crabdb_t *db, const char *model_name, const char *insight_query, char *insight_buffer, size_t buffer_size);

#ifdef __cplusplus
}
#endif

#endif // FOSSIL_CRABDB_QUERY_H

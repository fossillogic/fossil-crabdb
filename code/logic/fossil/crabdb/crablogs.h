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
#ifndef FOSSIL_CRABDB_LOGS_H
#define FOSSIL_CRABDB_LOGS_H

#include "crabdb.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Enumeration of logging levels.
 */
typedef enum {
    CRABLOG_DEBUG,
    CRABLOG_INFO,
    CRABLOG_WARNING,
    CRABLOG_ERROR,
    CRABLOG_CRITICAL
} crablog_level_t;

/**
 * @brief Enumeration of logging outputs.
 */
typedef enum {
    CRABLOG_OUTPUT_STDOUT,   // Log to standard output
    CRABLOG_OUTPUT_FILE,     // Log to a file
    CRABLOG_OUTPUT_REMOTE    // Log to a remote server (future implementation)
} crablog_output_t;

/**
 * @brief Enumeration of log formats.
 */
typedef enum {
    CRABLOG_FORMAT_PLAIN_TEXT,  // Standard plain text logging
    CRABLOG_FORMAT_JSON         // JSON formatted logging for structured logs
} crablog_format_t;

/**
 * @brief Initialize the logging system with default settings.
 */
void fossil_crablog_init(void);

/**
 * @brief Set the output destination for logs.
 * @param output The logging output type.
 * @param file_path_or_address File path for file output, or address for remote output (if applicable).
 */
void fossil_crablog_set_output(crablog_output_t output, const char* file_path_or_address);

/**
 * @brief Set the minimum logging level to capture.
 * @param level The minimum logging level (e.g., CRABLOG_WARNING will capture warnings and above).
 */
void fossil_crablog_set_minimum_level(crablog_level_t level);

/**
 * @brief Set the format for log messages.
 * @param format The desired log format (plain text or JSON).
 */
void fossil_crablog_set_format(crablog_format_t format);

/**
 * @brief Log a message with a specific level.
 * @param level The logging level (e.g., CRABLOG_DEBUG, CRABLOG_ERROR).
 * @param module The module or component where the log originates (e.g., "CrabQL").
 * @param message The log message.
 */
void fossil_crablog_log(crablog_level_t level, const char* module, const char* message);

/**
 * @brief Log a message with context, including file, line, and function.
 * @param level The logging level.
 * @param file The file where the log originates.
 * @param line The line number where the log originates.
 * @param function The function name where the log originates.
 * @param message The log message.
 */
void fossil_crablog_log_with_context(crablog_level_t level, const char* file, int line, const char* function, const char* message);

/**
 * @brief Log a debug message.
 * @param module The module or component where the log originates.
 * @param message The log message.
 */
void fossil_crablog_debug(const char* module, const char* message);

/**
 * @brief Log an informational message.
 * @param module The module or component where the log originates.
 * @param message The log message.
 */
void fossil_crablog_info(const char* module, const char* message);

/**
 * @brief Log a warning message.
 * @param module The module or component where the log originates.
 * @param message The log message.
 */
void fossil_crablog_warning(const char* module, const char* message);

/**
 * @brief Log an error message.
 * @param module The module or component where the log originates.
 * @param message The log message.
 */
void fossil_crablog_error(const char* module, const char* message);

/**
 * @brief Log a critical error message.
 * @param module The module or component where the log originates.
 * @param message The log message.
 */
void fossil_crablog_critical(const char* module, const char* message);

/**
 * @brief Log a message with a timestamp.
 * @param level The logging level.
 * @param module The module or component where the log originates.
 * @param message The log message.
 */
void fossil_crablog_with_timestamp(crablog_level_t level, const char* module, const char* message);

/**
 * @brief Rotate log files when they exceed a certain size.
 * @param max_file_size Maximum file size before rotation (in bytes).
 */
void fossil_crablog_set_rotation_size(size_t max_file_size);

/**
 * @brief Manually rotate logs if necessary.
 */
void fossil_crablog_rotate_logs(void);

/**
 * @brief Flush the log buffer to the output (used in async mode).
 */
void fossil_crablog_flush(void);

/**
 * @brief Log query execution time for performance tracking.
 * @param query The query that was executed.
 * @param execution_time The time it took to execute the query (in seconds).
 */
void fossil_crablog_query_timing(const char* query, double execution_time);

/**
 * @brief Log resource usage (e.g., memory, CPU).
 * @param memory_usage The amount of memory used (in bytes).
 * @param cpu_usage The CPU usage percentage.
 */
void fossil_crablog_resource_usage(size_t memory_usage, double cpu_usage);

/**
 * @brief Clean up the logging system and release any resources.
 */
void fossil_crablog_cleanup(void);

#ifdef __cplusplus
}
#endif

#endif /* FOSSIL_CRABDB_FRAMEWORK_H */

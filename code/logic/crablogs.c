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
#include "fossil/crabdb/crablogs.h"
#include <stdarg.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

static crablog_output_t log_output = CRABLOG_OUTPUT_STDOUT;
static crablog_level_t min_log_level = CRABLOG_DEBUG;
static crablog_format_t log_format = CRABLOG_FORMAT_PLAIN_TEXT;
static FILE* log_file = NULL;
static size_t rotation_size = 0;

void fossil_crablog_init(void) {
    log_output = CRABLOG_OUTPUT_STDOUT;
    min_log_level = CRABLOG_DEBUG;
    log_format = CRABLOG_FORMAT_PLAIN_TEXT;
    log_file = NULL;
    rotation_size = 0;
}

void fossil_crablog_set_output(crablog_output_t output, const char* file_path_or_address) {
    log_output = output;
    if (log_output == CRABLOG_OUTPUT_FILE && file_path_or_address != NULL) {
        log_file = fopen(file_path_or_address, "a");
        if (!log_file) {
            fprintf(stderr, "Failed to open log file: %s\n", file_path_or_address);
            exit(EXIT_FAILURE);
        }
    }
}

void fossil_crablog_set_minimum_level(crablog_level_t level) {
    min_log_level = level;
}

void fossil_crablog_set_format(crablog_format_t format) {
    log_format = format;
}

static const char* level_to_string(crablog_level_t level) {
    switch (level) {
        case CRABLOG_DEBUG:    return "DEBUG";
        case CRABLOG_INFO:     return "INFO";
        case CRABLOG_WARNING:  return "WARNING";
        case CRABLOG_ERROR:    return "ERROR";
        case CRABLOG_CRITICAL: return "CRITICAL";
        default: return "UNKNOWN";
    }
}

static void log_message(crablog_level_t level, const char* module, const char* message, const char* file, int line, const char* function) {
    if (level < min_log_level) return;

    time_t now = time(NULL);
    struct tm* local_time = localtime(&now);
    char timestamp[32];
    strftime(timestamp, sizeof(timestamp), "%Y-%m-%d %H:%M:%S", local_time);

    if (log_format == CRABLOG_FORMAT_JSON) {
        if (log_output == CRABLOG_OUTPUT_FILE && log_file != NULL) {
            fprintf(log_file, "{\"timestamp\":\"%s\",\"level\":\"%s\",\"module\":\"%s\",\"message\":\"%s\"}\n", timestamp, level_to_string(level), module, message);
        } else {
            printf("{\"timestamp\":\"%s\",\"level\":\"%s\",\"module\":\"%s\",\"message\":\"%s\"}\n", timestamp, level_to_string(level), module, message);
        }
    } else {
        if (log_output == CRABLOG_OUTPUT_FILE && log_file != NULL) {
            fprintf(log_file, "[%s] [%s] [%s] %s\n", timestamp, level_to_string(level), module, message);
        } else {
            printf("[%s] [%s] [%s] %s\n", timestamp, level_to_string(level), module, message);
        }
    }

    if (log_output == CRABLOG_OUTPUT_FILE && log_file != NULL && rotation_size > 0) {
        fseek(log_file, 0, SEEK_END);
        size_t file_size = ftell(log_file);
        if (file_size >= rotation_size) {
            fclose(log_file);
            // Handle file rotation (renaming and reopening)
            log_file = fopen("logfile.log", "a");
        }
    }
}

void fossil_crablog_log(crablog_level_t level, const char* module, const char* message) {
    log_message(level, module, message, NULL, 0, NULL);
}

void fossil_crablog_log_with_context(crablog_level_t level, const char* file, int line, const char* function, const char* message) {
    if (level < min_log_level) return;

    char formatted_message[512];
    snprintf(formatted_message, sizeof(formatted_message), "[%s:%d %s()] %s", file, line, function, message);
    log_message(level, "context", formatted_message, file, line, function);
}

void fossil_crablog_debug(const char* module, const char* message) {
    fossil_crablog_log(CRABLOG_DEBUG, module, message);
}

void fossil_crablog_info(const char* module, const char* message) {
    fossil_crablog_log(CRABLOG_INFO, module, message);
}

void fossil_crablog_warning(const char* module, const char* message) {
    fossil_crablog_log(CRABLOG_WARNING, module, message);
}

void fossil_crablog_error(const char* module, const char* message) {
    fossil_crablog_log(CRABLOG_ERROR, module, message);
}

void fossil_crablog_critical(const char* module, const char* message) {
    fossil_crablog_log(CRABLOG_CRITICAL, module, message);
}

void fossil_crablog_with_timestamp(crablog_level_t level, const char* module, const char* message) {
    log_message(level, module, message, NULL, 0, NULL);
}

void fossil_crablog_set_rotation_size(size_t max_file_size) {
    rotation_size = max_file_size;
}

void fossil_crablog_rotate_logs(void) {
    if (log_output == CRABLOG_OUTPUT_FILE && log_file != NULL) {
        fclose(log_file);

        // Generate a timestamp for the rotated log file
        time_t now = time(NULL);
        struct tm* local_time = localtime(&now);
        char timestamp[32];
        strftime(timestamp, sizeof(timestamp), "%Y%m%d%H%M%S", local_time);

        // Create the new log file name with the timestamp
        char new_log_file_name[256];
        snprintf(new_log_file_name, sizeof(new_log_file_name), "logfile_%s.log", timestamp);

        // Rename the current log file
        if (rename("logfile.log", new_log_file_name) != 0) {
            fprintf(stderr, "Failed to rename log file\n");
            exit(EXIT_FAILURE);
        }

        // Open a new log file
        log_file = fopen("logfile.log", "a");
        if (!log_file) {
            fprintf(stderr, "Failed to open new log file\n");
            exit(EXIT_FAILURE);
        }
    }
}

void fossil_crablog_flush(void) {
    if (log_file != NULL) {
        fflush(log_file);
    }
}

void fossil_crablog_query_timing(const char* query, double execution_time) {
    char message[256];
    snprintf(message, sizeof(message), "Query: %s executed in %.2f seconds", query, execution_time);
    fossil_crablog_log(CRABLOG_INFO, "performance", message);
}

void fossil_crablog_resource_usage(size_t memory_usage, double cpu_usage) {
    char message[256];
    snprintf(message, sizeof(message), "Memory usage: %zu bytes, CPU usage: %.2f%%", memory_usage, cpu_usage);
    fossil_crablog_log(CRABLOG_INFO, "resources", message);
}

void fossil_crablog_cleanup(void) {
    if (log_file != NULL) {
        fclose(log_file);
    }
}

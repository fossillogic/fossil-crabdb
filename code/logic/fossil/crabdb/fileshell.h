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
#ifndef FOSSIL_CRABDB_FILESHELL_H
#define FOSSIL_CRABDB_FILESHELL_H

#include <stdbool.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <sys/stat.h>   // for file size
#include <errno.h>
#include <stdio.h>
#include <time.h>

#ifdef __cplusplus
extern "C" {
#endif

// *****************************************************************************
// FileShell API - Prototypes
// *****************************************************************************

// ===========================================================
// Basic File Operations
// ===========================================================

/**
 * @brief Writes text data to a file (overwrites if exists).
 *
 * @param path      File path.
 * @param data      Null-terminated string to write.
 * @return          true on success, false on failure.
 */
bool fossil_bluecrab_fileshell_write(const char *path, const char *data);

/**
 * @brief Appends text data to a file (creates if missing).
 *
 * @param path      File path.
 * @param data      Null-terminated string to append.
 * @return          true on success, false on failure.
 */
bool fossil_bluecrab_fileshell_append(const char *path, const char *data);

/**
 * @brief Reads text data from a file.
 *
 * @param path          File path.
 * @param out_buf       Buffer to store file contents.
 * @param buf_size      Size of output buffer.
 * @return              true on success, false on failure.
 */
bool fossil_bluecrab_fileshell_read(const char *path, char *out_buf, size_t buf_size);

/**
 * @brief Deletes a file.
 *
 * @param path      File path.
 * @return          true if deleted, false otherwise.
 */
bool fossil_bluecrab_fileshell_delete(const char *path);

// ===========================================================
// File Information
// ===========================================================

/**
 * @brief Checks if a file exists.
 *
 * @param path      File path.
 * @return          true if file exists, false otherwise.
 */
bool fossil_bluecrab_fileshell_exists(const char *path);

/**
 * @brief Returns the file size in bytes.
 *
 * @param path      File path.
 * @return          File size in bytes, or -1 on error.
 */
long fossil_bluecrab_fileshell_size(const char *path);

// ===========================================================
// File Listing
// ===========================================================

/**
 * @brief Lists files in a directory.
 *
 * @param dir_path      Directory path.
 * @param out_files     Array of strings to store file names.
 * @param max_files     Maximum number of files to return.
 * @return              Number of files listed, or -1 on error.
 */
int fossil_bluecrab_fileshell_list(const char *dir_path, char **out_files, size_t max_files);

#ifdef __cplusplus
}
#include <string>

namespace fossil {

namespace bluecrab {



}

}

#endif

#endif /* FOSSIL_CRABDB_FRAMEWORK_H */

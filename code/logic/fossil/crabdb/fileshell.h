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
        /**
         * @brief FileShell class provides static methods for file operations.
         *
         * This class wraps the C API for file operations, providing a C++ interface
         * using std::string and std::vector. All methods are static and do not require
         * instantiation.
         */
        class FileShell {
        public:
            /**
             * @brief Writes text data to a file (overwrites if exists).
             *
             * @param path  Path to the file.
             * @param data  String data to write.
             * @return      true on success, false on failure.
             */
            static bool write(const std::string& path, const std::string& data) {
                return fossil_bluecrab_fileshell_write(path.c_str(), data.c_str());
            }

            /**
             * @brief Appends text data to a file (creates if missing).
             *
             * @param path  Path to the file.
             * @param data  String data to append.
             * @return      true on success, false on failure.
             */
            static bool append(const std::string& path, const std::string& data) {
                return fossil_bluecrab_fileshell_append(path.c_str(), data.c_str());
            }

            /**
             * @brief Reads text data from a file.
             *
             * @param path  Path to the file.
             * @param out   Output string to store file contents.
             * @return      true on success, false on failure.
             */
            static bool read(const std::string& path, std::string& out) {
                long sz = fossil_bluecrab_fileshell_size(path.c_str());
                if (sz < 0) return false;
                std::string buf(sz, '\0');
                bool ok = fossil_bluecrab_fileshell_read(path.c_str(), &buf[0], sz + 1);
                if (ok) {
                    out = buf.c_str();
                }
                return ok;
            }

            /**
             * @brief Deletes a file.
             *
             * @param path  Path to the file.
             * @return      true if deleted, false otherwise.
             */
            static bool remove(const std::string& path) {
                return fossil_bluecrab_fileshell_delete(path.c_str());
            }

            /**
             * @brief Checks if a file exists.
             *
             * @param path  Path to the file.
             * @return      true if file exists, false otherwise.
             */
            static bool exists(const std::string& path) {
                return fossil_bluecrab_fileshell_exists(path.c_str());
            }

            /**
             * @brief Returns the file size in bytes.
             *
             * @param path  Path to the file.
             * @return      File size in bytes, or -1 on error.
             */
            static long size(const std::string& path) {
                return fossil_bluecrab_fileshell_size(path.c_str());
            }

            /**
             * @brief Lists files in a directory.
             *
             * @param dir_path  Path to the directory.
             * @param files     Output vector to store file names.
             * @param max_files Maximum number of files to list.
             * @return          Number of files listed, or -1 on error.
             */
            static int list(const std::string& dir_path, std::vector<std::string>& files, size_t max_files) {
                std::vector<char*> c_files(max_files, nullptr);
                int count = fossil_bluecrab_fileshell_list(dir_path.c_str(), c_files.data(), max_files);
                if (count > 0) {
                    files.clear();
                    for (int i = 0; i < count; ++i) {
                        if (c_files[i]) {
                            files.emplace_back(c_files[i]);
                            free(c_files[i]);
                        }
                    }
                }
                return count;
            }
        };

    } // namespace bluecrab

} // namespace fossil

#endif

#endif /* FOSSIL_CRABDB_FRAMEWORK_H */

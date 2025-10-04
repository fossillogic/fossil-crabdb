/**
 * -----------------------------------------------------------------------------
 * Project: Fossil Logic
 *
 * This file is part of the Fossil Logic project, which aims to develop
 * high-performance, cross-platform applications and libraries. The code
 * contained herein is licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License. You may obtain
 * a copy of the License at:
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS, WITHOUT
 * WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied. See the
 * License for the specific language governing permissions and limitations
 * under the License.
 *
 * Author: Michael Gene Brockus (Dreamer)
 * Date: 04/05/2014
 *
 * Copyright (C) 2014-2025 Fossil Logic. All rights reserved.
 * -----------------------------------------------------------------------------
 */
#include "fossil/crabdb/fileshell.h"
#include <sys/stat.h>
#include <dirent.h>
#include <unistd.h>

/**
 * @brief Custom strdup implementation.
 * 
 * @param src       The source string to duplicate.
 * @return          Pointer to newly allocated string, or NULL on failure.
 */
char* fossil_fileshell_strdup(const char *src) {
    if (!src) return NULL;

    size_t len = 0;
    while (src[len] != '\0') len++;   // compute length manually

    char *dup = (char*)malloc(len + 1); // +1 for null terminator
    if (!dup) return NULL;

    for (size_t i = 0; i < len; i++) {
        dup[i] = src[i];
    }
    dup[len] = '\0';

    return dup;
}

// ===========================================================
// Basic File Operations
// ===========================================================

bool fossil_bluecrab_fileshell_write(const char *path, const char *data) {
    if (!path || !data) return false;

    FILE *fp = fopen(path, "w");
    if (!fp) return false;

    size_t written = fwrite(data, 1, strlen(data), fp);
    fclose(fp);

    return (written == strlen(data));
}

bool fossil_bluecrab_fileshell_append(const char *path, const char *data) {
    if (!path || !data) return false;

    FILE *fp = fopen(path, "a");
    if (!fp) return false;

    size_t written = fwrite(data, 1, strlen(data), fp);
    fclose(fp);

    return (written == strlen(data));
}

bool fossil_bluecrab_fileshell_read(const char *path, char *out_buf, size_t buf_size) {
    if (!path || !out_buf || buf_size == 0) return false;

    FILE *fp = fopen(path, "r");
    if (!fp) return false;

    size_t read_bytes = fread(out_buf, 1, buf_size - 1, fp);
    out_buf[read_bytes] = '\0';
    fclose(fp);

    return true;
}

bool fossil_bluecrab_fileshell_delete(const char *path) {
    if (!path) return false;

    // Ensure file is not open by this process (best effort, not strictly necessary)
    // The test itself opens and closes the file, so we don't need to do it here.

    int result = remove(path);
    // After removal, check if the file still exists
    if (result == 0 && !fossil_bluecrab_fileshell_exists(path)) {
        return true;
    }
    return false;
}

// ===========================================================
// File Information
// ===========================================================

bool fossil_bluecrab_fileshell_exists(const char *path) {
    if (!path) return false;
    return (access(path, F_OK) == 0);
}

long fossil_bluecrab_fileshell_size(const char *path) {
    if (!path) return -1;

    struct stat st;
    if (stat(path, &st) != 0) {
        return -1;
    }
    return (long)st.st_size;
}

// ===========================================================
// File Listing
// ===========================================================

int fossil_bluecrab_fileshell_list(const char *dir_path, char **out_files, size_t max_files) {
    if (!dir_path || !out_files || max_files == 0) return -1;

    DIR *dir = opendir(dir_path);
    if (!dir) return -1;

    struct dirent *entry;
    int count = 0;

    while ((entry = readdir(dir)) != NULL && count < (int)max_files) {
        if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0)
            continue;

        out_files[count] = fossil_fileshell_strdup(entry->d_name);
        if (!out_files[count]) {
            closedir(dir);
            return -1;
        }
        count++;
    }

    closedir(dir);
    return count;
}

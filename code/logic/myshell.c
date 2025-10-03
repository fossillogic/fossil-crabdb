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
#include "fossil/crabdb/myshell.h"

/**
 * Custom strdup implementation.
 */
static char *myshell_strdup(const char *s) {
    if (!s) return NULL;
    size_t len = strlen(s);
    char *copy = (char *)malloc(len + 1);
    if (copy) {
        memcpy(copy, s, len + 1);
    }
    return copy;
}

/**
 * Advanced 64-bit hash algorithm for strings (MurmurHash3 variant).
 * Returns a 64-bit hash value for the given input string.
 */
uint64_t myshell_hash64(const char *str) {
    if (!str) return 0;
    uint64_t seed = 0xe17a1465ULL;
    uint64_t m = 0xc6a4a7935bd1e995ULL;
    int r = 47;
    size_t len = strlen(str);
    uint64_t hash = seed ^ (len * m);

    const uint8_t *data = (const uint8_t *)str;
    const uint8_t *end = data + (len & ~0x7);

    while (data != end) {
        uint64_t k;
        memcpy(&k, data, sizeof(uint64_t));
        k *= m;
        k ^= k >> r;
        k *= m;
        hash ^= k;
        hash *= m;
        data += 8;
    }

    switch (len & 7) {
        case 7: hash ^= (uint64_t)data[6] << 48;
        case 6: hash ^= (uint64_t)data[5] << 40;
        case 5: hash ^= (uint64_t)data[4] << 32;
        case 4: hash ^= (uint64_t)data[3] << 24;
        case 3: hash ^= (uint64_t)data[2] << 16;
        case 2: hash ^= (uint64_t)data[1] << 8;
        case 1: hash ^= (uint64_t)data[0];
                hash *= m;
    }

    hash ^= hash >> r;
    hash *= m;
    hash ^= hash >> r;
    return hash;
}

fossil_bluecrab_myshell_t *fossil_myshell_open(const char *path, fossil_bluecrab_myshell_error_t *err) {
    if (!path) {
        if (err) *err = FOSSIL_MYSHELL_ERROR_INVALID_FILE;
        return NULL;
    }

    // Enforce .myshell extension
    const char *ext = strrchr(path, '.');
    if (!ext || strcmp(ext, ".myshell") != 0) {
        if (err) *err = FOSSIL_MYSHELL_ERROR_INVALID_FILE;
        return NULL;
    }

    FILE *file = fopen(path, "rb+");
    if (!file) {
        if (err) *err = FOSSIL_MYSHELL_ERROR_FILE_NOT_FOUND;
        return NULL;
    }

    fossil_bluecrab_myshell_t *db = (fossil_bluecrab_myshell_t *)calloc(1, sizeof(fossil_bluecrab_myshell_t));
    if (!db) {
        fclose(file);
        if (err) *err = FOSSIL_MYSHELL_ERROR_OUT_OF_MEMORY;
        return NULL;
    }

    db->path = myshell_strdup(path);
    if (!db->path) {
        fclose(file);
        free(db);
        if (err) *err = FOSSIL_MYSHELL_ERROR_OUT_OF_MEMORY;
        return NULL;
    }

    db->file = file;
    db->is_open = true;

    if (fseek(file, 0, SEEK_END) != 0) {
        free(db->path);
        free(db);
        fclose(file);
        if (err) *err = FOSSIL_MYSHELL_ERROR_IO;
        return NULL;
    }
    db->file_size = (size_t)ftell(file);
    if (fseek(file, 0, SEEK_SET) != 0) {
        free(db->path);
        free(db);
        fclose(file);
        if (err) *err = FOSSIL_MYSHELL_ERROR_IO;
        return NULL;
    }

    struct stat st;
    if (stat(path, &st) == 0)
        db->last_modified = st.st_mtime;
    else
        db->last_modified = 0;

    db->commit_head = myshell_hash64(path);
    db->error_code = FOSSIL_MYSHELL_ERROR_SUCCESS;

    if (err) *err = FOSSIL_MYSHELL_ERROR_SUCCESS;
    return db;
}

fossil_bluecrab_myshell_t *fossil_myshell_create(const char *path, fossil_bluecrab_myshell_error_t *err) {
    if (!path) {
        if (err) *err = FOSSIL_MYSHELL_ERROR_INVALID_FILE;
        return NULL;
    }

    // Enforce .myshell extension
    const char *ext = strrchr(path, '.');
    if (!ext || strcmp(ext, ".myshell") != 0) {
        if (err) *err = FOSSIL_MYSHELL_ERROR_INVALID_FILE;
        return NULL;
    }

    // Check if file already exists
    FILE *check = fopen(path, "rb");
    if (check) {
        fclose(check);
        if (err) *err = FOSSIL_MYSHELL_ERROR_ALREADY_EXISTS;
        return NULL;
    }

    FILE *file = fopen(path, "wb+");
    if (!file) {
        if (err) *err = FOSSIL_MYSHELL_ERROR_IO;
        return NULL;
    }

    fossil_bluecrab_myshell_t *db = (fossil_bluecrab_myshell_t *)calloc(1, sizeof(fossil_bluecrab_myshell_t));
    if (!db) {
        fclose(file);
        if (err) *err = FOSSIL_MYSHELL_ERROR_OUT_OF_MEMORY;
        return NULL;
    }

    db->path = myshell_strdup(path);
    if (!db->path) {
        fclose(file);
        free(db);
        if (err) *err = FOSSIL_MYSHELL_ERROR_OUT_OF_MEMORY;
        return NULL;
    }

    db->file = file;
    db->is_open = true;
    db->file_size = 0;
    db->last_modified = time(NULL);
    db->commit_head = myshell_hash64(path);
    db->error_code = FOSSIL_MYSHELL_ERROR_SUCCESS;

    if (err) *err = FOSSIL_MYSHELL_ERROR_SUCCESS;
    return db;
}

void fossil_myshell_close(fossil_bluecrab_myshell_t *db) {
    if (db) {
        if (db->file) {
            fclose(db->file);
            db->file = NULL;
        }
        if (db->path) {
            free(db->path);
            db->path = NULL;
        }
        if (db->branch) {
            free(db->branch);
            db->branch = NULL;
        }
        if (db->author) {
            free(db->author);
            db->author = NULL;
        }
        if (db->commit_message) {
            free(db->commit_message);
            db->commit_message = NULL;
        }
        if (db->parent_branch) {
            free(db->parent_branch);
            db->parent_branch = NULL;
        }
        free(db);
    }
}

fossil_bluecrab_myshell_error_t fossil_myshell_put(fossil_bluecrab_myshell_t *db, const char *key, const char *value) {
    if (!db || !db->is_open) {
        return FOSSIL_MYSHELL_ERROR_INVALID_FILE;
    }
    if (!key || !value) {
        return FOSSIL_MYSHELL_ERROR_INVALID_QUERY;
    }
    if (key[0] == '\0') {
        return FOSSIL_MYSHELL_ERROR_INVALID_QUERY;
    }

    uint64_t key_hash = myshell_hash64(key);

    fseek(db->file, 0, SEEK_SET);
    char temp_path[256];
    snprintf(temp_path, sizeof(temp_path), "%s.tmp", db->path);
    FILE *temp_file = fopen(temp_path, "wb");
    if (!temp_file) {
        return FOSSIL_MYSHELL_ERROR_IO;
    }

    char line[1024];
    bool updated = false;
    while (fgets(line, sizeof(line), db->file)) {
        char *eq = strchr(line, '=');
        if (eq) {
            *eq = '\0';
            char *hash_comment = strstr(eq + 1, "#hash=");
            if (hash_comment) {
                uint64_t file_hash = 0;
                sscanf(hash_comment, "#hash=%llx", &file_hash);
                if (strcmp(line, key) == 0 && file_hash == key_hash) {
                    fprintf(temp_file, "%s=%s #hash=%016llx\n", key, value, key_hash);
                    updated = true;
                    *eq = '='; // Restore
                    continue;
                }
            } else {
                if (strcmp(line, key) == 0) {
                    fprintf(temp_file, "%s=%s #hash=%016llx\n", key, value, key_hash);
                    updated = true;
                    *eq = '='; // Restore
                    continue;
                }
            }
            *eq = '='; // Restore
        }
        fputs(line, temp_file);
    }

    if (!updated) {
        fprintf(temp_file, "%s=%s #hash=%016llx\n", key, value, key_hash);
    }

    fclose(temp_file);
    fclose(db->file);

    if (remove(db->path) != 0) {
        remove(temp_path);
        return FOSSIL_MYSHELL_ERROR_IO;
    }
    if (rename(temp_path, db->path) != 0) {
        return FOSSIL_MYSHELL_ERROR_IO;
    }

    db->file = fopen(db->path, "rb+");
    if (!db->file) {
        return FOSSIL_MYSHELL_ERROR_IO;
    }

    db->file_size = (size_t)ftell(db->file);
    db->last_modified = time(NULL);
    return FOSSIL_MYSHELL_ERROR_SUCCESS;
}

fossil_bluecrab_myshell_error_t fossil_myshell_get(fossil_bluecrab_myshell_t *db, const char *key, char *out_value, size_t out_size) {
    if (!db || !db->is_open) {
        return FOSSIL_MYSHELL_ERROR_INVALID_FILE;
    }
    if (!key || !out_value || out_size == 0) {
        return FOSSIL_MYSHELL_ERROR_INVALID_QUERY;
    }
    if (key[0] == '\0') {
        return FOSSIL_MYSHELL_ERROR_INVALID_QUERY;
    }

    uint64_t key_hash = myshell_hash64(key);

    fseek(db->file, 0, SEEK_SET);
    char line[1024];
    while (fgets(line, sizeof(line), db->file)) {
        char *eq = strchr(line, '=');
        if (eq) {
            *eq = '\0';
            char *hash_comment = strstr(eq + 1, "#hash=");
            if (hash_comment) {
                uint64_t file_hash = 0;
                sscanf(hash_comment, "#hash=%llx", &file_hash);
                if (strcmp(line, key) == 0 && file_hash == key_hash) {
                    size_t value_len = hash_comment - (eq + 1);
                    if (value_len >= out_size) value_len = out_size - 1;
                    strncpy(out_value, eq + 1, value_len);
                    out_value[value_len] = '\0';
                    size_t len = strlen(out_value);
                    while (len > 0 && (out_value[len - 1] == '\n' || out_value[len - 1] == ' ')) {
                        out_value[--len] = '\0';
                    }
                    *eq = '='; // Restore
                    return FOSSIL_MYSHELL_ERROR_SUCCESS;
                }
            } else {
                if (strcmp(line, key) == 0) {
                    strncpy(out_value, eq + 1, out_size - 1);
                    out_value[out_size - 1] = '\0';
                    size_t len = strlen(out_value);
                    if (len > 0 && out_value[len - 1] == '\n') {
                        out_value[len - 1] = '\0';
                    }
                    *eq = '='; // Restore
                    return FOSSIL_MYSHELL_ERROR_SUCCESS;
                }
            }
            *eq = '='; // Restore
        }
    }

    return FOSSIL_MYSHELL_ERROR_NOT_FOUND;
}

fossil_bluecrab_myshell_error_t fossil_myshell_del(fossil_bluecrab_myshell_t *db, const char *key) {
    if (!db || !db->is_open) {
        return FOSSIL_MYSHELL_ERROR_INVALID_FILE;
    }
    if (!key || key[0] == '\0') {
        return FOSSIL_MYSHELL_ERROR_INVALID_QUERY;
    }

    uint64_t key_hash = myshell_hash64(key);

    // Read all lines, rewrite excluding the deleted key (matching both key and hash)
    fseek(db->file, 0, SEEK_SET);
    char temp_path[256];
    snprintf(temp_path, sizeof(temp_path), "%s.tmp", db->path);
    FILE *temp_file = fopen(temp_path, "wb");
    if (!temp_file) {
        return FOSSIL_MYSHELL_ERROR_IO;
    }

    char line[1024];
    bool found = false;
    while (fgets(line, sizeof(line), db->file)) {
        char *eq = strchr(line, '=');
        if (eq) {
            *eq = '\0';
            char *hash_comment = strstr(eq + 1, "#hash=");
            if (hash_comment) {
                uint64_t file_hash = 0;
                sscanf(hash_comment, "#hash=%llx", &file_hash);
                if (strcmp(line, key) == 0 && file_hash == key_hash) {
                    found = true; // Skip this line
                    *eq = '='; // Restore
                    continue;
                }
            } else {
                if (strcmp(line, key) == 0) {
                    found = true; // Skip this line
                    *eq = '='; // Restore
                    continue;
                }
            }
            *eq = '='; // Restore
        }
        fputs(line, temp_file);
    }

    fclose(temp_file);
    fclose(db->file);

    if (found) {
        if (remove(db->path) != 0) {
            remove(temp_path);
            db->file = fopen(db->path, "rb+");
            if (!db->file) return FOSSIL_MYSHELL_ERROR_IO;
            return FOSSIL_MYSHELL_ERROR_IO;
        }
        if (rename(temp_path, db->path) != 0) {
            db->file = fopen(db->path, "rb+");
            if (!db->file) return FOSSIL_MYSHELL_ERROR_IO;
            return FOSSIL_MYSHELL_ERROR_IO;
        }
        db->file = fopen(db->path, "rb+");
        if (!db->file) {
            return FOSSIL_MYSHELL_ERROR_IO;
        }
        db->last_modified = time(NULL);
        return FOSSIL_MYSHELL_ERROR_SUCCESS;
    } else {
        remove(temp_path); // No change
        db->file = fopen(db->path, "rb+");
        if (!db->file) {
            return FOSSIL_MYSHELL_ERROR_IO;
        }
        return FOSSIL_MYSHELL_ERROR_NOT_FOUND;
    }
}

fossil_bluecrab_myshell_error_t fossil_myshell_commit(fossil_bluecrab_myshell_t *db, const char *message) {
    if (!db) {
        return FOSSIL_MYSHELL_ERROR_INVALID_FILE;
    }
    if (!db->is_open) {
        return FOSSIL_MYSHELL_ERROR_LOCKED;
    }
    if (!message) {
        return FOSSIL_MYSHELL_ERROR_INVALID_QUERY;
    }
    if (strlen(message) == 0) {
        return FOSSIL_MYSHELL_ERROR_INVALID_QUERY;
    }

    // Store the commit message
    if (db->commit_message) {
        free(db->commit_message);
    }
    db->commit_message = myshell_strdup(message);
    if (!db->commit_message) {
        return FOSSIL_MYSHELL_ERROR_OUT_OF_MEMORY;
    }
    db->commit_timestamp = time(NULL);

    // Prepare commit data for hashing
    char commit_data[1024];
    if (snprintf(commit_data, sizeof(commit_data), "%s:%lld", message, (long long)db->commit_timestamp) < 0) {
        return FOSSIL_MYSHELL_ERROR_IO;
    }

    // Update commit hashes (chain)
    db->prev_commit_hash = db->commit_head;
    db->commit_head = myshell_hash64(commit_data);

    // Optionally, create a new commit object (simulate by updating author and parent_branch)
    if (db->author) {
        free(db->author);
    }
    db->author = myshell_strdup("system");
    if (!db->author) {
        return FOSSIL_MYSHELL_ERROR_OUT_OF_MEMORY;
    }

    if (db->parent_branch) {
        free(db->parent_branch);
    }
    db->parent_branch = db->branch ? myshell_strdup(db->branch) : NULL;
    if (db->branch && !db->parent_branch) {
        return FOSSIL_MYSHELL_ERROR_OUT_OF_MEMORY;
    }

    db->next_commit_hash = 0;

    // Write commit info to the file for history (simple append)
    if (fseek(db->file, 0, SEEK_END) != 0) {
        return FOSSIL_MYSHELL_ERROR_IO;
    }
    if (fprintf(db->file, "#commit %016llx %s %lld\n", db->commit_head, message, (long long)db->commit_timestamp) < 0) {
        return FOSSIL_MYSHELL_ERROR_IO;
    }
    fflush(db->file);

    db->last_modified = time(NULL);

    return FOSSIL_MYSHELL_ERROR_SUCCESS;
}

fossil_bluecrab_myshell_error_t fossil_myshell_branch(fossil_bluecrab_myshell_t *db, const char *branch_name) {
    if (!db) {
        return FOSSIL_MYSHELL_ERROR_INVALID_FILE;
    }
    if (!db->is_open) {
        return FOSSIL_MYSHELL_ERROR_LOCKED;
    }
    if (!branch_name || branch_name[0] == '\0') {
        return FOSSIL_MYSHELL_ERROR_INVALID_QUERY;
    }

    // Check for schema mismatch or unsupported version (simulate)
    if (db->commit_head == 0) {
        return FOSSIL_MYSHELL_ERROR_SCHEMA_MISMATCH;
    }

    // Update branch pointer
    if (db->branch) {
        free(db->branch);
    }
    db->branch = myshell_strdup(branch_name);
    if (!db->branch) {
        return FOSSIL_MYSHELL_ERROR_OUT_OF_MEMORY;
    }

    // Set parent branch if not already set
    if (!db->parent_branch) {
        db->parent_branch = myshell_strdup(db->branch);
        if (!db->parent_branch) {
            return FOSSIL_MYSHELL_ERROR_OUT_OF_MEMORY;
        }
    }

    // Update commit_head to branch hash
    db->commit_head = myshell_hash64(branch_name);

    // Optionally, write branch info to the file for history (simple append)
    if (fseek(db->file, 0, SEEK_END) != 0) {
        return FOSSIL_MYSHELL_ERROR_IO;
    }
    if (fprintf(db->file, "#branch %016llx %s\n", db->commit_head, branch_name) < 0) {
        return FOSSIL_MYSHELL_ERROR_IO;
    }
    fflush(db->file);

    db->last_modified = time(NULL);

    // Update branch pointers and commit chain (simple simulation)
    db->prev_commit_hash = db->commit_head;
    db->next_commit_hash = 0; // No next commit yet

    // Optionally, reset commit message and timestamp for new branch
    if (db->commit_message) {
        free(db->commit_message);
        db->commit_message = NULL;
    }
    db->commit_timestamp = 0;

    return FOSSIL_MYSHELL_ERROR_SUCCESS;
}

fossil_bluecrab_myshell_error_t fossil_myshell_checkout(fossil_bluecrab_myshell_t *db, const char *branch_or_commit) {
    if (!db) {
        return FOSSIL_MYSHELL_ERROR_INVALID_FILE;
    }
    if (!db->is_open) {
        return FOSSIL_MYSHELL_ERROR_LOCKED;
    }
    if (!branch_or_commit || branch_or_commit[0] == '\0') {
        return FOSSIL_MYSHELL_ERROR_INVALID_QUERY;
    }

    uint64_t hash = myshell_hash64(branch_or_commit);

    bool branch_found = false;
    bool commit_found = false;
    fseek(db->file, 0, SEEK_SET);
    char line[1024];
    while (fgets(line, sizeof(line), db->file)) {
        if (strncmp(line, "#branch ", 8) == 0) {
            char hash_str[17] = {0};
            char name[512] = {0};
            int n = sscanf(line, "#branch %16s %511[^\n]", hash_str, name);
            if (n >= 2) {
                uint64_t parsed_hash = 0;
                sscanf(hash_str, "%llx", &parsed_hash);
                if ((strcmp(name, branch_or_commit) == 0) || (parsed_hash == hash)) {
                    branch_found = true;
                    break;
                }
            }
        } else if (strncmp(line, "#commit ", 8) == 0) {
            char hash_str[17] = {0};
            int n = sscanf(line, "#commit %16s", hash_str);
            if (n == 1) {
                uint64_t parsed_hash = 0;
                sscanf(hash_str, "%llx", &parsed_hash);
                if (parsed_hash == hash) {
                    commit_found = true;
                    break;
                }
            }
        }
    }

    if (!branch_found && !commit_found) {
        return FOSSIL_MYSHELL_ERROR_NOT_FOUND;
    }

    // Set branch name and update commit_head to hash
    if (db->branch) {
        free(db->branch);
    }
    db->branch = myshell_strdup(branch_or_commit);
    if (!db->branch) {
        return FOSSIL_MYSHELL_ERROR_OUT_OF_MEMORY;
    }
    db->commit_head = hash;

    db->last_modified = time(NULL);

    return FOSSIL_MYSHELL_ERROR_SUCCESS;
}

fossil_bluecrab_myshell_error_t fossil_myshell_log(fossil_bluecrab_myshell_t *db, fossil_myshell_commit_cb cb, void *user) {
    if (!db) {
        return FOSSIL_MYSHELL_ERROR_INVALID_FILE;
    }
    if (!db->is_open) {
        return FOSSIL_MYSHELL_ERROR_LOCKED;
    }
    if (!cb) {
        return FOSSIL_MYSHELL_ERROR_INVALID_QUERY;
    }

    // Iterate over file and invoke callback for each commit line
    if (fseek(db->file, 0, SEEK_SET) != 0) {
        return FOSSIL_MYSHELL_ERROR_IO;
    }
    char line[1024];
    while (fgets(line, sizeof(line), db->file)) {
        if (strncmp(line, "#commit ", 8) == 0) {
            char hash_str[17] = {0};
            char message[512] = {0};
            long long timestamp = 0;

            // Parse: "#commit %016llx %s %lld\n"
            int n = sscanf(line, "#commit %16s %511[^\n] %lld", hash_str, message, &timestamp);
            if (n >= 2) {
                uint64_t parsed_hash = 0;
                sscanf(hash_str, "%llx", &parsed_hash);

                if (n == 3) {
                    char commit_data[1024];
                    snprintf(commit_data, sizeof(commit_data), "%s:%lld", message, timestamp);
                    uint64_t computed_hash = myshell_hash64(commit_data);
                    if (parsed_hash == computed_hash) {
                        if (!cb(hash_str, message, user)) {
                            return FOSSIL_MYSHELL_ERROR_SUCCESS;
                        }
                    } else {
                        // Hash mismatch, integrity error
                        return FOSSIL_MYSHELL_ERROR_INTEGRITY;
                    }
                } else {
                    if (!cb(hash_str, message, user)) {
                        return FOSSIL_MYSHELL_ERROR_SUCCESS;
                    }
                }
            } else {
                // Parse failed
                return FOSSIL_MYSHELL_ERROR_PARSE_FAILED;
            }
        }
    }

    return FOSSIL_MYSHELL_ERROR_SUCCESS;
}

fossil_bluecrab_myshell_error_t fossil_myshell_backup(fossil_bluecrab_myshell_t *db, const char *backup_path) {
    if (!db || !db->is_open) {
        return FOSSIL_MYSHELL_ERROR_INVALID_FILE;
    }
    if (!backup_path || backup_path[0] == '\0') {
        return FOSSIL_MYSHELL_ERROR_CONFIG_INVALID;
    }

    FILE *backup_file = fopen(backup_path, "wb");
    if (!backup_file) {
        return FOSSIL_MYSHELL_ERROR_BACKUP_FAILED;
    }

    // Write a hash of the backup path as a comment for integrity
    uint64_t backup_hash = myshell_hash64(backup_path);
    if (fprintf(backup_file, "#backup_hash=%016llx\n", backup_hash) < 0) {
        fclose(backup_file);
        return FOSSIL_MYSHELL_ERROR_IO;
    }

    fseek(db->file, 0, SEEK_SET);
    char buffer[4096];
    size_t bytes;
    while ((bytes = fread(buffer, 1, sizeof(buffer), db->file)) > 0) {
        if (fwrite(buffer, 1, bytes, backup_file) != bytes) {
            fclose(backup_file);
            return FOSSIL_MYSHELL_ERROR_IO;
        }
    }

    if (ferror(db->file)) {
        fclose(backup_file);
        return FOSSIL_MYSHELL_ERROR_IO;
    }

    fclose(backup_file);
    fseek(db->file, 0, SEEK_END); // Restore file position
    return FOSSIL_MYSHELL_ERROR_SUCCESS;
}

fossil_bluecrab_myshell_error_t fossil_myshell_restore(const char *backup_path, const char *target_path) {
    if (!backup_path || !target_path) {
        return FOSSIL_MYSHELL_ERROR_INVALID_FILE;
    }

    FILE *backup_file = fopen(backup_path, "rb");
    if (!backup_file) {
        return FOSSIL_MYSHELL_ERROR_FILE_NOT_FOUND;
    }

    // Verify backup hash for integrity (first line should be "#backup_hash=...")
    char hash_line[128];
    uint64_t expected_hash = myshell_hash64(backup_path);
    if (fgets(hash_line, sizeof(hash_line), backup_file)) {
        uint64_t file_hash = 0;
        if (strncmp(hash_line, "#backup_hash=", 13) == 0) {
            sscanf(hash_line, "#backup_hash=%llx", &file_hash);
            if (file_hash != expected_hash) {
                fclose(backup_file);
                return FOSSIL_MYSHELL_ERROR_INTEGRITY;
            }
        } else {
            // If no hash line, treat as corrupted
            fclose(backup_file);
            return FOSSIL_MYSHELL_ERROR_CORRUPTED;
        }
    } else {
        fclose(backup_file);
        return FOSSIL_MYSHELL_ERROR_CORRUPTED;
    }

    FILE *target_file = fopen(target_path, "wb");
    if (!target_file) {
        fclose(backup_file);
        return FOSSIL_MYSHELL_ERROR_IO;
    }

    char buffer[4096];
    size_t bytes;
    while ((bytes = fread(buffer, 1, sizeof(buffer), backup_file)) > 0) {
        if (fwrite(buffer, 1, bytes, target_file) != bytes) {
            fclose(backup_file);
            fclose(target_file);
            return FOSSIL_MYSHELL_ERROR_IO;
        }
    }

    if (ferror(backup_file)) {
        fclose(backup_file);
        fclose(target_file);
        return FOSSIL_MYSHELL_ERROR_IO;
    }

    fclose(backup_file);
    fclose(target_file);
    return FOSSIL_MYSHELL_ERROR_SUCCESS;
}

const char *fossil_myshell_errstr(fossil_bluecrab_myshell_error_t err) {
    switch (err) {
        case FOSSIL_MYSHELL_ERROR_SUCCESS: return "Success";
        case FOSSIL_MYSHELL_ERROR_INVALID_FILE: return "Invalid file";
        case FOSSIL_MYSHELL_ERROR_FILE_NOT_FOUND: return "File not found";
        case FOSSIL_MYSHELL_ERROR_IO: return "I/O error";
        case FOSSIL_MYSHELL_ERROR_INVALID_QUERY: return "Invalid query";
        case FOSSIL_MYSHELL_ERROR_CONCURRENCY: return "Concurrency error";
        case FOSSIL_MYSHELL_ERROR_NOT_FOUND: return "Not found";
        case FOSSIL_MYSHELL_ERROR_PERMISSION_DENIED: return "Permission denied";
        case FOSSIL_MYSHELL_ERROR_CORRUPTED: return "Corrupted data";
        case FOSSIL_MYSHELL_ERROR_OUT_OF_MEMORY: return "Out of memory";
        case FOSSIL_MYSHELL_ERROR_UNSUPPORTED: return "Unsupported operation";
        case FOSSIL_MYSHELL_ERROR_LOCKED: return "Resource locked";
        case FOSSIL_MYSHELL_ERROR_TIMEOUT: return "Operation timed out";
        case FOSSIL_MYSHELL_ERROR_ALREADY_EXISTS: return "Already exists";
        case FOSSIL_MYSHELL_ERROR_BACKUP_FAILED: return "Backup failed";
        case FOSSIL_MYSHELL_ERROR_PARSE_FAILED: return "Parse failed";
        case FOSSIL_MYSHELL_ERROR_RESTORE_FAILED: return "Restore failed";
        case FOSSIL_MYSHELL_ERROR_LOCK_FAILED: return "Lock failed";
        case FOSSIL_MYSHELL_ERROR_SCHEMA_MISMATCH: return "Schema mismatch";
        case FOSSIL_MYSHELL_ERROR_VERSION_UNSUPPORTED: return "Version unsupported";
        case FOSSIL_MYSHELL_ERROR_INDEX_CORRUPTED: return "Index corrupted";
        case FOSSIL_MYSHELL_ERROR_INTEGRITY: return "Integrity check failed";
        case FOSSIL_MYSHELL_ERROR_TRANSACTION_FAILED: return "Transaction failed";
        case FOSSIL_MYSHELL_ERROR_CAPACITY_EXCEEDED: return "Capacity exceeded";
        case FOSSIL_MYSHELL_ERROR_CONFIG_INVALID: return "Invalid configuration";
        case FOSSIL_MYSHELL_ERROR_UNKNOWN:
        default:
            return "Unknown error";
    }
}

fossil_bluecrab_myshell_error_t fossil_myshell_check_integrity(fossil_bluecrab_myshell_t *db) {
    if (!db || !db->is_open) {
        return FOSSIL_MYSHELL_ERROR_INVALID_FILE;
    }

    // Check file size consistency
    if (fseek(db->file, 0, SEEK_END) != 0)
        return FOSSIL_MYSHELL_ERROR_IO;
    size_t current_size = (size_t)ftell(db->file);
    if (current_size != db->file_size) {
        return FOSSIL_MYSHELL_ERROR_CORRUPTED;
    }
    if (fseek(db->file, 0, SEEK_SET) != 0)
        return FOSSIL_MYSHELL_ERROR_IO;

    // Verify commit hash chain integrity
    char line[1024];
    while (fgets(line, sizeof(line), db->file)) {
        if (strncmp(line, "#commit ", 8) == 0) {
            char hash_str[17] = {0};
            char message[512] = {0};
            long long timestamp = 0;
            int n = sscanf(line, "#commit %16s %511[^\n] %lld", hash_str, message, &timestamp);
            if (n == 3) {
                uint64_t parsed_hash = 0;
                sscanf(hash_str, "%llx", &parsed_hash);
                char commit_data[1024];
                snprintf(commit_data, sizeof(commit_data), "%s:%lld", message, timestamp);
                uint64_t computed_hash = myshell_hash64(commit_data);
                if (parsed_hash != computed_hash) {
                    return FOSSIL_MYSHELL_ERROR_INTEGRITY;
                }
            } else {
                return FOSSIL_MYSHELL_ERROR_PARSE_FAILED;
            }
        }
        // Additional integrity checks: verify key-value hash
        else {
            char *eq = strchr(line, '=');
            if (eq) {
                *eq = '\0';
                char *hash_comment = strstr(eq + 1, "#hash=");
                if (hash_comment) {
                    uint64_t file_hash = 0;
                    sscanf(hash_comment, "#hash=%llx", &file_hash);
                    uint64_t key_hash = myshell_hash64(line);
                    if (file_hash != key_hash) {
                        *eq = '='; // Restore
                        return FOSSIL_MYSHELL_ERROR_INTEGRITY;
                    }
                }
                *eq = '='; // Restore
            }
        }
    }

    return FOSSIL_MYSHELL_ERROR_SUCCESS;
}

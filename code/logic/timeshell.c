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
#include "fossil/crabdb/timeshell.h"

#define TIMESHELL_LINE_MAX 256
#define TIMESHELL_TEMP_FILE "tmp_timeshell.crabdb"

// ============================================================================
// Internal 64-bit enhanced hash (FNV-1a + mixing)
// ============================================================================
static uint64_t fossil_bluecrab_hash64(const char *str) {
    uint64_t hash = 14695981039346656037ULL; // FNV-1a offset basis
    unsigned char c;
    while ((c = (unsigned char)*str++)) {
        hash ^= c;
        hash *= 1099511628211ULL; // FNV-1a prime
    }
    // final mixing (MurmurHash3-inspired)
    hash ^= hash >> 33;
    hash *= 0xff51afd7ed558ccdULL;
    hash ^= hash >> 33;
    hash *= 0xc4ceb9fe1a85ec53ULL;
    hash ^= hash >> 33;
    return hash;
}

// ============================================================================
// Internal helpers for parsing and formatting lines
// Format: "<start>,<end>|<hash>\n"
// ============================================================================
static bool timeshell_split_line(char *line,
                                 uint64_t *out_start,
                                 uint64_t *out_end,
                                 uint64_t *out_stored_hash)
{
    // trim newline
    size_t len = strlen(line);
    if (len && (line[len-1] == '\n' || line[len-1] == '\r')) {
        line[len-1] = '\0';
        len--;
        if (len && (line[len-1] == '\r')) { line[len-1] = '\0'; len--; }
    }

    char *pipe = strrchr(line, '|');
    if (!pipe) return false;
    *pipe = '\0';
    char *hash_str = pipe + 1;

    // parse stored hash
    errno = 0;
    uint64_t stored_hash = strtoull(hash_str, NULL, 10);
    if (errno != 0) return false;

    // split start,end
    char *comma = strchr(line, ',');
    if (!comma) return false;
    *comma = '\0';
    char *start_str = line;
    char *end_str = comma + 1;

    errno = 0;
    uint64_t start = strtoull(start_str, NULL, 10);
    if (errno != 0) return false;
    errno = 0;
    uint64_t end = strtoull(end_str, NULL, 10);
    if (errno != 0) return false;

    *out_start = start;
    *out_end = end;
    *out_stored_hash = stored_hash;
    return true;
}

static void timeshell_format_record(char *buf, size_t bufsize, uint64_t start, uint64_t end) {
    // format "<start>,<end>"
    snprintf(buf, bufsize, "%llu,%llu",
             (unsigned long long)start,
             (unsigned long long)end);
}

// ============================================================================
// Validation
// ============================================================================
bool fossil_bluecrab_timeshell_validate_extension(const char *file_name) {
    if (!file_name) return false;
    const char *ext = strrchr(file_name, '.');
    return ext && strcmp(ext, ".crabdb") == 0;
}

bool fossil_bluecrab_timeshell_validate_interval(const fossil_timeshell_interval_t *interval) {
    if (!interval) return false;
    return interval->start_time < interval->end_time;
}

// ============================================================================
// Database management
// ============================================================================
fossil_timeshell_error_t fossil_bluecrab_timeshell_create_database(const char *file_name) {
    if (!fossil_bluecrab_timeshell_validate_extension(file_name))
        return FOSSIL_TIMESHELL_ERROR_INVALID_FILE;

    FILE *fp = fopen(file_name, "w");
    if (!fp) return FOSSIL_TIMESHELL_ERROR_IO;
    fclose(fp);
    return FOSSIL_TIMESHELL_ERROR_SUCCESS;
}

fossil_timeshell_error_t fossil_bluecrab_timeshell_open_database(const char *file_name) {
    if (!fossil_bluecrab_timeshell_validate_extension(file_name))
        return FOSSIL_TIMESHELL_ERROR_INVALID_FILE;

    FILE *fp = fopen(file_name, "r");
    if (!fp) return FOSSIL_TIMESHELL_ERROR_FILE_NOT_FOUND;
    fclose(fp);
    return FOSSIL_TIMESHELL_ERROR_SUCCESS;
}

fossil_timeshell_error_t fossil_bluecrab_timeshell_delete_database(const char *file_name) {
    if (!fossil_bluecrab_timeshell_validate_extension(file_name))
        return FOSSIL_TIMESHELL_ERROR_INVALID_FILE;

    if (remove(file_name) == 0) return FOSSIL_TIMESHELL_ERROR_SUCCESS;
    return FOSSIL_TIMESHELL_ERROR_IO;
}

// ============================================================================
// Insert
// Each record: "<start>,<end>|<hash>\n"
// hash = fossil_bluecrab_hash64("<start>,<end>")
// ============================================================================
fossil_timeshell_error_t fossil_bluecrab_timeshell_insert(const char *file_name, const fossil_timeshell_interval_t *interval) {
    if (!file_name || !interval) return FOSSIL_TIMESHELL_ERROR_INVALID_INTERVAL;
    if (!fossil_bluecrab_timeshell_validate_extension(file_name))
        return FOSSIL_TIMESHELL_ERROR_INVALID_FILE;
    if (!fossil_bluecrab_timeshell_validate_interval(interval))
        return FOSSIL_TIMESHELL_ERROR_INVALID_INTERVAL;

    FILE *fp = fopen(file_name, "a");
    if (!fp) return FOSSIL_TIMESHELL_ERROR_IO;

    char record[TIMESHELL_LINE_MAX];
    timeshell_format_record(record, sizeof(record), interval->start_time, interval->end_time);
    uint64_t hash = fossil_bluecrab_hash64(record);

    fprintf(fp, "%s|%llu\n", record, (unsigned long long)hash);
    fclose(fp);
    return FOSSIL_TIMESHELL_ERROR_SUCCESS;
}

// ============================================================================
// Find overlaps
// Finds up to max_results intervals that overlap the query interval.
// Overlap condition: (a.start < b.end) && (b.start < a.end)
// ============================================================================
fossil_timeshell_error_t fossil_bluecrab_timeshell_find(
    const char *file_name,
    const fossil_timeshell_interval_t *query,
    fossil_timeshell_interval_t *results,
    size_t max_results,
    size_t *out_count)
{
    if (!file_name || !query || !results || !out_count) return FOSSIL_TIMESHELL_ERROR_INVALID_INTERVAL;
    if (!fossil_bluecrab_timeshell_validate_extension(file_name))
        return FOSSIL_TIMESHELL_ERROR_INVALID_FILE;
    if (!fossil_bluecrab_timeshell_validate_interval(query))
        return FOSSIL_TIMESHELL_ERROR_INVALID_INTERVAL;

    FILE *fp = fopen(file_name, "r");
    if (!fp) return FOSSIL_TIMESHELL_ERROR_FILE_NOT_FOUND;

    size_t found = 0;
    char line[TIMESHELL_LINE_MAX];
    while (fgets(line, sizeof(line), fp)) {
        uint64_t start, end, stored_hash;
        if (!timeshell_split_line(line, &start, &end, &stored_hash)) {
            fclose(fp);
            return FOSSIL_TIMESHELL_ERROR_UNKNOWN;
        }

        // verify hash
        char record[TIMESHELL_LINE_MAX];
        timeshell_format_record(record, sizeof(record), start, end);
        uint64_t calc_hash = fossil_bluecrab_hash64(record);
        if (calc_hash != stored_hash) {
            fclose(fp);
            return FOSSIL_TIMESHELL_ERROR_CORRUPTED;
        }

        // overlap test
        if ((start < query->end_time) && (query->start_time < end)) {
            if (found < max_results) {
                results[found].start_time = start;
                results[found].end_time = end;
            }
            found++;
        }
    }

    fclose(fp);
    *out_count = (found <= max_results) ? found : found; // caller can determine if truncated
    return (found == 0) ? FOSSIL_TIMESHELL_ERROR_NOT_FOUND : FOSSIL_TIMESHELL_ERROR_SUCCESS;
}

// ============================================================================
// Update (exact match on start & end) - replaces the first exact match
// ============================================================================
fossil_timeshell_error_t fossil_bluecrab_timeshell_update(
    const char *file_name,
    const fossil_timeshell_interval_t *old_interval,
    const fossil_timeshell_interval_t *new_interval)
{
    if (!file_name || !old_interval || !new_interval) return FOSSIL_TIMESHELL_ERROR_INVALID_INTERVAL;
    if (!fossil_bluecrab_timeshell_validate_extension(file_name))
        return FOSSIL_TIMESHELL_ERROR_INVALID_FILE;
    if (!fossil_bluecrab_timeshell_validate_interval(old_interval) ||
        !fossil_bluecrab_timeshell_validate_interval(new_interval))
        return FOSSIL_TIMESHELL_ERROR_INVALID_INTERVAL;

    FILE *fp = fopen(file_name, "r");
    if (!fp) return FOSSIL_TIMESHELL_ERROR_FILE_NOT_FOUND;

    FILE *tmp = fopen(TIMESHELL_TEMP_FILE, "w");
    if (!tmp) { fclose(fp); return FOSSIL_TIMESHELL_ERROR_IO; }

    char line[TIMESHELL_LINE_MAX];
    bool updated = false;
    while (fgets(line, sizeof(line), fp)) {
        uint64_t start, end, stored_hash;
        if (!timeshell_split_line(line, &start, &end, &stored_hash)) {
            // preserve invalid/untokenizable line as-is (safer), but we could fail
            fputs(line, tmp);
            continue;
        }

        if (!updated && start == old_interval->start_time && end == old_interval->end_time) {
            // write new interval
            char record[TIMESHELL_LINE_MAX];
            timeshell_format_record(record, sizeof(record), new_interval->start_time, new_interval->end_time);
            uint64_t new_hash = fossil_bluecrab_hash64(record);
            fprintf(tmp, "%s|%llu\n", record, (unsigned long long)new_hash);
            updated = true;
        } else {
            // preserve original (reformat to normalized line)
            char record[TIMESHELL_LINE_MAX];
            timeshell_format_record(record, sizeof(record), start, end);
            fprintf(tmp, "%s|%llu\n", record, (unsigned long long)stored_hash);
        }
    }

    fclose(fp);
    fclose(tmp);

    if (updated) {
        if (remove(file_name) != 0) { remove(TIMESHELL_TEMP_FILE); return FOSSIL_TIMESHELL_ERROR_IO; }
        if (rename(TIMESHELL_TEMP_FILE, file_name) != 0) { remove(TIMESHELL_TEMP_FILE); return FOSSIL_TIMESHELL_ERROR_IO; }
        return FOSSIL_TIMESHELL_ERROR_SUCCESS;
    } else {
        remove(TIMESHELL_TEMP_FILE);
        return FOSSIL_TIMESHELL_ERROR_NOT_FOUND;
    }
}

// ============================================================================
// Remove (exact match on start & end) - removes first exact match
// ============================================================================
fossil_timeshell_error_t fossil_bluecrab_timeshell_remove(
    const char *file_name,
    const fossil_timeshell_interval_t *interval)
{
    if (!file_name || !interval) return FOSSIL_TIMESHELL_ERROR_INVALID_INTERVAL;
    if (!fossil_bluecrab_timeshell_validate_extension(file_name))
        return FOSSIL_TIMESHELL_ERROR_INVALID_FILE;
    if (!fossil_bluecrab_timeshell_validate_interval(interval))
        return FOSSIL_TIMESHELL_ERROR_INVALID_INTERVAL;

    FILE *fp = fopen(file_name, "r");
    if (!fp) return FOSSIL_TIMESHELL_ERROR_FILE_NOT_FOUND;

    FILE *tmp = fopen(TIMESHELL_TEMP_FILE, "w");
    if (!tmp) { fclose(fp); return FOSSIL_TIMESHELL_ERROR_IO; }

    char line[TIMESHELL_LINE_MAX];
    bool removed = false;
    while (fgets(line, sizeof(line), fp)) {
        uint64_t start, end, stored_hash;
        if (!timeshell_split_line(line, &start, &end, &stored_hash)) {
            fputs(line, tmp);
            continue;
        }

        if (!removed && start == interval->start_time && end == interval->end_time) {
            removed = true;
            continue; // skip writing this line
        }

        char record[TIMESHELL_LINE_MAX];
        timeshell_format_record(record, sizeof(record), start, end);
        fprintf(tmp, "%s|%llu\n", record, (unsigned long long)stored_hash);
    }

    fclose(fp);
    fclose(tmp);

    if (removed) {
        if (remove(file_name) != 0) { remove(TIMESHELL_TEMP_FILE); return FOSSIL_TIMESHELL_ERROR_IO; }
        if (rename(TIMESHELL_TEMP_FILE, file_name) != 0) { remove(TIMESHELL_TEMP_FILE); return FOSSIL_TIMESHELL_ERROR_IO; }
        return FOSSIL_TIMESHELL_ERROR_SUCCESS;
    } else {
        remove(TIMESHELL_TEMP_FILE);
        return FOSSIL_TIMESHELL_ERROR_NOT_FOUND;
    }
}

// ============================================================================
// Backup / Restore (verify hashes while copying)
// ============================================================================
fossil_timeshell_error_t fossil_bluecrab_timeshell_backup_database(const char *source_file, const char *backup_file) {
    if (!fossil_bluecrab_timeshell_validate_extension(source_file) ||
        !fossil_bluecrab_timeshell_validate_extension(backup_file))
        return FOSSIL_TIMESHELL_ERROR_INVALID_FILE;

    FILE *src = fopen(source_file, "r");
    if (!src) return FOSSIL_TIMESHELL_ERROR_FILE_NOT_FOUND;

    FILE *dst = fopen(backup_file, "w");
    if (!dst) { fclose(src); return FOSSIL_TIMESHELL_ERROR_IO; }

    char line[TIMESHELL_LINE_MAX];
    while (fgets(line, sizeof(line), src)) {
        uint64_t start, end, stored_hash;
        if (!timeshell_split_line(line, &start, &end, &stored_hash)) {
            fclose(src); fclose(dst); remove(backup_file);
            return FOSSIL_TIMESHELL_ERROR_CORRUPTED;
        }

        char record[TIMESHELL_LINE_MAX];
        timeshell_format_record(record, sizeof(record), start, end);
        uint64_t calc_hash = fossil_bluecrab_hash64(record);
        if (calc_hash != stored_hash) {
            fclose(src); fclose(dst); remove(backup_file);
            return FOSSIL_TIMESHELL_ERROR_CORRUPTED;
        }

        fprintf(dst, "%s|%llu\n", record, (unsigned long long)stored_hash);
    }

    fclose(src);
    fclose(dst);
    return FOSSIL_TIMESHELL_ERROR_SUCCESS;
}

fossil_timeshell_error_t fossil_bluecrab_timeshell_restore_database(const char *backup_file, const char *destination_file) {
    // same pattern as backup (verify then write)
    if (!fossil_bluecrab_timeshell_validate_extension(backup_file) ||
        !fossil_bluecrab_timeshell_validate_extension(destination_file))
        return FOSSIL_TIMESHELL_ERROR_INVALID_FILE;

    FILE *src = fopen(backup_file, "r");
    if (!src) return FOSSIL_TIMESHELL_ERROR_FILE_NOT_FOUND;

    FILE *dst = fopen(destination_file, "w");
    if (!dst) { fclose(src); return FOSSIL_TIMESHELL_ERROR_IO; }

    char line[TIMESHELL_LINE_MAX];
    while (fgets(line, sizeof(line), src)) {
        uint64_t start, end, stored_hash;
        if (!timeshell_split_line(line, &start, &end, &stored_hash)) {
            fclose(src); fclose(dst); remove(destination_file);
            return FOSSIL_TIMESHELL_ERROR_CORRUPTED;
        }

        char record[TIMESHELL_LINE_MAX];
        timeshell_format_record(record, sizeof(record), start, end);
        uint64_t calc_hash = fossil_bluecrab_hash64(record);
        if (calc_hash != stored_hash) {
            fclose(src); fclose(dst); remove(destination_file);
            return FOSSIL_TIMESHELL_ERROR_CORRUPTED;
        }

        fprintf(dst, "%s|%llu\n", record, (unsigned long long)stored_hash);
    }

    fclose(src);
    fclose(dst);
    return FOSSIL_TIMESHELL_ERROR_SUCCESS;
}

// ============================================================================
// Verify database (scan all records, fail at first corrupted line)
// ============================================================================
fossil_timeshell_error_t fossil_bluecrab_timeshell_verify_database(const char *file_name) {
    if (!fossil_bluecrab_timeshell_validate_extension(file_name))
        return FOSSIL_TIMESHELL_ERROR_INVALID_FILE;

    FILE *fp = fopen(file_name, "r");
    if (!fp) return FOSSIL_TIMESHELL_ERROR_FILE_NOT_FOUND;

    char line[TIMESHELL_LINE_MAX];
    while (fgets(line, sizeof(line), fp)) {
        uint64_t start, end, stored_hash;
        if (!timeshell_split_line(line, &start, &end, &stored_hash)) {
            fclose(fp);
            return FOSSIL_TIMESHELL_ERROR_CORRUPTED;
        }

        char record[TIMESHELL_LINE_MAX];
        timeshell_format_record(record, sizeof(record), start, end);
        uint64_t calc_hash = fossil_bluecrab_hash64(record);
        if (calc_hash != stored_hash) {
            fclose(fp);
            return FOSSIL_TIMESHELL_ERROR_CORRUPTED;
        }
    }

    fclose(fp);
    return FOSSIL_TIMESHELL_ERROR_SUCCESS;
}

// ============================================================================
// Metadata helpers
// ============================================================================
fossil_timeshell_error_t fossil_bluecrab_timeshell_count_intervals(const char *file_name, size_t *count) {
    if (!file_name || !count) return FOSSIL_TIMESHELL_ERROR_INVALID_FILE;
    if (!fossil_bluecrab_timeshell_validate_extension(file_name))
        return FOSSIL_TIMESHELL_ERROR_INVALID_FILE;

    FILE *fp = fopen(file_name, "r");
    if (!fp) return FOSSIL_TIMESHELL_ERROR_FILE_NOT_FOUND;

    size_t c = 0;
    char line[TIMESHELL_LINE_MAX];
    while (fgets(line, sizeof(line), fp)) {
        // count lines that look like valid records
        uint64_t s, e, h;
        if (timeshell_split_line(line, &s, &e, &h)) c++;
    }

    fclose(fp);
    *count = c;
    return FOSSIL_TIMESHELL_ERROR_SUCCESS;
}

fossil_timeshell_error_t fossil_bluecrab_timeshell_get_file_size(const char *file_name, size_t *size_bytes) {
    if (!file_name || !size_bytes) return FOSSIL_TIMESHELL_ERROR_INVALID_FILE;
    if (!fossil_bluecrab_timeshell_validate_extension(file_name))
        return FOSSIL_TIMESHELL_ERROR_INVALID_FILE;

    struct stat st;
    if (stat(file_name, &st) != 0) return FOSSIL_TIMESHELL_ERROR_FILE_NOT_FOUND;
    *size_bytes = (size_t)st.st_size;
    return FOSSIL_TIMESHELL_ERROR_SUCCESS;
}

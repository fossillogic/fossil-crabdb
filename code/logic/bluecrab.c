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
#include "fossil/crabdb/bluecrab.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

int fossil_bluecrab_init(fossil_bluecrab_t *crab, const char *protocol, const char *crabfile_path, const char *storage_path) {
    if (!crab || !protocol || !crabfile_path || !storage_path) {
        return -1; // Invalid parameters
    }

    strncpy(crab->protocol, protocol, FOSSIL_BLUECRAB_MAX_PROTOCOL_NAME - 1);
    crab->protocol[FOSSIL_BLUECRAB_MAX_PROTOCOL_NAME - 1] = '\0';

    strncpy(crab->crabfile_path, crabfile_path, FOSSIL_BLUECRAB_MAX_PATH - 1);
    crab->crabfile_path[FOSSIL_BLUECRAB_MAX_PATH - 1] = '\0';

    strncpy(crab->storage_path, storage_path, FOSSIL_BLUECRAB_MAX_PATH - 1);
    crab->storage_path[FOSSIL_BLUECRAB_MAX_PATH - 1] = '\0';

    crab->last_block_index = 0;
    crab->field_count = 0;
    crab->initialized = false;
    crab->writable = true;
    crab->chain = NULL;
    crab->chain_length = 0;
    crab->protocol_context = NULL;
    memset(crab->last_error, 0, sizeof(crab->last_error));
    crab->validation_hook = NULL;
    crab->scratch_buffer = NULL;
    crab->scratch_buffer_size = 0;

    return 0; // Success
}

int fossil_bluecrab_shutdown(fossil_bluecrab_t *crab) {
    if (!crab) {
        return -1; // Invalid parameter
    }

    // Free any dynamically allocated resources
    free(crab->chain);
    free(crab->scratch_buffer);
    memset(crab, 0, sizeof(fossil_bluecrab_t)); // Reset internal state

    return 0; // Success
}

int fossil_bluecrab_load_crabfile(fossil_bluecrab_t *crab) {
    if (!crab || !crab->crabfile_path) {
        return -1; // Invalid parameter
    }

    FILE *file = fopen(crab->crabfile_path, "r");
    if (!file) {
        return -2; // Failed to open crabfile
    }

    char line[512];
    size_t field_count = 0;

    // Simple parser: extract field names from table()/document()/collection() blocks
    while (fgets(line, sizeof(line), file)) {
        // Skip comments and empty lines
        char *trim = line;
        while (*trim == ' ' || *trim == '\t') trim++;
        if (*trim == '#' || *trim == '\n' || *trim == '\0')
            continue;

        // Detect table, document, or collection blocks
        if (strstr(trim, "table(") || strstr(trim, "document(") || strstr(trim, "collection(")) {
            // Inside a block, look for fields/schema definitions
            while (fgets(line, sizeof(line), file)) {
                char *block_trim = line;
                while (*block_trim == ' ' || *block_trim == '\t') block_trim++;
                if (*block_trim == '}' || *block_trim == '\0')
                    break;

                // Parse fields: look for "fields:" or "schema:"
                if (strncmp(block_trim, "fields:", 7) == 0 || strncmp(block_trim, "schema:", 7) == 0) {
                    // Find the opening bracket
                    char *open_bracket = strchr(block_trim, '[');
                    char *open_brace = strchr(block_trim, '{');
                    if (open_bracket || open_brace) {
                        int in_fields = 1;
                        while (in_fields && fgets(line, sizeof(line), file)) {
                            char *ftrim = line;
                            while (*ftrim == ' ' || *ftrim == '\t') ftrim++;
                            if (*ftrim == ']' || *ftrim == '}' || *ftrim == '\0') {
                                in_fields = 0;
                                break;
                            }
                            // Parse type and field name: e.g., "string user_id,"
                            char type[32], name[64];
                            if (sscanf(ftrim, "%31s %63[^,],", type, name) == 2 ||
                                sscanf(ftrim, "%31s %63s", type, name) == 2) {
                                if (field_count < FOSSIL_BLUECRAB_MAX_FIELDS) {
                                    strncpy(crab->field_names[field_count], name, FOSSIL_BLUECRAB_MAX_FIELD_NAME - 1);
                                    crab->field_names[field_count][FOSSIL_BLUECRAB_MAX_FIELD_NAME - 1] = '\0';
                                    field_count++;
                                }
                            }
                        }
                    }
                }
            }
        }
    }

    fclose(file);
    crab->field_count = field_count;

    return 0; // Success
}

int fossil_bluecrab_sync(fossil_bluecrab_t *crab) {
    if (!crab) {
        return -1; // Invalid parameter
    }

    FILE *file = fopen(crab->storage_path, "rb");
    if (!file) {
        return -2; // Failed to open storage file
    }

    // Determine file size and number of blocks
    fseek(file, 0, SEEK_END);
    long filesize = ftell(file);
    fseek(file, 0, SEEK_SET);

    if (filesize < 0 || filesize % sizeof(fossil_bluecrab_block_t) != 0) {
        fclose(file);
        return -3; // Corrupted file or invalid size
    }

    size_t block_count = filesize / sizeof(fossil_bluecrab_block_t);
    fossil_bluecrab_block_t *blocks = malloc(sizeof(fossil_bluecrab_block_t) * block_count);
    if (!blocks) {
        fclose(file);
        return -4; // Memory allocation failed
    }

    size_t read = fread(blocks, sizeof(fossil_bluecrab_block_t), block_count, file);
    if (read != block_count) {
        fclose(file);
        free(blocks);
        return -5; // Failed to read all blocks
    }
    fclose(file);

    // Validate the chain
    crab->chain = blocks;
    crab->chain_length = block_count;
    crab->last_block_index = (block_count > 0) ? blocks[block_count - 1].block_index : 0;

    if (fossil_bluecrab_verify_chain(crab) != 0) {
        free(crab->chain);
        crab->chain = NULL;
        crab->chain_length = 0;
        crab->last_block_index = 0;
        return -6; // Chain validation failed
    }

    crab->initialized = true;
    return 0; // Success
}

int fossil_bluecrab_flush(fossil_bluecrab_t *crab) {
    if (!crab || !crab->writable) {
        return -1; // Invalid parameter or not writable
    }

    FILE *file = fopen(crab->storage_path, "wb");
    if (!file) {
        return -2; // Failed to open storage file
    }

    size_t written = fwrite(crab->chain, sizeof(fossil_bluecrab_block_t), crab->chain_length, file);
    fclose(file);

    if (written != crab->chain_length) {
        return -3; // Failed to write all blocks
    }

    return 0; // Success
}

int fossil_bluecrab_append_block(fossil_bluecrab_t *crab, const void *payload, size_t payload_len) {
    if (!crab || !crab->writable || !payload || payload_len == 0) {
        return -1; // Invalid parameters or not writable
    }

    // Create a new block and populate its fields
    fossil_bluecrab_block_t new_block;
    new_block.block_index = crab->last_block_index + 1;
    new_block.timestamp_unix = time(NULL);
    memcpy(new_block.data_payload, payload, payload_len);
    new_block.data_length = (uint32_t)payload_len;

    // Calculate hashes for the new block
    if (fossil_bluecrab_calculate_hash((const uint8_t *)&new_block, sizeof(new_block), new_block.curr_hash) != 0) {
        return -2; // Hash calculation failed
    }

    // Link to the previous block's hash
    if (crab->chain_length > 0) {
        memcpy(new_block.prev_hash, crab->chain[crab->chain_length - 1].curr_hash, FOSSIL_BLUECRAB_HASH_SIZE);
    } else {
        memset(new_block.prev_hash, 0, FOSSIL_BLUECRAB_HASH_SIZE); // Genesis block
    }

    // Append the new block to the chain
    crab->chain = realloc(crab->chain, sizeof(fossil_bluecrab_block_t) * (crab->chain_length + 1));
    if (!crab->chain) {
        return -3; // Memory allocation failed
    }
    
    crab->chain[crab->chain_length] = new_block;
    crab->chain_length++;
    crab->last_block_index++;

    return 0; // Success
}

int fossil_bluecrab_verify_chain(fossil_bluecrab_t *crab) {
    if (!crab || crab->chain_length == 0) {
        return -1; // Invalid parameter or empty chain
    }

    for (size_t i = 0; i < crab->chain_length; i++) {
        const fossil_bluecrab_block_t *block = &crab->chain[i];

        // Recalculate the current hash
        char calculated_hash[FOSSIL_BLUECRAB_HASH_SIZE];
        if (fossil_bluecrab_calculate_hash((const uint8_t *)block, sizeof(*block), calculated_hash) != 0) {
            return -2; // Hash calculation failed
        }

        // Compare with stored hash
        if (strcmp(block->curr_hash, calculated_hash) != 0) {
            return -3; // Chain is broken
        }

        // Check previous block linkage
        if (i > 0 && strcmp(block->prev_hash, crab->chain[i - 1].curr_hash) != 0) {
            return -4; // Previous block hash mismatch
        }
    }

    return 0; // Chain is valid
}

int fossil_bluecrab_calculate_hash(const uint8_t *data, size_t len, char out_hash[FOSSIL_BLUECRAB_HASH_SIZE]) {
    if (!data || len == 0 || !out_hash) {
        return -1; // Invalid input
    }

    // Simulate hash calculation (replace with actual hashing logic)
    snprintf(out_hash, FOSSIL_BLUECRAB_HASH_SIZE, "hash_%zu", len);
    return 0;
}

int fossil_bluecrab_field_index(const fossil_bluecrab_t *crab, const char *field_name) {
    if (!crab || !field_name) {
        return -1; // Invalid parameters
    }

    for (size_t i = 0; i < crab->field_count; i++) {
        if (strcmp(crab->field_names[i], field_name) == 0) {
            return (int)i; // Found field index
        }
    }

    return -2; // Field not found
}

size_t fossil_bluecrab_field_count(const fossil_bluecrab_t *crab) {
    if (!crab) {
        return 0; // Invalid parameter
    }

    return crab->field_count; // Return the count of fields
}

void fossil_bluecrab_debug_dump(const fossil_bluecrab_t *crab) {
    if (!crab) {
        printf("Blue Crab instance is NULL.\n");
        return;
    }

    printf("Blue Crab Protocol: %s\n", crab->protocol);
    printf("Crabfile Path: %s\n", crab->crabfile_path);
    printf("Storage Path: %s\n", crab->storage_path);
    printf("Field Count: %zu\n", crab->field_count);
    printf("Last Block Index: %llu\n", (unsigned long long)crab->last_block_index);
    printf("Chain Length: %zu\n", crab->chain_length);
    printf("Writable: %s\n", crab->writable ? "Yes" : "No");
    printf("Initialized: %s\n", crab->initialized ? "Yes" : "No");

    if (crab->validation_hook) {
        printf("Custom Validation Hook Registered.\n");
    } else {
        printf("No Custom Validation Hook.\n");
    }
}

void fossil_bluecrab_print_block(const fossil_bluecrab_t *crab, size_t block_idx) {
    if (!crab || block_idx >= crab->chain_length) {
        printf("Invalid block index or Blue Crab instance.\n");
        return;
    }

    const fossil_bluecrab_block_t *block = &crab->chain[block_idx];
    printf("Block Index: %llu\n", (unsigned long long)block->block_index);
    printf("Timestamp: %llu\n", (unsigned long long)block->timestamp_unix);
    printf("Previous Hash: %s\n", block->prev_hash);
    printf("Current Hash: %s\n", block->curr_hash);
    printf("Data Length: %u bytes\n", block->data_length);

    // Print key=value pairs if available
    for (size_t i = 0; i < block->field_count; i++) {
        if (block->data && block->data[i]) {
            printf("%s=%s\n", block->data[i], block->data[i + 1]);
        }
    }
}

int fossil_bluecrab_repair_chain(fossil_bluecrab_t *crab) {
    if (!crab || crab->chain_length == 0) {
        return -1; // Invalid parameter or empty chain
    }

    for (size_t i = 0; i < crab->chain_length; i++) {
        fossil_bluecrab_block_t *block = &crab->chain[i];

        // Recalculate the current hash
        char calculated_hash[FOSSIL_BLUECRAB_HASH_SIZE];
        if (fossil_bluecrab_calculate_hash((const uint8_t *)block, sizeof(*block), calculated_hash) != 0) {
            return -2; // Hash calculation failed
        }

        // Update the block's current hash
        strncpy(block->curr_hash, calculated_hash, FOSSIL_BLUECRAB_HASH_SIZE);

        // Check previous block linkage
        if (i > 0) {
            strncpy(block->prev_hash, crab->chain[i - 1].curr_hash, FOSSIL_BLUECRAB_HASH_SIZE);
        } else {
            memset(block->prev_hash, 0, FOSSIL_BLUECRAB_HASH_SIZE); // Genesis block
        }
    }

    return 0; // Chain repaired successfully
}

int fossil_bluecrab_find_block_by_field(const fossil_bluecrab_t *crab, const char *field_name, const char *value, size_t start_idx) {
    if (!crab || !field_name || !value || start_idx >= crab->chain_length) {
        return -1; // Invalid parameters
    }

    for (size_t i = start_idx; i < crab->chain_length; i++) {
        const fossil_bluecrab_block_t *block = &crab->chain[i];

        // Check if the block contains the field and matches the value
        for (size_t j = 0; j < block->field_count; j++) {
            if (strcmp(block->data[j], field_name) == 0 && strcmp(block->data[j + 1], value) == 0) {
                return (int)i; // Found matching block
            }
        }
    }

    return -2; // No matching block found
}

const fossil_bluecrab_block_t *fossil_bluecrab_get_block(const fossil_bluecrab_t *crab, size_t block_idx) {
    if (!crab || block_idx >= crab->chain_length) {
        return NULL; // Invalid parameters
    }
    return &crab->chain[block_idx];
}

int fossil_bluecrab_export_log(const fossil_bluecrab_t *crab, const char *outpath) {
    if (!crab || !outpath) {
        return -1; // Invalid parameters
    }

    FILE *file = fopen(outpath, "w");
    if (!file) {
        return -2; // File open error
    }

    for (size_t i = 0; i < crab->chain_length; i++) {
        const fossil_bluecrab_block_t *block = &crab->chain[i];
        fprintf(file, "Block %zu:\n", i);
        fprintf(file, "  Index: %llu\n", (unsigned long long)block->block_index);
        fprintf(file, "  Timestamp: %llu\n", (unsigned long long)block->timestamp_unix);
        fprintf(file, "  Previous Hash: %s\n", block->prev_hash);
        fprintf(file, "  Current Hash: %s\n", block->curr_hash);
        fprintf(file, "  Data Length: %u bytes\n", block->data_length);

        for (size_t j = 0; j < block->field_count; j++) {
            fprintf(file, "  %s=%s\n", block->data[j], block->data[j + 1]);
        }
        fprintf(file, "\n");
    }

    fclose(file);
    return 0; // Success
}

int fossil_bluecrab_import_log(fossil_bluecrab_t *crab, const char *inpath) {
    if (!crab || !inpath) {
        return -1; // Invalid parameters
    }

    FILE *file = fopen(inpath, "r");
    if (!file) {
        return -2; // File open error
    }

    // Temporary variables for parsing
    char line[256];
    fossil_bluecrab_block_t block;
    memset(&block, 0, sizeof(block));
    size_t block_count = 0;

    while (fgets(line, sizeof(line), file)) {
        // Parse the line and import block data
        if (strncmp(line, "Block ", 6) == 0) {
            // If not the first block, append the previous one
            if (block_count > 0) {
                crab->chain = realloc(crab->chain, sizeof(fossil_bluecrab_block_t) * (crab->chain_length + 1));
                if (!crab->chain) {
                    fclose(file);
                    return -3; // Memory allocation failed
                }
                crab->chain[crab->chain_length++] = block;
            }
            memset(&block, 0, sizeof(block));
            block_count++;
        } else if (sscanf(line, "  Index: %llu", &block.block_index) == 1) {
            // Parsed block index
        } else if (sscanf(line, "  Timestamp: %llu", &block.timestamp_unix) == 1) {
            // Parsed timestamp
        } else if (strncmp(line, "  Previous Hash: ", 16) == 0) {
            strncpy(block.prev_hash, line + 16, FOSSIL_BLUECRAB_HASH_SIZE - 1);
            block.prev_hash[FOSSIL_BLUECRAB_HASH_SIZE - 1] = '\0';
            // Remove newline
            block.prev_hash[strcspn(block.prev_hash, "\r\n")] = 0;
        } else if (strncmp(line, "  Current Hash: ", 16) == 0) {
            strncpy(block.curr_hash, line + 16, FOSSIL_BLUECRAB_HASH_SIZE - 1);
            block.curr_hash[FOSSIL_BLUECRAB_HASH_SIZE - 1] = '\0';
            block.curr_hash[strcspn(block.curr_hash, "\r\n")] = 0;
        } else if (sscanf(line, "  Data Length: %u bytes", &block.data_length) == 1) {
            // Parsed data length
        } else if (strchr(line, '=') && block.field_count < FOSSIL_BLUECRAB_MAX_FIELDS) {
            // Parse key=value pairs
            char *eq = strchr(line, '=');
            if (eq) {
                *eq = '\0';
                char *key = line;
                char *val = eq + 1;
                // Remove newline from value
                val[strcspn(val, "\r\n")] = 0;
                strncpy(block.data[block.field_count], key, FOSSIL_BLUECRAB_MAX_FIELD_NAME - 1);
                block.data[block.field_count][FOSSIL_BLUECRAB_MAX_FIELD_NAME - 1] = '\0';
                block.field_count++;
                strncpy(block.data[block.field_count], val, FOSSIL_BLUECRAB_MAX_FIELD_VALUE - 1);
                block.data[block.field_count][FOSSIL_BLUECRAB_MAX_FIELD_VALUE - 1] = '\0';
                block.field_count++;
            }
        }
    }
    // Append the last block if any
    if (block_count > 0) {
        crab->chain = realloc(crab->chain, sizeof(fossil_bluecrab_block_t) * (crab->chain_length + 1));
        if (!crab->chain) {
            fclose(file);
            return -3; // Memory allocation failed
        }
        crab->chain[crab->chain_length++] = block;
    }

    crab->last_block_index = (crab->chain_length > 0) ? crab->chain[crab->chain_length - 1].block_index : 0;
    crab->initialized = true;

    fclose(file);
    return 0; // Success
}

int fossil_bluecrab_set_validation_hook(fossil_bluecrab_t *crab, int (*validate)(const fossil_bluecrab_block_t *block)) {
    if (!crab) {
        return -1; // Invalid parameter
    }
    crab->validation_hook = validate;
    return 0; // Success
}

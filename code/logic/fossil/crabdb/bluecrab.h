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
#ifndef FOSSIL_CRABDB_MYSHELL_H
#define FOSSIL_CRABDB_MYSHELL_H

#include <stdbool.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <stdio.h>
#include <time.h>

#define FOSSIL_BLUECRAB_MAX_FIELDS 64
#define FOSSIL_BLUECRAB_MAX_PROTOCOL_NAME 32
#define FOSSIL_BLUECRAB_MAX_FIELD_NAME 64
#define FOSSIL_BLUECRAB_MAX_FIELD_VALUE 256
#define FOSSIL_BLUECRAB_MAX_PATH 260
#define FOSSIL_BLUECRAB_HASH_SIZE 64

#ifdef __cplusplus
extern "C" {
#endif

// Unified data block representing a hashed record
typedef struct fossil_bluecrab_block_t {
    uint64_t block_index;                            // Position in the chain (starting from 0)
    uint64_t timestamp_unix;                         // UTC timestamp for block creation
    char prev_hash[FOSSIL_BLUECRAB_HASH_SIZE];       // Hash of the previous block
    char curr_hash[FOSSIL_BLUECRAB_HASH_SIZE];       // Hash of current block (calculated after data)
    uint32_t data_length;                            // Length of payload in bytes
    uint8_t *data_payload;                           // Pointer to raw data (serialized key/values)
    char **data;                                     // Array of strings for key/value pairs

    // Missing/optional fields for completeness:
    size_t field_count;                              // Number of fields in this block
    bool valid;                                      // Validation status of this block
    void *reserved[2];                               // Reserved for future use/expansion
} fossil_bluecrab_block_t;

// Core interface structure for unified database protocol I/O
typedef struct fossil_bluecrab_t {
    // == Metadata ==
    char protocol[FOSSIL_BLUECRAB_MAX_PROTOCOL_NAME]; // e.g., "MyShell", "NoShell", "TimeShell"
    char crabfile_path[FOSSIL_BLUECRAB_MAX_PATH];     // Full path to the schema/definition file (.crab)
    char storage_path[FOSSIL_BLUECRAB_MAX_PATH];      // Storage location for blockchain blocks or logs

    // == Schema ==
    char **field_names;                  // Array of field names loaded from .crab schema
    char **field_types;                  // Array of field types loaded from .crab schema

    // == State ==
    uint64_t last_block_index;           // Last used block index
    size_t field_count;                  // Number of fields loaded from the .crab schema
    bool initialized;                    // Whether the engine has been initialized
    bool writable;                       // Whether the engine can write new data

    // == Chain ==
    fossil_bluecrab_block_t *chain;      // Pointer to array of blocks (or dynamically loaded)
    size_t chain_length;                 // Number of blocks in memory

    // == Interface ==
    void *protocol_context;              // Protocol-specific state/context (if needed)

    // == Error tracking ==
    char last_error[256];                // Verbose human-readable last error

    // == Validation hook ==
    int (*validation_hook)(const fossil_bluecrab_block_t *block); // Custom validation

    // == Internal buffers ==
    void *scratch_buffer;                // Temporary buffer for parsing/serialization
    size_t scratch_buffer_size;          // Size of the scratch buffer

    // == Reserved for future use ==
    void *reserved[4];                   // Reserved for future expansion

} fossil_bluecrab_t;

/**
 * @brief Initialize a Blue Crab instance with the given protocol and paths.
 *
 * @param crab             Pointer to the Blue Crab instance to initialize.
 * @param protocol         Name of the protocol (e.g., "MyShell", "NoShell").
 * @param crabfile_path    Path to the .crab schema file describing the structure.
 * @param storage_path     Path to the block storage or log directory.
 * @return 0 on success, non-zero on failure.
 */
int fossil_bluecrab_init(fossil_bluecrab_t *crab, const char *protocol, const char *crabfile_path, const char *storage_path);

/**
 * @brief Clean up resources held by a Blue Crab instance.
 *
 * Frees all dynamically allocated memory and resets internal state.
 *
 * @param crab Pointer to the Blue Crab instance to shut down.
 * @return 0 on success, non-zero on failure.
 */
int fossil_bluecrab_shutdown(fossil_bluecrab_t *crab);

/**
 * @brief Load the .crab schema file into the Blue Crab instance.
 *
 * Parses the CrabDSL definition and populates internal field metadata.
 *
 * @param crab Pointer to an initialized Blue Crab instance.
 * @return 0 on success, non-zero on parse or validation error.
 */
int fossil_bluecrab_load_crabfile(fossil_bluecrab_t *crab);

/**
 * @brief Load and validate block data from storage.
 *
 * Reads the existing block chain into memory and validates structure.
 *
 * @param crab Pointer to the Blue Crab instance.
 * @return 0 on success, non-zero on I/O or format error.
 */
int fossil_bluecrab_sync(fossil_bluecrab_t *crab);

/**
 * @brief Write the in-memory block chain to persistent storage.
 *
 * Ensures that all appended blocks are flushed to disk or storage backend.
 *
 * @param crab Pointer to the Blue Crab instance.
 * @return 0 on success, non-zero on write or permission error.
 */
int fossil_bluecrab_flush(fossil_bluecrab_t *crab);

/**
 * @brief Append a new data block to the chain with the given payload.
 *
 * Automatically calculates and links hashes to preserve blockchain integrity.
 *
 * @param crab         Pointer to the Blue Crab instance.
 * @param payload      Raw serialized data to store in the block.
 * @param payload_len  Length of the payload in bytes.
 * @return 0 on success, non-zero on failure (e.g., block overflow, invalid state).
 */
int fossil_bluecrab_append_block(fossil_bluecrab_t *crab, const void *payload, size_t payload_len);

/**
 * @brief Verify the hash chain from genesis block to the latest.
 *
 * Recalculates all hashes and validates linkage to ensure tamper-resistance.
 *
 * @param crab Pointer to the Blue Crab instance.
 * @return 0 if the chain is valid, non-zero if broken or corrupted.
 */
int fossil_bluecrab_verify_chain(fossil_bluecrab_t *crab);

/**
 * @brief Calculate a cryptographic hash of a given data buffer.
 *
 * Used internally to generate the current and previous block hashes.
 *
 * @param data       Pointer to input data to hash.
 * @param len        Length of the data in bytes.
 * @param out_hash   Output buffer to store the resulting hash (null-terminated).
 * @return 0 on success, non-zero on failure (e.g., invalid input).
 */
int fossil_bluecrab_calculate_hash(const uint8_t *data, size_t len, char out_hash[FOSSIL_BLUECRAB_HASH_SIZE]);

/**
 * @brief Get the index of a named field from the loaded schema.
 *
 * @param crab         Pointer to the Blue Crab instance.
 * @param field_name   Null-terminated string name of the field.
 * @return Zero-based index of field, or -1 if not found.
 */
int fossil_bluecrab_field_index(const fossil_bluecrab_t *crab, const char *field_name);

/**
 * @brief Return the number of fields loaded from the .crab schema.
 *
 * @param crab Pointer to the Blue Crab instance.
 * @return Number of fields, or 0 if schema not loaded.
 */
size_t fossil_bluecrab_field_count(const fossil_bluecrab_t *crab);

/**
 * @brief Print summary information about the Blue Crab state.
 *
 * Includes protocol, field layout, block count, and validation status.
 *
 * @param crab Pointer to the Blue Crab instance.
 */
void fossil_bluecrab_debug_dump(const fossil_bluecrab_t *crab);

/**
 * @brief Dump a specific block's contents as key=value pairs to stdout.
 *
 * @param crab       Pointer to the Blue Crab instance.
 * @param block_idx  Index of the block to display (0 = genesis).
 */
void fossil_bluecrab_print_block(const fossil_bluecrab_t *crab, size_t block_idx);

/**
 * @brief Recalculate and repair broken hash links in the chain.
 *
 * Useful when recovering from partial corruption or rebuild scenarios.
 *
 * @param crab Pointer to the Blue Crab instance.
 * @return 0 on success, non-zero on error.
 */
int fossil_bluecrab_repair_chain(fossil_bluecrab_t *crab);

/**
 * @brief Search for a block matching a specific field value.
 *
 * Only exact string comparisons are supported.
 *
 * @param crab         Pointer to the Blue Crab instance.
 * @param field_name   Name of the field to match.
 * @param value        String value to search for.
 * @param start_idx    Index to start searching from.
 * @return Index of matching block, or -1 if not found.
 */
int fossil_bluecrab_find_block_by_field(const fossil_bluecrab_t *crab, const char *field_name, const char *value, size_t start_idx);

/**
 * @brief Get a pointer to a specific block by index.
 *
 * @param crab       Pointer to the Blue Crab instance.
 * @param block_idx  Index of the block to retrieve.
 * @return Pointer to the block, or NULL if out of range.
 */
const fossil_bluecrab_block_t *fossil_bluecrab_get_block(const fossil_bluecrab_t *crab, size_t block_idx);

/**
 * @brief Export all block data to a plain-text log file (human-readable).
 *
 * @param crab     Pointer to the Blue Crab instance.
 * @param outpath  Path to output file.
 * @return 0 on success, non-zero on error.
 */
int fossil_bluecrab_export_log(const fossil_bluecrab_t *crab, const char *outpath);

/**
 * @brief Import blocks from a previously exported log file.
 *
 * @param crab     Pointer to the Blue Crab instance.
 * @param inpath   Path to the input file.
 * @return 0 on success, non-zero on parse or consistency error.
 */
int fossil_bluecrab_import_log(fossil_bluecrab_t *crab, const char *inpath);

/**
 * @brief Register a callback for custom block validation.
 *
 * @param crab      Pointer to the Blue Crab instance.
 * @param validate  Callback function accepting a block and returning 0/1.
 * @return 0 on success, non-zero on failure.
 */
int fossil_bluecrab_set_validation_hook(fossil_bluecrab_t *crab, int (*validate)(const fossil_bluecrab_block_t *block));

#ifdef __cplusplus
}

#include <string>

namespace fossil {

    class BlueCrab {
    public:
        /**
         * @brief Initializes a Blue Crab instance with the given protocol and paths.
         *
         * @param crab             Pointer to the Blue Crab instance to initialize.
         * @param protocol         Name of the protocol (e.g., "MyShell", "NoShell").
         * @param crabfile_path    Path to the .crab schema file describing the structure.
         * @param storage_path     Path to the block storage or log directory.
         * @return 0 on success, non-zero on failure.
         */
        static int init(fossil_bluecrab_t *crab, const std::string &protocol, const std::string &crabfile_path, const std::string &storage_path) {
            return fossil_bluecrab_init(crab, protocol.c_str(), crabfile_path.c_str(), storage_path.c_str());
        }

        /**
         * @brief Cleans up resources held by a Blue Crab instance.
         *
         * Frees all dynamically allocated memory and resets internal state.
         *
         * @param crab Pointer to the Blue Crab instance to shut down.
         * @return 0 on success, non-zero on failure.
         */
        static int shutdown(fossil_bluecrab_t *crab) {
            return fossil_bluecrab_shutdown(crab);
        }

        /**
         * @brief Loads the .crab schema file into the Blue Crab instance.
         *
         * Parses the CrabDSL definition and populates internal field metadata.
         *
         * @param crab Pointer to an initialized Blue Crab instance.
         * @return 0 on success, non-zero on parse or validation error.
         */
        static int load_crabfile(fossil_bluecrab_t *crab) {
            return fossil_bluecrab_load_crabfile(crab);
        }

        /**
         * @brief Loads and validates block data from storage.
         *
         * Reads the existing block chain into memory and validates structure.
         *
         * @param crab Pointer to the Blue Crab instance.
         * @return 0 on success, non-zero on I/O or format error.
         */
        static int sync(fossil_bluecrab_t *crab) {
            return fossil_bluecrab_sync(crab);
        }

        /**
         * @brief Writes the in-memory block chain to persistent storage.
         *
         * Ensures that all appended blocks are flushed to disk or storage backend.
         *
         * @param crab Pointer to the Blue Crab instance.
         * @return 0 on success, non-zero on write or permission error.
         */
        static int flush(fossil_bluecrab_t *crab) {
            return fossil_bluecrab_flush(crab);
        }

        /**
         * @brief Appends a new data block to the chain with the given payload.
         *
         * Automatically calculates and links hashes to preserve blockchain integrity.
         *
         * @param crab         Pointer to the Blue Crab instance.
         * @param payload      Raw serialized data to store in the block.
         * @param payload_len  Length of the payload in bytes.
         * @return 0 on success, non-zero on failure (e.g., block overflow, invalid state).
         */
        static int append_block(fossil_bluecrab_t *crab, const void *payload, size_t payload_len) {
            return fossil_bluecrab_append_block(crab, payload, payload_len);
        }

        /**
         * @brief Verifies the hash chain from genesis block to the latest.
         *
         * Recalculates all hashes and validates linkage to ensure tamper-resistance.
         *
         * @param crab Pointer to the Blue Crab instance.
         * @return 0 if the chain is valid, non-zero if broken or corrupted.
         */
        static int verify_chain(fossil_bluecrab_t *crab) {
            return fossil_bluecrab_verify_chain(crab);
        }

        /**
         * @brief Calculates a cryptographic hash of a given data buffer.
         *
         * Used internally to generate the current and previous block hashes.
         *
         * @param data       Pointer to input data to hash.
         * @param len        Length of the data in bytes.
         * @param out_hash   Output buffer to store the resulting hash (null-terminated).
         * @return 0 on success, non-zero on failure (e.g., invalid input).
         */
        static int calculate_hash(const uint8_t *data, size_t len, char out_hash[FOSSIL_BLUECRAB_HASH_SIZE]) {
            return fossil_bluecrab_calculate_hash(data, len, out_hash);
        }

        /**
         * @brief Gets the index of a named field from the loaded schema.
         *
         * @param crab         Pointer to the Blue Crab instance.
         * @param field_name   Null-terminated string name of the field.
         * @return Zero-based index of field, or -1 if not found.
         */
        static int field_index(const fossil_bluecrab_t *crab, const std::string &field_name) {
            return fossil_bluecrab_field_index(crab, field_name.c_str());
        }

        /**
         * @brief Returns the number of fields loaded from the .crab schema.
         *
         * @param crab Pointer to the Blue Crab instance.
         * @return Number of fields, or 0 if schema not loaded.
         */
        static size_t field_count(const fossil_bluecrab_t *crab) {
            return fossil_bluecrab_field_count(crab);
        }

        /**
         * @brief Prints summary information about the Blue Crab state.
         *
         * Includes protocol, field layout, block count, and validation status.
         *
         * @param crab Pointer to the Blue Crab instance.
         */
        static void debug_dump(const fossil_bluecrab_t *crab) {
            fossil_bluecrab_debug_dump(crab);
        }

        /**
         * @brief Dumps a specific block's contents as key=value pairs to stdout.
         *
         * @param crab       Pointer to the Blue Crab instance.
         * @param block_idx  Index of the block to display (0 = genesis).
         */
        static void print_block(const fossil_bluecrab_t *crab, size_t block_idx) {
            fossil_bluecrab_print_block(crab, block_idx);
        }

        /**
         * @brief Recalculates and repairs broken hash links in the chain.
         *
         * Useful when recovering from partial corruption or rebuild scenarios.
         *
         * @param crab Pointer to the Blue Crab instance.
         * @return 0 on success, non-zero on error.
         */
        static int repair_chain(fossil_bluecrab_t *crab) {
            return fossil_bluecrab_repair_chain(crab);
        }

        /**
         * @brief Searches for a block matching a specific field value.
         *
         * Only exact string comparisons are supported.
         *
         * @param crab         Pointer to the Blue Crab instance.
         * @param field_name   Name of the field to match.
         * @param value        String value to search for.
         * @param start_idx    Index to start searching from.
         * @return Index of matching block, or -1 if not found.
         */
        static int find_block_by_field(const fossil_bluecrab_t *crab, const std::string &field_name, const std::string &value, size_t start_idx) {
            return fossil_bluecrab_find_block_by_field(crab, field_name.c_str(), value.c_str(), start_idx);
        }

        /**
         * @brief Gets a pointer to a specific block by index.
         *
         * @param crab       Pointer to the Blue Crab instance.
         * @param block_idx  Index of the block to retrieve.
         * @return Pointer to the block, or NULL if out of range.
         */
        static const fossil_bluecrab_block_t *get_block(const fossil_bluecrab_t *crab, size_t block_idx) {
            return fossil_bluecrab_get_block(crab, block_idx);
        }

        /**
         * @brief Exports all block data to a plain-text log file (human-readable).
         *
         * @param crab     Pointer to the Blue Crab instance.
         * @param outpath  Path to output file.
         * @return 0 on success, non-zero on error.
         */
        static int export_log(const fossil_bluecrab_t *crab, const std::string &outpath) {
            return fossil_bluecrab_export_log(crab, outpath.c_str());
        }

        /**
         * @brief Imports blocks from a previously exported log file.
         *
         * @param crab     Pointer to the Blue Crab instance.
         * @param inpath   Path to the input file.
         * @return 0 on success, non-zero on parse or consistency error.
         */
        static int import_log(fossil_bluecrab_t *crab, const std::string &inpath) {
            return fossil_bluecrab_import_log(crab, inpath.c_str());
        }

        /**
         * @brief Registers a callback for custom block validation.
         *
         * @param crab      Pointer to the Blue Crab instance.
         * @param validate  Callback function accepting a block and returning 0/1.
         * @return 0 on success, non-zero on failure.
         */
        static int set_validation_hook(fossil_bluecrab_t *crab, int (*validate)(const fossil_bluecrab_block_t *block)) {
            return fossil_bluecrab_set_validation_hook(crab, validate);
        }

    };

}

#endif

#endif /* FOSSIL_CRABDB_FRAMEWORK_H */
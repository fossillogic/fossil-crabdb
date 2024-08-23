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
#include "fossil/crabdb/datastore.h"
#include "fossil/crabdb/database.h"
#include "fossil/crabdb/namespace.h"
#include "fossil/crabdb/internal.h"

#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

fossil_crabdb_error_t fossil_crabdb_serialize_to_file(fossil_crabdb_t *db, const char *filename) {
    FILE *file = fopen(filename, "wb");
    if (!file) {
        return CRABDB_ERR_MEM;
    }

    fossil_crabdb_namespace_t *ns = db->namespaces;

    // Serialize namespaces
    while (ns) {
        size_t name_len = strlen(ns->name) + 1;
        fwrite(&name_len, sizeof(size_t), 1, file);
        fwrite(ns->name, sizeof(char), name_len, file);
        fwrite(&ns->sub_namespace_count, sizeof(size_t), 1, file);

        // Serialize key-value pairs
        fossil_crabdb_keyvalue_t *kv = ns->data;
        while (kv) {
            size_t key_len = strlen(kv->key) + 1;
            size_t value_len = strlen(kv->value) + 1;

            fwrite(&key_len, sizeof(size_t), 1, file);
            fwrite(kv->key, sizeof(char), key_len, file);
            fwrite(&value_len, sizeof(size_t), 1, file);
            fwrite(kv->value, sizeof(char), value_len, file);

            kv = kv->next;
        }

        ns = ns->next;
    }

    fclose(file);
    return CRABDB_OK;
}

fossil_crabdb_error_t fossil_crabdb_deserialize_from_file(fossil_crabdb_t *db, const char *filename) {
    FILE *file = fopen(filename, "rb");
    if (!file) {
        return CRABDB_ERR_MEM;
    }

    fossil_crabdb_namespace_t *prev_ns = NULL;
    while (!feof(file)) {
        size_t name_len;
        if (fread(&name_len, sizeof(size_t), 1, file) != 1) break;

        char *name = (char *)fossil_crabdb_alloc(name_len);
        if (fread(name, sizeof(char), name_len, file) != name_len) {
            fossil_crabdb_free(name);
            break;
        }

        fossil_crabdb_namespace_t *ns = (fossil_crabdb_namespace_t *)fossil_crabdb_alloc(sizeof(fossil_crabdb_namespace_t));
        ns->name = name;
        ns->sub_namespaces = NULL;
        ns->data = NULL;
        ns->next = NULL;

        if (fread(&ns->sub_namespace_count, sizeof(size_t), 1, file) != 1) {
            fossil_crabdb_free(name);
            fossil_crabdb_free(ns);
            break;
        }

        // Deserialize key-value pairs
        fossil_crabdb_keyvalue_t *prev_kv = NULL;
        while (!feof(file)) {
            size_t key_len;
            if (fread(&key_len, sizeof(size_t), 1, file) != 1) break;

            char *key = (char *)fossil_crabdb_alloc(key_len);
            if (fread(key, sizeof(char), key_len, file) != key_len) {
                fossil_crabdb_free(key);
                break;
            }

            size_t value_len;
            if (fread(&value_len, sizeof(size_t), 1, file) != 1) {
                fossil_crabdb_free(key);
                break;
            }

            char *value = (char *)fossil_crabdb_alloc(value_len);
            if (fread(value, sizeof(char), value_len, file) != value_len) {
                fossil_crabdb_free(key);
                fossil_crabdb_free(value);
                break;
            }

            fossil_crabdb_keyvalue_t *kv = (fossil_crabdb_keyvalue_t *)fossil_crabdb_alloc(sizeof(fossil_crabdb_keyvalue_t));
            kv->key = key;
            kv->value = value;
            kv->next = NULL;

            if (prev_kv) {
                prev_kv->next = kv;
            } else {
                ns->data = kv;
            }
            prev_kv = kv;
        }

        if (prev_ns) {
            prev_ns->next = ns;
        } else {
            db->namespaces = ns;
        }
        prev_ns = ns;
    }

    fclose(file);
    return CRABDB_OK;
}

// Save the database to a file
int fossil_crabdb_save_to_file(fossil_crabdb_t *db, const char *filename) {
    if (!db || !filename) {
        return -1; // Invalid input
    }

    FILE *file = fopen(filename, "wb");
    if (!file) {
        return -1; // Error opening file
    }

    fossil_crabdb_namespace_t *ns = db->namespaces;

    // Write namespaces and their data to the file
    while (ns) {
        // Write the namespace name length and name
        size_t name_len = strlen(ns->name) + 1; // Include the null terminator
        fwrite(&name_len, sizeof(size_t), 1, file);
        fwrite(ns->name, sizeof(char), name_len, file);

        // Write the number of key-value pairs
        fossil_crabdb_keyvalue_t *kv = ns->data;
        size_t kv_count = 0;
        for (kv = ns->data; kv; kv = kv->next) {
            kv_count++;
        }
        fwrite(&kv_count, sizeof(size_t), 1, file);

        // Write key-value pairs
        for (kv = ns->data; kv; kv = kv->next) {
            // Write key length and key
            size_t key_len = strlen(kv->key) + 1;
            fwrite(&key_len, sizeof(size_t), 1, file);
            fwrite(kv->key, sizeof(char), key_len, file);

            // Write value length and value
            size_t value_len = strlen(kv->value) + 1;
            fwrite(&value_len, sizeof(size_t), 1, file);
            fwrite(kv->value, sizeof(char), value_len, file);
        }

        ns = ns->next;
    }

    fclose(file);
    return 0; // Success
}

// Load the database from a file
int fossil_crabdb_load_from_file(fossil_crabdb_t *db, const char *filename) {
    if (!db || !filename) {
        return -1; // Invalid input
    }

    FILE *file = fopen(filename, "rb");
    if (!file) {
        return -1; // Error opening file
    }

    // Clear the current database content
    fossil_crabdb_erase(db);

    while (!feof(file)) {
        // Read namespace name length and name
        size_t name_len = 0;
        if (fread(&name_len, sizeof(size_t), 1, file) != 1) {
            break; // EOF or error
        }

        char *ns_name = (char *)fossil_crabdb_alloc(name_len);
        if (fread(ns_name, sizeof(char), name_len, file) != name_len) {
            fossil_crabdb_free(ns_name);
            break;
        }

        fossil_crabdb_create_namespace(db, ns_name);

        // Read number of key-value pairs
        size_t kv_count = 0;
        fread(&kv_count, sizeof(size_t), 1, file);

        for (size_t i = 0; i < kv_count; i++) {
            // Read key length and key
            size_t key_len = 0;
            fread(&key_len, sizeof(size_t), 1, file);
            char *key = (char *)fossil_crabdb_alloc(key_len);
            fread(key, sizeof(char), key_len, file);

            // Read value length and value
            size_t value_len = 0;
            fread(&value_len, sizeof(size_t), 1, file);
            char *value = (char *)fossil_crabdb_alloc(value_len);
            fread(value, sizeof(char), value_len, file);

            // Insert the key-value pair into the namespace
            fossil_crabdb_insert(db, ns_name, key, value);

            fossil_crabdb_free(key);
            fossil_crabdb_free(value);
        }

        fossil_crabdb_free(ns_name);
    }

    fclose(file);
    return 0; // Success
}

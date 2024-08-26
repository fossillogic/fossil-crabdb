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
#include "fossil/crabdb/database.h"

#include <string.h>
#include <stdlib.h>
#include <stddef.h>
#include <stdio.h>
#include <ctype.h>

/**
 * @brief Create a new namespace.
 * 
 * @param db Pointer to the fossil_crabdb_t database.
 * @param namespace_name Name of the new namespace.
 * @return Error code indicating the result of the operation.
 */
fossil_crabdb_t* fossil_crabdb_create(void) {
    fossil_crabdb_t *db = fossil_crabdb_alloc(sizeof(fossil_crabdb_t));
    if (!db) {
        return NULL;
    }
    db->namespaces = NULL;
    return db;
} // end of fun

/**
 * @brief Erase the fossil_crabdb_t database.
 * 
 * @param db Pointer to the fossil_crabdb_t database to erase.
 */
void fossil_crabdb_erase(fossil_crabdb_t *db) {
    if (!db) return;

    fossil_crabdb_namespace_t *current = db->namespaces;
    while (current) {
        fossil_crabdb_namespace_t *next = current->next;
        fossil_crabdb_free(current->name);

        for (size_t i = 0; i < current->sub_namespace_count; i++) {
            fossil_crabdb_free(current->sub_namespaces[i].name);
        }
        fossil_crabdb_free(current->sub_namespaces);

        fossil_crabdb_keyvalue_t *kv = current->data;
        while (kv) {
            fossil_crabdb_keyvalue_t *kv_next = kv->next;
            fossil_crabdb_free(kv->key);
            fossil_crabdb_free(kv->value);
            fossil_crabdb_free(kv);
            kv = kv_next;
        }

        fossil_crabdb_free(current);
        current = next;
    }

    fossil_crabdb_free(db);
} // end of fun

/**
 * @brief Insert data into a namespace.
 * 
 * @param db Pointer to the fossil_crabdb_t database.
 * @param namespace_name Name of the namespace.
 * @param key Key of the data to insert.
 * @param value Value of the data to insert.
 * @return Error code indicating the result of the operation.
 */
fossil_crabdb_error_t fossil_crabdb_insert(fossil_crabdb_t *db, const char *namespace_name, const char *key, const char *value) {
    if (!db || !namespace_name || !key || !value) return CRABDB_ERR_MEM;

    fossil_crabdb_namespace_t *current = db->namespaces;
    while (current) {
        if (strcmp(current->name, namespace_name) == 0) {
            fossil_crabdb_keyvalue_t *kv = current->data;
            while (kv) {
                if (strcmp(kv->key, key) == 0) {
                    return CRABDB_ERR_KEY_ALREADY_EXISTS; // Key already exists
                }
                kv = kv->next;
            }

            fossil_crabdb_keyvalue_t *new_kv = fossil_crabdb_alloc(sizeof(fossil_crabdb_keyvalue_t));
            if (!new_kv) return CRABDB_ERR_MEM;

            new_kv->key = fossil_crabdb_strdup(key);
            if (!new_kv->key) {
                fossil_crabdb_free(new_kv); // Free allocated memory on failure
                return CRABDB_ERR_MEM;
            }

            new_kv->value = fossil_crabdb_strdup(value);
            if (!new_kv->value) {
                fossil_crabdb_free(new_kv->key); // Free allocated key if value allocation fails
                fossil_crabdb_free(new_kv); // Free the new key-value structure
                return CRABDB_ERR_MEM;
            }

            new_kv->next = current->data;
            current->data = new_kv;

            return CRABDB_OK;
        }
        current = current->next;
    }

    return CRABDB_ERR_NS_NOT_FOUND;
} // end of fun

/**
 * @brief Get data from a namespace.
 * 
 * @param db Pointer to the fossil_crabdb_t database.
 * @param namespace_name Name of the namespace.
 * @param key Key of the data to get.
 * @param value Pointer to store the retrieved value.
 * @return Error code indicating the result of the operation.
 */
fossil_crabdb_error_t fossil_crabdb_get(fossil_crabdb_t *db, const char *namespace_name, const char *key, char **value) {
    if (!db || !namespace_name || !key || !value) return CRABDB_ERR_MEM;

    fossil_crabdb_namespace_t *current = db->namespaces;
    while (current) {
        if (strcmp(current->name, namespace_name) == 0) {
            fossil_crabdb_keyvalue_t *kv = current->data;
            while (kv) {
                if (strcmp(kv->key, key) == 0) {
                    *value = fossil_crabdb_strdup(kv->value);
                    if (!*value) return CRABDB_ERR_MEM; // Check if strdup succeeded
                    return CRABDB_OK;
                }
                kv = kv->next;
            }
            return CRABDB_ERR_KEY_NOT_FOUND;
        }
        current = current->next;
    }

    return CRABDB_ERR_NS_NOT_FOUND;
} // end of fun

/**
 * @brief Update data in a namespace.
 * 
 * @param db Pointer to the fossil_crabdb_t database.
 * @param namespace_name Name of the namespace.
 * @param key Key of the data to update.
 * @param value New value for the data.
 * @return Error code indicating the result of the operation.
 */
fossil_crabdb_error_t fossil_crabdb_update(fossil_crabdb_t *db, const char *namespace_name, const char *key, const char *value) {
    if (!db || !namespace_name || !key || !value) return CRABDB_ERR_MEM;

    fossil_crabdb_namespace_t *current = db->namespaces;
    while (current) {
        if (strcmp(current->name, namespace_name) == 0) {
            fossil_crabdb_keyvalue_t *kv = current->data;
            while (kv) {
                if (strcmp(kv->key, key) == 0) {
                    // Free the old value
                    fossil_crabdb_free(kv->value);
                    // Duplicate the new value
                    kv->value = fossil_crabdb_strdup(value);
                    // Check if strdup succeeded
                    if (!kv->value) return CRABDB_ERR_MEM;
                    return CRABDB_OK;
                }
                kv = kv->next;
            }
            return CRABDB_ERR_KEY_NOT_FOUND;
        }
        current = current->next;
    }

    return CRABDB_ERR_NS_NOT_FOUND;
} // end of fun

/**
 * @brief Delete data from a namespace.
 * 
 * @param db Pointer to the fossil_crabdb_t database.
 * @param namespace_name Name of the namespace.
 * @param key Key of the data to delete.
 * @return Error code indicating the result of the operation.
 */
fossil_crabdb_error_t fossil_crabdb_delete(fossil_crabdb_t *db, const char *namespace_name, const char *key) {
    if (!db || !namespace_name || !key) return CRABDB_ERR_MEM;

    fossil_crabdb_namespace_t *current = db->namespaces;
    while (current) {
        if (strcmp(current->name, namespace_name) == 0) {
            fossil_crabdb_keyvalue_t *prev = NULL;
            fossil_crabdb_keyvalue_t *kv = current->data;
            while (kv) {
                if (strcmp(kv->key, key) == 0) {
                    if (prev) {
                        prev->next = kv->next;
                    } else {
                        current->data = kv->next;
                    }
                    fossil_crabdb_free(kv->key);
                    fossil_crabdb_free(kv->value);
                    fossil_crabdb_free(kv);
                    return CRABDB_OK;
                }
                prev = kv;
                kv = kv->next;
            }
            return CRABDB_ERR_KEY_NOT_FOUND;
        }
        current = current->next;
    }

    return CRABDB_ERR_NS_NOT_FOUND;
} // end of fun

//
// DATABASE STORAGE
//

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

//
// DATABASE NAMESPACES
//

/**
 * @brief Create a new namespace.
 * 
 * @param db Pointer to the fossil_crabdb_t database.
 * @param namespace_name Name of the new namespace.
 * @return Error code indicating the result of the operation.
 */
fossil_crabdb_error_t fossil_crabdb_create_namespace(fossil_crabdb_t *db, const char *namespace_name) {
    if (!db || !namespace_name) return CRABDB_ERR_MEM;

    fossil_crabdb_namespace_t *current = db->namespaces;
    while (current) {
        if (strcmp(current->name, namespace_name) == 0) {
            return CRABDB_ERR_NS_EXISTS;
        }
        current = current->next;
    }

    fossil_crabdb_namespace_t *new_namespace = fossil_crabdb_alloc(sizeof(fossil_crabdb_namespace_t));
    if (!new_namespace) return CRABDB_ERR_MEM;

    new_namespace->name = fossil_crabdb_strdup(namespace_name);
    new_namespace->sub_namespaces = NULL;
    new_namespace->sub_namespace_count = 0;
    new_namespace->next = db->namespaces;
    new_namespace->data = NULL;
    db->namespaces = new_namespace;

    return CRABDB_OK;
} // end of fun

/**
 * @brief Create a new sub-namespace.
 * 
 * @param db Pointer to the fossil_crabdb_t database.
 * @param namespace_name Name of the parent namespace.
 * @param sub_namespace_name Name of the new sub-namespace.
 * @return Error code indicating the result of the operation.
 */
fossil_crabdb_error_t fossil_crabdb_create_sub_namespace(fossil_crabdb_t *db, const char *namespace_name, const char *sub_namespace_name) {
    if (!db || !namespace_name || !sub_namespace_name) return CRABDB_ERR_MEM;

    fossil_crabdb_namespace_t *current = db->namespaces;
    while (current) {
        if (strcmp(current->name, namespace_name) == 0) {
            for (size_t i = 0; i < current->sub_namespace_count; i++) {
                if (strcmp(current->sub_namespaces[i].name, sub_namespace_name) == 0) {
                    return CRABDB_ERR_SUB_NS_EXISTS;
                }
            }

            current->sub_namespaces = (fossil_crabdb_namespace_t*)fossil_crabdb_realloc(current->sub_namespaces, sizeof(fossil_crabdb_namespace_t) * (current->sub_namespace_count + 1));
            if (!current->sub_namespaces) return CRABDB_ERR_MEM;

            current->sub_namespaces[current->sub_namespace_count].name = fossil_crabdb_strdup(sub_namespace_name);
            current->sub_namespaces[current->sub_namespace_count].sub_namespaces = NULL;
            current->sub_namespaces[current->sub_namespace_count].sub_namespace_count = 0;
            current->sub_namespaces[current->sub_namespace_count].next = NULL;
            current->sub_namespaces[current->sub_namespace_count].data = NULL;

            current->sub_namespace_count++;
            return CRABDB_OK;
        }
        current = current->next;
    }

    return CRABDB_ERR_NS_NOT_FOUND;
} // end of fun

/**
 * @brief Erase a namespace.
 * 
 * @param db Pointer to the fossil_crabdb_t database.
 * @param namespace_name Name of the namespace to erase.
 * @return Error code indicating the result of the operation.
 */
fossil_crabdb_error_t fossil_crabdb_erase_namespace(fossil_crabdb_t *db, const char *namespace_name) {
    if (!db || !namespace_name) return CRABDB_ERR_MEM;

    fossil_crabdb_namespace_t *prev = NULL;
    fossil_crabdb_namespace_t *current = db->namespaces;

    while (current) {
        if (strcmp(current->name, namespace_name) == 0) {
            if (prev) {
                prev->next = current->next;
            } else {
                db->namespaces = current->next;
            }

            fossil_crabdb_free(current->name);

            for (size_t i = 0; i < current->sub_namespace_count; i++) {
                fossil_crabdb_free(current->sub_namespaces[i].name);
            }
            fossil_crabdb_free(current->sub_namespaces);

            fossil_crabdb_keyvalue_t *kv = current->data;
            while (kv) {
                fossil_crabdb_keyvalue_t *kv_next = kv->next;
                fossil_crabdb_free(kv->key);
                fossil_crabdb_free(kv->value);
                fossil_crabdb_free(kv);
                kv = kv_next;
            }

            fossil_crabdb_free(current);
            return CRABDB_OK;
        }
        prev = current;
        current = current->next;
    }

    return CRABDB_ERR_NS_NOT_FOUND;
} // end of fun

/**
 * @brief Erase a sub-namespace.
 * 
 * @param db Pointer to the fossil_crabdb_t database.
 * @param namespace_name Name of the parent namespace.
 * @param sub_namespace_name Name of the sub-namespace to erase.
 * @return Error code indicating the result of the operation.
 */
fossil_crabdb_error_t fossil_crabdb_erase_sub_namespace(fossil_crabdb_t *db, const char *namespace_name, const char *sub_namespace_name) {
    if (!db || !namespace_name || !sub_namespace_name) return CRABDB_ERR_MEM;

    fossil_crabdb_namespace_t *current = db->namespaces;
    while (current) {
        if (strcmp(current->name, namespace_name) == 0) {
            for (size_t i = 0; i < current->sub_namespace_count; i++) {
                if (strcmp(current->sub_namespaces[i].name, sub_namespace_name) == 0) {
                    fossil_crabdb_free(current->sub_namespaces[i].name);

                    for (size_t j = i; j < current->sub_namespace_count - 1; j++) {
                        current->sub_namespaces[j] = current->sub_namespaces[j + 1];
                    }

                    current->sub_namespaces = (fossil_crabdb_namespace_t*)fossil_crabdb_realloc(current->sub_namespaces, sizeof(fossil_crabdb_namespace_t) * (current->sub_namespace_count - 1));
                    current->sub_namespace_count--;

                    return CRABDB_OK;
                }
            }
        }
        current = current->next;
    }

    return CRABDB_ERR_SUB_NS_NOT_FOUND;
} // end of fun

/**
 * @brief Create a deep copy of a namespace.
 * 
 * @param original Pointer to the original namespace to copy.
 * @return Pointer to the copied namespace.
 */
fossil_crabdb_namespace_t* fossil_crabdb_copy_namespace(const fossil_crabdb_namespace_t *original) {
    if (!original) return NULL;

    fossil_crabdb_namespace_t *copy = fossil_crabdb_alloc(sizeof(fossil_crabdb_namespace_t));
    if (!copy) return NULL;

    copy->name = fossil_crabdb_strdup(original->name);
    copy->sub_namespace_count = original->sub_namespace_count;
    copy->sub_namespaces = (fossil_crabdb_namespace_t *)fossil_crabdb_alloc(sizeof(fossil_crabdb_namespace_t) * original->sub_namespace_count);

    for (size_t i = 0; i < original->sub_namespace_count; i++) {
        copy->sub_namespaces[i] = *fossil_crabdb_copy_namespace(&original->sub_namespaces[i]);
    }

    // Deep copy of key-value pairs
    copy->data = NULL;
    fossil_crabdb_keyvalue_t *kv_current = original->data;
    while (kv_current) {
        fossil_crabdb_keyvalue_t *kv_copy = fossil_crabdb_alloc(sizeof(fossil_crabdb_keyvalue_t));
        kv_copy->key = fossil_crabdb_strdup(kv_current->key);
        kv_copy->value = fossil_crabdb_strdup(kv_current->value);
        kv_copy->next = copy->data;
        copy->data = kv_copy;
        kv_current = kv_current->next;
    }

    return copy;
}

/**
 * @brief List all namespaces.
 * 
 * @param db Pointer to the fossil_crabdb_t database.
 * @param namespaces Pointer to store the list of namespace names.
 * @param count Pointer to store the number of namespaces.
 * @return Error code indicating the result of the operation.
 */
fossil_crabdb_error_t fossil_crabdb_list_namespaces(fossil_crabdb_t *db, char ***namespaces, size_t *count) {
    if (!db || !namespaces || !count) return CRABDB_ERR_MEM;

    size_t ns_count = 0;
    fossil_crabdb_namespace_t *current = db->namespaces;

    // First, count the number of namespaces
    while (current) {
        ns_count++;
        current = current->next;
    }

    // Allocate memory for the array of namespace names
    *namespaces = (char **)fossil_crabdb_alloc(sizeof(char *) * ns_count);
    if (!*namespaces) return CRABDB_ERR_MEM;

    // Populate the array with namespace names
    current = db->namespaces;
    for (size_t i = 0; i < ns_count; i++) {
        (*namespaces)[i] = fossil_crabdb_strdup(current->name);
        if (!(*namespaces)[i]) return CRABDB_ERR_MEM; // Handle memory allocation failure
        current = current->next;
    }

    *count = ns_count;
    return CRABDB_OK;
}

/**
 * @brief List all keys in a namespace.
 * 
 * @param db Pointer to the fossil_crabdb_t database.
 * @param namespace_name Name of the namespace.
 * @param keys Pointer to store the list of keys.
 * @param count Pointer to store the number of keys.
 * @return Error code indicating the result of the operation.
 */
fossil_crabdb_error_t fossil_crabdb_list_namespaces_keys(fossil_crabdb_t *db, const char *namespace_name, char ***keys, size_t *count) {
    if (!db || !namespace_name || !keys || !count) return CRABDB_ERR_MEM;

    fossil_crabdb_namespace_t *current = db->namespaces;
    while (current) {
        if (strcmp(current->name, namespace_name) == 0) {
            size_t key_count = 0;
            fossil_crabdb_keyvalue_t *kv_current = current->data;

            // First, count the number of keys
            while (kv_current) {
                key_count++;
                kv_current = kv_current->next;
            }

            // Allocate memory for the array of keys
            *keys = (char **)fossil_crabdb_alloc(sizeof(char *) * key_count);
            if (!*keys) return CRABDB_ERR_MEM;

            // Populate the array with keys
            kv_current = current->data;
            for (size_t i = 0; i < key_count; i++) {
                (*keys)[i] = fossil_crabdb_strdup(kv_current->key);
                if (!(*keys)[i]) return CRABDB_ERR_MEM; // Handle memory allocation failure
                kv_current = kv_current->next;
            }

            *count = key_count;
            return CRABDB_OK;
        }
        current = current->next;
    }

    return CRABDB_ERR_NS_NOT_FOUND;
}

/**
 * @brief Get statistics for a namespace.
 * 
 * @param db Pointer to the fossil_crabdb_t database.
 * @param namespace_name Name of the namespace to get statistics for.
 * @param key_count Pointer to store the number of keys.
 * @param sub_namespace_count Pointer to store the number of sub-namespaces.
 * @return Error code indicating the result of the operation.
 */
fossil_crabdb_error_t fossil_crabdb_get_namespace_stats(fossil_crabdb_t *db, const char *namespace_name, size_t *key_count, size_t *sub_namespace_count) {
    if (!db || !namespace_name || !key_count || !sub_namespace_count) return CRABDB_ERR_MEM;

    fossil_crabdb_namespace_t *current = db->namespaces;
    while (current) {
        if (strcmp(current->name, namespace_name) == 0) {
            *key_count = 0;
            *sub_namespace_count = current->sub_namespace_count;

            fossil_crabdb_keyvalue_t *kv_current = current->data;
            while (kv_current) {
                (*key_count)++;
                kv_current = kv_current->next;
            }

            return CRABDB_OK;
        }
        current = current->next;
    }

    return CRABDB_ERR_NS_NOT_FOUND;
}

/**
 * @brief Rename a namespace.
 * 
 * @param db Pointer to the fossil_crabdb_t database.
 * @param old_namespace_name Current name of the namespace.
 * @param new_namespace_name New name for the namespace.
 * @return Error code indicating the result of the operation.
 */
fossil_crabdb_error_t fossil_crabdb_rename_namespace(fossil_crabdb_t *db, const char *old_namespace_name, const char *new_namespace_name) {
    if (!db || !old_namespace_name || !new_namespace_name) return CRABDB_ERR_MEM;

    fossil_crabdb_namespace_t *current = db->namespaces;
    while (current) {
        if (strcmp(current->name, old_namespace_name) == 0) {
            fossil_crabdb_free(current->name);
            current->name = fossil_crabdb_strdup(new_namespace_name);
            return CRABDB_OK;
        }
        current = current->next;
    }

    return CRABDB_ERR_NS_NOT_FOUND;
}

//
// DATABASE QUERRY LANGUAGE
//

/**
 * @brief Create a new namespace.
 * 
 * @param db Pointer to the fossil_crabdb_t database.
 * @param namespace_name Name of the new namespace.
 * @return Error code indicating the result of the operation.
 */
static fossil_crabdb_error_t parse_and_execute(fossil_crabdb_t *db, char *command, char **tokens, int token_count) {
    if (strcmp(command, "create_namespace") == 0) {
        if (token_count == 1) {
            return fossil_crabdb_create_namespace(db, tokens[0]);
        }
    } else if (strcmp(command, "create_sub_namespace") == 0) {
        if (token_count == 2) {
            return fossil_crabdb_create_sub_namespace(db, tokens[0], tokens[1]);
        }
    } else if (strcmp(command, "erase_namespace") == 0) {
        if (token_count == 1) {
            return fossil_crabdb_erase_namespace(db, tokens[0]);
        }
    } else if (strcmp(command, "erase_sub_namespace") == 0) {
        if (token_count == 2) {
            return fossil_crabdb_erase_sub_namespace(db, tokens[0], tokens[1]);
        }
    } else if (strcmp(command, "insert") == 0) {
        if (token_count == 3) {
            return fossil_crabdb_insert(db, tokens[0], tokens[1], tokens[2]);
        }
    } else if (strcmp(command, "get") == 0) {
        if (token_count == 2) {
            char *value;
            fossil_crabdb_error_t err = fossil_crabdb_get(db, tokens[0], tokens[1], &value);
            if (err == CRABDB_OK) {
                printf("Value: %s\n", value);
                fossil_crabdb_free(value);
            }
            return err;
        }
    } else if (strcmp(command, "update") == 0) {
        if (token_count == 3) {
            return fossil_crabdb_update(db, tokens[0], tokens[1], tokens[2]);
        }
    } else if (strcmp(command, "delete") == 0) {
        if (token_count == 2) {
            return fossil_crabdb_delete(db, tokens[0], tokens[1]);
        }
    } else if (strcmp(command, "list_namespaces") == 0) {
        if (token_count == 0) {
            char **namespaces;
            size_t count;
            fossil_crabdb_error_t err = fossil_crabdb_list_namespaces(db, &namespaces, &count);
            if (err == CRABDB_OK) {
                for (size_t i = 0; i < count; i++) {
                    printf("Namespace: %s\n", namespaces[i]);
                    fossil_crabdb_free(namespaces[i]);
                }
                fossil_crabdb_free(namespaces);
            }
            return err;
        }
    } else if (strcmp(command, "list_namespaces_keys") == 0) {
        if (token_count == 1) {
            char **keys;
            size_t count;
            fossil_crabdb_error_t err = fossil_crabdb_list_namespaces_keys(db, tokens[0], &keys, &count);
            if (err == CRABDB_OK) {
                for (size_t i = 0; i < count; i++) {
                    printf("Key: %s\n", keys[i]);
                    fossil_crabdb_free(keys[i]);
                }
                fossil_crabdb_free(keys);
            }
            return err;
        }
    } else if (strcmp(command, "get_namespace_stats") == 0) {
        if (token_count == 1) {
            size_t key_count, sub_namespace_count;
            fossil_crabdb_error_t err = fossil_crabdb_get_namespace_stats(db, tokens[0], &key_count, &sub_namespace_count);
            if (err == CRABDB_OK) {
                printf("Key Count: %llu\n", key_count);
                printf("Sub-Namespace Count: %llu\n", sub_namespace_count);
            }
            return err;
        }
    } else if (strcmp(command, "rename_namespace") == 0) {
        if (token_count == 2) {
            return fossil_crabdb_rename_namespace(db, tokens[0], tokens[1]);
        }
    }

    return CRABDB_ERR_INVALID_QUERY;
} // end of fun

/**
 * @brief Execute a query.
 * 
 * @param db Pointer to the fossil_crabdb_t database.
 * @param query Query to execute.
 * @return Error code indicating the result of the operation.
 */
fossil_crabdb_error_t fossil_crabdb_execute_query(fossil_crabdb_t *db, const char *query) {
    if (!db || !query) return CRABDB_ERR_INVALID_QUERY;

    char *query_copy = fossil_crabdb_strdup(query);
    if (!query_copy) return CRABDB_ERR_MEM;

    char *command = strtok(query_copy, "(");
    if (!command) {
        fossil_crabdb_free(query_copy);
        return CRABDB_ERR_INVALID_QUERY;
    }

    char *args = strtok(NULL, ")");
    if (!args) {
        fossil_crabdb_free(query_copy);
        return CRABDB_ERR_INVALID_QUERY;
    }

    // Trim spaces from the command
    while (isspace((unsigned char)*command)) command++;
    char *end = command + strlen(command) - 1;
    while (end > command && isspace((unsigned char)*end)) end--;
    end[1] = '\0';

    // Tokenize the arguments
    char *token;
    char *tokens[10]; // Assuming a max of 10 arguments
    int token_count = 0;
    token = strtok(args, ",");
    while (token) {
        while (isspace((unsigned char)*token)) token++;
        end = token + strlen(token) - 1;
        while (end > token && isspace((unsigned char)*end)) end--;
        end[1] = '\0';
        tokens[token_count++] = token;
        token = strtok(NULL, ",");
    }

    fossil_crabdb_error_t result = parse_and_execute(db, command, tokens, token_count);

    fossil_crabdb_free(query_copy);
    return result;
} // end of fun

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

#include <stdio.h>


// Helper function to create a new node
static fossil_crabdb_node_t* create_node(const char* key, const char* value, fossil_crabdb_type_t type) {
    fossil_crabdb_node_t* node = (fossil_crabdb_node_t*)malloc(sizeof(fossil_crabdb_node_t));
    if (!node) return NULL;
    strncpy(node->key, key, _FOSSIL_CRABDB_KEY_SIZE);
    strncpy(node->value, value, _FOSSIL_CRABDB_VAL_SIZE);
    node->type = type;
    node->prev = NULL;
    node->next = NULL;
    return node;
}

// Function to create a new db with mutex initialization
fossil_crabdb_t* fossil_crabdb_create(void) {
    fossil_crabdb_t* db = (fossil_crabdb_t*)malloc(sizeof(fossil_crabdb_t));
    if (!db) return NULL;
    
    db->head = NULL;
    db->tail = NULL;
    
    return db;
}

// Function to destroy a db with mutex destruction
void fossil_crabdb_destroy(fossil_crabdb_t* db) {
    if (!db) return;

    fossil_crabdb_node_t* current = db->head;
    while (current) {
        fossil_crabdb_node_t* next = current->next;
        free(current);  // Free each node
        current = next;
    }
    
    fossil_crabdb_free(db);  // Free the db structure
}

// Insert a key-value pair
bool fossil_crabdb_insert(fossil_crabdb_t* db, const char* key, const char* value, fossil_crabdb_type_t type) {
    if (!db || !key || !value) return false;
    
    // Check if key already exists and update if needed
    fossil_crabdb_node_t* current = db->head;
    while (current) {
        if (strcmp(current->key, key) == 0) {
            strncpy(current->value, value, _FOSSIL_CRABDB_VAL_SIZE);
            current->type = type;
            return true;
        }
        current = current->next;
    }
    
    // Add new node
    fossil_crabdb_node_t* new_node = create_node(key, value, type);
    if (!new_node) {
        return false;
    }
    if (!db->head) {
        db->head = new_node;
        db->tail = new_node;
    } else {
        db->tail->next = new_node;
        new_node->prev = db->tail;
        db->tail = new_node;
    }

    return true;
}

// Update an existing key-value pair
bool fossil_crabdb_update(fossil_crabdb_t* db, const char* key, const char* value) {
    return fossil_crabdb_insert(db, key, value, FOSSIL_CRABDB_TYPE_STRING);
}

// Delete a key-value pair
bool fossil_crabdb_delete(fossil_crabdb_t* db, const char* key) {
    if (!db || !key) return false;
    
    fossil_crabdb_node_t* current = db->head;
    while (current) {
        if (strcmp(current->key, key) == 0) {
            if (current->prev) current->prev->next = current->next;
            if (current->next) current->next->prev = current->prev;
            if (current == db->head) db->head = current->next;
            if (current == db->tail) db->tail = current->prev;
            fossil_crabdb_free(current);
            return true;
        }
        current = current->next;
    }
    
    return false;
}

// Select a value by key
bool fossil_crabdb_select(fossil_crabdb_t* db, const char* key, char* value, size_t value_size) {
    if (!db || !key || !value) return false;
    
    fossil_crabdb_node_t* current = db->head;
    while (current) {
        if (strcmp(current->key, key) == 0) {
            strncpy(value, current->value, value_size);
            return true;
        }
        current = current->next;
    }
    
    return false;
}

// List all key-value pairs
bool fossil_crabdb_list(fossil_crabdb_t* db, char* list_buffer, size_t buffer_size) {
    if (!db || !list_buffer) return false;

    size_t offset = 0;
    fossil_crabdb_node_t* current = db->head;
    while (current) {
        int written = snprintf(list_buffer + offset, buffer_size - offset, "%s: %s\n", current->key, current->value);
        if (written < 0 || (size_t)written >= buffer_size - offset) {
            return false;
        }
        offset += written;
        current = current->next;
    }
    return true;
}

// Clear all key-value pairs
bool fossil_crabdb_clear(fossil_crabdb_t* db) {
    if (!db) return false;

    fossil_crabdb_node_t* current = db->head;
    while (current) {
        fossil_crabdb_node_t* next = current->next;
        fossil_crabdb_free(current);
        current = next;
    }
    db->head = NULL;
    db->tail = NULL;
    return true;
}

// Show all key-value pairs (similar to LIST but with specific formatting if needed)
bool fossil_crabdb_show(fossil_crabdb_t* db) {
    if (!db) return false;

    fossil_crabdb_node_t* current = db->head;
    while (current) {
        printf("%s: %s\n", current->key, current->value);
        current = current->next;
    }
    return true;
}

// Drop the database (clear all data)
bool fossil_crabdb_drop(fossil_crabdb_t* db) {
    return fossil_crabdb_clear(db); // Clear is already thread-safe
}

// Check if a key exists
bool fossil_crabdb_exist(fossil_crabdb_t* db, const char* key) {
    if (!db || !key) return false;

    fossil_crabdb_node_t* current = db->head;
    while (current) {
        if (strcmp(current->key, key) == 0) {
            return true;
        }
        current = current->next;
    }
    return false;
}

// Compact the db by removing nodes marked for deletion (empty keys)
bool fossil_crabdb_compact(fossil_crabdb_t* db) {
    if (!db || !db->head) return false;

    fossil_crabdb_node_t* current = db->head;
    while (current) {
        fossil_crabdb_node_t* next = current->next;

        // Assuming an empty key means the node is marked for deletion
        if (strcmp(current->key, "") == 0) {
            // Update the pointers of adjacent nodes
            if (current->prev) {
                current->prev->next = current->next;
            } else {
                db->head = current->next; // Adjust head if deleting the first node
            }

            if (current->next) {
                current->next->prev = current->prev;
            } else {
                db->tail = current->prev; // Adjust tail if deleting the last node
            }

            // Free the node memory
            fossil_crabdb_free(current);
        }

        current = next;
    }
    return true;
}

bool fossil_crabdb_rename_key(fossil_crabdb_t* db, const char* old_key, const char* new_key) {
    if (!db || !old_key || !new_key) return false;

    fossil_crabdb_node_t* current = db->head;
    while (current) {
        if (strcmp(current->key, old_key) == 0) {
            strncpy(current->key, new_key, _FOSSIL_CRABDB_KEY_SIZE);
            return true;
        }
        current = current->next;
    }

    return false;
}

bool fossil_crabdb_check_duplicates(fossil_crabdb_t* db) {
    if (!db) return false;

    fossil_crabdb_node_t* current = db->head;
    while (current) {
        fossil_crabdb_node_t* next = current->next;
        while (next) {
            if (strcmp(current->value, next->value) == 0) {
                return true;
            }
            next = next->next;
        }
        current = current->next;
    }

    return false;
}

bool fossil_crabdb_merge(fossil_crabdb_t* dest_db, fossil_crabdb_t* src_db) {
    if (!dest_db || !src_db) return false;

    fossil_crabdb_node_t* current = src_db->head;
    while (current) {
        if (!fossil_crabdb_insert(dest_db, current->key, current->value, current->type)) {
            return false;
        }
        current = current->next;
    }

    return true;
}

bool fossil_crabdb_copy(fossil_crabdb_t* src_db, fossil_crabdb_t* dest_db) {
    if (!src_db || !dest_db) return false;

    fossil_crabdb_node_t* current = src_db->head;
    while (current) {
        if (!fossil_crabdb_insert(dest_db, current->key, current->value, current->type)) {
            return false;
        }
        current = current->next;
    }

    return true;
}

size_t fossil_crabdb_memory_usage(fossil_crabdb_t* db) {
    if (!db) return 0;

    size_t total = 0;
    fossil_crabdb_node_t* current = db->head;
    while (current) {
        total += sizeof(fossil_crabdb_node_t);
        current = current->next;
    }

    return total;
}

bool fossil_crabdb_iterate(fossil_crabdb_t* db, void (*callback)(const char* key, const char* value)) {
    if (!db || !callback) return false;

    fossil_crabdb_node_t* current = db->head;
    while (current) {
        callback(current->key, current->value);
        current = current->next;
    }

    return true;
}

// Thread-safe batch insert of key-value pairs
bool fossil_crabdb_batch_insert(fossil_crabdb_t* db, const char keys[][_FOSSIL_CRABDB_KEY_SIZE], const char values[][_FOSSIL_CRABDB_VAL_SIZE], fossil_crabdb_type_t types[], size_t count) {
    if (!db || !keys || !values || !types || count == 0) {
        return false;
    }

    for (size_t i = 0; i < count; i++) {
        fossil_crabdb_node_t* new_node = (fossil_crabdb_node_t*)malloc(sizeof(fossil_crabdb_node_t));
        if (!new_node) {
            return false;
        }

        // Copy key, value, and type into the new node
        strncpy(new_node->key, keys[i], _FOSSIL_CRABDB_KEY_SIZE);
        strncpy(new_node->value, values[i], _FOSSIL_CRABDB_VAL_SIZE);
        new_node->type = types[i];
        new_node->prev = db->tail;
        new_node->next = NULL;

        // Update db links
        if (db->tail) {
            db->tail->next = new_node;
        } else {
            db->head = new_node; // First node inserted, update head
        }

        db->tail = new_node; // Always update the tail to the new node
    }

    return true;
}

// Thread-safe batch delete of key-value pairs by key
bool fossil_crabdb_batch_delete(fossil_crabdb_t* db, const char keys[][_FOSSIL_CRABDB_KEY_SIZE], size_t count) {
    if (!db || !keys || count == 0) {
        return false;
    }

    for (size_t i = 0; i < count; i++) {
        fossil_crabdb_node_t* current = db->head;
        while (current) {
            if (strncmp(current->key, keys[i], _FOSSIL_CRABDB_KEY_SIZE) == 0) {
                // Mark the node as deleted by clearing its key and value
                memset(current->key, 0, _FOSSIL_CRABDB_KEY_SIZE);
                memset(current->value, 0, _FOSSIL_CRABDB_VAL_SIZE);
                break; // Move to the next key after deleting the match
            }
            current = current->next;
        }
    }

    return true;
}

// Thread-safe backup of the database to a file
bool fossil_crabdb_backup(const char* filename, fossil_crabdb_t* db) {
    if (!filename || !db) {
        return false;
    }

    FILE* file = fopen(filename, "wb");
    if (!file) {
        return false;
    }

    fossil_crabdb_node_t* current = db->head;
    while (current) {
        if (strcmp(current->key, "") != 0) { // Skip deleted nodes (empty key)
            fwrite(&current->type, sizeof(current->type), 1, file);
            fwrite(current->key, sizeof(char), _FOSSIL_CRABDB_KEY_SIZE, file);
            fwrite(current->value, sizeof(char), _FOSSIL_CRABDB_VAL_SIZE, file);
        }
        current = current->next;
    }

    fclose(file);
    return true;
}

bool fossil_crabdb_restore(const char* filename, fossil_crabdb_t* db) {
    if (!filename || !db) {
        return false;
    }

    FILE* file = fopen(filename, "rb");
    if (!file) {
        return false;
    }

    // Clear the db before restoring
    fossil_crabdb_node_t* current = db->head;
    while (current) {
        fossil_crabdb_node_t* next = current->next;
        free(current);
        current = next;
    }
    db->head = db->tail = NULL;

    while (1) {
        fossil_crabdb_node_t* new_node = (fossil_crabdb_node_t*)malloc(sizeof(fossil_crabdb_node_t));
        if (!new_node) {
            fclose(file);
            return false;  // Memory allocation failed
        }

        // Read the type first
        if (fread(&new_node->type, sizeof(new_node->type), 1, file) != 1) {
            free(new_node);
            if (feof(file)) {
                break;  // End of file reached
            } else {
                fclose(file);
                return false;  // Error reading the file
            }
        }

        // Read the key
        if (fread(new_node->key, sizeof(char), _FOSSIL_CRABDB_KEY_SIZE, file) != _FOSSIL_CRABDB_KEY_SIZE) {
            free(new_node);
            fclose(file);
            return false;  // Error reading the key
        }

        // Read the value
        if (fread(new_node->value, sizeof(char), _FOSSIL_CRABDB_VAL_SIZE, file) != _FOSSIL_CRABDB_VAL_SIZE) {
            free(new_node);
            fclose(file);
            return false;  // Error reading the value
        }

        // Insert node into the db
        new_node->prev = db->tail;
        new_node->next = NULL;

        if (db->tail) {
            db->tail->next = new_node;
        } else {
            db->head = new_node;
        }

        db->tail = new_node;
    }

    fclose(file);
    return true;
}

//
// ALGORITHMS
//

// Compare nodes by key
static int compare_by_key(const void* a, const void* b) {
    return strcmp(((fossil_crabdb_node_t*)a)->key, ((fossil_crabdb_node_t*)b)->key);
}

// Compare nodes by value
static int compare_by_value(const void* a, const void* b) {
    return strcmp(((fossil_crabdb_node_t*)a)->value, ((fossil_crabdb_node_t*)b)->value);
}

// Search for a node by key
bool fossil_crabdb_search_by_key(fossil_crabdb_t* db, const char* key, char* value, size_t value_size) {
    if (!db || !key || !value) return false;

    fossil_crabdb_node_t* current = db->head;
    while (current) {
        if (strcmp(current->key, key) == 0) {
            strncpy(value, current->value, value_size);
            return true;
        }
        current = current->next;
    }
    return false;
}

// Search for a node by value
bool fossil_crabdb_search_by_value(fossil_crabdb_t* db, const char* value, char* key_buffer, size_t key_buffer_size) {
    if (!db || !value || !key_buffer) return false;

    fossil_crabdb_node_t* current = db->head;
    while (current) {
        if (strcmp(current->value, value) == 0) {
            strncpy(key_buffer, current->key, key_buffer_size);
            return true;
        }
        current = current->next;
    }
    return false;
}

// Sort nodes by key
bool fossil_crabdb_sort_by_key(fossil_crabdb_t* db) {
    if (!db) return false;

    // Count nodes
    size_t count = 0;
    fossil_crabdb_node_t* current = db->head;
    while (current) {
        count++;
        current = current->next;
    }

    // Create an array of nodes
    fossil_crabdb_node_t** nodes = (fossil_crabdb_node_t**)fossil_crabdb_alloc(count * sizeof(fossil_crabdb_node_t*));
    if (!nodes) return false;

    // Fill the array
    size_t index = 0;
    current = db->head;
    while (current) {
        nodes[index++] = current;
        current = current->next;
    }

    // Sort the array by key
    qsort(nodes, count, sizeof(fossil_crabdb_node_t*), compare_by_key);

    // Rebuild the db
    db->head = nodes[0];
    db->tail = nodes[count - 1];
    db->head->prev = NULL;
    db->tail->next = NULL;
    for (size_t i = 0; i < count - 1; i++) {
        nodes[i]->next = nodes[i + 1];
        nodes[i + 1]->prev = nodes[i];
    }

    fossil_crabdb_free(nodes);
    return true;
}

// Sort nodes by value
bool fossil_crabdb_sort_by_value(fossil_crabdb_t* db) {
    if (!db) return false;

    // Count nodes
    size_t count = 0;
    fossil_crabdb_node_t* current = db->head;
    while (current) {
        count++;
        current = current->next;
    }

    // Create an array of nodes
    fossil_crabdb_node_t** nodes = (fossil_crabdb_node_t**)fossil_crabdb_alloc(count * sizeof(fossil_crabdb_node_t*));
    if (!nodes) return false;

    // Fill the array
    size_t index = 0;
    current = db->head;
    while (current) {
        nodes[index++] = current;
        current = current->next;
    }

    // Sort the array by value
    qsort(nodes, count, sizeof(fossil_crabdb_node_t*), compare_by_value);

    // Rebuild the db
    db->head = nodes[0];
    db->tail = nodes[count - 1];
    db->head->prev = NULL;
    db->tail->next = NULL;
    for (size_t i = 0; i < count - 1; i++) {
        nodes[i]->next = nodes[i + 1];
        nodes[i + 1]->prev = nodes[i];
    }

    fossil_crabdb_free(nodes);
    return true;
}

//
// DATABASE SERIALIZATION
//

const char* fossil_crabdb_type_to_string(fossil_crabdb_type_t type) {
    switch (type) {
        case FOSSIL_CRABDB_TYPE_INT8: return "i8";
        case FOSSIL_CRABDB_TYPE_INT16: return "i16";
        case FOSSIL_CRABDB_TYPE_INT32: return "i32";
        case FOSSIL_CRABDB_TYPE_INT64: return "i64";
        case FOSSIL_CRABDB_TYPE_UINT8: return "u8";
        case FOSSIL_CRABDB_TYPE_UINT16: return "u16";
        case FOSSIL_CRABDB_TYPE_UINT32: return "u32";
        case FOSSIL_CRABDB_TYPE_UINT64: return "u64";
        case FOSSIL_CRABDB_TYPE_OCTAL8: return "o8";
        case FOSSIL_CRABDB_TYPE_OCTAL16: return "o16";
        case FOSSIL_CRABDB_TYPE_OCTAL32: return "o32";
        case FOSSIL_CRABDB_TYPE_OCTAL64: return "o64";
        case FOSSIL_CRABDB_TYPE_HEX8: return "h8";
        case FOSSIL_CRABDB_TYPE_HEX16: return "h16";
        case FOSSIL_CRABDB_TYPE_HEX32: return "h32";
        case FOSSIL_CRABDB_TYPE_HEX64: return "h64";
        case FOSSIL_CRABDB_TYPE_BIN8: return "b8";
        case FOSSIL_CRABDB_TYPE_BIN16: return "b16";
        case FOSSIL_CRABDB_TYPE_BIN32: return "b32";
        case FOSSIL_CRABDB_TYPE_BIN64: return "b64";
        case FOSSIL_CRABDB_TYPE_FLOAT: return "f32";
        case FOSSIL_CRABDB_TYPE_DOUBLE: return "f64";
        case FOSSIL_CRABDB_TYPE_STRING: return "cstr";
        case FOSSIL_CRABDB_TYPE_BOOL: return "bool";
        case FOSSIL_CRABDB_TYPE_CHAR: return "char";
        case FOSSIL_CRABDB_TYPE_NULL: return "null";
        default: return "unknown";
    }
}

fossil_crabdb_type_t fossil_crabdb_type_from_string(const char* type_str) {
    if (strcmp(type_str, "i8") == 0) return FOSSIL_CRABDB_TYPE_INT8;
    if (strcmp(type_str, "i16") == 0) return FOSSIL_CRABDB_TYPE_INT16;
    if (strcmp(type_str, "i32") == 0) return FOSSIL_CRABDB_TYPE_INT32;
    if (strcmp(type_str, "i64") == 0) return FOSSIL_CRABDB_TYPE_INT64;
    if (strcmp(type_str, "u8") == 0) return FOSSIL_CRABDB_TYPE_UINT8;
    if (strcmp(type_str, "u16") == 0) return FOSSIL_CRABDB_TYPE_UINT16;
    if (strcmp(type_str, "u32") == 0) return FOSSIL_CRABDB_TYPE_UINT32;
    if (strcmp(type_str, "u64") == 0) return FOSSIL_CRABDB_TYPE_UINT64;
    if (strcmp(type_str, "o8") == 0) return FOSSIL_CRABDB_TYPE_OCTAL8;
    if (strcmp(type_str, "o16") == 0) return FOSSIL_CRABDB_TYPE_OCTAL16;
    if (strcmp(type_str, "o32") == 0) return FOSSIL_CRABDB_TYPE_OCTAL32;
    if (strcmp(type_str, "o64") == 0) return FOSSIL_CRABDB_TYPE_OCTAL64;
    if (strcmp(type_str, "h8") == 0) return FOSSIL_CRABDB_TYPE_HEX8;
    if (strcmp(type_str, "h16") == 0) return FOSSIL_CRABDB_TYPE_HEX16;
    if (strcmp(type_str, "h32") == 0) return FOSSIL_CRABDB_TYPE_HEX32;
    if (strcmp(type_str, "h64") == 0) return FOSSIL_CRABDB_TYPE_HEX64;
    if (strcmp(type_str, "b8") == 0) return FOSSIL_CRABDB_TYPE_BIN8;
    if (strcmp(type_str, "b16") == 0) return FOSSIL_CRABDB_TYPE_BIN16;
    if (strcmp(type_str, "b32") == 0) return FOSSIL_CRABDB_TYPE_BIN32;
    if (strcmp(type_str, "b64") == 0) return FOSSIL_CRABDB_TYPE_BIN64;
    if (strcmp(type_str, "f32") == 0) return FOSSIL_CRABDB_TYPE_FLOAT;
    if (strcmp(type_str, "f64") == 0) return FOSSIL_CRABDB_TYPE_DOUBLE;
    if (strcmp(type_str, "cstr") == 0) return FOSSIL_CRABDB_TYPE_STRING;
    if (strcmp(type_str, "bool") == 0) return FOSSIL_CRABDB_TYPE_BOOL;
    if (strcmp(type_str, "char") == 0) return FOSSIL_CRABDB_TYPE_CHAR;
    if (strcmp(type_str, "null") == 0) return FOSSIL_CRABDB_TYPE_NULL;
    
    // If no match, return a default or unknown type, based on your design.
    return FOSSIL_CRABDB_TYPE_NULL;  // or a custom "unknown" type if defined
}

// Function to encode the contents of a CrabDB db and save it to a file
bool fossil_crabdb_encode(const char* filename, fossil_crabdb_t* db) {
    if (!filename || !db) return false;

    // Check if filename ends with .crabdb
    size_t len = strlen(filename);
    if (len < 8 || strcmp(filename + len - 8, ".crabdb") != 0) return false;

    FILE* file = fopen(filename, "w");
    if (!file) return false;

    fossil_crabdb_node_t* current = db->head;
    while (current) {
        fprintf(file, "%s:%s:%s\n", fossil_crabdb_type_to_string(current->type), current->key, current->value);
        current = current->next;
    }

    fclose(file);
    return true;
}

// Function to decode the contents of a file and load it into a CrabDB db
bool fossil_crabdb_decode(const char* filename, fossil_crabdb_t* db) {
    if (!filename || !db) return false;

    // Check if filename ends with .crabdb
    size_t len = strlen(filename);
    if (len < 8 || strcmp(filename + len - 8, ".crabdb") != 0) return false;

    FILE* file = fopen(filename, "r");
    if (!file) return false;

    char line[1024];
    while (fgets(line, sizeof(line), file)) {
        line[strcspn(line, "\n")] = '\0';

        char* type_delim = strchr(line, ':');
        if (!type_delim) continue;

        *type_delim = '\0';
        char* type = line;
        char* key_value = type_delim + 1;

        char* key_delim = strchr(key_value, ':');
        if (!key_delim) continue;

        *key_delim = '\0';
        char* key = key_value;
        char* value = key_delim + 1;

        fossil_crabdb_insert(db, key, value, fossil_crabdb_type_from_string(type));
    }

    fclose(file);
    return true;
}

// Function to count the number of key-value pairs in a CrabDB db
size_t fossil_crabdb_count(fossil_crabdb_t* db) {
    if (!db) return 0;

    size_t count = 0;
    fossil_crabdb_node_t* current = db->head;
    while (current) {
        count++;
        current = current->next;
    }

    return count;
}

bool fossil_crabdb_export_csv(const char* filename, fossil_crabdb_t* db) {
    if (!filename || !db) return false;

    // Check if filename ends with .csv
    size_t len = strlen(filename);
    if (len < 4 || strcmp(filename + len - 4, ".csv") != 0) return false;

    FILE* file = fopen(filename, "w");
    if (!file) return false;

    fossil_crabdb_node_t* current = db->head;
    while (current) {
        fprintf(file, "\"%s\",\"%s\",\"%s\"\n", fossil_crabdb_type_to_string(current->type), current->key, current->value);
        current = current->next;
    }

    fclose(file);
    return true;
}

bool fossil_crabdb_import_csv(const char* filename, fossil_crabdb_t* db) {
    if (!filename || !db) return false;

    // Check if filename ends with .csv
    size_t len = strlen(filename);
    if (len < 4 || strcmp(filename + len - 4, ".csv") != 0) return false;

    FILE* file = fopen(filename, "r");
    if (!file) return false;

    char line[1024];
    while (fgets(line, sizeof(line), file)) {
        line[strcspn(line, "\n")] = '\0';

        char* type_start = strchr(line, '"') + 1;
        char* type_end = strchr(type_start, '"');
        if (!type_start || !type_end) continue;
        *type_end = '\0';

        char* key_start = strchr(type_end + 1, '"') + 1;
        char* key_end = strchr(key_start, '"');
        if (!key_start || !key_end) continue;
        *key_end = '\0';

        char* value_start = strchr(key_end + 1, '"') + 1;
        char* value_end = strchr(value_start, '"');
        if (!value_start || !value_end) continue;
        *value_end = '\0';

        fossil_crabdb_insert(db, key_start, value_start, fossil_crabdb_type_from_string(type_start));
    }

    fclose(file);
    return true;
}

// Define constants for command-line flags and options
#define FLAG_HELP "--help"
#define FLAG_VERBOSE "--verbose"

// Define a maximum length for each command line
#define MAX_COMMAND_LENGTH 1024
#define MAX_ARGS 10

// Helper function to trim leading and trailing whitespace
char* trim_whitespace(char* str) {
    char* end;

    // Trim leading space
    while (isspace((unsigned char)*str)) str++;

    if (*str == 0)  // All spaces?
        return str;

    // Trim trailing space
    end = str + strlen(str) - 1;
    while (end > str && isspace((unsigned char)*end)) end--;

    // Write new null terminator
    *(end + 1) = 0;

    return str;
}

//
// DATABASE COMMANDS
//

// Function to parse options
bool parse_options(char* arg, bool* verbose) {
    if (strcmp(arg, FLAG_HELP) == 0) {
        printf("Available commands:\n");
        printf("  insert <key> <value>\n");
        printf("  select <key>\n");
        printf("  update <key> <value>\n");
        printf("  delete <key>\n");
        printf("  list\n");
        printf("  clear\n");
        printf("  show\n");
        printf("  drop\n");
        printf("  exist <key>\n");
        printf("  search_key <key>\n");
        printf("  search_value <value>\n");
        printf("  sort_key\n");
        printf("  sort_value\n");
        printf("Flags:\n");
        printf("  --help          Show this help message\n");
        printf("  --verbose       Enable verbose output\n");
        return false;
    } else if (strcmp(arg, FLAG_VERBOSE) == 0) {
        *verbose = true;
        return true;
    }
    return false;
}

// Function to execute commands from the command line with support for flags and options
bool fossil_crabdb_commandline(fossil_crabdb_t* db) {
    if (!db) return false;

    char line[MAX_COMMAND_LENGTH];
    bool verbose = false;

    while (true) {
        printf("fossil_crabdb> ");
        if (!fgets(line, sizeof(line), stdin)) return false;

        // Remove newline character if present
        line[strcspn(line, "\n")] = '\0';

        // Exit on "exit" command
        if (strcmp(line, "exit") == 0) {
            break;
        }

        // Remove leading and trailing whitespace
        char* trimmed_line = trim_whitespace(line);
        if (*trimmed_line == '\0') continue;

        // Handle flags and options
        char* arg = strtok(trimmed_line, " ");
        if (arg && parse_options(arg, &verbose)) continue;

        // Extract the command type and arguments
        char* command_type = arg;
        char* args[MAX_COMMAND_LENGTH];
        int arg_count = 0;
        while (true) {
            char* arg = strtok(NULL, " ");
            if (!arg) break;

            // Handle single-quoted strings
            char* quoted_str = extract_single_quoted_string(arg);
            if (quoted_str) {
                args[arg_count++] = quoted_str;
            } else {
                args[arg_count++] = arg;
            }
        }

        // Execute command
        if (strcmp(command_type, "insert") == 0) {
            if (arg_count >= 2) {
                char* key = args[0];
                char* value = args[1];
                if (verbose) printf("Inserting key '%s' with value '%s'\n", key, value);
                fossil_crabdb_insert(db, key, value, FOSSIL_CRABDB_TYPE_STRING);
            }
        } else if (strcmp(command_type, "select") == 0) {
            if (arg_count >= 1) {
                char* key = args[0];
                char value[1024];
                if (fossil_crabdb_select(db, key, value, sizeof(value))) {
                    printf("Value: %s\n", value);
                }
            }
        } else if (strcmp(command_type, "update") == 0) {
            if (arg_count >= 2) {
                char* key = args[0];
                char* value = args[1];
                if (verbose) printf("Updating key '%s' with value '%s'\n", key, value);
                fossil_crabdb_update(db, key, value);
            }
        } else if (strcmp(command_type, "delete") == 0) {
            if (arg_count >= 1) {
                char* key = args[0];
                if (verbose) printf("Deleting key '%s'\n", key);
                fossil_crabdb_delete(db, key);
            }
        } else if (strcmp(command_type, "list") == 0) {
            char list_buffer[1024 * 10];
            if (fossil_crabdb_list(db, list_buffer, sizeof(list_buffer))) {
                printf("List:\n%s\n", list_buffer);
            }
        } else if (strcmp(command_type, "clear") == 0) {
            if (verbose) printf("Clearing the db\n");
            fossil_crabdb_clear(db);
        } else if (strcmp(command_type, "show") == 0) {
            if (verbose) printf("Showing the db\n");
            fossil_crabdb_show(db);
        } else if (strcmp(command_type, "drop") == 0) {
            if (verbose) printf("Dropping the db\n");
            fossil_crabdb_drop(db);
        } else if (strcmp(command_type, "exist") == 0) {
            if (arg_count >= 1) {
                char* key = args[0];
                bool exists = fossil_crabdb_exist(db, key);
                printf(exists ? "Key exists\n" : "Key does not exist\n");
            }
        } else if (strcmp(command_type, "search_key") == 0) {
            if (arg_count >= 1) {
                char* key = args[0];
                char value[1024];
                if (fossil_crabdb_search_by_key(db, key, value, sizeof(value))) {
                    printf("Value for key '%s': %s\n", key, value);
                }
            }
        } else if (strcmp(command_type, "search_value") == 0) {
            if (arg_count >= 1) {
                char* value = args[0];
                char key[1024];
                if (fossil_crabdb_search_by_value(db, value, key, sizeof(key))) {
                    printf("Key for value '%s': %s\n", value, key);
                }
            }
        } else if (strcmp(command_type, "sort_key") == 0) {
            if (verbose) printf("Sorting by key\n");
            fossil_crabdb_sort_by_key(db);
        } else if (strcmp(command_type, "sort_value") == 0) {
            if (verbose) printf("Sorting by value\n");
            fossil_crabdb_sort_by_value(db);
        } else {
            fprintf(stderr, "Unknown command: %s\n", command_type);
        }
    }

    return true;
}

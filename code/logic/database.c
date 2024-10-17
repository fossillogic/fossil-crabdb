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


// Initialize the mutex
static void fossil_crabdb_mutex_init(fossil_crabdb_mutex_t* mutex) {
#ifdef _WIN32
    InitializeCriticalSection(&mutex->cs);
#else
    pthread_mutex_init(&mutex->mutex, NULL);
#endif
}

// Lock the mutex
static void fossil_crabdb_mutex_lock(fossil_crabdb_mutex_t* mutex) {
#ifdef _WIN32
    EnterCriticalSection(&mutex->cs);
#else
    pthread_mutex_lock(&mutex->mutex);
#endif
}

// Unlock the mutex
static void fossil_crabdb_mutex_unlock(fossil_crabdb_mutex_t* mutex) {
#ifdef _WIN32
    LeaveCriticalSection(&mutex->cs);
#else
    pthread_mutex_unlock(&mutex->mutex);
#endif
}

// Destroy the mutex
static void fossil_crabdb_mutex_destroy(fossil_crabdb_mutex_t* mutex) {
#ifdef _WIN32
    DeleteCriticalSection(&mutex->cs);
#else
    pthread_mutex_destroy(&mutex->mutex);
#endif
}

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

// Function to create a new deque with mutex initialization
fossil_crabdb_deque_t* fossil_crabdb_create(void) {
    fossil_crabdb_deque_t* deque = (fossil_crabdb_deque_t*)malloc(sizeof(fossil_crabdb_deque_t));
    if (!deque) return NULL;
    
    deque->head = NULL;
    deque->tail = NULL;

    // Initialize the cross-platform mutex
    fossil_crabdb_mutex_init(&deque->mutex);
    
    return deque;
}

// Function to destroy a deque with mutex destruction
void fossil_crabdb_destroy(fossil_crabdb_deque_t* deque) {
    if (!deque) return;

    // Lock the mutex for thread-safe destruction
    fossil_crabdb_mutex_lock(&deque->mutex);

    fossil_crabdb_node_t* current = deque->head;
    while (current) {
        fossil_crabdb_node_t* next = current->next;
        free(current);  // Free each node
        current = next;
    }

    // Unlock and destroy the mutex
    fossil_crabdb_mutex_unlock(&deque->mutex);
    fossil_crabdb_mutex_destroy(&deque->mutex);
    
    fossil_crabdb_free(deque);  // Free the deque structure
}

// Insert a key-value pair
bool fossil_crabdb_insert(fossil_crabdb_deque_t* deque, const char* key, const char* value, fossil_crabdb_type_t type) {
    if (!deque || !key || !value) return false;
    
    fossil_crabdb_lock(deque);
    
    // Check if key already exists and update if needed
    fossil_crabdb_node_t* current = deque->head;
    while (current) {
        if (strcmp(current->key, key) == 0) {
            strncpy(current->value, value, _FOSSIL_CRABDB_VAL_SIZE);
            current->type = type;
            fossil_crabdb_unlock(deque);
            return true;
        }
        current = current->next;
    }
    
    // Add new node
    fossil_crabdb_node_t* new_node = create_node(key, value, type);
    if (!new_node) {
        fossil_crabdb_unlock(deque);
        return false;
    }
    if (!deque->head) {
        deque->head = new_node;
        deque->tail = new_node;
    } else {
        deque->tail->next = new_node;
        new_node->prev = deque->tail;
        deque->tail = new_node;
    }

    fossil_crabdb_unlock(deque);
    return true;
}

// Update an existing key-value pair
bool fossil_crabdb_update(fossil_crabdb_deque_t* deque, const char* key, const char* value) {
    return fossil_crabdb_insert(deque, key, value, FOSSIL_CRABDB_TYPE_STRING);
}

// Delete a key-value pair
bool fossil_crabdb_delete(fossil_crabdb_deque_t* deque, const char* key) {
    if (!deque || !key) return false;
    
    fossil_crabdb_lock(deque);
    
    fossil_crabdb_node_t* current = deque->head;
    while (current) {
        if (strcmp(current->key, key) == 0) {
            if (current->prev) current->prev->next = current->next;
            if (current->next) current->next->prev = current->prev;
            if (current == deque->head) deque->head = current->next;
            if (current == deque->tail) deque->tail = current->prev;
            fossil_crabdb_free(current);
            fossil_crabdb_unlock(deque);
            return true;
        }
        current = current->next;
    }
    
    fossil_crabdb_unlock(deque);
    return false;
}

// Select a value by key
bool fossil_crabdb_select(fossil_crabdb_deque_t* deque, const char* key, char* value, size_t value_size) {
    if (!deque || !key || !value) return false;
    
    fossil_crabdb_lock(deque);
    
    fossil_crabdb_node_t* current = deque->head;
    while (current) {
        if (strcmp(current->key, key) == 0) {
            strncpy(value, current->value, value_size);
            fossil_crabdb_unlock(deque);
            return true;
        }
        current = current->next;
    }
    
    fossil_crabdb_unlock(deque);
    return false;
}

// List all key-value pairs
bool fossil_crabdb_list(fossil_crabdb_deque_t* deque, char* list_buffer, size_t buffer_size) {
    if (!deque || !list_buffer) return false;

    pthread_mutex_lock(&deque->mutex); // Lock the deque for thread safety
    size_t offset = 0;
    fossil_crabdb_node_t* current = deque->head;
    while (current) {
        int written = snprintf(list_buffer + offset, buffer_size - offset, "%s: %s\n", current->key, current->value);
        if (written < 0 || (size_t)written >= buffer_size - offset) {
            pthread_mutex_unlock(&deque->mutex); // Unlock on failure
            return false;
        }
        offset += written;
        current = current->next;
    }
    pthread_mutex_unlock(&deque->mutex); // Unlock after operation
    return true;
}

// Clear all key-value pairs
bool fossil_crabdb_clear(fossil_crabdb_deque_t* deque) {
    if (!deque) return false;

    pthread_mutex_lock(&deque->mutex); // Lock the deque for thread safety
    fossil_crabdb_node_t* current = deque->head;
    while (current) {
        fossil_crabdb_node_t* next = current->next;
        fossil_crabdb_free(current);
        current = next;
    }
    deque->head = NULL;
    deque->tail = NULL;
    pthread_mutex_unlock(&deque->mutex); // Unlock after clearing
    return true;
}

// Show all key-value pairs (similar to LIST but with specific formatting if needed)
bool fossil_crabdb_show(fossil_crabdb_deque_t* deque) {
    if (!deque) return false;

    pthread_mutex_lock(&deque->mutex); // Lock the deque for thread safety
    fossil_crabdb_node_t* current = deque->head;
    while (current) {
        printf("%s: %s\n", current->key, current->value);
        current = current->next;
    }
    pthread_mutex_unlock(&deque->mutex); // Unlock after operation
    return true;
}

// Drop the database (clear all data)
bool fossil_crabdb_drop(fossil_crabdb_deque_t* deque) {
    return fossil_crabdb_clear(deque); // Clear is already thread-safe
}

// Check if a key exists
bool fossil_crabdb_exist(fossil_crabdb_deque_t* deque, const char* key) {
    if (!deque || !key) return false;

    pthread_mutex_lock(&deque->mutex); // Lock the deque for thread safety
    fossil_crabdb_node_t* current = deque->head;
    while (current) {
        if (strcmp(current->key, key) == 0) {
            pthread_mutex_unlock(&deque->mutex); // Unlock after finding key
            return true;
        }
        current = current->next;
    }
    pthread_mutex_unlock(&deque->mutex); // Unlock if key not found
    return false;
}

// Compact the deque by removing nodes marked for deletion (empty keys)
bool fossil_crabdb_compact(fossil_crabdb_deque_t* deque) {
    if (!deque || !deque->head) return false;

    pthread_mutex_lock(&deque->mutex); // Lock the deque for thread safety
    fossil_crabdb_node_t* current = deque->head;
    while (current) {
        fossil_crabdb_node_t* next = current->next;

        // Assuming an empty key means the node is marked for deletion
        if (strcmp(current->key, "") == 0) {
            // Update the pointers of adjacent nodes
            if (current->prev) {
                current->prev->next = current->next;
            } else {
                deque->head = current->next; // Adjust head if deleting the first node
            }

            if (current->next) {
                current->next->prev = current->prev;
            } else {
                deque->tail = current->prev; // Adjust tail if deleting the last node
            }

            // Free the node memory
            fossil_crabdb_free(current);
        }

        current = next;
    }
    pthread_mutex_unlock(&deque->mutex); // Unlock after operation
    return true;
}

// Thread-safe batch insert of key-value pairs
bool fossil_crabdb_batch_insert(fossil_crabdb_deque_t* deque, const char keys[][_FOSSIL_CRABDB_KEY_SIZE], const char values[][_FOSSIL_CRABDB_VAL_SIZE], fossil_crabdb_type_t types[], size_t count) {
    if (!deque || !keys || !values || !types || count == 0) {
        return false;
    }

    pthread_mutex_lock(&deque->mutex); // Lock the deque for thread safety

    for (size_t i = 0; i < count; i++) {
        fossil_crabdb_node_t* new_node = (fossil_crabdb_node_t*)malloc(sizeof(fossil_crabdb_node_t));
        if (!new_node) {
            pthread_mutex_unlock(&deque->mutex); // Unlock before returning
            return false;
        }

        // Copy key, value, and type into the new node
        strncpy(new_node->key, keys[i], _FOSSIL_CRABDB_KEY_SIZE);
        strncpy(new_node->value, values[i], _FOSSIL_CRABDB_VAL_SIZE);
        new_node->type = types[i];
        new_node->prev = deque->tail;
        new_node->next = NULL;

        // Update deque links
        if (deque->tail) {
            deque->tail->next = new_node;
        } else {
            deque->head = new_node; // First node inserted, update head
        }

        deque->tail = new_node; // Always update the tail to the new node
    }

    pthread_mutex_unlock(&deque->mutex); // Unlock after inserting
    return true;
}

// Thread-safe batch delete of key-value pairs by key
bool fossil_crabdb_batch_delete(fossil_crabdb_deque_t* deque, const char keys[][_FOSSIL_CRABDB_KEY_SIZE], size_t count) {
    if (!deque || !keys || count == 0) {
        return false;
    }

    pthread_mutex_lock(&deque->mutex); // Lock the deque for thread safety

    for (size_t i = 0; i < count; i++) {
        fossil_crabdb_node_t* current = deque->head;
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

    pthread_mutex_unlock(&deque->mutex); // Unlock after deleting
    return true;
}

// Thread-safe backup of the database to a file
bool fossil_crabdb_backup(const char* filename, fossil_crabdb_deque_t* deque) {
    if (!filename || !deque) {
        return false;
    }

    pthread_mutex_lock(&deque->mutex); // Lock the deque for thread safety

    FILE* file = fopen(filename, "wb");
    if (!file) {
        pthread_mutex_unlock(&deque->mutex); // Unlock before returning
        return false;
    }

    fossil_crabdb_node_t* current = deque->head;
    while (current) {
        if (strcmp(current->key, "") != 0) { // Skip deleted nodes (empty key)
            fwrite(&current->type, sizeof(current->type), 1, file);
            fwrite(current->key, sizeof(char), _FOSSIL_CRABDB_KEY_SIZE, file);
            fwrite(current->value, sizeof(char), _FOSSIL_CRABDB_VAL_SIZE, file);
        }
        current = current->next;
    }

    fclose(file);
    pthread_mutex_unlock(&deque->mutex); // Unlock after backing up
    return true;
}

bool fossil_crabdb_restore(const char* filename, fossil_crabdb_deque_t* deque) {
    if (!filename || !deque) {
        return false;
    }

    FILE* file = fopen(filename, "rb");
    if (!file) {
        return false;
    }

    // Clear the deque before restoring
    fossil_crabdb_node_t* current = deque->head;
    while (current) {
        fossil_crabdb_node_t* next = current->next;
        free(current);
        current = next;
    }
    deque->head = deque->tail = NULL;

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

        // Insert node into the deque
        new_node->prev = deque->tail;
        new_node->next = NULL;

        if (deque->tail) {
            deque->tail->next = new_node;
        } else {
            deque->head = new_node;
        }

        deque->tail = new_node;
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
bool fossil_crabdb_search_by_key(fossil_crabdb_deque_t* deque, const char* key, char* value, size_t value_size) {
    if (!deque || !key || !value) return false;

    fossil_crabdb_node_t* current = deque->head;
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
bool fossil_crabdb_search_by_value(fossil_crabdb_deque_t* deque, const char* value, char* key_buffer, size_t key_buffer_size) {
    if (!deque || !value || !key_buffer) return false;

    fossil_crabdb_node_t* current = deque->head;
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
bool fossil_crabdb_sort_by_key(fossil_crabdb_deque_t* deque) {
    if (!deque) return false;

    // Count nodes
    size_t count = 0;
    fossil_crabdb_node_t* current = deque->head;
    while (current) {
        count++;
        current = current->next;
    }

    // Create an array of nodes
    fossil_crabdb_node_t** nodes = (fossil_crabdb_node_t**)fossil_crabdb_alloc(count * sizeof(fossil_crabdb_node_t*));
    if (!nodes) return false;

    // Fill the array
    size_t index = 0;
    current = deque->head;
    while (current) {
        nodes[index++] = current;
        current = current->next;
    }

    // Sort the array by key
    qsort(nodes, count, sizeof(fossil_crabdb_node_t*), compare_by_key);

    // Rebuild the deque
    deque->head = nodes[0];
    deque->tail = nodes[count - 1];
    deque->head->prev = NULL;
    deque->tail->next = NULL;
    for (size_t i = 0; i < count - 1; i++) {
        nodes[i]->next = nodes[i + 1];
        nodes[i + 1]->prev = nodes[i];
    }

    fossil_crabdb_free(nodes);
    return true;
}

// Sort nodes by value
bool fossil_crabdb_sort_by_value(fossil_crabdb_deque_t* deque) {
    if (!deque) return false;

    // Count nodes
    size_t count = 0;
    fossil_crabdb_node_t* current = deque->head;
    while (current) {
        count++;
        current = current->next;
    }

    // Create an array of nodes
    fossil_crabdb_node_t** nodes = (fossil_crabdb_node_t**)fossil_crabdb_alloc(count * sizeof(fossil_crabdb_node_t*));
    if (!nodes) return false;

    // Fill the array
    size_t index = 0;
    current = deque->head;
    while (current) {
        nodes[index++] = current;
        current = current->next;
    }

    // Sort the array by value
    qsort(nodes, count, sizeof(fossil_crabdb_node_t*), compare_by_value);

    // Rebuild the deque
    deque->head = nodes[0];
    deque->tail = nodes[count - 1];
    deque->head->prev = NULL;
    deque->tail->next = NULL;
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

// Function to encode the contents of a CrabDB deque and save it to a file
bool fossil_crabdb_encode(const char* filename, fossil_crabdb_deque_t* deque) {
    if (!filename || !deque) return false;

    // Check if filename ends with .crabdb
    size_t len = strlen(filename);
    if (len < 8 || strcmp(filename + len - 8, ".crabdb") != 0) return false;

    FILE* file = fopen(filename, "w");
    if (!file) return false;

    fossil_crabdb_node_t* current = deque->head;
    while (current) {
        fprintf(file, "%s:%s:%s\n", fossil_crabdb_type_to_string(current->type), current->key, current->value);
        current = current->next;
    }

    fclose(file);
    return true;
}

// Function to decode the contents of a file and load it into a CrabDB deque
bool fossil_crabdb_decode(const char* filename, fossil_crabdb_deque_t* deque) {
    if (!filename || !deque) return false;

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

        fossil_crabdb_insert(deque, key, value, fossil_crabdb_type_from_string(type));
    }

    fclose(file);
    return true;
}

// Function to count the number of key-value pairs in a CrabDB deque
size_t fossil_crabdb_count(fossil_crabdb_deque_t* deque) {
    if (!deque) return 0;

    size_t count = 0;
    fossil_crabdb_node_t* current = deque->head;
    while (current) {
        count++;
        current = current->next;
    }

    return count;
}

bool fossil_crabdb_export_csv(const char* filename, fossil_crabdb_deque_t* deque) {
    if (!filename || !deque) return false;

    // Check if filename ends with .csv
    size_t len = strlen(filename);
    if (len < 4 || strcmp(filename + len - 4, ".csv") != 0) return false;

    FILE* file = fopen(filename, "w");
    if (!file) return false;

    fossil_crabdb_node_t* current = deque->head;
    while (current) {
        fprintf(file, "\"%s\",\"%s\",\"%s\"\n", fossil_crabdb_type_to_string(current->type), current->key, current->value);
        current = current->next;
    }

    fclose(file);
    return true;
}

bool fossil_crabdb_import_csv(const char* filename, fossil_crabdb_deque_t* deque) {
    if (!filename || !deque) return false;

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

        fossil_crabdb_insert(deque, key_start, value_start, fossil_crabdb_type_from_string(type_start));
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

// Function to handle single-quoted strings
char* extract_single_quoted_string(char* str) {
    char* start = strchr(str, '\'');
    if (!start) return NULL;
    start++;

    char* end = strchr(start, '\'');
    if (!end) return NULL;

    *end = '\0';
    return start;
}

// Helper function to check condition
bool check_condition(fossil_crabdb_deque_t* deque, char* condition) {
    if (!condition) return true; // No condition means condition is met

    // Parse condition type
    if (strncmp(condition, "EXISTS ", 7) == 0) {
        char* key = trim_whitespace(condition + 7);
        return fossil_crabdb_exist(deque, key);
    } else if (strncmp(condition, "VALUE_IS ", 9) == 0) {
        char* key = strtok(condition + 9, " ");
        char* expected_value = strtok(NULL, " ");
        if (key && expected_value) {
            char value[1024];
            return fossil_crabdb_select(deque, key, value, sizeof(value)) &&
                   strcmp(value, expected_value) == 0;
        }
    } else if (strncmp(condition, "KEY_STARTS_WITH ", 16) == 0) {
        char* prefix = trim_whitespace(condition + 16);
        fossil_crabdb_node_t* current = deque->head;
        while (current) {
            if (strncmp(current->key, prefix, strlen(prefix)) == 0) {
                return true;
            }
            current = current->next;
        }
    } else if (strncmp(condition, "VALUE_CONTAINS ", 15) == 0) {
        char* substring = trim_whitespace(condition + 15);
        fossil_crabdb_node_t* current = deque->head;
        while (current) {
            if (strstr(current->value, substring)) {
                return true;
            }
            current = current->next;
        }
    }

    return false; // Condition not met or unknown condition
}

// Helper function to execute a command
void execute_command(fossil_crabdb_deque_t* deque, char* command_type, char* args) {
    if (strcmp(command_type, "INSERT") == 0) {
        char* key = strtok(args, " ");
        char* value = strtok(NULL, " ");
        if (key && value) {
            fossil_crabdb_insert(deque, key, value, FOSSIL_CRABDB_TYPE_STRING);
        }
    } else if (strcmp(command_type, "SELECT") == 0) {
        char* key = strtok(args, " ");
        if (key) {
            char value[1024];
            if (fossil_crabdb_select(deque, key, value, sizeof(value))) {
                printf("%s\n", value);
            }
        }
    } else if (strcmp(command_type, "UPDATE") == 0) {
        char* key = strtok(args, " ");
        char* value = strtok(NULL, " ");
        if (key && value) {
            fossil_crabdb_update(deque, key, value);
        }
    } else if (strcmp(command_type, "DELETE") == 0) {
        char* key = strtok(args, " ");
        if (key) {
            fossil_crabdb_delete(deque, key);
        }
    } else if (strcmp(command_type, "LIST") == 0) {
        char list_buffer[1024 * 10];
        if (fossil_crabdb_list(deque, list_buffer, sizeof(list_buffer))) {
            printf("%s\n", list_buffer);
        }
    } else if (strcmp(command_type, "CLEAR") == 0) {
        fossil_crabdb_clear(deque);
    } else if (strcmp(command_type, "SHOW") == 0) {
        fossil_crabdb_show(deque);
    } else if (strcmp(command_type, "DROP") == 0) {
        fossil_crabdb_drop(deque);
    } else if (strcmp(command_type, "EXIST") == 0) {
        char* key = strtok(args, " ");
        if (key) {
            bool exists = fossil_crabdb_exist(deque, key);
            printf(exists ? "true\n" : "false\n");
        }
    } else if (strcmp(command_type, "SEARCH_KEY") == 0) {
        char* key = strtok(args, " ");
        char value[1024];
        if (key && fossil_crabdb_search_by_key(deque, key, value, sizeof(value))) {
            printf("%s\n", value);
        }
    } else if (strcmp(command_type, "SEARCH_VALUE") == 0) {
        char* value = strtok(args, " ");
        char key[1024];
        if (value && fossil_crabdb_search_by_value(deque, value, key, sizeof(key))) {
            printf("%s\n", key);
        }
    } else if (strcmp(command_type, "SORT_KEY") == 0) {
        fossil_crabdb_sort_by_key(deque);
    } else if (strcmp(command_type, "SORT_VALUE") == 0) {
        fossil_crabdb_sort_by_value(deque);
    } else {
        fprintf(stderr, "Unknown command: %s\n", command_type);
    }
}

//
// DATABASE QUERY LANGUAGE
//

// Main function to execute commands from a .crabql script file with conditions
bool fossil_crabdb_exec(const char* filename, fossil_crabdb_deque_t* deque) {
    if (!filename || !deque) {
        fprintf(stderr, "Invalid filename or deque.\n");
        return false;
    }

    // Check if filename ends with .crabql
    size_t len = strlen(filename);
    if (len < 7 || strcmp(filename + len - 7, ".crabql") != 0) {
        fprintf(stderr, "File must have a .crabql extension.\n");
        return false;
    }

    FILE* file = fopen(filename, "r");
    if (!file) {
        perror("Error opening file");
        return false;
    }

    char line[MAX_COMMAND_LENGTH];
    while (fgets(line, sizeof(line), file)) {
        // Remove newline character if present
        line[strcspn(line, "\n")] = '\0';

        // Trim whitespace from the line
        char* trimmed_line = trim_whitespace(line);

        // Parse the command type
        char* command_type = strtok(trimmed_line, " ");
        if (!command_type) continue;

        // Check for conditionals
        char* condition = strchr(trimmed_line, '?');
        if (condition) {
            *condition = '\0'; // Split command and condition
            condition++; // Skip '?'
        }

        // Check if the condition is met
        bool condition_met = check_condition(deque, condition);

        // Execute the command if the condition is met
        if (condition_met) {
            char* args = strtok(NULL, ""); // Get the rest of the arguments
            execute_command(deque, command_type, args);
        }
    }

    fclose(file);
    return true;
}

// Helper function to check if filename ends with .crabql
bool has_crabql_extension(const char* filename) {
    size_t len = strlen(filename);
    return len >= 6 && strcmp(filename + len - 6, ".crabql") == 0;
}

// Helper function to parse arguments within parentheses
int parse_arguments(char* line, char* args[], int max_args) {
    int arg_count = 0;

    // Find the opening parenthesis
    char* open_paren = strchr(line, '(');
    if (!open_paren) return 0;

    // Find the closing parenthesis
    char* close_paren = strchr(open_paren, ')');
    if (!close_paren) return 0;

    // Extract the arguments within the parentheses
    *close_paren = '\0';
    char* token = strtok(open_paren + 1, ",");
    while (token != NULL && arg_count < max_args) {
        args[arg_count++] = trim_whitespace(token);
        token = strtok(NULL, ",");
    }
    return arg_count;
}

// Helper function to execute a single command
void execute_script(const char* command_type, char* args[], int arg_count, fossil_crabdb_deque_t* deque) {
    if (strcmp(command_type, "insert") == 0) {
        if (arg_count >= 2) {
            fossil_crabdb_insert(deque, args[0], args[1], FOSSIL_CRABDB_TYPE_STRING);
        }
    } else if (strcmp(command_type, "select") == 0) {
        if (arg_count >= 1) {
            char value[1024];
            if (fossil_crabdb_select(deque, args[0], value, sizeof(value))) {
                printf("%s\n", value);
            }
        }
    } else if (strcmp(command_type, "update") == 0) {
        if (arg_count >= 2) {
            fossil_crabdb_update(deque, args[0], args[1]);
        }
    } else if (strcmp(command_type, "delete") == 0) {
        if (arg_count >= 1) {
            fossil_crabdb_delete(deque, args[0]);
        }
    } else if (strcmp(command_type, "list") == 0) {
        char list_buffer[1024 * 10];
        if (fossil_crabdb_list(deque, list_buffer, sizeof(list_buffer))) {
            printf("%s\n", list_buffer);
        }
    } else if (strcmp(command_type, "clear") == 0) {
        fossil_crabdb_clear(deque);
    } else if (strcmp(command_type, "show") == 0) {
        fossil_crabdb_show(deque);
    } else if (strcmp(command_type, "drop") == 0) {
        fossil_crabdb_drop(deque);
    } else if (strcmp(command_type, "exist") == 0) {
        if (arg_count >= 1) {
            bool exists = fossil_crabdb_exist(deque, args[0]);
            printf(exists ? "true\n" : "false\n");
        }
    } else if (strcmp(command_type, "search_key") == 0) {
        if (arg_count >= 1) {
            char value[1024];
            if (fossil_crabdb_search_by_key(deque, args[0], value, sizeof(value))) {
                printf("%s\n", value);
            }
        }
    } else if (strcmp(command_type, "search_value") == 0) {
        if (arg_count >= 1) {
            char key[1024];
            if (fossil_crabdb_search_by_value(deque, args[0], key, sizeof(key))) {
                printf("%s\n", key);
            }
        }
    } else if (strcmp(command_type, "sort_key") == 0) {
        fossil_crabdb_sort_by_key(deque);
    } else if (strcmp(command_type, "sort_value") == 0) {
        fossil_crabdb_sort_by_value(deque);
    } else {
        fprintf(stderr, "Unknown command: %s\n", command_type);
    }
}

// Main function to execute commands from a .crabql script file
bool fossil_crabdb_script(const char* filename, fossil_crabdb_deque_t* deque) {
    if (!filename || !deque) return false;
    if (!has_crabql_extension(filename)) return false;

    FILE* file = fopen(filename, "r");
    if (!file) return false;
    puts("TESTING\n\n");

    char line[MAX_COMMAND_LENGTH];
    while (fgets(line, sizeof(line), file)) {
        // Trim the line and skip comments and empty lines
        char* trimmed_line = trim_whitespace(line);
        if (*trimmed_line == '\0' || *trimmed_line == '#') continue;

        // Remove semicolon at the end if present
        if (trimmed_line[strlen(trimmed_line) - 1] == ';') {
            trimmed_line[strlen(trimmed_line) - 1] = '\0';
        }

        // Parse the command type
        char* command_type = strtok(trimmed_line, " ");
        if (!command_type) continue;

        // Parse arguments within parentheses
        char* args[MAX_ARGS];
        int arg_count = parse_arguments(trimmed_line, args, MAX_ARGS);

        // Execute the parsed command
        execute_script(command_type, args, arg_count, deque);
    }

    fclose(file);
    return true;
}

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
bool fossil_crabdb_commandline(fossil_crabdb_deque_t* deque) {
    if (!deque) return false;

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
                fossil_crabdb_insert(deque, key, value, FOSSIL_CRABDB_TYPE_STRING);
            }
        } else if (strcmp(command_type, "select") == 0) {
            if (arg_count >= 1) {
                char* key = args[0];
                char value[1024];
                if (fossil_crabdb_select(deque, key, value, sizeof(value))) {
                    printf("Value: %s\n", value);
                }
            }
        } else if (strcmp(command_type, "update") == 0) {
            if (arg_count >= 2) {
                char* key = args[0];
                char* value = args[1];
                if (verbose) printf("Updating key '%s' with value '%s'\n", key, value);
                fossil_crabdb_update(deque, key, value);
            }
        } else if (strcmp(command_type, "delete") == 0) {
            if (arg_count >= 1) {
                char* key = args[0];
                if (verbose) printf("Deleting key '%s'\n", key);
                fossil_crabdb_delete(deque, key);
            }
        } else if (strcmp(command_type, "list") == 0) {
            char list_buffer[1024 * 10];
            if (fossil_crabdb_list(deque, list_buffer, sizeof(list_buffer))) {
                printf("List:\n%s\n", list_buffer);
            }
        } else if (strcmp(command_type, "clear") == 0) {
            if (verbose) printf("Clearing the deque\n");
            fossil_crabdb_clear(deque);
        } else if (strcmp(command_type, "show") == 0) {
            if (verbose) printf("Showing the deque\n");
            fossil_crabdb_show(deque);
        } else if (strcmp(command_type, "drop") == 0) {
            if (verbose) printf("Dropping the deque\n");
            fossil_crabdb_drop(deque);
        } else if (strcmp(command_type, "exist") == 0) {
            if (arg_count >= 1) {
                char* key = args[0];
                bool exists = fossil_crabdb_exist(deque, key);
                printf(exists ? "Key exists\n" : "Key does not exist\n");
            }
        } else if (strcmp(command_type, "search_key") == 0) {
            if (arg_count >= 1) {
                char* key = args[0];
                char value[1024];
                if (fossil_crabdb_search_by_key(deque, key, value, sizeof(value))) {
                    printf("Value for key '%s': %s\n", key, value);
                }
            }
        } else if (strcmp(command_type, "search_value") == 0) {
            if (arg_count >= 1) {
                char* value = args[0];
                char key[1024];
                if (fossil_crabdb_search_by_value(deque, value, key, sizeof(key))) {
                    printf("Key for value '%s': %s\n", value, key);
                }
            }
        } else if (strcmp(command_type, "sort_key") == 0) {
            if (verbose) printf("Sorting by key\n");
            fossil_crabdb_sort_by_key(deque);
        } else if (strcmp(command_type, "sort_value") == 0) {
            if (verbose) printf("Sorting by value\n");
            fossil_crabdb_sort_by_value(deque);
        } else {
            fprintf(stderr, "Unknown command: %s\n", command_type);
        }
    }

    return true;
}

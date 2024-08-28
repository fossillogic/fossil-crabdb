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

typedef struct {
    char key[MAX_ARG_LENGTH];
    char value[MAX_ARG_LENGTH];
} kwargs_t;

//
// CRUD OPERATIONS
//

fossil_crabdb_t* fossil_crabdb_create(void) {
    fossil_crabdb_t *db = (fossil_crabdb_t*)fossil_crabdb_alloc(sizeof(fossil_crabdb_t));
    db->namespaceHead = NULL;
    return db;
}

int fossil_crabdb_add_namespace(fossil_crabdb_t *db, const char *name) {
    fossil_crabdb_namespace_t *ns = (fossil_crabdb_namespace_t*)fossil_crabdb_alloc(sizeof(fossil_crabdb_namespace_t));
    if (!ns) return -1;

    strncpy(ns->name, name, MAX_KEY_LENGTH);
    ns->key_values = NULL;
    ns->prev = NULL;
    ns->next = db->namespaceHead;
    ns->subNamespacesHead = NULL;

    if (db->namespaceHead) {
        db->namespaceHead->prev = ns;
    }

    db->namespaceHead = ns;
    return 0;
}

fossil_crabdb_namespace_t* fossil_crabdb_find_namespace(fossil_crabdb_t *db, const char *name) {
    fossil_crabdb_namespace_t *ns = db->namespaceHead;
    while (ns) {
        if (strcmp(ns->name, name) == 0) {
            return ns;
        }
        ns = ns->next;
    }
    return NULL;
}

int fossil_crabdb_add_key_value(fossil_crabdb_namespace_t *ns, const char *key, const char *value) {
    fossil_crabdb_keyvalue_t *kv = (fossil_crabdb_keyvalue_t*)fossil_crabdb_alloc(sizeof(fossil_crabdb_keyvalue_t));
    if (!kv) return -1;

    strncpy(kv->key, key, MAX_KEY_LENGTH);
    strncpy(kv->value, value, MAX_VALUE_LENGTH);
    kv->prev = NULL;
    kv->next = ns->key_values;

    if (ns->key_values) {
        ns->key_values->prev = kv;
    }

    ns->key_values = kv;
    return 0;
}

const char* fossil_crabdb_get_value(fossil_crabdb_namespace_t *ns, const char *key) {
    fossil_crabdb_keyvalue_t *kv = ns->key_values;
    while (kv) {
        if (strcmp(kv->key, key) == 0) {
            return kv->value;
        }
        kv = kv->next;
    }
    return NULL;
}

int fossil_crabdb_delete_key_value(fossil_crabdb_namespace_t *ns, const char *key) {
    fossil_crabdb_keyvalue_t *kv = ns->key_values;
    while (kv) {
        if (strcmp(kv->key, key) == 0) {
            if (kv->prev) {
                kv->prev->next = kv->next;
            }
            if (kv->next) {
                kv->next->prev = kv->prev;
            }
            if (ns->key_values == kv) {
                ns->key_values = kv->next;
            }
            fossil_crabdb_free(kv);
            return 0;
        }
        kv = kv->next;
    }
    return -1;
}

int fossil_crabdb_delete_namespace(fossil_crabdb_t *db, const char *name) {
    fossil_crabdb_namespace_t *ns = db->namespaceHead;
    while (ns) {
        if (strcmp(ns->name, name) == 0) {
            if (ns->prev) {
                ns->prev->next = ns->next;
            }
            if (ns->next) {
                ns->next->prev = ns->prev;
            }
            if (db->namespaceHead == ns) {
                db->namespaceHead = ns->next;
            }
            fossil_crabdb_keyvalue_t *kv = ns->key_values;
            while (kv) {
                fossil_crabdb_keyvalue_t *next = kv->next;
                fossil_crabdb_free(kv);
                kv = next;
            }
            fossil_crabdb_namespace_t *subNs = ns->subNamespacesHead;
            while (subNs) {
                fossil_crabdb_namespace_t *next = subNs->next;
                fossil_crabdb_delete_namespace(db, subNs->name);
                subNs = next;
            }
            fossil_crabdb_free(ns);
            return 0;
        }
        ns = ns->next;
    }
    return -1;
}

/**
 * @brief Renames a namespace in the CrabDB database.
 * @param db The CrabDB database.
 * @param old_name The current name of the namespace.
 * @param new_name The new name of the namespace.
 * @return 0 if successful, -1 otherwise.
 */
int fossil_crabdb_rename_namespace(fossil_crabdb_t *db, const char *old_name, const char *new_name) {
    if (!db || !old_name || !new_name) {
        return -1;
    }

    fossil_crabdb_namespace_t *ns = fossil_crabdb_find_namespace(db, old_name);
    if (!ns) {
        return -1;  // Namespace not found
    }

    // Check if a namespace with the new name already exists
    if (fossil_crabdb_find_namespace(db, new_name)) {
        return -1;  // Namespace with new_name already exists
    }

    // Rename the namespace
    strncpy(ns->name, new_name, sizeof(ns->name));
    ns->name[sizeof(ns->name) - 1] = '\0';  // Ensure null-termination

    return 0;
}

/**
 * @brief Updates the value associated with a key in a namespace of the CrabDB database.
 * @param ns The namespace to update the key-value pair in.
 * @param key The key of the pair to update.
 * @param new_value The new value to associate with the key.
 * @return 0 if successful, -1 otherwise.
 */
int fossil_crabdb_update_key_value(fossil_crabdb_namespace_t *ns, const char *key, const char *new_value) {
    fossil_crabdb_keyvalue_t *kv = ns->key_values;
    
    while (kv) {
        if (strcmp(kv->key, key) == 0) {
            // Check if new_value is empty
            if (new_value == NULL || strlen(new_value) == 0) {
                // Handle empty value case
                return -1; // Or some other error handling
            }
            strncpy(kv->value, new_value, MAX_VALUE_LENGTH - 1);
            kv->value[MAX_VALUE_LENGTH - 1] = '\0'; // Ensure null-termination
            return 0; // Success
        }
        kv = kv->next;
    }

    return -1; // Key not found
}

/**
 * @brief Retrieves all keys in a namespace of the CrabDB database.
 * @param ns The namespace to retrieve keys from.
 * @param keys Array to store the retrieved keys.
 * @param max_keys The maximum number of keys to retrieve.
 * @return The number of keys retrieved, or -1 if an error occurred.
 */
int fossil_crabdb_get_all_keys(fossil_crabdb_namespace_t *ns, char **keys, size_t max_keys) {
    if (!ns || !keys || max_keys == 0) {
        return -1;
    }

    fossil_crabdb_keyvalue_t *kv = ns->key_values;
    size_t count = 0;

    while (kv && count < max_keys) {
        keys[count] = kv->key;
        count++;
        kv = kv->next;
    }

    return count;
}

//
//
//

// Helper functions to get type as string
const char* type_to_string(value_type_t type) {
    switch (type) {
        case TYPE_U8: return "u8";
        case TYPE_U16: return "u16";
        case TYPE_U32: return "u32";
        case TYPE_U64: return "u64";
        case TYPE_I8: return "i8";
        case TYPE_I16: return "i16";
        case TYPE_I32: return "i32";
        case TYPE_I64: return "i64";
        case TYPE_H8: return "h8";
        case TYPE_H16: return "h16";
        case TYPE_H32: return "h32";
        case TYPE_H64: return "h64";
        case TYPE_O8: return "o8";
        case TYPE_O16: return "o16";
        case TYPE_O32: return "o32";
        case TYPE_O64: return "o64";
        case TYPE_F32: return "f32";
        case TYPE_F64: return "f64";
        case TYPE_CSTR: return "cstr";
        case TYPE_BOOL: return "bool";
        case TYPE_CHAR: return "char";
        default: return "unknown";
    }
}

// Helper functions to parse type from string
value_type_t string_to_type(const char *str) {
    if (strcmp(str, "u8") == 0) return TYPE_U8;
    if (strcmp(str, "u16") == 0) return TYPE_U16;
    if (strcmp(str, "u32") == 0) return TYPE_U32;
    if (strcmp(str, "u64") == 0) return TYPE_U64;
    if (strcmp(str, "i8") == 0) return TYPE_I8;
    if (strcmp(str, "i16") == 0) return TYPE_I16;
    if (strcmp(str, "i32") == 0) return TYPE_I32;
    if (strcmp(str, "i64") == 0) return TYPE_I64;
    if (strcmp(str, "h8") == 0) return TYPE_H8;
    if (strcmp(str, "h16") == 0) return TYPE_H16;
    if (strcmp(str, "h32") == 0) return TYPE_H32;
    if (strcmp(str, "h64") == 0) return TYPE_H64;
    if (strcmp(str, "o8") == 0) return TYPE_O8;
    if (strcmp(str, "o16") == 0) return TYPE_O16;
    if (strcmp(str, "o32") == 0) return TYPE_O32;
    if (strcmp(str, "o64") == 0) return TYPE_O64;
    if (strcmp(str, "f32") == 0) return TYPE_F32;
    if (strcmp(str, "f64") == 0) return TYPE_F64;
    if (strcmp(str, "cstr") == 0) return TYPE_CSTR;
    if (strcmp(str, "bool") == 0) return TYPE_BOOL;
    if (strcmp(str, "char") == 0) return TYPE_CHAR;
    return TYPE_UNKNOWN;
}

// Encoding functions
void encode_value(value_type_t type, const void *value, char *buffer, size_t buffer_size) {
    switch (type) {
        case TYPE_U8:
            snprintf(buffer, buffer_size, "u8:%u", *(uint8_t *)value);
            break;
        case TYPE_U16:
            snprintf(buffer, buffer_size, "u16:%u", *(uint16_t *)value);
            break;
        case TYPE_U32:
            snprintf(buffer, buffer_size, "u32:%u", *(uint32_t *)value);
            break;
        case TYPE_U64:
            snprintf(buffer, buffer_size, "u64:%llu", *(uint64_t *)value);
            break;
        case TYPE_I8:
            snprintf(buffer, buffer_size, "i8:%d", *(int8_t *)value);
            break;
        case TYPE_I16:
            snprintf(buffer, buffer_size, "i16:%d", *(int16_t *)value);
            break;
        case TYPE_I32:
            snprintf(buffer, buffer_size, "i32:%d", *(int32_t *)value);
            break;
        case TYPE_I64:
            snprintf(buffer, buffer_size, "i64:%lld", *(int64_t *)value);
            break;
        case TYPE_H8:
            snprintf(buffer, buffer_size, "h8:0x%02X", *(uint8_t *)value);
            break;
        case TYPE_H16:
            snprintf(buffer, buffer_size, "h16:0x%04X", *(uint16_t *)value);
            break;
        case TYPE_H32:
            snprintf(buffer, buffer_size, "h32:0x%08X", *(uint32_t *)value);
            break;
        case TYPE_H64:
            snprintf(buffer, buffer_size, "h64:0x%016llX", *(uint64_t *)value);
            break;
        case TYPE_O8:
            snprintf(buffer, buffer_size, "o8:0%o", *(uint8_t *)value);
            break;
        case TYPE_O16:
            snprintf(buffer, buffer_size, "o16:0%o", *(uint16_t *)value);
            break;
        case TYPE_O32:
            snprintf(buffer, buffer_size, "o32:0%o", *(uint32_t *)value);
            break;
        case TYPE_O64:
            snprintf(buffer, buffer_size, "o64:0%llo", *(uint64_t *)value);
            break;
        case TYPE_F32:
            snprintf(buffer, buffer_size, "f32:%f", *(float *)value);
            break;
        case TYPE_F64:
            snprintf(buffer, buffer_size, "f64:%lf", *(double *)value);
            break;
        case TYPE_CSTR:
            snprintf(buffer, buffer_size, "cstr:%s", (char *)value);
            break;
        case TYPE_BOOL:
            snprintf(buffer, buffer_size, "bool:%s", (*(int *)value ? "true" : "false"));
            break;
        case TYPE_CHAR:
            snprintf(buffer, buffer_size, "char:%c", *(char *)value);
            break;
        default:
            snprintf(buffer, buffer_size, "unknown:");
            break;
    }
}

// Decoding functions
int decode_value(const char *encoded, value_type_t *type, void *value) {
    char type_str[16];
    int result = sscanf(encoded, "%15[^:]:%s", type_str, (char *)value);
    if (result != 2) return -1;

    *type = string_to_type(type_str);
    switch (*type) {
        case TYPE_U8:
            sscanf((char *)value, "%hhu", (uint8_t *)value);
            break;
        case TYPE_U16:
            sscanf((char *)value, "%hu", (uint16_t *)value);
            break;
        case TYPE_U32:
            sscanf((char *)value, "%u", (uint32_t *)value);
            break;
        case TYPE_U64:
            sscanf((char *)value, "%llu", (uint64_t *)value);
            break;
        case TYPE_I8:
            sscanf((char *)value, "%hhd", (int8_t *)value);
            break;
        case TYPE_I16:
            sscanf((char *)value, "%hd", (int16_t *)value);
            break;
        case TYPE_I32:
            sscanf((char *)value, "%d", (int32_t *)value);
            break;
        case TYPE_I64:
            sscanf((char *)value, "%lld", (int64_t *)value);
            break;
        case TYPE_H8:
            sscanf((char *)value, "%hhX", (uint8_t *)value);
            break;
        case TYPE_H16:
            sscanf((char *)value, "%hx", (uint16_t *)value);
            break;
        case TYPE_H32:
            sscanf((char *)value, "%X", (uint32_t *)value);
            break;
        case TYPE_H64:
            sscanf((char *)value, "%llX", (uint64_t *)value);
            break;
        case TYPE_O8:
            sscanf((char *)value, "%hho", (uint8_t *)value);
            break;
        case TYPE_O16:
            sscanf((char *)value, "%ho", (uint16_t *)value);
            break;
        case TYPE_O32:
            sscanf((char *)value, "%o", (uint32_t *)value);
            break;
        case TYPE_O64:
            sscanf((char *)value, "%llo", (uint64_t *)value);
            break;
        case TYPE_F32:
            sscanf((char *)value, "%f", (float *)value);
            break;
        case TYPE_F64:
            sscanf((char *)value, "%lf", (double *)value);
            break;
        case TYPE_CSTR:
            // `value` is already a string, nothing extra to do
            break;
        case TYPE_BOOL:
            *(int *)value = (strcmp((char *)value, "true") == 0);
            break;
        case TYPE_CHAR:
            *(char *)value = ((char *)value)[0];
            break;
        default:
            return -1;
    }

    return 0;
}

int fossil_crabdb_export(fossil_crabdb_t *db, const char *filename) {
    FILE *file = fopen(filename, "w");
    if (!file) return -1;

    fossil_crabdb_namespace_t *ns = db->namespaceHead;
    while (ns) {
        fprintf(file, "[%s]\n", ns->name);
        fossil_crabdb_keyvalue_t *kv = ns->key_values;
        while (kv) {
            char encoded_value[MAX_VALUE_LENGTH];
            value_type_t type = TYPE_UNKNOWN; // Determine type if necessary
            // Assume type is embedded or retrieved from elsewhere
            encode_value(type, kv->value, encoded_value, sizeof(encoded_value));
            fprintf(file, "%s=%s\n", kv->key, encoded_value);
            kv = kv->next;
        }
        fprintf(file, "\n");
        ns = ns->next;
    }

    fclose(file);
    return 0;
}

int fossil_crabdb_import(fossil_crabdb_t *db, const char *filename) {
    FILE *file = fopen(filename, "r");
    if (!file) return -1;

    char line[256];
    fossil_crabdb_namespace_t *current_ns = NULL;

    while (fgets(line, sizeof(line), file)) {
        line[strcspn(line, "\n")] = 0; // Remove newline

        if (line[0] == '[') {
            // Handle namespace
            current_ns = malloc(sizeof(fossil_crabdb_namespace_t));
            if (!current_ns) return -1;

            sscanf(line, "[%255[^]]", current_ns->name);
            current_ns->key_values = NULL;
            current_ns->next = db->namespaceHead;
            db->namespaceHead = current_ns;
        } else if (current_ns && strchr(line, '=')) {
            // Handle key-value pair
            fossil_crabdb_keyvalue_t *kv = malloc(sizeof(fossil_crabdb_keyvalue_t));
            if (!kv) return -1;

            char *equal_sign = strchr(line, '=');
            *equal_sign = '\0';
            strncpy(kv->key, line, MAX_KEY_LENGTH - 1);
            strncpy(kv->value, equal_sign + 1, MAX_VALUE_LENGTH - 1);

            // Decode value
            value_type_t type;
            if (decode_value(kv->value, &type, kv->value) != 0) {
                free(kv);
                return -1;
            }

            kv->next = current_ns->key_values;
            current_ns->key_values = kv;
        }
    }

    fclose(file);
    return 0;
}

//
// Memory Catch
//

fossil_crabdb_cache_t* fossil_crabdb_create_cache(void) {
    fossil_crabdb_cache_t *cache = (fossil_crabdb_cache_t*)fossil_crabdb_alloc(sizeof(fossil_crabdb_cache_t));
    cache->head = NULL;
    return cache;
}

void fossil_crabdb_cache_add(fossil_crabdb_cache_t *cache, const char *key, const char *value) {
    fossil_crabdb_cache_entry_t *entry = (fossil_crabdb_cache_entry_t*)fossil_crabdb_alloc(sizeof(fossil_crabdb_cache_entry_t));
    strncpy(entry->key, key, MAX_KEY_LENGTH);
    strncpy(entry->value, value, MAX_VALUE_LENGTH);
    entry->next = cache->head;
    cache->head = entry;
}

const char* fossil_crabdb_cache_get(fossil_crabdb_cache_t *cache, const char *key) {
    fossil_crabdb_cache_entry_t *entry = cache->head;
    while (entry) {
        if (strcmp(entry->key, key) == 0) {
            return entry->value;
        }
        entry = entry->next;
    }
    return NULL;
}

void fossil_crabdb_cache_free(fossil_crabdb_cache_t *cache) {
    fossil_crabdb_cache_entry_t *entry = cache->head;
    while (entry) {
        fossil_crabdb_cache_entry_t *next = entry->next;
        fossil_crabdb_free(entry);
        entry = next;
    }
    fossil_crabdb_free(cache);
}

//
// Persistence
//

int fossil_crabdb_save(fossil_crabdb_t *db, const char *filename) {
    FILE *file = fopen(filename, "wb");
    if (!file) return -1;

    fossil_crabdb_namespace_t *ns = db->namespaceHead;
    while (ns) {
        fwrite(ns->name, sizeof(ns->name), 1, file);
        fossil_crabdb_keyvalue_t *kv = ns->key_values;
        while (kv) {
            fwrite(kv->key, sizeof(kv->key), 1, file);
            fwrite(kv->value, sizeof(kv->value), 1, file);
            kv = kv->next;
        }
        ns = ns->next;
    }

    fclose(file);
    return 0;
}

int fossil_crabdb_load(fossil_crabdb_t *db, const char *filename) {
    FILE *file = fopen(filename, "rb");
    if (!file) return -1;

    char namespaceName[MAX_KEY_LENGTH];
    while (fread(namespaceName, sizeof(namespaceName), 1, file)) {
        fossil_crabdb_add_namespace(db, namespaceName);
        fossil_crabdb_namespace_t *ns = fossil_crabdb_find_namespace(db, namespaceName);

        char key[MAX_KEY_LENGTH];
        while (fread(key, sizeof(key), 1, file)) {
            char value[MAX_VALUE_LENGTH];
            fread(value, sizeof(value), 1, file);
            fossil_crabdb_add_key_value(ns, key, value);
        }
    }

    fclose(file);
    return 0;
}

//
// Query Language
//

// Function to execute a query from a file
int fossil_crabdb_execute_query(fossil_crabdb_t *db, const char *filename) {
    FILE *file = fopen(filename, "r");
    if (!file) return -1; // Error opening file

    char line[1024];
    while (fgets(line, sizeof(line), file)) {
        // Trim whitespace from the line
        char *trimmed_line = line;
        while (isspace(*trimmed_line)) trimmed_line++;
        if (*trimmed_line == '\0' || *trimmed_line == '#') continue; // Skip empty lines and comments

        if (strncmp(trimmed_line, "CREATE NAMESPACE", 17) == 0) {
            char *name = trimmed_line + 18;
            name[strcspn(name, "\n")] = '\0'; // Remove newline character
            fossil_crabdb_add_namespace(db, name);
        } else if (strncmp(trimmed_line, "DELETE NAMESPACE", 17) == 0) {
            char *name = trimmed_line + 18;
            name[strcspn(name, "\n")] = '\0'; // Remove newline character
            fossil_crabdb_delete_namespace(db, name);
        } else if (strncmp(trimmed_line, "RENAME NAMESPACE", 16) == 0) {
            char old_name[MAX_KEY_LENGTH], new_name[MAX_KEY_LENGTH];
            sscanf(trimmed_line + 17, "%s %s", old_name, new_name);
            fossil_crabdb_rename_namespace(db, old_name, new_name);
        } else if (strncmp(trimmed_line, "SET", 3) == 0) {
            char key[MAX_KEY_LENGTH];
            char value[MAX_VALUE_LENGTH];
            sscanf(trimmed_line + 4, "%s %s", key, value);
            fossil_crabdb_namespace_t *ns = db->namespaceHead; // Assuming the namespace is the default one
            fossil_crabdb_add_key_value(ns, key, value);
        } else if (strncmp(trimmed_line, "UPDATE", 6) == 0) {
            char key[MAX_KEY_LENGTH];
            char new_value[MAX_VALUE_LENGTH];
            sscanf(trimmed_line + 7, "%s %s", key, new_value);
            fossil_crabdb_namespace_t *ns = db->namespaceHead; // Assuming the namespace is the default one
            fossil_crabdb_update_key_value(ns, key, new_value);
        } else if (strncmp(trimmed_line, "GET", 3) == 0) {
            char key[MAX_KEY_LENGTH];
            sscanf(trimmed_line + 4, "%s", key);
            fossil_crabdb_namespace_t *ns = db->namespaceHead; // Assuming the namespace is the default one
            const char *value = fossil_crabdb_get_value(ns, key);
            if (value) {
                printf("%s\n", value);
            } else {
                printf("Not found\n");
            }
        } else if (strncmp(trimmed_line, "PRINT", 5) == 0) {
            char *message = trimmed_line + 6;
            message[strcspn(message, "\n")] = '\0'; // Remove newline character
            printf("%s\n", message);
        } else if (strncmp(trimmed_line, "EXPORT", 6) == 0) {
            char filename[MAX_PATH_LENGTH];
            sscanf(trimmed_line + 7, "%s", filename);
            fossil_crabdb_export(db, filename);
        } else if (strncmp(trimmed_line, "IMPORT", 6) == 0) {
            char filename[MAX_PATH_LENGTH];
            sscanf(trimmed_line + 7, "%s", filename);
            fossil_crabdb_import(db, filename);
        }
    }

    fclose(file);
    return 0; // Success
}

// Utility function to trim leading and trailing whitespace from a string
static void trim_whitespace(char *str) {
    char *end;
    while (isspace(*str)) str++;
    if (*str == 0) return;
    end = str + strlen(str) - 1;
    while (end > str && isspace(*end)) end--;
    *(end + 1) = '\0';
}

// Parse key-value arguments from a line of text
static int parse_kwargs(const char *line, kwargs_t *kwargs, size_t *num_kwargs) {
    const char *p = line;
    size_t count = 0;

    while (*p) {
        while (*p && isspace(*p)) p++; // Skip whitespace
        if (*p == '\0') break;

        char key[MAX_ARG_LENGTH] = {0};
        char value[MAX_ARG_LENGTH] = {0};

        sscanf(p, "%[^=]=%s", key, value);

        strncpy(kwargs[count].key, key, MAX_ARG_LENGTH);
        strncpy(kwargs[count].value, value, MAX_ARG_LENGTH);
        count++;

        while (*p && *p != '\n') p++;
        if (*p == '\n') p++;
    }

    *num_kwargs = count;
    return 0;
}

// Execute a command with arguments parsed from the script
static int execute_command(fossil_crabdb_t *db, const char *command, kwargs_t *kwargs, size_t num_kwargs) {
    if (strcmp(command, "CREATE_NAMESPACE") == 0) {
        for (size_t i = 0; i < num_kwargs; i++) {
            if (strcmp(kwargs[i].key, "name") == 0) {
                return fossil_crabdb_add_namespace(db, kwargs[i].value);
            }
        }
        return -1; // Required argument missing
    } else if (strcmp(command, "DELETE_NAMESPACE") == 0) {
        for (size_t i = 0; i < num_kwargs; i++) {
            if (strcmp(kwargs[i].key, "name") == 0) {
                return fossil_crabdb_delete_namespace(db, kwargs[i].value);
            }
        }
        return -1; // Required argument missing
    } else if (strcmp(command, "SET") == 0) {
        const char *namespace_name = NULL;
        const char *key = NULL;
        const char *value = NULL;
        for (size_t i = 0; i < num_kwargs; i++) {
            if (strcmp(kwargs[i].key, "namespace") == 0) {
                namespace_name = kwargs[i].value;
            } else if (strcmp(kwargs[i].key, "key") == 0) {
                key = kwargs[i].value;
            } else if (strcmp(kwargs[i].key, "value") == 0) {
                value = kwargs[i].value;
            }
        }
        fossil_crabdb_namespace_t *ns = namespace_name ? fossil_crabdb_find_namespace(db, namespace_name) : db->namespaceHead;
        if (ns && key && value) {
            return fossil_crabdb_add_key_value(ns, key, value);
        }
        return -1; // Arguments missing or namespace not found
    } else if (strcmp(command, "GET") == 0) {
        const char *namespace_name = NULL;
        const char *key = NULL;
        for (size_t i = 0; i < num_kwargs; i++) {
            if (strcmp(kwargs[i].key, "namespace") == 0) {
                namespace_name = kwargs[i].value;
            } else if (strcmp(kwargs[i].key, "key") == 0) {
                key = kwargs[i].value;
            }
        }
        fossil_crabdb_namespace_t *ns = namespace_name ? fossil_crabdb_find_namespace(db, namespace_name) : db->namespaceHead;
        if (ns && key) {
            const char *value = fossil_crabdb_get_value(ns, key);
            if (value) {
                printf("%s\n", value);
            } else {
                printf("Not found\n");
            }
            return 0;
        }
        return -1; // Arguments missing or namespace not found
    } else if (strcmp(command, "PRINT") == 0) {
        for (size_t i = 0; i < num_kwargs; i++) {
            if (strcmp(kwargs[i].key, "message") == 0) {
                printf("%s\n", kwargs[i].value);
                return 0;
            }
        }
        return -1; // Required argument missing
    }
    return -1; // Unsupported command
}

// Function to execute a script file containing .crabql commands
int fossil_crabdb_execute_script(fossil_crabdb_t *db, const char *filename) {
    FILE *file = fopen(filename, "r");
    if (!file) return -1;

    char line[MAX_LINE_LENGTH];
    char command[MAX_ARG_LENGTH];
    kwargs_t kwargs[10]; // Adjust the size as needed
    size_t num_kwargs = 0;
    int in_condition = 0; // 0: not inside any condition, 1: inside if, 2: inside else
    int in_foreach = 0;   // 0: not inside foreach, 1: inside foreach
    char variable[MAX_ARG_LENGTH] = {0};
    char **list = NULL;
    size_t list_index = 0, list_size = 0;

    while (fgets(line, sizeof(line), file)) {
        trim_whitespace(line);
        if (line[0] == '\0' || line[0] == '#') continue; // Skip empty lines and comments

        // Convert the entire line to lowercase
        for (int i = 0; line[i]; i++) {
            line[i] = tolower(line[i]);
        }

        // Handle control structures
        if (strstr(line, "if") == line) {
            in_condition = 1;
            continue;
        } else if (strstr(line, "elseif") == line) {
            if (in_condition == 1) in_condition = 2;
            continue;
        } else if (strstr(line, "else") == line) {
            if (in_condition == 1) in_condition = 2;
            else if (in_condition == 2) in_condition = 1;
            continue;
        } else if (strstr(line, "end") == line) {
            in_condition = 0;
            in_foreach = 0;
            list_index = 0;
            continue;
        } else if (strstr(line, "foreach") == line) {
            in_foreach = 1;
            sscanf(line, "foreach %s in %s", variable, line);
            // Assume list is a comma-separated string
            char *token = strtok(line, ",");
            while (token) {
                list = realloc(list, (list_size + 1) * sizeof(char *));
                list[list_size++] = fossil_crabdb_strdup(token);
                token = strtok(NULL, ",");
            }
            list_index = 0;
            continue;
        }

        // Handle variable assignment
        if (strstr(line, "let") == line) {
            char var_name[MAX_ARG_LENGTH] = {0}, var_value[MAX_ARG_LENGTH] = {0};
            sscanf(line, "let %s = %s", var_name, var_value);
            // Store the variable in some context, this example assumes a simple key-value store.
            // You will need to implement this part based on how you manage variables.
            continue;
        }

        // Execution logic based on conditionals and loops
        if (in_condition == 0 || in_condition == 1) {
            // Process commands only if not inside else or elseif
            if (in_foreach && list_index < list_size) {
                // Assign current value to the loop variable
                strcpy(variable, list[list_index++]);
            }

            if (strchr(line, '(')) {
                char *p = strtok(line, "(");
                char *command_end = strtok(NULL, ")");
                if (p && command_end) {
                    strcpy(command, p);
                    char *args_start = command_end + 1;
                    parse_kwargs(args_start, kwargs, &num_kwargs);
                    if (execute_command(db, command, kwargs, num_kwargs) != 0) {
                        fclose(file);
                        return -1; // Error executing command
                    }
                }
            }
        }
    }

    // Clean up dynamically allocated list
    for (size_t i = 0; i < list_size; i++) {
        free(list[i]);
    }
    free(list);

    fclose(file);
    return 0; // Success
}

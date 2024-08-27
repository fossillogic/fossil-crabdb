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
    ns->keyValueHead = NULL;
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
    kv->next = ns->keyValueHead;

    if (ns->keyValueHead) {
        ns->keyValueHead->prev = kv;
    }

    ns->keyValueHead = kv;
    return 0;
}

const char* fossil_crabdb_get_value(fossil_crabdb_namespace_t *ns, const char *key) {
    fossil_crabdb_keyvalue_t *kv = ns->keyValueHead;
    while (kv) {
        if (strcmp(kv->key, key) == 0) {
            return kv->value;
        }
        kv = kv->next;
    }
    return NULL;
}

int fossil_crabdb_delete_key_value(fossil_crabdb_namespace_t *ns, const char *key) {
    fossil_crabdb_keyvalue_t *kv = ns->keyValueHead;
    while (kv) {
        if (strcmp(kv->key, key) == 0) {
            if (kv->prev) {
                kv->prev->next = kv->next;
            }
            if (kv->next) {
                kv->next->prev = kv->prev;
            }
            if (ns->keyValueHead == kv) {
                ns->keyValueHead = kv->next;
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
            fossil_crabdb_keyvalue_t *kv = ns->keyValueHead;
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
        fossil_crabdb_keyvalue_t *kv = ns->keyValueHead;
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

int fossil_crabdb_execute_query(fossil_crabdb_t *db, const char *filename) {
    FILE *file = fopen(filename, "r");
    if (!file) return -1;

    char line[1024];
    while (fgets(line, sizeof(line), file)) {
        if (strncmp(line, "CREATE NAMESPACE", 17) == 0) {
            char *name = line + 18;
            name[strcspn(name, "\n")] = '\0'; // Remove newline character
            fossil_crabdb_add_namespace(db, name);
        } else if (strncmp(line, "DELETE NAMESPACE", 17) == 0) {
            char *name = line + 18;
            name[strcspn(name, "\n")] = '\0'; // Remove newline character
            fossil_crabdb_delete_namespace(db, name);
        } else if (strncmp(line, "SET", 3) == 0) {
            char key[MAX_KEY_LENGTH];
            char value[MAX_VALUE_LENGTH];
            sscanf(line + 4, "%s %s", key, value);
            fossil_crabdb_namespace_t *ns = db->namespaceHead; // Assuming the namespace is the default one
            fossil_crabdb_add_key_value(ns, key, value);
        } else if (strncmp(line, "GET", 3) == 0) {
            char key[MAX_KEY_LENGTH];
            sscanf(line + 4, "%s", key);
            fossil_crabdb_namespace_t *ns = db->namespaceHead; // Assuming the namespace is the default one
            const char *value = fossil_crabdb_get_value(ns, key);
            if (value) {
                printf("%s\n", value);
            } else {
                printf("Not found\n");
            }
        }
    }

    fclose(file);
    return 0; // Success
}

static void trim_whitespace(char *str) {
    char *end;
    while (isspace(*str)) str++;
    if (*str == 0) return;
    end = str + strlen(str) - 1;
    while (end > str && isspace(*end)) end--;
    *(end + 1) = '\0';
}

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
    int condition_result = 0; // Result of the last condition check

    while (fgets(line, sizeof(line), file)) {
        trim_whitespace(line);
        if (line[0] == '\0' || line[0] == '#') continue; // Skip empty lines and comments

        if (strstr(line, "if") == line) {
            in_condition = 1;
            condition_result = 0; // Reset condition result
            continue;
        } else if (strstr(line, "elseif") == line) {
            if (in_condition == 1) {
                in_condition = 2;
            }
            continue;
        } else if (strstr(line, "else") == line) {
            if (in_condition == 1) {
                in_condition = 2;
            } else if (in_condition == 2) {
                in_condition = 1;
            }
            continue;
        } else if (strstr(line, "end") == line) {
            in_condition = 0;
            continue;
        }

        if (in_condition == 1 || in_condition == 2) {
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

    fclose(file);
    return 0; // Success
}

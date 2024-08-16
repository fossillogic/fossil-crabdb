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
#include "fossil/crabdb/namespace.h"
#include "fossil/crabdb/internal.h"

#include <string.h>
#include <stdlib.h>
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
                    return CRABDB_ERR_KEY_NOT_FOUND; // Key already exists
                }
                kv = kv->next;
            }

            fossil_crabdb_keyvalue_t *new_kv = fossil_crabdb_alloc(sizeof(fossil_crabdb_keyvalue_t));
            if (!new_kv) return CRABDB_ERR_MEM;

            new_kv->key = fossil_crabdb_strdup(key);
            new_kv->value = fossil_crabdb_strdup(value);
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
                    fossil_crabdb_free(kv->value);
                    kv->value = fossil_crabdb_strdup(value);
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
    } else if (strcmp(command, "serialize") == 0) {
        if (token_count == 1) {
            return fossil_crabdb_serialize(db, tokens[0]);
        }
    } else if (strcmp(command, "deserialize") == 0) {
        if (token_count == 1) {
            fossil_crabdb_t *new_db = fossil_crabdb_deserialize(tokens[0]);
            if (new_db) {
                fossil_crabdb_erase(db);
                db = new_db;
                return CRABDB_OK;
            } else {
                return CRABDB_ERR_DESERIALIZE_FAILED;
            }
        }
    } else if (strcmp(command, "backup") == 0) {
        if (token_count == 1) {
            return fossil_crabdb_backup(db, tokens[0]);
        }
    } else if (strcmp(command, "restore") == 0) {
        if (token_count == 1) {
            return fossil_crabdb_restore(db, tokens[0]);
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
        char *end = token + strlen(token) - 1;
        while (end > token && isspace((unsigned char)*end)) end--;
        end[1] = '\0';
        tokens[token_count++] = token;
        token = strtok(NULL, ",");
    }

    fossil_crabdb_error_t result = parse_and_execute(db, command, tokens, token_count);

    fossil_crabdb_free(query_copy);
    return result;
} // end of fun

fossil_crabdb_error_t fossil_crabdb_serialize(fossil_crabdb_t *db, const char *filename) {
    if (!db || !filename) return CRABDB_ERR_MEM;

    FILE *file = fopen(filename, "w");
    if (!file) return CRABDB_ERR_IO;

    fossil_crabdb_namespace_t *current = db->namespaces;
    while (current) {
        fprintf(file, "namespace:%s\n", current->name);
        fossil_crabdb_keyvalue_t *kv = current->data;
        while (kv) {
            fprintf(file, "key:%s,value:%s\n", kv->key, kv->value);
            kv = kv->next;
        }
        current = current->next;
    }

    fclose(file);
    return CRABDB_OK;
}

fossil_crabdb_t* fossil_crabdb_deserialize(const char *filename) {
    if (!filename) return NULL;

    FILE *file = fopen(filename, "r");
    if (!file) return NULL;

    fossil_crabdb_t *db = fossil_crabdb_create();
    if (!db) {
        fclose(file);
        return NULL;
    }

    char line[256];
    fossil_crabdb_namespace_t *current_namespace = NULL;

    while (fgets(line, sizeof(line), file)) {
        if (strncmp(line, "namespace:", 10) == 0) {
            char *namespace_name = strtok(line + 10, "\n");
            if (namespace_name) {
                current_namespace = fossil_crabdb_alloc(sizeof(fossil_crabdb_namespace_t));
                current_namespace->name = fossil_crabdb_strdup(namespace_name);
                current_namespace->next = db->namespaces;
                db->namespaces = current_namespace;
            }
        } else if (strncmp(line, "key:", 4) == 0 && current_namespace) {
            char *key = strtok(line + 4, ",");
            char *value = strtok(NULL, "\n");
            if (key && value && strncmp(value, "value:", 6) == 0) {
                fossil_crabdb_insert(db, current_namespace->name, key + 6, value + 6);
            }
        }
    }

    fclose(file);
    return db;
}

/**
 * @brief Backup the database to a file.
 * 
 * @param db Pointer to the fossil_crabdb_t database.
 * @param backup_filename Name of the backup file.
 * @return Error code indicating the result of the operation.
 */
fossil_crabdb_error_t fossil_crabdb_backup(fossil_crabdb_t *db, const char *backup_filename) {
    if (db == NULL || backup_filename == NULL) {
        return CRABDB_ERR_INVALID_ARG;
    }

    // Use the serialize function to back up the database to the file
    fossil_crabdb_error_t result = fossil_crabdb_serialize(db, backup_filename);
    
    if (result != CRABDB_OK) {
        return CRABDB_ERR_BACKUP_FAILED;
    }

    return CRABDB_OK;
}

/**
 * @brief Restore the database from a backup file.
 * 
 * @param db Pointer to the fossil_crabdb_t database.
 * @param backup_filename Name of the backup file.
 * @return Error code indicating the result of the operation.
 */
fossil_crabdb_error_t fossil_crabdb_restore(fossil_crabdb_t *db, const char *backup_filename) {
    if (db == NULL || backup_filename == NULL) {
        return CRABDB_ERR_INVALID_ARG;
    }

    // Erase the current contents of the database
    fossil_crabdb_erase(db);

    // Deserialize the backup file to restore the database
    fossil_crabdb_t *restored_db = fossil_crabdb_deserialize(backup_filename);
    if (restored_db == NULL) {
        return CRABDB_ERR_RESTORE_FAILED;
    }

    // Copy the restored database contents back to the original database
    *db = *restored_db;

    // Clean up the temporary restored_db object
    fossil_crabdb_erase(restored_db);

    return CRABDB_OK;
}

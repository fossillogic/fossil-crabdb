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

char *custom_strdup(const char *str) {
    if (!str) return NULL;

    size_t len = strlen(str);
    char *new_str = (char *)malloc(len + 1);
    if (!new_str) return NULL;

    strcpy(new_str, str);
    return new_str;
}

// In a header file, e.g., crabdb.h
struct fossil_crabdb* backup_current_db_state(struct fossil_crabdb *db) {
    struct fossil_crabdb* backup = (struct fossil_crabdb*)malloc(sizeof(struct fossil_crabdb));
    if (!backup) {
        return NULL;
    }

    backup->head = backup->tail = NULL;
    backup->in_transaction = db->in_transaction;
    backup->logging_enabled = db->logging_enabled;

    struct fossil_crabdb_node* current = db->head;
    while (current) {
        fossil_crabdb_insert(backup, current->key, current->value, current->type);
        current = current->next;
    }

    return backup;
}

void persist_changes_to_disk(struct fossil_crabdb *db) {
    if (db->db_file) {
        fclose(db->db_file);
    }

    db->db_file = fopen(db->file_path, "wb");
    if (!db->db_file) {
        return;
    }

    struct fossil_crabdb_node* current = db->head;
    while (current) {
        fwrite(&current->type, sizeof(current->type), 1, db->db_file);
        fwrite(current->key, sizeof(char), FOSSIL_CRABDB_KEY_SIZE, db->db_file);
        fwrite(current->value, sizeof(char), FOSSIL_CRABDB_VAL_SIZE, db->db_file);
        current = current->next;
    }

    fclose(db->db_file);
}

void restore_from_backup(struct fossil_crabdb *db, struct fossil_crabdb *backup) {
    struct fossil_crabdb_node* current = db->head;
    while (current) {
        struct fossil_crabdb_node* next = current->next;
        free(current);
        current = next;
    }

    db->head = db->tail = NULL;
    db->in_transaction = backup->in_transaction;
    db->logging_enabled = backup->logging_enabled;

    current = backup->head;
    while (current) {
        fossil_crabdb_insert(db, current->key, current->value, current->type);
        current = current->next;
    }
}


/* Create a new database */
fossil_crabdb_t* fossil_crabdb_create(void) {
    fossil_crabdb_t* db = (fossil_crabdb_t*)malloc(sizeof(fossil_crabdb_t));
    if (!db) {
        return NULL;
    }

    db->head = db->tail = NULL;
    db->in_transaction = false;
    db->transaction_backup = NULL;
    db->db_file = NULL;
    db->file_path = "crabdb.dat";
    db->logging_enabled = false;
    db->node_count = 0;

    return db;
}

/* Destroy the database */
void fossil_crabdb_destroy(fossil_crabdb_t* db) {
    if (!db) return;

    /* Free all nodes in the db */
    fossil_crabdb_node_t* current = db->head;
    while (current) {
        fossil_crabdb_node_t* next = current->next;
        free(current);
        current = next;
    }

    /* Close the db file if open */
    if (db->db_file) {
        fclose(db->db_file);
    }

    /* Free the db */
    free(db);
}

/* Insert a new key-value pair */
bool fossil_crabdb_insert(fossil_crabdb_t* db, const char* key, const char* value, fossil_crabdb_type_t type) {
    if (!db || !key || !value) return false;

    /* Check if the key already exists and update if needed */
    fossil_crabdb_node_t* current = db->head;
    while (current) {
        if (strcmp(current->key, key) == 0) {
            current->type = type;
            strncpy(current->value, value, FOSSIL_CRABDB_VAL_SIZE);
            return true;
        }
        current = current->next;
    }

    /* Add a new node */
    fossil_crabdb_node_t* new_node = (fossil_crabdb_node_t*)malloc(sizeof(fossil_crabdb_node_t));
    if (!new_node) {
        return false;
    }
    strncpy(new_node->key, key, FOSSIL_CRABDB_KEY_SIZE);
    strncpy(new_node->value, value, FOSSIL_CRABDB_VAL_SIZE);
    new_node->type = type;
    new_node->prev = db->tail;
    new_node->next = NULL;

    if (db->tail) {
        db->tail->next = new_node;
    } else {
        db->head = new_node;
    }

    db->tail = new_node;
    db->node_count++; // Increment the node count
    return true;
}

/* Update an existing key-value pair */
bool fossil_crabdb_update(fossil_crabdb_t* db, const char* key, const char* value) {
    if (!db || !key || !value) return false;

    /* Check if the key already exists and update if needed */
    fossil_crabdb_node_t* current = db->head;
    while (current) {
        if (strcmp(current->key, key) == 0) {
            strncpy(current->value, value, FOSSIL_CRABDB_VAL_SIZE);
            return true;
        }
        current = current->next;
    }

    return false;
}

/* Delete a key-value pair */
bool fossil_crabdb_delete(fossil_crabdb_t* db, const char* key) {
    if (!db || !key) return false;

    /* Check if the key exists and delete if found */
    fossil_crabdb_node_t* current = db->head;
    while (current) {
        if (strcmp(current->key, key) == 0) {
            if (current->prev) {
                current->prev->next = current->next;
            } else {
                db->head = current->next;
            }
            if (current->next) {
                current->next->prev = current->prev;
            } else {
                db->tail = current->prev;
            }
            free(current);
            db->node_count--; // Decrement the node count
            return true;
        }
        current = current->next;
    }

    return false;
}

/* Select a key-value pair */
bool fossil_crabdb_select(fossil_crabdb_t* db, const char* key, char* value, size_t value_size) {
    if (!db || !key || !value) return false;

    /* Check if the key exists and select if found */
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

bool fossil_crabdb_begin_transaction(fossil_crabdb_t* db) {
    // Check if a transaction is already active
    if (db->in_transaction) {
        return false; // A transaction is already in progress
    }

    // Start the transaction by marking the beginning state
    db->in_transaction = true;
    db->transaction_backup = backup_current_db_state(db); // Backup the current state
    return true;
}

bool fossil_crabdb_commit_transaction(fossil_crabdb_t* db) {
    if (!db->in_transaction) {
        return false; // No active transaction
    }

    // Commit the transaction by writing changes to disk
    persist_changes_to_disk(db);
    db->in_transaction = false;
    free(db->transaction_backup); // Clear the backup as it's no longer needed
    return true;
}

bool fossil_crabdb_rollback_transaction(fossil_crabdb_t* db) {
    if (!db->in_transaction) {
        return false; // No active transaction
    }

    // Roll back to the backed-up state
    restore_from_backup(db, db->transaction_backup);
    db->in_transaction = false;
    free(db->transaction_backup); // Clear the backup
    return true;
}

bool fossil_crabdb_insert_batch(fossil_crabdb_t* db, const char** keys, const char** values, fossil_crabdb_type_t* types, size_t count) {
    if (!db || !keys || !values || !types) return false;

    for (size_t i = 0; i < count; i++) {
        if (!fossil_crabdb_insert(db, keys[i], values[i], types[i])) {
            return false;
        }
    }

    return true;
}

bool fossil_crabdb_delete_batch(fossil_crabdb_t* db, const char** keys, size_t count) {
    if (!db || !keys) return false;

    for (size_t i = 0; i < count; i++) {
        if (!fossil_crabdb_delete(db, keys[i])) {
            return false;
        }
    }

    return true;
}

bool fossil_crabdb_update_batch(fossil_crabdb_t* db, const char** keys, const char** values, size_t count) {
    if (!db || !keys || !values) return false;

    for (size_t i = 0; i < count; i++) {
        if (!fossil_crabdb_update(db, keys[i], values[i])) {
            return false;
        }
    }

    return true;
}

bool fossil_crabdb_select_batch(fossil_crabdb_t* db, const char** keys, char** values, size_t* value_sizes, size_t count) {
    if (!db || !keys || !values || !value_sizes) return false;

    for (size_t i = 0; i < count; i++) {
        if (!fossil_crabdb_select(db, keys[i], values[i], value_sizes[i])) {
            return false;
        }
    }

    return true;
}

bool fossil_crabdb_search_by_pattern(fossil_crabdb_t* db, const char* pattern, char* result_buffer, size_t buffer_size) {
    if (!db || !pattern || !result_buffer) return false;

    /* Search for key-value pairs that match the pattern */
    fossil_crabdb_node_t* current = db->head;
    size_t offset = 0;
    while (current) {
        if (strstr(current->key, pattern) != NULL) {
            size_t length = snprintf(result_buffer + offset, buffer_size - offset, "%s=%s\n", current->key, current->value);
            if (length >= buffer_size - offset) {
                return false;
            }
            offset += length;
        }
        current = current->next;
    }

    return true;
}

bool fossil_crabdb_insert_with_ttl(fossil_crabdb_t* db, const char* key, const char* value, fossil_crabdb_type_t type, unsigned int ttl) {
    if (!db || !key || !value) return false;

    /* Check if the key already exists and update if needed */
    fossil_crabdb_node_t* current = db->head;
    while (current) {
        if (strcmp(current->key, key) == 0) {
            current->type = type;
            strncpy(current->value, value, FOSSIL_CRABDB_VAL_SIZE);
            current->ttl = ttl;
            current->timestamp = time(NULL);
            return true;
        }
        current = current->next;
    }

    /* Add a new node */
    fossil_crabdb_node_t* new_node = (fossil_crabdb_node_t*)malloc(sizeof(fossil_crabdb_node_t));
    if (!new_node) {
        return false;
    }
    strncpy(new_node->key, key, FOSSIL_CRABDB_KEY_SIZE);
    strncpy(new_node->value, value, FOSSIL_CRABDB_VAL_SIZE);
    new_node->type = type;
    new_node->ttl = ttl;
    new_node->timestamp = time(NULL);
    new_node->prev = db->tail;
    new_node->next = NULL;

    if (db->tail) {
        db->tail->next = new_node;
    } else {
        db->head = new_node;
    }

    db->tail = new_node;
    return true;
}

bool fossil_crabdb_cleanup_expired(fossil_crabdb_t* db) {
    if (!db) return false;

    time_t current_time = time(NULL);
    fossil_crabdb_node_t* current = db->head;
    while (current) {
        if (current->ttl > 0 && current_time - (uint32_t)current->timestamp > current->ttl) {
            fossil_crabdb_node_t* temp = current;
            current = current->next;
            if (temp->prev) {
                temp->prev->next = temp->next;
            } else {
                db->head = temp->next;
            }
            if (temp->next) {
                temp->next->prev = temp->prev;
            } else {
                db->tail = temp->prev;
            }
            free(temp);
        } else {
            current = current->next;
        }
    }

    return true;
}

unsigned int fossil_crabdb_get_version(fossil_crabdb_t* db) {
    if (!db) return 0;

    /* Count the number of nodes in the db */
    unsigned int version = 0;
    fossil_crabdb_node_t* current = db->head;
    while (current) {
        version++;
        current = current->next;
    }

    return version;
}

bool fossil_crabdb_restore_version(fossil_crabdb_t* db, unsigned int version) {
    if (!db) return false;

    /* Clear the db before restoring */
    fossil_crabdb_node_t* current = db->head;
    while (current) {
        fossil_crabdb_node_t* next = current->next;
        free(current);
        current = next;
    }
    db->head = db->tail = NULL;

    /* Restore the db to the specified version */
    char filename[256];
    snprintf(filename, sizeof(filename), "crabdb-%u.dat", version);
    FILE* file = fopen(filename, "rb");
    if (!file) {
        return false;
    }

    while (1) {
        fossil_crabdb_node_t* new_node = (fossil_crabdb_node_t*)malloc(sizeof(fossil_crabdb_node_t));
        if (!new_node) {
            fclose(file);
            return false;
        }

        if (fread(&new_node->type, sizeof(new_node->type), 1, file) != 1) {
            free(new_node);
            if (feof(file)) {
                break;  /* End of file reached */
            } else {
                fclose(file);
                return false;  /* Error reading the file */
            }
        }

        /* Read the key */
        if (fread(new_node->key, sizeof(char), FOSSIL_CRABDB_KEY_SIZE, file) != FOSSIL_CRABDB_KEY_SIZE) {
            free(new_node);
            fclose(file);
            return false;  /* Error reading the key */
        }

        /* Read the value */
        if (fread(new_node->value, sizeof(char), FOSSIL_CRABDB_VAL_SIZE, file) != FOSSIL_CRABDB_VAL_SIZE) {
            free(new_node);
            fclose(file);
            return false;  /* Error reading the value */
        }

        /* Add the node to the db */
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

bool fossil_crabdb_enable_logging(fossil_crabdb_t* db, const char* log_filename) {
    if (!db || !log_filename) return false;

    /* Open the log file for writing */
    db->db_file = fopen(log_filename, "w");
    if (!db->db_file) {
        return false;
    }

    /* Enable logging */
    db->logging_enabled = true;
    return true;
}

bool fossil_crabdb_disable_logging(fossil_crabdb_t* db) {
    if (!db) return false;

    /* Disable logging */
    db->logging_enabled = false;
    return true;
}

bool fossil_crabdb_check_integrity(fossil_crabdb_t* db) {
    if (!db) return false;

    fossil_crabdb_node_t* current = db->head;
    while (current) {
        fossil_crabdb_node_t* next = current->next;
        while (next) {
            if (strcmp(current->key, next->key) == 0) {
                return false;
            }
            next = next->next;
        }
        current = current->next;
    }

    return true;
}

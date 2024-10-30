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
    db->tables = NULL;
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

bool fossil_crabdb_create_table(fossil_crabdb_t* db, const char* table_name) {
    if (!db || !table_name) return false;

    /* Check if the table already exists */
    fossil_crabdb_table_t* current = db->tables;
    while (current) {
        if (strcmp(current->table_name, table_name) == 0) {
            return false; // Table already exists
        }
        current = current->next;
    }

    /* Create a new table */
    fossil_crabdb_table_t* new_table = (fossil_crabdb_table_t*)malloc(sizeof(fossil_crabdb_table_t));
    if (!new_table) {
        return false;
    }
    strncpy(new_table->table_name, table_name, FOSSIL_CRABDB_KEY_SIZE);
    new_table->rows = NULL;
    new_table->row_count = 0;
    new_table->next = NULL;

    if (db->tables) {
        fossil_crabdb_table_t* last = db->tables;
        while (last->next) {
            last = last->next;
        }
        last->next = new_table;
    } else {
        db->tables = new_table;
    }

    return true;
}

bool fossil_crabdb_delete_table(fossil_crabdb_t* db, const char* table_name) {
    if (!db || !table_name) return false;

    /* Check if the table exists and delete if found */
    fossil_crabdb_table_t* current = db->tables;
    fossil_crabdb_table_t* prev = NULL;
    while (current) {
        if (strcmp(current->table_name, table_name) == 0) {
            if (prev) {
                prev->next = current->next;
            } else {
                db->tables = current->next;
            }

            /* Free all rows in the table */
            fossil_crabdb_node_t* row = current->rows;
            while (row) {
                fossil_crabdb_node_t* next = row->next;
                free(row);
                row = next;
            }

            free(current);
            return true;
        }
        prev = current;
        current = current->next;
    }

    return false;
}

bool fossil_crabdb_table_exists(fossil_crabdb_t* db, const char* table_name) {
    if (!db || !table_name) return false;

    /* Check if the table exists */
    fossil_crabdb_table_t* current = db->tables;
    while (current) {
        if (strcmp(current->table_name, table_name) == 0) {
            return true;
        }
        current = current->next;
    }

    return false;
}

size_t fossil_crabdb_count_keys(fossil_crabdb_t* db) {
    if (!db) return 0;

    return db->node_count;
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

bool fossil_crabdb_list(fossil_crabdb_t* db, char* list_buffer, size_t buffer_size) {
    if (!db || !list_buffer) return false;

    /* List all key-value pairs */
    fossil_crabdb_node_t* current = db->head;
    size_t offset = 0;
    while (current) {
        size_t length = snprintf(list_buffer + offset, buffer_size - offset, "%s=%s\n", current->key, current->value);
        if (length >= buffer_size - offset) {
            return false;
        }
        offset += length;
        current = current->next;
    }

    return true;
}

bool fossil_crabdb_clear(fossil_crabdb_t* db) {
    if (!db) return false;

    /* Free all nodes in the db */
    fossil_crabdb_node_t* current = db->head;
    while (current) {
        fossil_crabdb_node_t* next = current->next;
        free(current);
        current = next;
    }

    db->head = db->tail = NULL;
    db->node_count = 0;
    return true;
}

bool fossil_crabdb_drop(fossil_crabdb_t* db) {
    if (!db) return false;

    /* Free all nodes in the db */
    fossil_crabdb_node_t* current = db->head;
    while (current) {
        fossil_crabdb_node_t* next = current->next;
        free(current);
        current = next;
    }

    db->head = db->tail = NULL;
    db->node_count = 0;
    return true;
}

bool fossil_crabdb_list_table(fossil_crabdb_t* db, const char* table_name, char* list_buffer, size_t buffer_size) {
    if (!db || !table_name || !list_buffer) return false;

    /* Check if the table exists */
    fossil_crabdb_table_t* current = db->tables;
    while (current) {
        if (strcmp(current->table_name, table_name) == 0) {
            /* List all rows in the table */
            fossil_crabdb_node_t* row = current->rows;
            size_t offset = 0;
            while (row) {
                size_t length = snprintf(list_buffer + offset, buffer_size - offset, "%s=%s\n", row->key, row->value);
                if (length >= buffer_size - offset) {
                    return false;
                }
                offset += length;
                row = row->next;
            }
            return true;
        }
        current = current->next;
    }

    return false;
}

bool fossil_crabdb_clear_table(fossil_crabdb_t* db, const char* table_name) {
    if (!db || !table_name) return false;

    /* Check if the table exists */
    fossil_crabdb_table_t* current = db->tables;
    while (current) {
        if (strcmp(current->table_name, table_name) == 0) {
            /* Free all rows in the table */
            fossil_crabdb_node_t* row = current->rows;
            while (row) {
                fossil_crabdb_node_t* next = row->next;
                free(row);
                row = next;
            }
            current->rows = NULL;
            current->row_count = 0;
            return true;
        }
        current = current->next;
    }

    return false;
}

bool fossil_crabdb_drop_table(fossil_crabdb_t* db, const char* table_name) {
    if (!db || !table_name) return false;

    /* Check if the table exists and delete if found */
    fossil_crabdb_table_t* current = db->tables;
    fossil_crabdb_table_t* prev = NULL;
    while (current) {
        if (strcmp(current->table_name, table_name) == 0) {
            if (prev) {
                prev->next = current->next;
            } else {
                db->tables = current->next;
            }

            /* Free all rows in the table */
            fossil_crabdb_node_t* row = current->rows;
            while (row) {
                fossil_crabdb_node_t* next = row->next;
                free(row);
                row = next;
            }

            free(current);
            return true;
        }
        prev = current;
        current = current->next;
    }

    return false;
}

bool fossil_crabdb_insert_into_table(fossil_crabdb_t* db, const char* table_name, const char* key, const char* value, fossil_crabdb_type_t type) {
    if (!db || !table_name || !key || !value) return false;

    /* Check if the table exists */
    fossil_crabdb_table_t* current = db->tables;
    while (current) {
        if (strcmp(current->table_name, table_name) == 0) {
            /* Check if the key already exists and update if needed */
            fossil_crabdb_node_t* current_node = current->rows;
            while (current_node) {
                if (strcmp(current_node->key, key) == 0) {
                    current_node->type = type;
                    strncpy(current_node->value, value, FOSSIL_CRABDB_VAL_SIZE);
                    return true;
                }
                current_node = current_node->next;
            }

            /* Add a new node */
            fossil_crabdb_node_t* new_node = (fossil_crabdb_node_t*)malloc(sizeof(fossil_crabdb_node_t));
            if (!new_node) {
                return false;
            }
            strncpy(new_node->key, key, FOSSIL_CRABDB_KEY_SIZE);
            strncpy(new_node->value, value, FOSSIL_CRABDB_VAL_SIZE);
            new_node->type = type;
            new_node->prev = current->rows ? current->rows->prev : NULL;
            new_node->next = current->rows;
            if (current->rows) {
                current->rows->prev = new_node;
            }
            current->rows = new_node;
            return true;
        }
        current = current->next;
    }

    return false;
}

bool fossil_crabdb_update_table(fossil_crabdb_t* db, const char* table_name, const char* key, const char* value) {
    if (!db || !table_name || !key || !value) return false;

    /* Check if the table exists */
    fossil_crabdb_table_t* current = db->tables;
    while (current) {
        if (strcmp(current->table_name, table_name) == 0) {
            /* Check if the key already exists and update if needed */
            fossil_crabdb_node_t* current_node = current->rows;
            while (current_node) {
                if (strcmp(current_node->key, key) == 0) {
                    strncpy(current_node->value, value, FOSSIL_CRABDB_VAL_SIZE);
                    return true;
                }
                current_node = current_node->next;
            }
        }
        current = current->next;
    }

    return false;
}

bool fossil_crabdb_delete_from_table(fossil_crabdb_t* db, const char* table_name, const char* key) {
    if (!db || !table_name || !key) return false;

    /* Check if the table exists */
    fossil_crabdb_table_t* current = db->tables;
    while (current) {
        if (strcmp(current->table_name, table_name) == 0) {
            /* Check if the key exists and delete if found */
            fossil_crabdb_node_t* current_node = current->rows;
            while (current_node) {
                if (strcmp(current_node->key, key) == 0) {
                    if (current_node->prev) {
                        current_node->prev->next = current_node->next;
                    } else {
                        current->rows = current_node->next;
                    }
                    if (current_node->next) {
                        current_node->next->prev = current_node->prev;
                    }
                    free(current_node);
                    return true;
                }
                current_node = current_node->next;
            }
        }
        current = current->next;
    }

    return false;
}

bool fossil_crabdb_select_from_table(fossil_crabdb_t* db, const char* table_name, const char* key, char* value, size_t value_size) {
    if (!db || !table_name || !key || !value) return false;

    /* Check if the table exists */
    fossil_crabdb_table_t* current = db->tables;
    while (current) {
        if (strcmp(current->table_name, table_name) == 0) {
            /* Check if the key exists and select if found */
            fossil_crabdb_node_t* current_node = current->rows;
            while (current_node) {
                if (strcmp(current_node->key, key) == 0) {
                    strncpy(value, current_node->value, value_size);
                    return true;
                }
                current_node = current_node->next;
            }
        }
        current = current->next;
    }

    return false;
}

bool fossil_crabdb_backup(const char* filename, fossil_crabdb_t* db) {
    if (!filename || !db) return false;

    FILE* backup_file = fopen(filename, "wb");
    if (!backup_file) {
        return false;
    }

    /* Write all key-value pairs to the backup file */
    fossil_crabdb_node_t* current = db->head;
    while (current) {
        fwrite(&current->type, sizeof(current->type), 1, backup_file);
        fwrite(current->key, sizeof(char), FOSSIL_CRABDB_KEY_SIZE, backup_file);
        fwrite(current->value, sizeof(char), FOSSIL_CRABDB_VAL_SIZE, backup_file);
        current = current->next;
    }

    fclose(backup_file);
    return true;
}

bool fossil_crabdb_restore(const char* filename, fossil_crabdb_t* db) {
    if (!filename || !db) return false;

    FILE* backup_file = fopen(filename, "rb");
    if (!backup_file) {
        return false;
    }

    /* Clear the current database */
    fossil_crabdb_clear(db);

    /* Read all key-value pairs from the backup file */
    while (!feof(backup_file)) {
        fossil_crabdb_node_t* new_node = (fossil_crabdb_node_t*)malloc(sizeof(fossil_crabdb_node_t));
        if (!new_node) {
            fclose(backup_file);
            return false;
        }

        fread(&new_node->type, sizeof(new_node->type), 1, backup_file);
        fread(new_node->key, sizeof(char), FOSSIL_CRABDB_KEY_SIZE, backup_file);
        fread(new_node->value, sizeof(char), FOSSIL_CRABDB_VAL_SIZE, backup_file);
        new_node->prev = db->tail;
        new_node->next = NULL;

        if (db->tail) {
            db->tail->next = new_node;
        } else {
            db->head = new_node;
        }

        db->tail = new_node;
        db->node_count++; // Increment the node count
    }

    fclose(backup_file);
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

bool fossil_crabdb_begin_transaction_table(fossil_crabdb_t* db, const char* table_name) {
    if (!db || !table_name) return false;

    // Check if a transaction is already active
    if (db->in_transaction) {
        return false; // A transaction is already in progress
    }

    // Start the transaction by marking the beginning state
    db->in_transaction = true;
    db->transaction_backup = backup_current_db_state(db); // Backup the current state

    // Check if the table exists
    fossil_crabdb_table_t* current = db->tables;
    while (current) {
        if (strcmp(current->table_name, table_name) == 0) {
            // Backup the table state
            fossil_crabdb_table_t* table_backup = (fossil_crabdb_table_t*)malloc(sizeof(fossil_crabdb_table_t));
            if (!table_backup) {
                return false;
            }
            strncpy(table_backup->table_name, current->table_name, FOSSIL_CRABDB_KEY_SIZE);
            table_backup->rows = current->rows;
            table_backup->row_count = current->row_count;
            table_backup->next = db->transaction_backup->tables;
            db->transaction_backup->tables = table_backup;
            return true;
        }
        current = current->next;
    }

    return false;
}

bool fossil_crabdb_commit_transaction_table(fossil_crabdb_t* db, const char* table_name) {
    if (!db->in_transaction) {
        return false; // No active transaction
    }

    // Check if the table exists
    fossil_crabdb_table_t* current = db->tables;
    while (current) {
        if (strcmp(current->table_name, table_name) == 0) {
            // Commit the transaction by writing changes to disk
            persist_changes_to_disk(db);
            db->in_transaction = false;
            free(db->transaction_backup); // Clear the backup as it's no longer needed
            return true;
        }
        current = current->next;
    }

    return false; // Table not found
}

bool fossil_crabdb_rollback_transaction_table(fossil_crabdb_t* db, const char* table_name) {
    if (!db->in_transaction) {
        return false; // No active transaction
    }

    // Check if the table exists
    fossil_crabdb_table_t* current = db->tables;
    while (current) {
        if (strcmp(current->table_name, table_name) == 0) {
            // Roll back to the backed-up state
            restore_from_backup(db, db->transaction_backup);
            db->in_transaction = false;
            free(db->transaction_backup); // Clear the backup
            return true;
        }
        current = current->next;
    }

    return false; // Table not found
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

bool fossil_crabdb_list_batch(fossil_crabdb_t* db, const char** keys, char** values, size_t* value_sizes, size_t count) {
    if (!db || !keys || !values || !value_sizes) return false;

    for (size_t i = 0; i < count; i++) {
        if (!fossil_crabdb_select(db, keys[i], values[i], value_sizes[i])) {
            return false;
        }
    }

    return true;
}

bool fossil_crabdb_clear_batch(fossil_crabdb_t* db, const char** keys, size_t count) {
    if (!db || !keys) return false;

    for (size_t i = 0; i < count; i++) {
        if (!fossil_crabdb_delete(db, keys[i])) {
            return false;
        }
    }

    return true;
}

bool fossil_crabdb_drop_batch(fossil_crabdb_t* db, const char** keys, size_t count) {
    if (!db || !keys) return false;

    for (size_t i = 0; i < count; i++) {
        if (!fossil_crabdb_delete(db, keys[i])) {
            return false;
        }
    }

    return true;
}

bool fossil_crabdb_insert_into_table_batch(fossil_crabdb_t* db, const char* table_name, const char** keys, const char** values, fossil_crabdb_type_t* types, size_t count) {
    if (!db || !table_name || !keys || !values || !types) return false;

    /* Check if the table exists */
    fossil_crabdb_table_t* current = db->tables;
    while (current) {
        if (strcmp(current->table_name, table_name) == 0) {
            for (size_t i = 0; i < count; i++) {
                if (!fossil_crabdb_insert_into_table(db, table_name, keys[i], values[i], types[i])) {
                    return false;
                }
            }
            return true;
        }
        current = current->next;
    }

    return false;
}

bool fossil_crabdb_delete_from_table_batch(fossil_crabdb_t* db, const char* table_name, const char** keys, size_t count) {
    if (!db || !table_name || !keys) return false;

    /* Check if the table exists */
    fossil_crabdb_table_t* current = db->tables;
    while (current) {
        if (strcmp(current->table_name, table_name) == 0) {
            for (size_t i = 0; i < count; i++) {
                if (!fossil_crabdb_delete_from_table(db, table_name, keys[i])) {
                    return false;
                }
            }
            return true;
        }
        current = current->next;
    }

    return false;
}

bool fossil_crabdb_update_table_batch(fossil_crabdb_t* db, const char* table_name, const char** keys, const char** values, size_t count) {
    if (!db || !table_name || !keys || !values) return false;

    /* Check if the table exists */
    fossil_crabdb_table_t* current = db->tables;
    while (current) {
        if (strcmp(current->table_name, table_name) == 0) {
            for (size_t i = 0; i < count; i++) {
                if (!fossil_crabdb_update_table(db, table_name, keys[i], values[i])) {
                    return false;
                }
            }
            return true;
        }
        current = current->next;
    }

    return false;
}

bool fossil_crabdb_select_from_table_batch(fossil_crabdb_t* db, const char* table_name, const char** keys, char** values, size_t* value_sizes, size_t count) {
    if (!db || !table_name || !keys || !values || !value_sizes) return false;

    /* Check if the table exists */
    fossil_crabdb_table_t* current = db->tables;
    while (current) {
        if (strcmp(current->table_name, table_name) == 0) {
            for (size_t i = 0; i < count; i++) {
                if (!fossil_crabdb_select_from_table(db, table_name, keys[i], values[i], value_sizes[i])) {
                    return false;
                }
            }
            return true;
        }
        current = current->next;
    }

    return false;
}

bool fossil_crabdb_list_table_batch(fossil_crabdb_t* db, const char* table_name, const char** keys, char** values, size_t* value_sizes, size_t count) {
    if (!db || !table_name || !keys || !values || !value_sizes) return false;

    /* Check if the table exists */
    fossil_crabdb_table_t* current = db->tables;
    while (current) {
        if (strcmp(current->table_name, table_name) == 0) {
            for (size_t i = 0; i < count; i++) {
                if (!fossil_crabdb_select_from_table(db, table_name, keys[i], values[i], value_sizes[i])) {
                    return false;
                }
            }
            return true;
        }
        current = current->next;
    }

    return false;
}

bool fossil_crabdb_clear_table_batch(fossil_crabdb_t* db, const char* table_name, const char** keys, size_t count) {
    if (!db || !table_name || !keys) return false;

    /* Check if the table exists */
    fossil_crabdb_table_t* current = db->tables;
    while (current) {
        if (strcmp(current->table_name, table_name) == 0) {
            for (size_t i = 0; i < count; i++) {
                if (!fossil_crabdb_delete_from_table(db, table_name, keys[i])) {
                    return false;
                }
            }
            return true;
        }
        current = current->next;
    }

    return false;
}

bool fossil_crabdb_drop_table_batch(fossil_crabdb_t* db, const char* table_name, const char** keys, size_t count) {
    if (!db || !table_name || !keys) return false;

    /* Check if the table exists */
    fossil_crabdb_table_t* current = db->tables;
    while (current) {
        if (strcmp(current->table_name, table_name) == 0) {
            for (size_t i = 0; i < count; i++) {
                if (!fossil_crabdb_delete_from_table(db, table_name, keys[i])) {
                    return false;
                }
            }
            return true;
        }
        current = current->next;
    }

    return false;
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

bool fossil_crabdb_sort_by_key(fossil_crabdb_t* db) {
    if (!db) return false;

    /* Sort the key-value pairs by key */
    fossil_crabdb_node_t* current = db->head;
    while (current) {
        fossil_crabdb_node_t* next = current->next;
        while (next) {
            if (strcmp(current->key, next->key) > 0) {
                char temp_key[FOSSIL_CRABDB_KEY_SIZE];
                char temp_value[FOSSIL_CRABDB_VAL_SIZE];
                fossil_crabdb_type_t temp_type;
                unsigned int temp_ttl;
                time_t temp_timestamp;

                strncpy(temp_key, current->key, FOSSIL_CRABDB_KEY_SIZE);
                strncpy(temp_value, current->value, FOSSIL_CRABDB_VAL_SIZE);
                temp_type = current->type;
                temp_ttl = current->ttl;
                temp_timestamp = current->timestamp;

                strncpy(current->key, next->key, FOSSIL_CRABDB_KEY_SIZE);
                strncpy(current->value, next->value, FOSSIL_CRABDB_VAL_SIZE);
                current->type = next->type;
                current->ttl = next->ttl;
                current->timestamp = next->timestamp;

                strncpy(next->key, temp_key, FOSSIL_CRABDB_KEY_SIZE);
                strncpy(next->value, temp_value, FOSSIL_CRABDB_VAL_SIZE);
                next->type = temp_type;
                next->ttl = temp_ttl;
                next->timestamp = temp_timestamp;
            }
            next = next->next;
        }
        current = current->next;
    }

    return true;
}

bool fossil_crabdb_sort_by_value(fossil_crabdb_t* db) {
    if (!db) return false;

    /* Sort the key-value pairs by value */
    fossil_crabdb_node_t* current = db->head;
    while (current) {
        fossil_crabdb_node_t* next = current->next;
        while (next) {
            if (strcmp(current->value, next->value) > 0) {
                char temp_key[FOSSIL_CRABDB_KEY_SIZE];
                char temp_value[FOSSIL_CRABDB_VAL_SIZE];
                fossil_crabdb_type_t temp_type;
                unsigned int temp_ttl;
                time_t temp_timestamp;

                strncpy(temp_key, current->key, FOSSIL_CRABDB_KEY_SIZE);
                strncpy(temp_value, current->value, FOSSIL_CRABDB_VAL_SIZE);
                temp_type = current->type;
                temp_ttl = current->ttl;
                temp_timestamp = current->timestamp;

                strncpy(current->key, next->key, FOSSIL_CRABDB_KEY_SIZE);
                strncpy(current->value, next->value, FOSSIL_CRABDB_VAL_SIZE);
                current->type = next->type;
                current->ttl = next->ttl;
                current->timestamp = next->timestamp;

                strncpy(next->key, temp_key, FOSSIL_CRABDB_KEY_SIZE);
                strncpy(next->value, temp_value, FOSSIL_CRABDB_VAL_SIZE);
                next->type = temp_type;
                next->ttl = temp_ttl;
                next->timestamp = temp_timestamp;
            }
            next = next->next;
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

bool fossil_crabdb_insert_with_ttl_into_table(fossil_crabdb_t* db, const char* table_name, const char* key, const char* value, fossil_crabdb_type_t type, unsigned int ttl) {
    if (!db || !table_name || !key || !value) return false;

    /* Check if the table exists */
    fossil_crabdb_table_t* current = db->tables;
    while (current) {
        if (strcmp(current->table_name, table_name) == 0) {
            /* Check if the key already exists and update if needed */
            fossil_crabdb_node_t* current_node = current->rows;
            while (current_node) {
                if (strcmp(current_node->key, key) == 0) {
                    current_node->type = type;
                    strncpy(current_node->value, value, FOSSIL_CRABDB_VAL_SIZE);
                    current_node->ttl = ttl;
                    current_node->timestamp = time(NULL);
                    return true;
                }
                current_node = current_node->next;
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
            new_node->prev = current->rows ? current->rows->prev : NULL;
            new_node->next = current->rows;
            if (current->rows) {
                current->rows->prev = new_node;
            }
            current->rows = new_node;
            return true;
        }
        current = current->next;
    }

    return false;
}

bool fossil_crabdb_cleanup_expired_table(fossil_crabdb_t* db, const char* table_name) {
    if (!db || !table_name) return false;

    /* Check if the table exists */
    fossil_crabdb_table_t* current = db->tables;
    while (current) {
        if (strcmp(current->table_name, table_name) == 0) {
            time_t current_time = time(NULL);
            fossil_crabdb_node_t* current_node = current->rows;
            while (current_node) {
                if (current_node->ttl > 0 && current_time - (uint32_t)current_node->timestamp > current_node->ttl) {
                    fossil_crabdb_node_t* temp = current_node;
                    current_node = current_node->next;
                    if (temp->prev) {
                        temp->prev->next = temp->next;
                    } else {
                        current->rows = temp->next;
                    }
                    if (temp->next) {
                        temp->next->prev = temp->prev;
                    }
                    free(temp);
                } else {
                    current_node = current_node->next;
                }
            }
            return true;
        }
        current = current->next;
    }

    return false;
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

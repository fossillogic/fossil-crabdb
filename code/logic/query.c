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
#include "fossil/crabdb/query.h"

char *custom_strdup(const char *str) {
    if (!str) return NULL;

    size_t len = strlen(str);
    char *new_str = (char *)malloc(len + 1);
    if (!new_str) return NULL;

    strcpy(new_str, str);
    return new_str;
}

bool fossil_crabql_query(fossil_crabdb_t *db, const char *query) {
    if (!db || !query) return false;

    int num_tokens = 0;
    
    // Create a modifiable copy of the query
    char *query_copy = custom_strdup(query);
    if (!query_copy) {
        return false;  // Memory allocation failed
    }

    char **tokens = fossil_crabql_tokenize(db, query_copy, &num_tokens);
    free(query_copy);  // Free the copy after tokenization

    if (!tokens) {
        return false;
    }

    bool result = fossil_crabql_parse_and_execute(db, tokens, num_tokens);
    for (int i = 0; i < num_tokens; i++) {
        free(tokens[i]);
    }
    free(tokens);

    return result;
}

char **fossil_crabql_tokenize(fossil_crabdb_t *db, const char *query, int *num_tokens) {
    if (!db || !query || !num_tokens) return NULL;

    // Tokenize the query string
    char **tokens = (char **)malloc(sizeof(char *) * MIN_BUFFER_SIZE);
    if (!tokens) {
        return NULL;
    }

    char *token = strtok((char *)query, " ");
    int count = 0;
    while (token) {
        // Ensure not exceeding allocated tokens space
        if (count >= MIN_BUFFER_SIZE) {
            // Optionally resize tokens array here
            break;  // Prevent buffer overflow
        }

        tokens[count] = custom_strdup(token);
        if (!tokens[count]) {
            for (int i = 0; i < count; i++) {
                free(tokens[i]);
            }
            free(tokens);
            return NULL;
        }
        count++;
        token = strtok(NULL, " ");
    }

    *num_tokens = count;
    return tokens;
}

bool fossil_crabql_parse_and_execute(fossil_crabdb_t *db, char **tokens, int num_tokens) {
    if (!db || !tokens || num_tokens < 1) return false;

    if (strcmp(tokens[0], "SELECT") == 0) {
        return fossil_crabql_execute_select(db, tokens, num_tokens);
    } else if (strcmp(tokens[0], "INSERT") == 0) {
        return fossil_crabql_execute_insert(db, tokens, num_tokens);
    } else if (strcmp(tokens[0], "UPDATE") == 0) {
        return fossil_crabql_execute_update(db, tokens, num_tokens);
    } else if (strcmp(tokens[0], "DELETE") == 0) {
        return fossil_crabql_execute_delete(db, tokens, num_tokens);
    } else if (strcmp(tokens[0], "CLEAR") == 0) {
        return fossil_crabql_execute_clear(db);
    } else if (strcmp(tokens[0], "BACKUP") == 0) {
        return fossil_crabql_execute_backup(db, tokens[1]);  // Assuming the filename is the second token
    } else if (strcmp(tokens[0], "RESTORE") == 0) {
        return fossil_crabql_execute_restore(db, tokens[1]);  // Assuming the filename is the second token
    } else if (strcmp(tokens[0], "BEGIN") == 0 && strcmp(tokens[1], "TRANSACTION") == 0) {
        return fossil_crabql_execute_begin_transaction(db);
    } else if (strcmp(tokens[0], "COMMIT") == 0 && strcmp(tokens[1], "TRANSACTION") == 0) {
        return fossil_crabql_execute_commit_transaction(db);
    } else if (strcmp(tokens[0], "ROLLBACK") == 0 && strcmp(tokens[1], "TRANSACTION") == 0) {
        return fossil_crabql_execute_rollback_transaction(db);
    } else if (strcmp(tokens[0], "INSERT") == 0 && strcmp(tokens[1], "BATCH") == 0) {
        return fossil_crabql_execute_insert_batch(db, &tokens[2], &tokens[num_tokens - 2], num_tokens - 2);  // Adjust as necessary
    } else if (strcmp(tokens[0], "DELETE") == 0 && strcmp(tokens[1], "BATCH") == 0) {
        return fossil_crabql_execute_delete_batch(db, &tokens[2], num_tokens - 2);  // Adjust as necessary
    } else if (strcmp(tokens[0], "UPDATE") == 0 && strcmp(tokens[1], "BATCH") == 0) {
        return fossil_crabql_execute_update_batch(db, &tokens[2], &tokens[num_tokens - 2], num_tokens - 2);  // Adjust as necessary
    } else if (strcmp(tokens[0], "SELECT") == 0 && strcmp(tokens[1], "BATCH") == 0) {
        return fossil_crabql_execute_select_batch(db, &tokens[2], &tokens[num_tokens - 2], num_tokens - 2);  // Adjust as necessary
    } else {
        fossil_crabdb_set_error(db, "Unknown command");
        return false;
    }
}

bool fossil_crabql_execute_select(fossil_crabdb_t *db, char **tokens, int num_tokens) {
    if (!db || !tokens || num_tokens < 4) {
        fossil_crabql_log_error("Invalid SELECT statement.");
        return false;
    }

    char value[FOSSIL_CRABDB_VAL_SIZE];
    if (!fossil_crabdb_select(db, tokens[1], value, sizeof(value))) {
        fossil_crabql_log_error("Failed to execute SELECT statement.");
        return false;
    }

    printf("%s=%s\n", tokens[1], value);
    return true;
}

bool fossil_crabql_execute_insert(fossil_crabdb_t *db, char **tokens, int num_tokens) {
    if (!db || !tokens || num_tokens < 4) {
        fossil_crabql_log_error("Invalid INSERT statement.");
        return false;
    }

    if (!fossil_crabdb_insert(db, tokens[1], tokens[3], FOSSIL_CRABDB_TYPE_STRING)) {
        fossil_crabql_log_error("Failed to execute INSERT statement.");
        return false;
    }

    return true;
}

bool fossil_crabql_execute_update(fossil_crabdb_t *db, char **tokens, int num_tokens) {
    if (!db || !tokens || num_tokens < 4) {
        fossil_crabql_log_error("Invalid UPDATE statement.");
        return false;
    }

    if (!fossil_crabdb_update(db, tokens[1], tokens[3])) {
        fossil_crabql_log_error("Failed to execute UPDATE statement.");
        return false;
    }

    return true;
}

bool fossil_crabql_execute_delete(fossil_crabdb_t *db, char **tokens, int num_tokens) {
    if (!db || !tokens || num_tokens < 2) {
        fossil_crabql_log_error("Invalid DELETE statement.");
        return false;
    }

    if (!fossil_crabdb_delete(db, tokens[1])) {
        fossil_crabql_log_error("Failed to execute DELETE statement.");
        return false;
    }

    return true;
}

bool fossil_crabql_execute_clear(fossil_crabdb_t* db) {
    if (!db) {
        fossil_crabql_log_error("Invalid CLEAR statement.");
        return false;
    }

    if (!fossil_crabdb_clear(db)) {
        fossil_crabql_log_error("Failed to execute CLEAR statement.");
        return false;
    }

    return true;
}

bool fossil_crabql_execute_backup(fossil_crabdb_t* db, const char* filename) {
    if (!db || !filename) {
        fossil_crabql_log_error("Invalid BACKUP statement.");
        return false;
    }

    if (!fossil_crabdb_backup(filename, db)) {
        fossil_crabql_log_error("Failed to execute BACKUP statement.");
        return false;
    }

    return true;
}

bool fossil_crabql_execute_restore(fossil_crabdb_t* db, const char* filename) {
    if (!db || !filename) {
        fossil_crabql_log_error("Invalid RESTORE statement.");
        return false;
    }

    if (!fossil_crabdb_restore(filename, db)) {
        fossil_crabql_log_error("Failed to execute RESTORE statement.");
        return false;
    }

    return true;
}

bool fossil_crabql_execute_begin_transaction(fossil_crabdb_t* db) {
    if (db->in_transaction) {
        fossil_crabql_log_error("Transaction already in progress.");
        return false;
    }

    if (!fossil_crabdb_begin_transaction(db)) {
        fossil_crabql_log_error("Failed to execute BEGIN TRANSACTION statement.");
        return false;
    }

    return true;
}

bool fossil_crabql_execute_commit_transaction(fossil_crabdb_t* db) {
    if (!db->in_transaction) {
        fossil_crabql_log_error("No active transaction.");
        return false;
    }

    if (!fossil_crabdb_commit_transaction(db)) {
        fossil_crabql_log_error("Failed to execute COMMIT TRANSACTION statement.");
        return false;
    }

    return true;
}

bool fossil_crabql_execute_rollback_transaction(fossil_crabdb_t* db) {
    if (!db->in_transaction) {
        fossil_crabql_log_error("No active transaction.");
        return false;
    }

    if (!fossil_crabdb_rollback_transaction(db)) {
        fossil_crabql_log_error("Failed to execute ROLLBACK TRANSACTION statement.");
        return false;
    }

    return true;
}

bool fossil_crabql_execute_insert_batch(fossil_crabdb_t* db, const char** keys, const char** values, size_t count) {
    if (!db || !keys || !values) {
        fossil_crabql_log_error("Invalid INSERT BATCH statement.");
        return false;
    }

    for (size_t i = 0; i < count; i++) {
        if (!fossil_crabdb_insert(db, keys[i], values[i], FOSSIL_CRABDB_TYPE_STRING)) {
            fossil_crabql_log_error("Failed to execute INSERT BATCH statement.");
            return false;
        }
    }

    return true;
}

bool fossil_crabql_execute_delete_batch(fossil_crabdb_t* db, const char** keys, size_t count) {
    if (!db || !keys) {
        fossil_crabql_log_error("Invalid DELETE BATCH statement.");
        return false;
    }

    for (size_t i = 0; i < count; i++) {
        if (!fossil_crabdb_delete(db, keys[i])) {
            fossil_crabql_log_error("Failed to execute DELETE BATCH statement.");
            return false;
        }
    }

    return true;
}

bool fossil_crabql_execute_update_batch(fossil_crabdb_t* db, const char** keys, const char** values, size_t count) {
    if (!db || !keys || !values) {
        fossil_crabql_log_error("Invalid UPDATE BATCH statement.");
        return false;
    }

    for (size_t i = 0; i < count; i++) {
        if (!fossil_crabdb_update(db, keys[i], values[i])) {
            fossil_crabql_log_error("Failed to execute UPDATE BATCH statement.");
            return false;
        }
    }

    return true;
}

bool fossil_crabql_execute_select_batch(fossil_crabdb_t* db, const char** keys, char** values, size_t count) {
    if (!db || !keys || !values) {
        fossil_crabql_log_error("Invalid SELECT BATCH statement.");
        return false;
    }

    for (size_t i = 0; i < count; i++) {
        char value[FOSSIL_CRABDB_VAL_SIZE];
        if (!fossil_crabdb_select(db, keys[i], value, sizeof(value))) {
            fossil_crabql_log_error("Failed to execute SELECT BATCH statement.");
            return false;
        }

        strncpy(values[i], value, FOSSIL_CRABDB_VAL_SIZE);
    }

    return true;
}

void fossil_crabql_log_error(const char *message) {
    if (message) {
        fprintf(stderr, "CrabQL Error: %s\n", message);
    }
}

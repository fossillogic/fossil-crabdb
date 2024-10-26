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
    } else if (strcmp(tokens[0], "BEGIN") == 0 && strcmp(tokens[1], "TRANSACTION") == 0) {
        return fossil_crabql_execute_begin_transaction(db);
    } else if (strcmp(tokens[0], "COMMIT") == 0 && strcmp(tokens[1], "TRANSACTION") == 0) {
        return fossil_crabql_execute_commit_transaction(db);
    } else if (strcmp(tokens[0], "ROLLBACK") == 0 && strcmp(tokens[1], "TRANSACTION") == 0) {
        return fossil_crabql_execute_rollback_transaction(db);
    } else {
        return false;
    }
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

void fossil_crabql_log_error(const char *message) {
    if (message) {
        fprintf(stderr, "CrabQL Error: %s\n", message);
    }
}
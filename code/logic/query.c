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

bool fossil_crabql_query(fossil_crabdb_t *db, const char *query) {
    if (!db || !query) return false;

    int num_tokens = 0;
    char **tokens = fossil_crabql_tokenize(db, query, &num_tokens);
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
        tokens[count] = strdup(token);
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
    } else {
        fossil_crabql_log_error("Invalid CrabQL statement.");
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

void fossil_crabql_log_error(const char *message) {
    if (message) {
        fprintf(stderr, "CrabQL Error: %s\n", message);
    }
}

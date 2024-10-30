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

extern char *custom_strdup(const char *str);

char **fossil_crabql_tokenize(fossil_crabdb_t *db, const char *query, int *num_tokens) {
    if (!db || !query || !num_tokens) return NULL;

    char **tokens = (char **)malloc(sizeof(char *) * MIN_BUFFER_SIZE);
    if (!tokens) return NULL;

    char *query_copy = custom_strdup(query); // Copy query since strtok modifies the string
    if (!query_copy) return NULL;

    char *token = strtok(query_copy, " ");
    int count = 0;

    while (token) {
        if (count >= MIN_BUFFER_SIZE) break;

        // Check if token is an operator and handle separately
        if (strcmp(token, "=") == 0 || strcmp(token, "!=") == 0 ||
            strcmp(token, "<") == 0 || strcmp(token, ">") == 0 ||
            strcmp(token, "<=") == 0 || strcmp(token, ">=") == 0 ||
            strcmp(token, "AND") == 0 || strcmp(token, "OR") == 0) {

            tokens[count++] = custom_strdup(token);
        } else {
            tokens[count++] = custom_strdup(token);
        }

        token = strtok(NULL, " ");
    }

    free(query_copy);
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

bool fossil_crabql_execute_select(fossil_crabdb_t *db, char **tokens, int num_tokens, int where_index) {
    if (!db || !tokens || num_tokens < 2) return false;

    // Basic SELECT without WHERE clause
    if (where_index == -1) {
        // Implement select logic here
        return true;
    } else {
        // Handle WHERE clause logic, e.g., field operator value
        char *field = tokens[where_index + 1];
        char *operator = tokens[where_index + 2];
        char *value = tokens[where_index + 3];

        // Apply the condition (e.g., equality check)
        char current_value[FOSSIL_CRABDB_VAL_SIZE];
        fossil_crabdb_select(db, field, current_value, sizeof(current_value));

        if (strcmp(operator, "=") == 0 && strcmp(current_value, value) == 0) {
            printf("%s = %s\n", field, current_value);
            return true;
        }
        // Additional operators can be handled here
        return false;
    }
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

// Function to load queries from a .crab file
crabql_status_t fossil_crabql_load_queries_from_file(fossil_crabdb_t *db, const char *filename) {
    FILE *file = fopen(filename, "r");
    if (!file) {
        return CRABQL_FILE_NOT_FOUND; // File could not be opened
    }

    char *query_buffer = NULL;
    size_t buffer_size = 0;
    crabql_status_t status = CRABQL_SUCCESS;

    while (getline(&query_buffer, &buffer_size, file) != -1) {
        // Strip trailing newline and whitespace
        size_t len = strlen(query_buffer);
        while (len > 0 && (query_buffer[len - 1] == '\n' || query_buffer[len - 1] == ' ')) {
            query_buffer[--len] = '\0';
        }

        // Handle semicolon-separated multiple queries in a single line
        char *query_start = query_buffer;
        char *query_end = NULL;

        while ((query_end = strchr(query_start, ';')) != NULL) {
            *query_end = '\0'; // Terminate the current query
            // Execute the current query segment if it has content
            if (*query_start != '\0') {
                status = fossil_crabql_execute(db, query_start);
                if (status != CRABQL_SUCCESS) {
                    free(query_buffer);
                    fclose(file);
                    return status; // Stop on first execution failure
                }
            }
            query_start = query_end + 1; // Move to the next segment
        }

        // Execute any remaining query in the buffer after the last semicolon
        if (*query_start != '\0') {
            status = fossil_crabql_execute(db, query_start);
            if (status != CRABQL_SUCCESS) {
                free(query_buffer);
                fclose(file);
                return status; // Stop on first execution failure
            }
        }
    }

    free(query_buffer);
    fclose(file);
    return CRABQL_SUCCESS; // Successfully loaded and executed all queries
}

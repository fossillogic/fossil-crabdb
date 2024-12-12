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

// Function to evaluate simple conditions with comparison operators
bool evaluate_condition(const char *field_value, const char *operator, const char *value) {
    if (strcmp(operator, "=") == 0) {
        return strcmp(field_value, value) == 0;
    } else if (strcmp(operator, "!=") == 0) {
        return strcmp(field_value, value) != 0;
    } else if (strcmp(operator, ">") == 0) {
        return strcmp(field_value, value) > 0;
    } else if (strcmp(operator, "<") == 0) {
        return strcmp(field_value, value) < 0;
    } else if (strcmp(operator, ">=") == 0) {
        return strcmp(field_value, value) >= 0;
    } else if (strcmp(operator, "<=") == 0) {
        return strcmp(field_value, value) <= 0;
    } else {
        return false;
    }
}

// Parse syntax
int fossil_crabql_validate_syntax(const char *query) {
    char *copy = custom_strdup(query);
    char *token = strtok(copy, " ");
    int result = 0;

    if (strcmp(token, "INSERT") == 0) {
        token = strtok(NULL, " ");
        if (strcmp(token, "INTO") != 0) {
            result = 1;
        }
    } else if (strcmp(token, "UPDATE") == 0) {
        token = strtok(NULL, " ");
        if (strcmp(token, "SET") != 0) {
            result = 2;
        }
    } else if (strcmp(token, "DELETE") == 0) {
        token = strtok(NULL, " ");
        if (strcmp(token, "FROM") != 0) {
            result = 3;
        }
    } else {
        result = 4;
    }

    free(copy);
    return result;
}

// *****************************************************************************
// Query Language Operations
// *****************************************************************************

// Execute a query to insert, update, or delete a key-value pair in the database
void fossil_crabql_execute(const char *query) {
    // Validate the syntax of the query
    int syntax_result = fossil_crabql_validate_syntax(query);
    if (syntax_result != 0) {
        fprintf(stderr, "Syntax error: %d\n", syntax_result);
        return;
    }

    // Parse the query
    char *copy = custom_strdup(query);
    char *token = strtok(copy, " ");
    char *table_name = NULL;
    char *field_name = NULL;
    char *field_value = NULL;
    char *operator = NULL;
    char *value = NULL;

    while (token != NULL) {
        if (strcmp(token, "FROM") == 0) {
            table_name = strtok(NULL, " ");
        } else if (strcmp(token, "WHERE") == 0) {
            field_name = strtok(NULL, " ");
            operator = strtok(NULL, " ");
            value = strtok(NULL, " ");
        } else if (strcmp(token, "VALUES") == 0) {
            field_value = strtok(NULL, " ");
        }

        token = strtok(NULL, " ");
    }

    // Execute the query
    if (strncmp(query, "INSERT", 6) == 0) {
        fossil_crabdb_create(field_name, field_value, strlen(field_value));
    } else if (strncmp(query, "UPDATE", 6) == 0) {
        if (evaluate_condition(field_name, operator, value)) {
            fossil_crabdb_update(field_name, field_value, strlen(field_value));
        }
    } else if (strncmp(query, "DELETE", 6) == 0) {
        if (evaluate_condition(field_name, operator, value)) {
            fossil_crabdb_delete(field_name);
        }
    } else {
        fprintf(stderr, "Invalid query: %s\n", query);
    }

    free(copy);
}

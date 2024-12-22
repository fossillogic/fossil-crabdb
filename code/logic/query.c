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
// Query Language API
// *****************************************************************************

int fossil_crabql_execute(fossil_crabdb_book_t *book, const char *query) {
    if (book == NULL || query == NULL) {
        return -1;
    }

    int syntax = fossil_crabql_validate_syntax(query);
    if (syntax != 0) {
        return syntax;
    }

    if (strncmp(query, "INSERT", 6) == 0) {
        return fossil_crabql_insert(book, query);
    } else if (strncmp(query, "UPDATE", 6) == 0) {
        return fossil_crabql_update(book, query);
    } else if (strncmp(query, "SELECT", 6) == 0) {
        return fossil_crabql_select(book, query);
    } else if (strncmp(query, "DELETE", 6) == 0) {
        return fossil_crabql_delete(book, query);
    } else if (strncmp(query, "JOIN", 4) == 0) {
        return fossil_crabql_join(book, query, NULL);
    } else {
        return -1;
    }
}

bool fossil_crabql_validate(const char *query) {
    if (query == NULL) {
        return false;
    }

    int syntax = fossil_crabql_validate_syntax(query);
    if (syntax != 0) {
        return false;
    }

    return true;
}

fossil_crabdb_book_t* fossil_crabql_select(fossil_crabdb_book_t *book, bool (*condition)(fossil_crabdb_entry_t *)) {
    if (book == NULL || condition == NULL) {
        return NULL;
    }

    fossil_crabdb_book_t *result = fossil_crabdb_init();
    if (result == NULL) {
        return NULL;
    }

    fossil_crabdb_page_t *current = book->head;
    while (current != NULL) {
        if (condition(&current->entry)) {
            fossil_crabdb_insert(result, custom_strdup(current->entry.key), custom_strdup(current->entry.value), current->entry.attributes);
        }
        current = current->next;
    }

    return result;
}

int fossil_crabql_insert(fossil_crabdb_book_t *book, const char *query) {
    if (book == NULL || query == NULL) {
        return -1;
    }

    char *copy = custom_strdup(query);
    char *token = strtok(copy, " ");
    if (strcmp(token, "INSERT") != 0) {
        free(copy);
        return -1;
    }

    token = strtok(NULL, " ");
    if (strcmp(token, "INTO") != 0) {
        free(copy);
        return -1;
    }

    token = strtok(NULL, " ");
    char *key = strtok(token, "=");
    char *value = strtok(NULL, "=");
    if (key == NULL || value == NULL) {
        free(copy);
        return -1;
    }

    fossil_crabdb_insert(book, key, value, (fossil_crabdb_attributes_t){false, false, false});
    free(copy);
    return 0;
}

int fossil_crabql_update(fossil_crabdb_book_t *book, const char *query) {
    if (book == NULL || query == NULL) {
        return -1;
    }

    char *copy = custom_strdup(query);
    char *token = strtok(copy, " ");
    if (strcmp(token, "UPDATE") != 0) {
        free(copy);
        return -1;
    }

    token = strtok(NULL, " ");
    char *key = strtok(token, "=");
    char *value = strtok(NULL, "=");
    if (key == NULL || value == NULL) {
        free(copy);
        return -1;
    }

    token = strtok(NULL, " ");
    if (strcmp(token, "WHERE") != 0) {
        free(copy);
        return -1;
    }

    token = strtok(NULL, " ");
    char *field = strtok(token, "=");
    char *operator = strtok(NULL, "=");
    char *condition = strtok(NULL, "=");
    if (field == NULL || operator == NULL || condition == NULL) {
        free(copy);
        return -1;
    }

    int count = 0;
    fossil_crabdb_page_t *current = book->head;
    while (current != NULL) {
        if (strcmp(current->entry.key, field) == 0 && evaluate_condition(current->entry.value, operator, condition)) {
            free(current->entry.value);
            current->entry.value = custom_strdup(value);
            count++;
        }
        current = current->next;
    }

    free(copy);
    return count;
}

int fossil_crabql_delete(fossil_crabdb_book_t *book, const char *query) {
    if (book == NULL || query == NULL) {
        return -1;
    }

    char *copy = custom_strdup(query);
    char *token = strtok(copy, " ");
    if (strcmp(token, "DELETE") != 0) {
        free(copy);
        return -1;
    }

    token = strtok(NULL, " ");
    if (strcmp(token, "FROM") != 0) {
        free(copy);
        return -1;
    }

    token = strtok(NULL, " ");
    if (strcmp(token, "WHERE") != 0) {
        free(copy);
        return -1;
    }

    token = strtok(NULL, " ");
    char *field = strtok(token, "=");
    char *operator = strtok(NULL, "=");
    char *condition = strtok(NULL, "=");
    if (field == NULL || operator == NULL || condition == NULL) {
        free(copy);
        return -1;
    }

    int count = 0;
    fossil_crabdb_page_t *current = book->head;
    while (current != NULL) {
        if (strcmp(current->entry.key, field) == 0 && evaluate_condition(current->entry.value, operator, condition)) {
            if (current->prev != NULL) {
                current->prev->next = current->next;
            } else {
                book->head = current->next;
            }
            if (current->next != NULL) {
                current->next->prev = current->prev;
            } else {
                book->tail = current->prev;
            }
            free(current->entry.key);
            free(current->entry.value);
            free(current);
            book->size--;
            count++;
        }
        current = current->next;
    }

    free(copy);
    return count;
}

int fossil_crabql_sort(fossil_crabdb_book_t *book, const char *query) {
    if (book == NULL || query == NULL) {
        return -1;
    }

    char *copy = custom_strdup(query);
    char *token = strtok(copy, " ");
    if (strcmp(token, "SORT") != 0) {
        free(copy);
        return -1;
    }

    token = strtok(NULL, " ");
    if (strcmp(token, "BY") != 0) {
        free(copy);
        return -1;
    }

    token = strtok(NULL, " ");
    char *field = strtok(token, " ");
    char *order = strtok(NULL, " ");
    if (field == NULL || order == NULL) {
        free(copy);
        return -1;
    }

    fossil_crabdb_sort_order_t sort_order = (strcmp(order, "ASC") == 0) ? FOSSIL_CRABDB_SORT_ASCENDING : FOSSIL_CRABDB_SORT_DESCENDING;
    int result = fossil_crabdb_sort(book, sort_order);

    free(copy);
    return result;
}

fossil_crabdb_book_t* fossil_crabql_join(fossil_crabdb_book_t *book1, fossil_crabdb_book_t *book2, const char *query) {
    if (book1 == NULL || book2 == NULL || query == NULL) {
        return NULL;
    }

    char *copy = custom_strdup(query);
    char *token = strtok(copy, " ");
    if (strcmp(token, "JOIN") != 0) {
        free(copy);
        return NULL;
    }

    token = strtok(NULL, " ");
    if (strcmp(token, "ON") != 0) {
        free(copy);
        return NULL;
    }

    token = strtok(NULL, " ");
    char *key = token;
    if (key == NULL) {
        free(copy);
        return NULL;
    }

    fossil_crabdb_book_t *result = fossil_crabdb_init();
    if (result == NULL) {
        free(copy);
        return NULL;
    }

    fossil_crabdb_page_t *current1 = book1->head;
    while (current1 != NULL) {
        fossil_crabdb_page_t *current2 = book2->head;
        while (current2 != NULL) {
            if (strcmp(current1->entry.key, key) == 0 && strcmp(current1->entry.key, current2->entry.key) == 0) {
                fossil_crabdb_insert(result, custom_strdup(current1->entry.key), custom_strdup(current1->entry.value), current1->entry.attributes);
                fossil_crabdb_insert(result, custom_strdup(current2->entry.key), custom_strdup(current2->entry.value), current2->entry.attributes);
            }
            current2 = current2->next;
        }
        current1 = current1->next;
    }

    free(copy);
    return result;
}

void fossil_crabql_print_results(fossil_crabdb_book_t *book) {
    if (book == NULL) {
        return;
    }

    fossil_crabdb_page_t *current = book->head;
    while (current != NULL) {
        printf("%s=%s\n", current->entry.key, current->entry.value);
        current = current->next;
    }
}

int fossil_crabql_export(fossil_crabdb_book_t *book, const char *filename) {
    if (book == NULL || filename == NULL) {
        return -1;
    }

    FILE *file = fopen(filename, "w");
    if (file == NULL) {
        return -1;
    }

    fossil_crabdb_page_t *current = book->head;
    while (current != NULL) {
        fprintf(file, "%s=%s\n", current->entry.key, current->entry.value);
        current = current->next;
    }

    fclose(file);
    return 0;
}

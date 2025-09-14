/**
 * -----------------------------------------------------------------------------
 * Project: Fossil Logic
 *
 * This file is part of the Fossil Logic project, which aims to develop
 * high-performance, cross-platform applications and libraries. The code
 * contained herein is licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License. You may obtain
 * a copy of the License at:
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS, WITHOUT
 * WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied. See the
 * License for the specific language governing permissions and limitations
 * under the License.
 *
 * Author: Michael Gene Brockus (Dreamer)
 * Date: 04/05/2014
 *
 * Copyright (C) 2014-2025 Fossil Logic. All rights reserved.
 * -----------------------------------------------------------------------------
 */
#include "fossil/crabdb/query.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// -----------------------------------------------------------------------------
// Internal Context
// -----------------------------------------------------------------------------

struct crabql_context {
    void* active_db;   // Handle to whichever backend is open
    char  backend[32]; // "myshell", "noshell", "timeshell", etc.
};

// -----------------------------------------------------------------------------
// Context Management
// -----------------------------------------------------------------------------

crabql_context_t* fossil_bluecrab_query_create(void) {
    crabql_context_t* ctx = calloc(1, sizeof(crabql_context_t));
    return ctx;
}

void fossil_bluecrab_query_destroy(crabql_context_t* ctx) {
    if (ctx) {
        fossil_bluecrab_query_close(ctx);
        free(ctx);
    }
}

// -----------------------------------------------------------------------------
// Script Execution (stub for interpreter engine)
// -----------------------------------------------------------------------------

bool fossil_bluecrab_query_exec(crabql_context_t* ctx, const char* code) {
    if (!ctx || !code) return false;
    // TODO: Parse & run CrabQL code
    printf("[CrabQL] Executing inline script:\n%s\n", code);
    return true;
}

bool fossil_bluecrab_query_exec_file(crabql_context_t* ctx, const char* path) {
    if (!ctx || !path) return false;
    FILE* f = fopen(path, "r");
    if (!f) return false;

    fseek(f, 0, SEEK_END);
    long size = ftell(f);
    fseek(f, 0, SEEK_SET);

    char* buffer = malloc(size + 1);
    fread(buffer, 1, size, f);
    buffer[size] = '\0';
    fclose(f);

    bool ok = fossil_bluecrab_query_exec(ctx, buffer);
    free(buffer);
    return ok;
}

// -----------------------------------------------------------------------------
// Database Operations (stubs; delegate to backends)
// -----------------------------------------------------------------------------

bool fossil_bluecrab_query_open(crabql_context_t* ctx, const char* dbfile) {
    if (!ctx || !dbfile) return false;

    // Simple heuristic: choose backend by file naming convention
    if (strstr(dbfile, "time")) {
        strcpy(ctx->backend, "timeshell");
        ctx->active_db = timeshell_open(dbfile);
    } else if (strstr(dbfile, "cache")) {
        strcpy(ctx->backend, "cacheshell");
        ctx->active_db = cacheshell_open(dbfile);
    } else {
        strcpy(ctx->backend, "myshell"); // default
        ctx->active_db = myshell_open(dbfile);
    }

    return ctx->active_db != NULL;
}

bool fossil_bluecrab_query_close(crabql_context_t* ctx) {
    if (!ctx || !ctx->active_db) return false;

    if (strcmp(ctx->backend, "timeshell") == 0)
        timeshell_close(ctx->active_db);
    else if (strcmp(ctx->backend, "cacheshell") == 0)
        cacheshell_close(ctx->active_db);
    else
        myshell_close(ctx->active_db);

    ctx->active_db = NULL;
    return true;
}

bool fossil_bluecrab_query_insert(crabql_context_t* ctx,
                                  const char* key,
                                  const char* json_value) {
    if (!ctx || !ctx->active_db) return false;
    if (strcmp(ctx->backend, "timeshell") == 0)
        return timeshell_insert(ctx->active_db, key, json_value);
    if (strcmp(ctx->backend, "cacheshell") == 0)
        return cacheshell_insert(ctx->active_db, key, json_value);
    return myshell_insert(ctx->active_db, key, json_value);
}

bool fossil_bluecrab_query_update(crabql_context_t* ctx,
                                  const char* key,
                                  const char* json_value) {
    if (!ctx || !ctx->active_db) return false;
    if (strcmp(ctx->backend, "timeshell") == 0)
        return timeshell_update(ctx->active_db, key, json_value);
    if (strcmp(ctx->backend, "cacheshell") == 0)
        return cacheshell_update(ctx->active_db, key, json_value);
    return myshell_update(ctx->active_db, key, json_value);
}

bool fossil_bluecrab_query_remove(crabql_context_t* ctx, const char* key) {
    if (!ctx || !ctx->active_db) return false;
    if (strcmp(ctx->backend, "timeshell") == 0)
        return timeshell_remove(ctx->active_db, key);
    if (strcmp(ctx->backend, "cacheshell") == 0)
        return cacheshell_remove(ctx->active_db, key);
    return myshell_remove(ctx->active_db, key);
}

char* fossil_bluecrab_query_get(crabql_context_t* ctx, const char* key) {
    if (!ctx || !ctx->active_db) return NULL;
    if (strcmp(ctx->backend, "timeshell") == 0)
        return timeshell_get(ctx->active_db, key);
    if (strcmp(ctx->backend, "cacheshell") == 0)
        return cacheshell_get(ctx->active_db, key);
    return myshell_get(ctx->active_db, key);
}

size_t fossil_bluecrab_query_count(crabql_context_t* ctx) {
    if (!ctx || !ctx->active_db) return 0;
    if (strcmp(ctx->backend, "timeshell") == 0)
        return timeshell_count(ctx->active_db);
    if (strcmp(ctx->backend, "cacheshell") == 0)
        return cacheshell_count(ctx->active_db);
    return myshell_count(ctx->active_db);
}

// -----------------------------------------------------------------------------
// Module Import
// -----------------------------------------------------------------------------

bool fossil_bluecrab_query_import(crabql_context_t* ctx, const char* module) {
    if (!ctx || !module) return false;
    printf("[CrabQL] Importing module: %s\n", module);
    // TODO: integrate with interpreter module registry
    return true;
}

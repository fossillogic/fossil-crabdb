#ifndef FOSSIL_BLUECRAB_QUERY_H
#define FOSSIL_BLUECRAB_QUERY_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

// Include all backends
#include "cacheshell.h"
#include "fileshell.h"
#include "timeshell.h"
#include "myshell.h"
#include "noshell.h"

#ifdef __cplusplus
extern "C" {
#endif

// -----------------------------------------------------------------------------
// Forward Declarations
// -----------------------------------------------------------------------------

typedef struct crabql_context crabql_context_t;

// -----------------------------------------------------------------------------
// Context Management
// -----------------------------------------------------------------------------

/**
 * @brief Create a new CrabQL context.
 */
crabql_context_t* fossil_bluecrab_query_create(void);

/**
 * @brief Destroy a CrabQL context.
 */
void fossil_bluecrab_query_destroy(crabql_context_t* ctx);

// -----------------------------------------------------------------------------
// Script Execution
// -----------------------------------------------------------------------------

/**
 * @brief Execute a CrabQL script from a string buffer.
 *
 * @param ctx  CrabQL context
 * @param code CrabQL source code (UTF-8)
 * @return true if script ran successfully, false otherwise
 */
bool fossil_bluecrab_query_exec(crabql_context_t* ctx, const char* code);

/**
 * @brief Execute a CrabQL script from a file (.crabql).
 *
 * @param ctx   CrabQL context
 * @param path  Path to script file
 * @return true if successful
 */
bool fossil_bluecrab_query_exec_file(crabql_context_t* ctx, const char* path);

// -----------------------------------------------------------------------------
// Database Utilities (Backend Agnostic)
// -----------------------------------------------------------------------------

bool fossil_bluecrab_query_open(crabql_context_t* ctx, const char* dbfile);
bool fossil_bluecrab_query_close(crabql_context_t* ctx);

bool fossil_bluecrab_query_insert(crabql_context_t* ctx,
                                  const char* key,
                                  const char* json_value);

bool fossil_bluecrab_query_update(crabql_context_t* ctx,
                                  const char* key,
                                  const char* json_value);

bool fossil_bluecrab_query_remove(crabql_context_t* ctx, const char* key);

char* fossil_bluecrab_query_get(crabql_context_t* ctx, const char* key);

size_t fossil_bluecrab_query_count(crabql_context_t* ctx);

// -----------------------------------------------------------------------------
// Module Import API
// -----------------------------------------------------------------------------

/**
 * @brief Import a backend-specific module into CrabQL runtime.
 *
 * Example: fossil_bluecrab_query_import(ctx, "timeshell");
 */
bool fossil_bluecrab_query_import(crabql_context_t* ctx, const char* module);

#ifdef __cplusplus
}
#endif

#endif // FOSSIL_BLUECRAB_QUERY_H

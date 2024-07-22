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

package fossil

class CrabDB(private val fossilCrabdb: FossilCrabdb) {
    /**
     * Create a new namespace in the CrabDB.
     *
     * @return Error code indicating the result of the operation.
     */
    fun createNamespace(): CrabDB {
        val db = fossilCrabdb.createNamespace()
        return CrabDB(db)
    }

    /**
     * Erase the CrabDB database.
     */
    fun erase() {
        fossilCrabdb.erase()
    }

    /**
     * Insert data into a namespace in the CrabDB.
     *
     * @param namespaceName Name of the namespace.
     * @param key Key of the data to insert.
     * @param value Value of the data to insert.
     * @return Error code indicating the result of the operation.
     */
    fun insert(namespaceName: String, key: String, value: String): FossilCrabdbError {
        return fossilCrabdb.insert(namespaceName, key, value)
    }

    /**
     * Get data from a namespace in the CrabDB.
     *
     * @param namespaceName Name of the namespace.
     * @param key Key of the data to get.
     * @return Retrieved value or null if not found.
     */
    fun get(namespaceName: String, key: String): String? {
        return fossilCrabdb.get(namespaceName, key)
    }

    /**
     * Update data in a namespace in the CrabDB.
     *
     * @param namespaceName Name of the namespace.
     * @param key Key of the data to update.
     * @param value New value for the data.
     * @return Error code indicating the result of the operation.
     */
    fun update(namespaceName: String, key: String, value: String): FossilCrabdbError {
        return fossilCrabdb.update(namespaceName, key, value)
    }

    /**
     * Delete data from a namespace in the CrabDB.
     *
     * @param namespaceName Name of the namespace.
     * @param key Key of the data to delete.
     * @return Error code indicating the result of the operation.
     */
    fun delete(namespaceName: String, key: String): FossilCrabdbError {
        return fossilCrabdb.delete(namespaceName, key)
    }
}

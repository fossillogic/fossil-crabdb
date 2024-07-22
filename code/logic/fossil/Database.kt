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

class FossilCrabdb {
    private data class KeyValue(val key: String, val value: String, var next: KeyValue? = null)

    private data class Namespace(
        val name: String,
        var subNamespaceCount: Int = 0,
        var subNamespaces: Array<Namespace> = emptyArray(),
        var data: KeyValue? = null,
        var next: Namespace? = null
    )

    private var namespaces: Namespace? = null

    /**
     * Create a new namespace.
     *
     * @return Error code indicating the result of the operation.
     */
    fun createNamespace(): FossilCrabdb? {
        val db = FossilCrabdb()
        db.namespaces = null
        return db
    }

    /**
     * Erase the FossilCrabdb database.
     */
    fun erase() {
        var current = namespaces
        while (current != null) {
            val next = current.next
            current.data?.let { kv ->
                var kvNext = kv.next
                while (kvNext != null) {
                    kvNext = kvNext.next
                }
            }
            current = next
        }
        namespaces = null
    }

    /**
     * Insert data into a namespace.
     *
     * @param namespaceName Name of the namespace.
     * @param key Key of the data to insert.
     * @param value Value of the data to insert.
     * @return Error code indicating the result of the operation.
     */
    fun insert(namespaceName: String, key: String, value: String): FossilCrabdbError {
        if (namespaceName.isBlank() || key.isBlank() || value.isBlank()) {
            return FossilCrabdbError.MEM
        }

        var current = namespaces
        while (current != null) {
            if (current.name == namespaceName) {
                var kv = current.data
                while (kv != null) {
                    if (kv.key == key) {
                        return FossilCrabdbError.KEY_NOT_FOUND // Key already exists
                    }
                    kv = kv.next
                }

                val newKv = KeyValue(key, value)
                newKv.next = current.data
                current.data = newKv

                return FossilCrabdbError.OK
            }
            current = current.next
        }

        return FossilCrabdbError.NS_NOT_FOUND
    }

    /**
     * Get data from a namespace.
     *
     * @param namespaceName Name of the namespace.
     * @param key Key of the data to get.
     * @return Retrieved value or null if not found.
     */
    fun get(namespaceName: String, key: String): String? {
        if (namespaceName.isBlank() || key.isBlank()) {
            return null
        }

        var current = namespaces
        while (current != null) {
            if (current.name == namespaceName) {
                var kv = current.data
                while (kv != null) {
                    if (kv.key == key) {
                        return kv.value
                    }
                    kv = kv.next
                }
                return null
            }
            current = current.next
        }

        return null
    }

    /**
     * Update data in a namespace.
     *
     * @param namespaceName Name of the namespace.
     * @param key Key of the data to update.
     * @param value New value for the data.
     * @return Error code indicating the result of the operation.
     */
    fun update(namespaceName: String, key: String, value: String): FossilCrabdbError {
        if (namespaceName.isBlank() || key.isBlank() || value.isBlank()) {
            return FossilCrabdbError.MEM
        }

        var current = namespaces
        while (current != null) {
            if (current.name == namespaceName) {
                var kv = current.data
                while (kv != null) {
                    if (kv.key == key) {
                        kv.value = value
                        return FossilCrabdbError.OK
                    }
                    kv = kv.next
                }
                return FossilCrabdbError.KEY_NOT_FOUND
            }
            current = current.next
        }

        return FossilCrabdbError.NS_NOT_FOUND
    }

    /**
     * Delete data from a namespace.
     *
     * @param namespaceName Name of the namespace.
     * @param key Key of the data to delete.
     * @return Error code indicating the result of the operation.
     */
    fun delete(namespaceName: String, key: String): FossilCrabdbError {
        if (namespaceName.isBlank() || key.isBlank()) {
            return FossilCrabdbError.MEM
        }

        var current = namespaces
        while (current != null) {
            if (current.name == namespaceName) {
                var prev: KeyValue? = null
                var kv = current.data
                while (kv != null) {
                    if (kv.key == key) {
                        if (prev != null) {
                            prev.next = kv.next
                        } else {
                            current.data = kv.next
                        }
                        return FossilCrabdbError.OK
                    }
                    prev = kv
                    kv = kv.next
                }
                return FossilCrabdbError.KEY_NOT_FOUND
            }
            current = current.next
        }

        return FossilCrabdbError.NS_NOT_FOUND
    }
}

enum class FossilCrabdbError {
    OK,
    MEM,
    NS_NOT_FOUND,
    KEY_NOT_FOUND,
    INVALID_QUERY
}

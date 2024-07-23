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

/**
 * Key-value pair structure.
 */
data class FossilCrabdbKeyValue(
    val key: String, // Key of the key-value pair
    var value: String // Value of the key-value pair
)

/**
 * Namespace structure.
 */
data class FossilCrabdbNamespace(
    val name: String, // Name of the namespace
    var subNamespaces: MutableList<FossilCrabdbNamespace> = mutableListOf(), // List of sub-namespaces
    var data: MutableList<FossilCrabdbKeyValue> = mutableListOf() // List of key-value pairs
)

/**
 * Database structure.
 */
class FossilCrabdb {
    val namespaces: MutableList<FossilCrabdbNamespace> = mutableListOf()

    fun createNamespace(name: String): FossilCrabdbNamespace {
        val namespace = FossilCrabdbNamespace(name)
        namespaces.add(namespace)
        return namespace
    }

    fun eraseNamespace(namespace: FossilCrabdbNamespace) {
        namespaces.remove(namespace)
    }

    fun insertData(namespace: FossilCrabdbNamespace, key: String, value: String) {
        val existingData = namespace.data.find { it.key == key }
        if (existingData != null) {
            throw IllegalArgumentException("Key already exists")
        }
        namespace.data.add(FossilCrabdbKeyValue(key, value))
    }

    fun getData(namespace: FossilCrabdbNamespace, key: String): String? {
        val data = namespace.data.find { it.key == key }
        return data?.value
    }

    fun updateData(namespace: FossilCrabdbNamespace, key: String, value: String) {
        val data = namespace.data.find { it.key == key }
        if (data != null) {
            data.value = value
        } else {
            throw IllegalArgumentException("Key not found")
        }
    }

    fun deleteData(namespace: FossilCrabdbNamespace, key: String) {
        val data = namespace.data.find { it.key == key }
        if (data != null) {
            namespace.data.remove(data)
        } else {
            throw IllegalArgumentException("Key not found")
        }
    }

    fun executeQuery(query: String) {
        val command = query.substringBefore("(").trim()
        val args = query.substringAfter("(").substringBefore(")").split(",").map { it.trim() }

        when (command) {
            "create_namespace" -> {
                if (args.size == 1) {
                    createNamespace(args[0])
                }
            }
            "erase_namespace" -> {
                if (args.size == 1) {
                    val namespace = namespaces.find { it.name == args[0] }
                    if (namespace != null) {
                        eraseNamespace(namespace)
                    }
                }
            }
            "insert" -> {
                if (args.size == 3) {
                    val namespace = namespaces.find { it.name == args[0] }
                    if (namespace != null) {
                        insertData(namespace, args[1], args[2])
                    }
                }
            }
            "get" -> {
                if (args.size == 2) {
                    val namespace = namespaces.find { it.name == args[0] }
                    if (namespace != null) {
                        val value = getData(namespace, args[1])
                        println("Value: $value")
                    }
                }
            }
            "update" -> {
                if (args.size == 3) {
                    val namespace = namespaces.find { it.name == args[0] }
                    if (namespace != null) {
                        updateData(namespace, args[1], args[2])
                    }
                }
            }
            "delete" -> {
                if (args.size == 2) {
                    val namespace = namespaces.find { it.name == args[0] }
                    if (namespace != null) {
                        deleteData(namespace, args[1])
                    }
                }
            }
            else -> throw IllegalArgumentException("Invalid query")
        }
    }
}

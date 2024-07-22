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

class FossilCrabdbNamespace(
    var name: String,
    var subNamespaces: MutableList<FossilCrabdbNamespace> = mutableListOf(),
    var data: MutableList<FossilCrabdbKeyValue> = mutableListOf()
)

class FossilCrabdbKeyValue(
    var key: String,
    var value: String
)

class FossilCrabdb(
    var namespaces: MutableList<FossilCrabdbNamespace> = mutableListOf()
) {
    fun createNamespace(namespaceName: String): FossilCrabdbError {
        if (namespaceName.isBlank()) return FossilCrabdbError.MEM

        if (namespaces.any { it.name == namespaceName }) {
            return FossilCrabdbError.NS_EXISTS
        }

        val newNamespace = FossilCrabdbNamespace(namespaceName)
        namespaces.add(newNamespace)

        return FossilCrabdbError.OK
    }

    fun createSubNamespace(namespaceName: String, subNamespaceName: String): FossilCrabdbError {
        if (namespaceName.isBlank() || subNamespaceName.isBlank()) return FossilCrabdbError.MEM

        val namespace = namespaces.find { it.name == namespaceName }
        if (namespace != null) {
            if (namespace.subNamespaces.any { it.name == subNamespaceName }) {
                return FossilCrabdbError.SUB_NS_EXISTS
            }

            val newSubNamespace = FossilCrabdbNamespace(subNamespaceName)
            namespace.subNamespaces.add(newSubNamespace)

            return FossilCrabdbError.OK
        }

        return FossilCrabdbError.NS_NOT_FOUND
    }

    fun eraseNamespace(namespaceName: String): FossilCrabdbError {
        if (namespaceName.isBlank()) return FossilCrabdbError.MEM

        val namespace = namespaces.find { it.name == namespaceName }
        if (namespace != null) {
            namespaces.remove(namespace)
            return FossilCrabdbError.OK
        }

        return FossilCrabdbError.NS_NOT_FOUND
    }

    fun eraseSubNamespace(namespaceName: String, subNamespaceName: String): FossilCrabdbError {
        if (namespaceName.isBlank() || subNamespaceName.isBlank()) return FossilCrabdbError.MEM

        val namespace = namespaces.find { it.name == namespaceName }
        if (namespace != null) {
            val subNamespace = namespace.subNamespaces.find { it.name == subNamespaceName }
            if (subNamespace != null) {
                namespace.subNamespaces.remove(subNamespace)
                return FossilCrabdbError.OK
            }
        }

        return FossilCrabdbError.SUB_NS_NOT_FOUND
    }
}

enum class FossilCrabdbError {
    OK,
    MEM,
    NS_EXISTS,
    NS_NOT_FOUND,
    SUB_NS_EXISTS,
    SUB_NS_NOT_FOUND
}

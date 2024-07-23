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

import org.junit.jupiter.api.Assertions.*
import org.junit.jupiter.api.BeforeEach
import org.junit.jupiter.api.Test

import fossil.FossilCrabdb

class FossilCrabdbTest {

    private lateinit var db: FossilCrabdb

    @BeforeEach
    fun setUp() {
        db = FossilCrabdb()
    }

    @Test
    fun testCreateNamespace() {
        val namespace = db.createNamespace("testNamespace")
        assertNotNull(namespace)
        assertEquals("testNamespace", namespace.name)
        assertTrue(namespace.subNamespaces.isEmpty())
        assertTrue(namespace.data.isEmpty())
    }

    @Test
    fun testEraseNamespace() {
        val namespace = db.createNamespace("testNamespace")
        assertEquals(1, db.namespaces.size)

        db.eraseNamespace(namespace)
        assertTrue(db.namespaces.isEmpty())
    }

    @Test
    fun testInsertData() {
        val namespace = db.createNamespace("testNamespace")

        db.insertData(namespace, "key1", "value1")
        assertEquals(1, namespace.data.size)
        assertEquals("value1", namespace.data[0].value)

        val exception = assertThrows(IllegalArgumentException::class.java) {
            db.insertData(namespace, "key1", "value2")
        }
        assertEquals("Key already exists", exception.message)
    }

    @Test
    fun testGetData() {
        val namespace = db.createNamespace("testNamespace")

        db.insertData(namespace, "key1", "value1")
        val value = db.getData(namespace, "key1")
        assertEquals("value1", value)

        val nullValue = db.getData(namespace, "key2")
        assertNull(nullValue)
    }

    @Test
    fun testUpdateData() {
        val namespace = db.createNamespace("testNamespace")

        db.insertData(namespace, "key1", "value1")
        db.updateData(namespace, "key1", "value2")
        assertEquals("value2", namespace.data[0].value)

        val exception = assertThrows(IllegalArgumentException::class.java) {
            db.updateData(namespace, "key2", "value3")
        }
        assertEquals("Key not found", exception.message)
    }

    @Test
    fun testDeleteData() {
        val namespace = db.createNamespace("testNamespace")

        db.insertData(namespace, "key1", "value1")
        db.deleteData(namespace, "key1")
        assertTrue(namespace.data.isEmpty())

        val exception = assertThrows(IllegalArgumentException::class.java) {
            db.deleteData(namespace, "key2")
        }
        assertEquals("Key not found", exception.message)
    }

    @Test
    fun testExecuteQuery() {
        db.executeQuery("create_namespace(testNamespace)")
        val namespace = db.namespaces.find { it.name == "testNamespace" }
        assertNotNull(namespace)

        db.executeQuery("insert(testNamespace, key1, value1)")
        val value = db.getData(namespace!!, "key1")
        assertEquals("value1", value)

        db.executeQuery("update(testNamespace, key1, value2)")
        val updatedValue = db.getData(namespace, "key1")
        assertEquals("value2", updatedValue)

        db.executeQuery("delete(testNamespace, key1)")
        val deletedValue = db.getData(namespace, "key1")
        assertNull(deletedValue)

        db.executeQuery("erase_namespace(testNamespace)")
        val erasedNamespace = db.namespaces.find { it.name == "testNamespace" }
        assertNull(erasedNamespace)
    }
}

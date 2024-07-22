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

class FossilCrabDBTest {

    private lateinit var db: FossilCrabdb

    @BeforeEach
    fun setUp() {
        db = FossilCrabdb()
        db = db.createNamespace() ?: throw IllegalStateException("Failed to create namespace")
    }

    @Test
    fun testCreateNamespace() {
        assertNotNull(db)
    }

    @Test
    fun testInsertAndGet() {
        assertEquals(FossilCrabdbError.NS_NOT_FOUND, db.insert("nonexistent", "key", "value"))
        assertNull(db.get("nonexistent", "key"))

        db.insert("root", "key1", "value1")
        assertEquals("value1", db.get("root", "key1"))

        assertEquals(FossilCrabdbError.KEY_NOT_FOUND, db.insert("root", "key1", "value2"))
    }

    @Test
    fun testUpdate() {
        db.insert("root", "key1", "value1")
        assertEquals(FossilCrabdbError.OK, db.update("root", "key1", "newvalue1"))
        assertEquals("newvalue1", db.get("root", "key1"))

        assertEquals(FossilCrabdbError.KEY_NOT_FOUND, db.update("root", "nonexistent", "value"))
        assertEquals(FossilCrabdbError.NS_NOT_FOUND, db.update("nonexistent", "key", "value"))
    }

    @Test
    fun testDelete() {
        db.insert("root", "key1", "value1")
        assertEquals(FossilCrabdbError.OK, db.delete("root", "key1"))
        assertNull(db.get("root", "key1"))

        assertEquals(FossilCrabdbError.KEY_NOT_FOUND, db.delete("root", "key1"))
        assertEquals(FossilCrabdbError.NS_NOT_FOUND, db.delete("nonexistent", "key1"))
    }

    @Test
    fun testErase() {
        db.insert("root", "key1", "value1")
        db.erase()
        assertNull(db.get("root", "key1"))
    }

    @Test
    fun testInvalidInputs() {
        assertEquals(FossilCrabdbError.MEM, db.insert("", "key", "value"))
        assertEquals(FossilCrabdbError.MEM, db.insert("namespace", "", "value"))
        assertEquals(FossilCrabdbError.MEM, db.insert("namespace", "key", ""))

        assertEquals(FossilCrabdbError.MEM, db.update("", "key", "value"))
        assertEquals(FossilCrabdbError.MEM, db.update("namespace", "", "value"))
        assertEquals(FossilCrabdbError.MEM, db.update("namespace", "key", ""))

        assertEquals(FossilCrabdbError.MEM, db.delete("", "key"))
        assertEquals(FossilCrabdbError.MEM, db.delete("namespace", ""))
    }
}

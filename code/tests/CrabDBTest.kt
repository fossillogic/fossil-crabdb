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

class CrabDBTest {

    private lateinit var fossilCrabdb: FossilCrabdb
    private lateinit var crabDB: CrabDB

    @BeforeEach
    fun setUp() {
        fossilCrabdb = FossilCrabdb()
        fossilCrabdb = fossilCrabdb.createNamespace() ?: throw IllegalStateException("Failed to create namespace")
        crabDB = CrabDB(fossilCrabdb)
    }

    @Test
    fun testCreateNamespace() {
        val newCrabDB = crabDB.createNamespace()
        assertNotNull(newCrabDB)
    }

    @Test
    fun testErase() {
        crabDB.insert("root", "key1", "value1")
        assertNotNull(crabDB.get("root", "key1"))

        crabDB.erase()
        assertNull(crabDB.get("root", "key1"))
    }

    @Test
    fun testInsertAndGet() {
        assertEquals(FossilCrabdbError.NS_NOT_FOUND, crabDB.insert("nonexistent", "key", "value"))
        assertNull(crabDB.get("nonexistent", "key"))

        crabDB.insert("root", "key1", "value1")
        assertEquals("value1", crabDB.get("root", "key1"))

        assertEquals(FossilCrabdbError.KEY_NOT_FOUND, crabDB.insert("root", "key1", "value2"))
    }

    @Test
    fun testUpdate() {
        crabDB.insert("root", "key1", "value1")
        assertEquals(FossilCrabdbError.OK, crabDB.update("root", "key1", "newvalue1"))
        assertEquals("newvalue1", crabDB.get("root", "key1"))

        assertEquals(FossilCrabdbError.KEY_NOT_FOUND, crabDB.update("root", "nonexistent", "value"))
        assertEquals(FossilCrabdbError.NS_NOT_FOUND, crabDB.update("nonexistent", "key", "value"))
    }

    @Test
    fun testDelete() {
        crabDB.insert("root", "key1", "value1")
        assertEquals(FossilCrabdbError.OK, crabDB.delete("root", "key1"))
        assertNull(crabDB.get("root", "key1"))

        assertEquals(FossilCrabdbError.KEY_NOT_FOUND, crabDB.delete("root", "key1"))
        assertEquals(FossilCrabdbError.NS_NOT_FOUND, crabDB.delete("nonexistent", "key1"))
    }
}

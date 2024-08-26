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
#include <fossil/unittest/framework.h> // Includes the Fossil Unit Test Framework
#include <fossil/mockup/framework.h>   // Includes the Fossil Mockup Framework
#include <fossil/xassume.h>            // Includes the Fossil Assume Framework

#include "fossil/crabdb/framework.h"


FOSSIL_FIXTURE(core_crabdb_namespace_fixture);
fossil_crabdb_t *db_namespace = NULL;

FOSSIL_SETUP(core_crabdb_namespace_fixture) {
    db_namespace = fossil_crabdb_create();
}

FOSSIL_TEARDOWN(core_crabdb_namespace_fixture) {
    fossil_crabdb_erase(db_namespace);
}

// * * * * * * * * * * * * * * * * * * * * * * * *
// * Fossil Logic Test Blue CrabDB Database
// * * * * * * * * * * * * * * * * * * * * * * * *

FOSSIL_TEST(test_create_sub_namespace_success) {
    ASSUME_NOT_CNULL(db_namespace);

    fossil_crabdb_create_namespace(db_namespace, "namespace1");

    fossil_crabdb_error_t result = fossil_crabdb_create_sub_namespace(db_namespace, "namespace1", "sub_namespace1");
    ASSUME_ITS_EQUAL_I32(CRABDB_OK, result);

    // Trying to create the same sub-namespace again
    result = fossil_crabdb_create_sub_namespace(db_namespace, "namespace1", "sub_namespace1");
    ASSUME_ITS_EQUAL_I32(CRABDB_ERR_SUB_NS_EXISTS, result);
}

FOSSIL_TEST(test_erase_namespace_success) {
    ASSUME_NOT_CNULL(db_namespace);

    fossil_crabdb_create_namespace(db_namespace, "namespace1");

    fossil_crabdb_error_t result = fossil_crabdb_erase_namespace(db_namespace, "namespace1");
    ASSUME_ITS_EQUAL_I32(CRABDB_OK, result);

    // Trying to erase the same namespace again
    result = fossil_crabdb_erase_namespace(db_namespace, "namespace1");
    ASSUME_ITS_EQUAL_I32(CRABDB_ERR_NS_NOT_FOUND, result);
}

FOSSIL_TEST(test_erase_sub_namespace_success) {
    ASSUME_NOT_CNULL(db_namespace);

    fossil_crabdb_create_namespace(db_namespace, "namespace1");
    fossil_crabdb_create_sub_namespace(db_namespace, "namespace1", "sub_namespace1");

    fossil_crabdb_error_t result = fossil_crabdb_erase_sub_namespace(db_namespace, "namespace1", "sub_namespace1");
    ASSUME_ITS_EQUAL_I32(CRABDB_OK, result);

    // Trying to erase the same sub-namespace again
    result = fossil_crabdb_erase_sub_namespace(db_namespace, "namespace1", "sub_namespace1");
    ASSUME_ITS_EQUAL_I32(CRABDB_ERR_SUB_NS_NOT_FOUND, result);
}

// * * * * * * * * * * * * * * * * * * * * * * * *
// * Fossil Logic Test Pool
// * * * * * * * * * * * * * * * * * * * * * * * *

FOSSIL_TEST_GROUP(c_crabdb_namespace_tests) {    
    ADD_TESTF(test_create_sub_namespace_success, core_crabdb_namespace_fixture);
    ADD_TESTF(test_erase_namespace_success, core_crabdb_namespace_fixture);
    ADD_TESTF(test_erase_sub_namespace_success, core_crabdb_namespace_fixture);
} // end of tests

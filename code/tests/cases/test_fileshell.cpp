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
#include <fossil/pizza/framework.h>

#include "fossil/crabdb/framework.h"

// * * * * * * * * * * * * * * * * * * * * * * * *
// * Fossil Logic Test Utilities
// * * * * * * * * * * * * * * * * * * * * * * * *
// Setup steps for things like test fixtures and
// mock objects are set here.
// * * * * * * * * * * * * * * * * * * * * * * * *

FOSSIL_SUITE(cpp_fileshell_fixture);

FOSSIL_SETUP(cpp_fileshell_fixture) {
    // Setup the test fixture
}

FOSSIL_TEARDOWN(cpp_fileshell_fixture) {
    // Teardown the test fixture
}

// * * * * * * * * * * * * * * * * * * * * * * * *
// * Fossil Logic Test Blue CrabDB Database
// * * * * * * * * * * * * * * * * * * * * * * * *

FOSSIL_TEST(cpp_test_fileshell_write_and_read) {
    std::string file = "cpp_test_file.txt";
    std::string data = "C++ Hello, Blue Crab!";
    std::string out;

    // Write and read
    ASSUME_ITS_TRUE(fossil::bluecrab::FileShell::write(file, data));
    ASSUME_ITS_TRUE(fossil::bluecrab::FileShell::read(file, out));
    ASSUME_ITS_TRUE(out == data);

    fossil::bluecrab::FileShell::remove(file);
}

FOSSIL_TEST(cpp_test_fileshell_overwrite) {
    std::string file = "cpp_test_overwrite.txt";
    std::string data1 = "First";
    std::string data2 = "Second";
    std::string out;

    ASSUME_ITS_TRUE(fossil::bluecrab::FileShell::write(file, data1));
    ASSUME_ITS_TRUE(fossil::bluecrab::FileShell::write(file, data2));
    ASSUME_ITS_TRUE(fossil::bluecrab::FileShell::read(file, out));
    ASSUME_ITS_TRUE(out == data2);

    fossil::bluecrab::FileShell::remove(file);
}

FOSSIL_TEST(cpp_test_fileshell_append_creates_file) {
    std::string file = "cpp_test_append_create.txt";
    std::string data = "Appended!";
    std::string out;

    // Remove if exists
    fossil::bluecrab::FileShell::remove(file);

    ASSUME_ITS_TRUE(fossil::bluecrab::FileShell::append(file, data));
    ASSUME_ITS_TRUE(fossil::bluecrab::FileShell::read(file, out));
    ASSUME_ITS_TRUE(out == data);

    fossil::bluecrab::FileShell::remove(file);
}

FOSSIL_TEST(cpp_test_fileshell_append_to_existing) {
    std::string file = "cpp_test_append_existing.txt";
    std::string data1 = "Line1\n";
    std::string data2 = "Line2";
    std::string out;

    fossil::bluecrab::FileShell::write(file, data1);
    fossil::bluecrab::FileShell::append(file, data2);
    fossil::bluecrab::FileShell::read(file, out);

    ASSUME_ITS_TRUE(out == data1 + data2);

    fossil::bluecrab::FileShell::remove(file);
}

FOSSIL_TEST(cpp_test_fileshell_exists_and_remove) {
    std::string file = "cpp_test_exists.txt";
    std::string data = "Exists?";

    fossil::bluecrab::FileShell::write(file, data);
    ASSUME_ITS_TRUE(fossil::bluecrab::FileShell::exists(file));
    ASSUME_ITS_TRUE(fossil::bluecrab::FileShell::remove(file));
    ASSUME_ITS_FALSE(fossil::bluecrab::FileShell::exists(file));
}

FOSSIL_TEST(cpp_test_fileshell_size) {
    std::string file = "cpp_test_size.txt";
    std::string data = "123456789";
    fossil::bluecrab::FileShell::write(file, data);

    long sz = fossil::bluecrab::FileShell::size(file);
    ASSUME_ITS_TRUE(sz == (long)data.size());

    fossil::bluecrab::FileShell::remove(file);
}

FOSSIL_TEST(cpp_test_fileshell_read_nonexistent) {
    std::string file = "cpp_test_no_file.txt";
    std::string out;
    ASSUME_ITS_FALSE(fossil::bluecrab::FileShell::read(file, out));
}

FOSSIL_TEST(cpp_test_fileshell_remove_nonexistent) {
    std::string file = "cpp_test_remove_no_file.txt";
    ASSUME_ITS_FALSE(fossil::bluecrab::FileShell::remove(file));
}

FOSSIL_TEST(cpp_test_fileshell_list_directory) {
    std::string dir = ".";
    std::vector<std::string> files;
    int count = fossil::bluecrab::FileShell::list(dir, files, 32);

    ASSUME_ITS_TRUE(count >= 0);
    // Optionally check that at least this test file is present
    // bool found = std::find(files.begin(), files.end(), "test_fileshell.cpp") != files.end();
    // ASSUME_ITS_TRUE(found || files.size() > 0);
}

FOSSIL_TEST(cpp_test_fileshell_list_invalid_directory) {
    std::string dir = "no_such_cpp_dir";
    std::vector<std::string> files;
    int count = fossil::bluecrab::FileShell::list(dir, files, 8);
    ASSUME_ITS_TRUE(count == -1);
}

FOSSIL_TEST(cpp_test_fileshell_size_nonexistent) {
    std::string file = "cpp_test_size_no_file.txt";
    long sz = fossil::bluecrab::FileShell::size(file);
    ASSUME_ITS_TRUE(sz == -1);
}

FOSSIL_TEST(cpp_test_fileshell_write_empty_string) {
    std::string file = "cpp_test_empty.txt";
    std::string data = "";
    std::string out;

    ASSUME_ITS_TRUE(fossil::bluecrab::FileShell::write(file, data));
    ASSUME_ITS_TRUE(fossil::bluecrab::FileShell::read(file, out));
    ASSUME_ITS_TRUE(out.empty());

    fossil::bluecrab::FileShell::remove(file);
}

// * * * * * * * * * * * * * * * * * * * * * * * *
// * Fossil Logic Test Pool
// * * * * * * * * * * * * * * * * * * * * * * * *
FOSSIL_TEST_GROUP(cpp_fileshell_database_tests) {
    FOSSIL_TEST_ADD(cpp_fileshell_fixture, cpp_test_fileshell_write_and_read);
    FOSSIL_TEST_ADD(cpp_fileshell_fixture, cpp_test_fileshell_append);
    FOSSIL_TEST_ADD(cpp_fileshell_fixture, cpp_test_fileshell_delete_and_exists);
    FOSSIL_TEST_ADD(cpp_fileshell_fixture, cpp_test_fileshell_size);
    FOSSIL_TEST_ADD(cpp_fileshell_fixture, cpp_test_fileshell_list);
    FOSSIL_TEST_ADD(cpp_fileshell_fixture, cpp_test_fileshell_read_nonexistent_file);
    FOSSIL_TEST_ADD(cpp_fileshell_fixture, cpp_test_fileshell_write_null_args);
    FOSSIL_TEST_ADD(cpp_fileshell_fixture, cpp_test_fileshell_append_null_args);
    FOSSIL_TEST_ADD(cpp_fileshell_fixture, cpp_test_fileshell_list_invalid_dir);

    FOSSIL_TEST_REGISTER(cpp_fileshell_fixture);
} // end of tests

/**
 * generic – common utilities for generic plugin tests
 * Copyright © 2026 John Erling Blad. All Rights Reserved.
 * 
 * Protected as a work of art under the Norwegian Copyright Act (Åndsverksloven).
 *
 * STATUS: PRIVATE WORK / ARTISTIC PROPERTY
 * 1. This code is provided for EXHIBITION AND PEER REVIEW ONLY.
 * 2. No license is granted for execution, linking, or commercial distribution.
 * 3. Derivative works are governed by Åndsverksloven § 6; only independent 
 *    new works of art may be created.
 *
 * WARNING: This work is not intended for functional use in jurisdictions 
 * that do not recognize software as an artistic work of authorship.
 *
 * See the full text at LICENSE.txt
 **/

#pragma once

#include <filesystem>
#include <random>
#include <string>

/**
 * RAII helper to create a unique temporary directory for a test.
 * Ensures isolation and automatic cleanup even if tests fail.
 */
struct ScopedTestDir {
    std::filesystem::path path;
    ScopedTestDir() {
        static std::random_device rd;
        static std::mt19937 gen(rd());
        std::uniform_int_distribution<> dis(0, 15);
        const char* hex = "0123456789abcdef";
        std::string rand_suffix;
        for(int i = 0; i < 8; ++i) rand_suffix += hex[dis(gen)];

        path = std::filesystem::temp_directory_path() / ("hera_test_" + rand_suffix);
        std::filesystem::create_directories(path);
    }
    ~ScopedTestDir() {
        std::error_code ec;
        std::filesystem::remove_all(path, ec);
    }
};

void test_hera_api_version();
void test_parse_list();
void test_hera_about_logic();
void test_hera_about_module_logic();
void test_hera_clone_integrity();
void test_hera_install_logic();
void test_hera_uninstall_logic();
void test_hera_list_logic();
void test_hera_prune_logic();
void test_hera_signal_stop_cont_logic();
void test_hera_down_logic();
void test_hera_signal_logic();
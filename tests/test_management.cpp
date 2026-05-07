/**
 * generic – tests for agent management subcommands
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

#include <iostream>
#include <cassert>
#include <filesystem>
#include "test_generic.hpp"
#include "generic/internal.hpp"

void test_hera_list_logic() {
    std::cout << "Testing hera_list_logic..." << std::endl;
    namespace fs = std::filesystem;
    ScopedTestDir env;
    fs::path lib_dir = env.path / "test_lib";
    fs::create_directories(lib_dir);
    
    // List should work even if the directory is empty
    assert(hera::list(lib_dir.string().c_str(), nullptr, nullptr, nullptr, 0, 0).code == 0);
}

void test_hera_prune_logic() {
    std::cout << "Testing hera_prune_logic..." << std::endl;
    namespace fs = std::filesystem;
    ScopedTestDir env;
    fs::path run_dir = env.path / "test_run";
    fs::create_directories(run_dir);
    // Prune should return success
    assert(hera::prune(nullptr, run_dir.string().c_str(), 0, 0).code == 0);
}

void test_hera_signal_logic() {
    std::cout << "Testing hera_signal_logic..." << std::endl;
    ScopedTestDir env;
    // This will fail without an actual running process, 
    // so we only test that it handles missing PID files correctly.
    assert(hera::signal(env.path.c_str(), env.path.c_str(), "non-existent", 0, HERA_SIGTERM).code == 1);
}

void test_hera_down_logic() {
    std::cout << "Testing hera_down_logic..." << std::endl;
    ScopedTestDir env;
    assert(hera::down(env.path.c_str(), env.path.c_str(), "non-existent", 0, 0).code == 1);
}

void test_hera_signal_stop_cont_logic() {
    std::cout << "Testing hera_signal_stop_cont_logic..." << std::endl;
    ScopedTestDir env;
    // Verifies flag handling
    assert(hera::signal(env.path.c_str(), env.path.c_str(), "none", 0, HERA_SIGSTOP).code == 1);
    assert(hera::signal(env.path.c_str(), env.path.c_str(), "none", 0, HERA_SIGCONT).code == 1);
}
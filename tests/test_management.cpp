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
#include <sstream>
#include <cassert>
#include <filesystem>
#include <fstream>
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

void test_list_json_format() {
    std::cout << "Testing list JSON format output..." << std::endl;
    namespace fs = std::filesystem;
    ScopedTestDir env;
    fs::path lib_dir = env.path / "test_lib";
    fs::create_directories(lib_dir);

    // Create a minimal .beve agent file
    hera::MetaData meta;
    meta.type = "metadata";
    meta.nickname = "TestAgent";
    std::string buf;
    assert(!glz::write_beve(meta, buf));
    std::ofstream ofs(lib_dir / "test-agent.beve", std::ios::binary);
    ofs.write(buf.data(), static_cast<std::streamsize>(buf.size()));
    ofs.close();

    std::ostringstream captured;
    std::streambuf* old_buf = std::cout.rdbuf(captured.rdbuf());
    hera::list(lib_dir.string().c_str(), nullptr, nullptr, nullptr, 0, HERA_FORMAT_JSON);
    std::cout.rdbuf(old_buf);

    glz::generic parsed;
    auto ec = glz::read_json(parsed, captured.str());
    assert(!ec);
}

void test_list_yaml_format() {
    std::cout << "Testing list YAML format output..." << std::endl;
    namespace fs = std::filesystem;
    ScopedTestDir env;
    fs::path lib_dir = env.path / "test_lib";
    fs::create_directories(lib_dir);

    // Create a minimal .beve agent file
    hera::MetaData meta;
    meta.type = "metadata";
    meta.nickname = "TestAgent";
    std::string buf;
    assert(!glz::write_beve(meta, buf));
    std::ofstream ofs(lib_dir / "test-agent.beve", std::ios::binary);
    ofs.write(buf.data(), static_cast<std::streamsize>(buf.size()));
    ofs.close();

    std::ostringstream captured;
    std::streambuf* old_buf = std::cout.rdbuf(captured.rdbuf());
    hera::list(lib_dir.string().c_str(), nullptr, nullptr, nullptr, 0, HERA_FORMAT_YAML);
    std::cout.rdbuf(old_buf);

    // Every non-empty line must start with "- " or "  key: value"
    std::istringstream lines(captured.str());
    std::string line;
    while (std::getline(lines, line)) {
        if (line.empty()) continue;
        assert(line.substr(0, 2) == "- " || line.substr(0, 2) == "  ");
    }
}

void test_hera_prune_logic() {
    std::cout << "Testing hera_prune_logic..." << std::endl;
    namespace fs = std::filesystem;
    ScopedTestDir env;
    fs::path state_dir = env.path / "test_state";
    fs::path run_dir   = env.path / "test_run";
    fs::path cache_dir = env.path / "test_cache";
    fs::create_directories(state_dir);
    fs::create_directories(run_dir);
    fs::create_directories(cache_dir);
    // Empty state_dir → no BEVE anchor → prune returns success immediately
    assert(hera::prune(state_dir.string().c_str(), run_dir.string().c_str(), cache_dir.string().c_str(), nullptr, 0, 0).code == 0);
}

void test_hera_signal_logic() {
    std::cout << "Testing hera_signal_logic..." << std::endl;
    ScopedTestDir env;
    // This will fail without an actual running process, 
    // so we only test that it handles missing PID files correctly.
    assert(hera::signal(env.path.c_str(), env.path.c_str(), "non-existent", 0, HERA_SIGTERM).code == 1);
}


void test_hera_signal_stop_cont_logic() {
    std::cout << "Testing hera_signal_stop_cont_logic..." << std::endl;
    ScopedTestDir env;
    // Verifies flag handling
    assert(hera::signal(env.path.c_str(), env.path.c_str(), "none", 0, HERA_SIGSTOP).code == 1);
    assert(hera::signal(env.path.c_str(), env.path.c_str(), "none", 0, HERA_SIGCONT).code == 1);
}
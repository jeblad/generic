/**
 * generic – tests for the “about” subcommand
 * Copyright © 2026 John Erling Blad. All Rights Reserved.
 * Protected under the Norwegian Copyright Act (Åndsverksloven).
 * See LEGAL_NOTICE.md for terms of use.
 **/

#include <iostream>
#include <sstream>
#include <cassert>
#include <filesystem>
#include <fstream>
#include "test_generic.hpp"
#include "generic/internal.hpp"
#include "hera/utility.hpp"

void test_hera_about_logic() {
    std::cout << "Testing hera_about_logic..." << std::endl;
    // Plugin and system information should return success (0)
    assert(hera::about_plugin(0).code == 0);
    assert(hera::about_system(0).code == 0);
}

void test_hera_about_module_logic() {
    std::cout << "Testing hera_about_module_logic..." << std::endl;
    namespace fs = std::filesystem;
    ScopedTestDir env;
    fs::path test_file = env.path / "about_test.beve";

    // Create a minimal multipart agent file for testing (JSON header only, no BEVE payload)
    hera::MetaData meta;
    meta.type = "metadata";
    meta.description = "Test Description";
    std::string buf;
    if (hera::write_json_part(meta, buf)) {
        std::cerr << "Failed to write test metadata" << std::endl;
    }
    std::ofstream ofs(test_file.string(), std::ios::binary);
    ofs.write(buf.data(), buf.size());
    ofs.close();

    assert(hera::about_module(test_file.string().c_str(), 0).code == 0);
}

void test_about_json_format() {
    std::cout << "Testing about JSON format output..." << std::endl;

    // Capture stdout
    std::ostringstream captured;
    std::streambuf* old_buf = std::cout.rdbuf(captured.rdbuf());
    hera::about_plugin(HERA_FORMAT_JSON | HERA_REPORT_ABOUT);
    hera::about_system(HERA_FORMAT_JSON | HERA_REPORT_ABOUT);
    std::cout.rdbuf(old_buf);

    // Each line must be valid JSON (one object per call)
    std::istringstream lines(captured.str());
    std::string line;
    int count = 0;
    while (std::getline(lines, line)) {
        if (line.empty()) continue;
        glz::generic parsed;
        auto ec = glz::read_json(parsed, line);
        assert(!ec);
        ++count;
    }
    assert(count >= 2); // one object per about_* call
}

void test_about_yaml_format() {
    std::cout << "Testing about YAML format output..." << std::endl;

    std::ostringstream captured;
    std::streambuf* old_buf = std::cout.rdbuf(captured.rdbuf());
    hera::about_plugin(HERA_FORMAT_YAML | HERA_REPORT_ABOUT);
    std::cout.rdbuf(old_buf);

    // Every non-empty line must match "key: value"
    std::istringstream lines(captured.str());
    std::string line;
    while (std::getline(lines, line)) {
        if (line.empty()) continue;
        auto colon = line.find(": ");
        assert(colon != std::string::npos);
        assert(colon > 0); // key must be non-empty
    }
}
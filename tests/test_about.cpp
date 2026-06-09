/**
 * generic – tests for the “about” subcommand
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
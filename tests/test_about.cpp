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
#include <cassert>
#include <filesystem>
#include <fstream>
#include "test_generic.hpp"
#include "generic/internal.hpp"

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
    
    // Create a minimal file for testing
    hera::MetaData meta;
    meta.type = "metadata";
    meta.description = "Test Description";
    std::string buf;
    if (glz::write_beve(meta, buf)) {
        std::cerr << "Failed to write test BEVE" << std::endl;
    }
    std::ofstream ofs(test_file.string(), std::ios::binary);
    ofs.write(buf.data(), buf.size());
    ofs.close();

    assert(hera::about_module(test_file.string().c_str(), 0).code == 0);
}
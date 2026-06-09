/**
 * generic – tests for the “clone” subcommand
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
#include <fstream>
#include <filesystem>
#include "test_generic.hpp"
#include "generic/internal.hpp"
#include "hera/utility.hpp"

void test_hera_clone_integrity() {
    std::cout << "Testing hera_clone_integrity..." << std::endl;
    namespace fs = std::filesystem;
    ScopedTestDir env;
    fs::path in_fn = env.path / "clone_in.beve";
    fs::path out_fn = env.path / "clone_out.beve";

    // Create source file
    hera::MetaData meta;
    meta.type = "metadata";
    meta.uuid = "original-uuid";
    meta.callsign = "Original";
    
    std::string buf;
    if (hera::write_json_part(meta, buf)) {
        std::cerr << "Failed to write test metadata" << std::endl;
    }
    std::ofstream ofs(in_fn.string(), std::ios::binary);
    ofs.write(buf.data(), buf.size());
    ofs.close();

    // Perform cloning
    auto res = hera::clone(in_fn.string().c_str(), out_fn.string().c_str(), "new-uuid", "Cloned", 0);
    assert(res.code == 0);
    assert(fs::exists(out_fn));

    // Verify that the output file can be read
    hera::AgentHeader new_doc;
    std::string scratch;
    assert(!hera::read_file(new_doc, out_fn.string(), scratch));
    assert(new_doc.meta.uuid == "new-uuid");
}
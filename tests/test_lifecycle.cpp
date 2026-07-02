/**
 * generic – tests for agent lifecycle subcommands
 * Copyright © 2026 John Erling Blad. All Rights Reserved.
 * Protected under the Norwegian Copyright Act (Åndsverksloven).
 * See LEGAL_NOTICE.md for terms of use.
 **/

#include <iostream>
#include <cassert>
#include <filesystem>
#include <fstream>
#include <random>
#include "test_generic.hpp"
#include "generic/internal.hpp"
#include "hera/utility.hpp"

namespace fs = std::filesystem;

void test_hera_install_logic() {
    std::cout << "Testing hera_install_logic..." << std::endl;
    ScopedTestDir env;
    fs::path install_dir = env.path / "install_dir";
    fs::create_directories(install_dir);
    fs::path in_fn = env.path / "to_be_installed.beve";

    hera::MetaData meta;
    meta.type = "metadata";
    meta.uuid = "install-uuid";
    std::string buf;
    if (hera::write_json_part(meta, buf)) {
        std::cerr << "Failed to write test metadata" << std::endl;
    }
    std::ofstream ofs(in_fn, std::ios::binary);
    ofs.write(buf.data(), buf.size());
    ofs.close();

    // Install
    auto res = hera::install(in_fn.string().c_str(), install_dir.string().c_str(), nullptr, "InstallTest", 0, 0);
    assert(res.code == 0);
    assert(fs::exists(install_dir / "install-uuid.beve"));
}

void test_hera_uninstall_logic() {
    std::cout << "Testing hera_uninstall_logic..." << std::endl;
    ScopedTestDir env;
    fs::path install_dir = env.path / "install_dir";
    fs::path target_dir = env.path / "target_dir";
    fs::create_directories(install_dir);
    fs::create_directories(target_dir);

    // Prepare an "already installed" file for the test
    fs::path installed_file = install_dir / "install-uuid.beve";
    hera::MetaData meta;
    meta.type = "metadata";
    meta.uuid = "install-uuid";
    std::string buf;
    if (hera::write_json_part(meta, buf)) {
        std::cerr << "Failed to write test metadata" << std::endl;
    }
    std::ofstream ofs(installed_file, std::ios::binary);
    ofs.write(buf.data(), buf.size());
    ofs.close();
    
    // Uninstall (move back and delete source)
    auto res = hera::uninstall(installed_file.string().c_str(), target_dir.string().c_str(), nullptr, nullptr, 0, HERA_REMOVE);
    assert(res.code == 0);
    assert(!fs::exists(installed_file));
    assert(fs::exists(target_dir / "install-uuid.beve"));
}
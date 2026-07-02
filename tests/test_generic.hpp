/**
 * generic – common utilities for generic plugin tests
 * Copyright © 2026 John Erling Blad. All Rights Reserved.
 * Protected under the Norwegian Copyright Act (Åndsverksloven).
 * See LEGAL_NOTICE.md for terms of use.
 **/

#pragma once

#include <filesystem>
#include <random>
#include <string>
#include "hera/testing/utility.hpp"

// --- Plugin Entry Points (C-API) ---
extern "C" {
    uint32_t hera_api();

    hera::Result hera_clone(
        const char * in_fn,
        const char * out_fn,
        const char * uuid_str,
        const char * callsign_str,
        int log_level,
        int flags
    );

    hera::Result hera_build(
        const char * in_fn,
        const char * out_fn,
        const char * uuid_str,
        const char * callsign_str,
        int log_level,
        int flags
    );

    hera::Result hera_install(
        const char * in_fn,
        const char * out_dir,
        const char * uuid_str,
        const char * callsign_str,
        int log_level,
        int flags
    );

    hera::Result hera_uninstall(
        const char * in_fn,
        const char * out_dir,
        const char * uuid_str,
        const char * callsign_str,
        int log_level,
        int flags
    );

    hera::Result hera_list(
        const char * in_dir,
        const char * run_dir,
        const char * id_filter,
        const char * fields,
        int log_level,
        int flags
    );

    hera::Result hera_about_plugin(
        const char * in_fn,
        const char * out_fn,
        const char * uuid_str,
        const char * callsign_str,
        int log_level,
        int flags
    );

    hera::Result hera_about_module(
        const char * in_fn,
        const char * out_fn,
        const char * uuid_str,
        const char * callsign_str,
        int log_level,
        int flags
    );
}

/**
 * Alias the central ScopedTestDir from hera
 */
using ScopedTestDir = hera::testing::ScopedTestDir;

void test_hera_api_version();
void test_hera_about_logic();
void test_hera_about_module_logic();
void test_about_json_format();
void test_about_yaml_format();
void test_hera_clone_integrity();
void test_hera_install_logic();
void test_hera_uninstall_logic();
void test_hera_list_logic();
void test_list_json_format();
void test_list_yaml_format();
void test_hera_prune_logic();
void test_hera_signal_stop_cont_logic();
void test_hera_down_logic();
void test_hera_signal_logic();
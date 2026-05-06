/**
 * generic – main entry point for generic plugin tests
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
#include "test_generic.hpp"

int main() {
    test_hera_api_version();
    test_parse_list();
    test_hera_about_logic();
    test_hera_about_module_logic();
    test_hera_clone_integrity();
    test_hera_install_logic();
    test_hera_uninstall_logic();
    test_hera_list_logic();
    test_hera_prune_logic();
    test_hera_signal_stop_cont_logic();
    test_hera_down_logic();
    test_hera_signal_logic();

    std::cout << "All generic API tests passed!" << std::endl;
    return 0;
}
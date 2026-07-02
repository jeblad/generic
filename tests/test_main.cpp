/**
 * generic – main entry point for generic plugin tests
 * Copyright © 2026 John Erling Blad. All Rights Reserved.
 * Protected under the Norwegian Copyright Act (Åndsverksloven).
 * See LEGAL_NOTICE.md for terms of use.
 **/

#include <iostream>
#include "test_generic.hpp"

int main() {
    test_hera_api_version();
    test_hera_about_logic();
    test_hera_about_module_logic();
    test_about_json_format();
    test_about_yaml_format();
    test_hera_clone_integrity();
    test_hera_install_logic();
    test_hera_uninstall_logic();
    test_hera_list_logic();
    test_list_json_format();
    test_list_yaml_format();
    test_hera_prune_logic();
    test_hera_signal_stop_cont_logic();
    test_hera_signal_logic();
    test_hera_down_logic();

    std::cout << "All generic API tests passed!" << std::endl;
    return 0;
}
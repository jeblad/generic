/**
 * generic – tests for utility functions
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
#include "hera/config.h"
#include "hera/utility.hpp"
#include "test_generic.hpp"

extern "C" {
    uint32_t hera_api();
}

void test_hera_api_version() {
    std::cout << "Checking hera_api() version..." << std::endl;
    assert(hera_api() == HERA_API_LEVEL);
}

void test_parse_list() {
    std::cout << "Testing parse_list helper..." << std::endl;
    std::string_view raw = "  Alpha ; Beta;Gamma  ; Delta \n ; ";
    std::array<std::string_view, 10> out;
    size_t count = hera::parse_list(raw, out.data(), out.size());
    assert(count == 4);
    assert(out[0] == "Alpha");
    assert(out[1] == "Beta");
    assert(out[2] == "Gamma");
    assert(out[3] == "Delta");
}
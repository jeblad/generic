/**
 * generic – tests for utility functions
 * Copyright © 2026 John Erling Blad. All Rights Reserved.
 * Protected under the Norwegian Copyright Act (Åndsverksloven).
 * See LEGAL_NOTICE.md for terms of use.
 **/

#include <iostream>
#include <cassert>
#include "hera/config.h"
#include "hera/utility.hpp"
#include "test_generic.hpp"

void test_hera_api_version() {
    std::cout << "Checking hera_api() version..." << std::endl;
    assert(hera_api() == HERA_API_LEVEL);
}
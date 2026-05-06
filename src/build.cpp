/**
 * generic – the “build” subcommand implementation
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

#include "generic/internal.hpp"
#include <rlog/rlog.hpp>

namespace hera {

/**
 * @brief Generic build implementation (not supported).
 */
Result build(
    const char * in_fn,
    const char * out_fn,
    const char * uuid_str,
    const char * callsign_str,
    int flags
) {
    ERROR_("Generic plugin does not support 'build' operations");
    return Result(2);
}

} // namespace hera
/**
 * generic – the “build” subcommand implementation
 * Copyright © 2026 John Erling Blad. All Rights Reserved.
 * Protected under the Norwegian Copyright Act (Åndsverksloven).
 * See LEGAL_NOTICE.md for terms of use.
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
    const char * nickname_str,
    int flags
) {
    ERROR_("Generic plugin does not support 'build' operations");
    return Result(2);
}

} // namespace hera
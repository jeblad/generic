/**
 * generic – the "prune" subcommand implementation
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

#include <csignal>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <string>
#include "generic/internal.hpp"
#include "hera/utility.hpp"
#include <rlog/rlog.hpp>

namespace hera {

static std::string file_status(const std::string& path) {
    if (path.empty())         return _("not set");
    if (std::filesystem::exists(path)) return _("found");
    return _("missing");
}

Result prune(
    const char * run_dir_str,
    const char * id_filter,
    int /*log_level*/,
    int flags
) {
    namespace fs = std::filesystem;
    fs::path run_dir(run_dir_str);
    if (!fs::exists(run_dir)) return Result(0);

    std::string filter = id_filter ? id_filter : "";
    bool found_any = false;

    for (const auto& entry : fs::directory_iterator(run_dir)) {
        if (entry.path().extension() != ".pid") continue;

        std::string stem = entry.path().stem().string();
        if (!filter.empty() && stem.find(filter) == std::string::npos) continue;

        std::ifstream ifs(entry.path());
        PidFileContent info;
        std::string buf((std::istreambuf_iterator<char>(ifs)), std::istreambuf_iterator<char>());
        if (glz::read<glz::opts{.error_on_unknown_keys = false}>(info, buf)) continue;

        // Only report/act on daemons whose process is dead.
        if (kill(info.pid, 0) == 0 || errno != ESRCH) continue;

        // Exactly one match required when id_filter is set — enforce below.
        if (!filter.empty() && found_any) {
            ERROR_FMT_(_("Identifier '{}' matched more than one entry."), filter);
            return Result(1);
        }
        found_any = true;

        bool mmio_exists = !info.mmio_path.empty() && fs::exists(info.mmio_path);

        std::cout << stem << "\n"
                  << "  " << _("PID file:  ") << entry.path().string() << "  [" << _("found")                      << "]\n"
                  << "  " << _("BEVE file: ") << info.beve_path         << "  [" << file_status(info.beve_path) << "]\n"
                  << "  " << _("MMIO file: ") << info.mmio_path         << "  [" << file_status(info.mmio_path) << "]\n";

        if (mmio_exists)
            std::cout << "  " << _("hint: MMIO state is available — run 'hera rebuild ") << stem << _("' to recover.\n");

        if (flags & HERA_FORCE) {
            std::error_code ec;
            fs::remove(entry.path(), ec);
            if (ec) WARNING_FMT_(_("Failed to remove PID file {}: {}"), entry.path().string(), ec.message());
            else    NOTICE_FMT_(_("Removed stale PID file for agent {}."), stem);
        }
    }

    return Result(0);
}

} // namespace hera

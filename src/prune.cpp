/**
 * generic – the “prune” subcommand implementation
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

#include <filesystem>
#include <fstream>
#include <csignal>
#include "generic/internal.hpp"
#include "hera/utility.hpp"
#include <rlog/rlog.hpp>

namespace hera {

Result prune(
    const char * in_dir,
    const char * run_dir_str,
    int log_level,
    int flags
) {
    namespace fs = std::filesystem;
    fs::path run_dir(run_dir_str);
    for (const auto& entry : fs::directory_iterator(run_dir)) {
        if (entry.path().extension() == ".pid") {
            std::ifstream ifs(entry.path());
            PidFileContent info;
            std::string buf((std::istreambuf_iterator<char>(ifs)), std::istreambuf_iterator<char>());
            if (!glz::read_json(info, buf)) {
                if (kill(info.pid, 0) != 0 && errno == ESRCH) {
                    fs::remove(entry.path());
                    WARNING_FMT_(_("Agent {} ({}) was found without a running process."), entry.path().stem().string(), info.filename);
                }
            }
        }
    }
    return Result(0);
}

} // namespace hera
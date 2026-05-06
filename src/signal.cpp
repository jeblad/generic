/**
 * generic – the “signal” and “down” subcommand implementation
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
#include <thread>
#include "generic/internal.hpp"
#include "hera/utility.hpp"
#include <rlog/rlog.hpp>

namespace hera {

Result signal(
    const char * in_dir,
    const char * run_dir_str,
    const char * id_filter,
    int log_level,
    int flags
) {
    namespace fs = std::filesystem;
    fs::path run_dir(run_dir_str);
    
    int sig = (flags & HERA_SIGKILL) ? SIGKILL : (flags & HERA_SIGTERM) ? SIGTERM : 
              (flags & HERA_SIGHUP) ? SIGHUP : (flags & HERA_SIGSTOP) ? SIGSTOP :
              (flags & HERA_SIGCONT) ? SIGCONT : 0;

    if (sig == 0) return Result(1);

    auto pid_path_opt = find_pid_file(run_dir, id_filter);
    if (!pid_path_opt) return Result(1);

    std::ifstream ifs(*pid_path_opt);
    PidFileContent info;
    std::string buf((std::istreambuf_iterator<char>(ifs)), std::istreambuf_iterator<char>());
    if (glz::read_json(info, buf)) return Result(1);

    if (kill(info.pid, sig) != 0) return Result(1);

    if (sig == SIGTERM || sig == SIGKILL) {
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
        if (kill(info.pid, 0) != 0 && errno == ESRCH) {
            fs::remove(*pid_path_opt);
            NOTICE_FMT_("Agent {} terminated.", id_filter);
        }
    }
    return Result(0);
}

Result down(
    const char * in_dir,
    const char * run_dir,
    const char * id_filter,
    int log_level,
    int flags
) {
    // 'down' serves as a high-level alias for sending SIGTERM
    return signal(in_dir, run_dir, id_filter, log_level, flags | HERA_SIGTERM);
}

} // namespace hera
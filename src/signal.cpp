/**
 * generic – the “signal” and “down” subcommand implementation
 * Copyright © 2026 John Erling Blad. All Rights Reserved.
 * Protected under the Norwegian Copyright Act (Åndsverksloven).
 * See LEGAL_NOTICE.md for terms of use.
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
    
    // Semantic signal → OS signal mapping.
    // HUP  uses SIGHUP for checkpoint (save BEVE, continue running).
    // STOP/CONT use SIGUSR1/SIGUSR2 so the agent can handle them gracefully.
    // KILL uses SIGABRT rather than SIGKILL to allow cleanup/core dump.
    int sig = 0;
    if      (flags & HERA_SIGSTOP)  sig = SIGUSR1;
    else if (flags & HERA_SIGCONT)  sig = SIGUSR2;
    else if (flags & HERA_SIGINT)   sig = SIGQUIT;
    else if (flags & HERA_SIGTERM)  sig = SIGTERM;
    else if (flags & HERA_SIGKILL)  sig = SIGABRT;
    else if (flags & HERA_SIGHUP)   sig = SIGHUP;

    if (sig == 0) return Result(1);

    auto pid_path_opt = find_pid_file(run_dir, id_filter);
    if (!pid_path_opt) return Result(1);

    std::ifstream ifs(*pid_path_opt);
    PidFileContent info;
    std::string buf((std::istreambuf_iterator<char>(ifs)), std::istreambuf_iterator<char>());
    if (glz::read<glz::opts{.error_on_unknown_keys = false}>(info, buf)) return Result(1);

    if (kill(info.pid, sig) != 0) return Result(1);

    return Result(0);
}

} // namespace hera
/**
 * generic – the "down" subcommand implementation
 * Copyright © 2026 John Erling Blad. All Rights Reserved.
 * Protected under the Norwegian Copyright Act (Åndsverksloven).
 * See LEGAL_NOTICE.md for terms of use.
 **/

#include <cerrno>
#include <chrono>
#include <csignal>
#include <cstring>
#include <filesystem>
#include <fstream>
#include <thread>

#include "generic/internal.hpp"
#include "hera/utility.hpp"
#include <rlog/rlog.hpp>

namespace hera {

Result down(
    const char * /*in_dir*/,
    const char * run_dir_str,
    const char * id_filter,
    int /*log_level*/,
    int flags
) {
    namespace fs = std::filesystem;
    fs::path run_dir(run_dir_str ? run_dir_str : "");

    auto pid_path_opt = hera::find_pid_file(run_dir, id_filter ? id_filter : "");
    if (!pid_path_opt) {
        ERROR_FMT_("No PID file found for agent: {}", id_filter ? id_filter : "<any>");
        return {1};
    }

    std::ifstream ifs(*pid_path_opt);
    std::string buf((std::istreambuf_iterator<char>(ifs)),
                     std::istreambuf_iterator<char>());
    hera::PidFileContent info;
    if (glz::read_json(info, buf)) {
        ERROR_("Failed to read PID file.")
            .hint(_("The PID file may be corrupt or from an older version — use `hera prune` to clean up stale files."));
        return {1};
    }

    if (::kill(info.pid, SIGTERM) != 0) {
        ERROR_FMT_("Failed to send SIGTERM to PID {}: {}", info.pid, std::strerror(errno));
        return {1};
    }

    constexpr int MAX_WAIT_MS = 5000;
    constexpr int POLL_MS     = 100;
    bool stopped = false;
    for (int elapsed = 0; elapsed < MAX_WAIT_MS; elapsed += POLL_MS) {
        std::this_thread::sleep_for(std::chrono::milliseconds(POLL_MS));
        if (::kill(info.pid, 0) != 0 && errno == ESRCH) {
            stopped = true;
            break;
        }
    }

    if (!stopped) {
        if (flags & HERA_FORCE) {
            WARNING_FMT_("Agent {} did not stop in time — sending SIGKILL.",
                         id_filter ? id_filter : "");
            ::kill(info.pid, SIGKILL);
            std::this_thread::sleep_for(std::chrono::milliseconds(POLL_MS));
        } else {
            WARNING_FMT_("Agent {} did not stop within 5 s; use --force to send SIGKILL.",
                         id_filter ? id_filter : "");
            return {1};
        }
    }

    std::error_code ec;
    fs::remove(*pid_path_opt, ec);
    NOTICE_FMT_("Agent {} stopped.", id_filter ? id_filter : "");
    return {0};
}

} // namespace hera

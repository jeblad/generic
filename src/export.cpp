/**
 * generic – the "export" subcommand implementation
 * Copyright © 2026 John Erling Blad. All Rights Reserved.
 * Protected under the Norwegian Copyright Act (Åndsverksloven).
 * See LEGAL_NOTICE.md for terms of use.
 **/

#include <fcntl.h>
#include <unistd.h>
#include <filesystem>

#include "generic/internal.hpp"
#include <rlog/rlog.hpp>

namespace hera {

Result agent_export(
    const char* workspace_dir,
    const char* to_dir,
    const char* /*uuid_str*/,
    const char* /*nickname_str*/,
    int /*log_level*/,
    int flags
) {
    namespace fs = std::filesystem;

    fs::path from_state = fs::path(workspace_dir) / "state";
    fs::path to_state   = fs::path(to_dir) / "state";

    if (!fs::exists(from_state)) {
        ERROR_FMT_("Workspace state directory not found: {}", from_state.string());
        return Result(1);
    }

    int copied = 0;
    for (const auto& entry : fs::directory_iterator(from_state)) {
        if (!entry.is_regular_file()) continue;
        if (entry.path().extension() != ".beve") continue;

        fs::path dst = to_state / entry.path().filename();
        if (!(flags & HERA_FORCE) && fs::exists(dst)) {
            NOTICE_FMT_("Skipping existing: {}", entry.path().filename().string());
            continue;
        }

        // Manual copy: avoids fchmod which is not in the seccomp whitelist.
        int in_fd = ::open(entry.path().c_str(), O_RDONLY);
        if (in_fd < 0) {
            ERROR_FMT_("Cannot open source: {}", entry.path().string());
            return Result(1);
        }
        int out_fd = ::open(dst.c_str(), O_WRONLY | O_CREAT | O_TRUNC, 0644);
        if (out_fd < 0) {
            ::close(in_fd);
            ERROR_FMT_("Cannot create destination: {}", dst.string());
            return Result(1);
        }
        char buf[65536];
        ssize_t n;
        bool ok = true;
        while ((n = ::read(in_fd, buf, sizeof(buf))) > 0) {
            ssize_t w = ::write(out_fd, buf, static_cast<size_t>(n));
            if (w < 0) { ok = false; break; }
        }
        ::close(in_fd);
        ::close(out_fd);
        if (!ok) {
            ERROR_FMT_("Write error for: {}", dst.string());
            return Result(1);
        }
        NOTICE_FMT_("Exported: {}", entry.path().filename().string());
        ++copied;
    }

    if (copied == 0)
        NOTICE_("No BEVE files found to export.");
    else
        INFO_FMT_("Exported {} agent file(s) to {}.", copied, to_state.string());

    return Result(0);
}

} // namespace hera

/**
 * generic – the "init" subcommand implementation
 * Copyright © 2026 John Erling Blad. All Rights Reserved.
 * Protected under the Norwegian Copyright Act (Åndsverksloven).
 * See LEGAL_NOTICE.md for terms of use.
 **/

#include <fcntl.h>
#include <unistd.h>
#include <filesystem>
#include <string>

#include "generic/internal.hpp"
#include <rlog/rlog.hpp>

namespace hera {

namespace {

// Manual copy: avoids fchmod which is not in the seccomp whitelist.
bool copy_raw(const std::filesystem::path& src, const std::filesystem::path& dst) {
    int in_fd = ::open(src.c_str(), O_RDONLY);
    if (in_fd < 0) return false;
    int out_fd = ::open(dst.c_str(), O_WRONLY | O_CREAT | O_TRUNC, 0755);
    if (out_fd < 0) { ::close(in_fd); return false; }
    char buf[65536];
    ssize_t n;
    bool ok = true;
    while ((n = ::read(in_fd, buf, sizeof(buf))) > 0) {
        if (::write(out_fd, buf, static_cast<size_t>(n)) < 0) { ok = false; break; }
    }
    ::close(in_fd);
    ::close(out_fd);
    return ok;
}

} // namespace

Result agent_init(
    const char* model_lib,
    const char* workspace_dir,
    const char* /*model_name*/,
    const char* /*nickname_str*/,
    int /*log_level*/,
    int /*flags*/
) {
    namespace fs = std::filesystem;

    fs::path lib_src  = fs::path(model_lib);
    fs::path cwd      = fs::path(workspace_dir);
    std::string model = lib_src.stem().string();
    fs::path generic_src = lib_src.parent_path() / "generic.so";

    // Create standard workspace directories.
    for (const char* d : {"state", "lib", "cache", "share", "config"})
        fs::create_directories(cwd / d);

    // Copy plugins — remove any existing symlink first so we write a real file.
    for (const auto& [src, name] : {
            std::pair{lib_src,    model + ".so"},
            std::pair{generic_src, std::string("generic.so")}
    }) {
        fs::path dst = cwd / "lib" / name;
        if (fs::is_symlink(dst) || fs::exists(dst))
            fs::remove(dst);
        if (!copy_raw(src, dst)) {
            ERROR_FMT_("Failed to copy {} to {}", src.string(), dst.string());
            return Result(1);
        }
        NOTICE_FMT_("Installed: lib/{}", name);
    }

    // Write hera.toml if absent.
    fs::path toml = cwd / "hera.toml";
    if (!fs::exists(toml)) {
        int fd = ::open(toml.c_str(), O_WRONLY | O_CREAT | O_EXCL, 0644);
        if (fd >= 0) {
            std::string content = "model = " + model + "\n";
            ssize_t w = ::write(fd, content.c_str(), content.size());
            ::close(fd);
            if (w < 0) {
                ERROR_("Failed to write hera.toml");
                return Result(1);
            }
        }
    }

    NOTICE_FMT_("Workspace initialised in {}", cwd.string());
    return Result(0);
}

} // namespace hera

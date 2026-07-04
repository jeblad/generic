/**
 * generic – the "prune", "clean", and "destroy" subcommand implementation
 * Copyright © 2026 John Erling Blad. All Rights Reserved.
 * Protected under the Norwegian Copyright Act (Åndsverksloven).
 * See LEGAL_NOTICE.md for terms of use.
 **/

#include <csignal>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <string>
#include <vector>
#include "generic/internal.hpp"
#include "hera/utility.hpp"
#include "uuid_ext/uuid_ext.hpp"
#include <rlog/rlog.hpp>

namespace hera {

namespace {

struct AgentFiles {
    std::string stem;             // beve stem (UUID with dashes)
    std::filesystem::path beve;
    std::filesystem::path pid;    // empty if not found
    std::filesystem::path mmio;   // empty if not found
    bool running{false};
};

static std::string file_status(const std::filesystem::path& p) {
    return p.empty() ? _("missing") : (std::filesystem::exists(p) ? _("found") : _("missing"));
}

static void try_remove(const std::filesystem::path& p, bool force) {
    if (p.empty() || !std::filesystem::exists(p)) return;
    if (!force) {
        std::cout << "    " << _("would remove: ") << p.string() << "\n";
        return;
    }
    std::error_code ec;
    std::filesystem::remove(p, ec);
    if (ec) WARNING_FMT_("Failed to remove {}: {}", p.string(), ec.message());
    else    std::cout << "    " << _("removed: ") << p.string() << "\n";
}

} // anonymous namespace

Result prune(
    const char* state_dir_str,
    const char* run_dir_str,
    const char* cache_dir_str,
    const char* id_filter,
    int /*log_level*/,
    int flags
) {
    namespace fs = std::filesystem;

    fs::path state_dir(state_dir_str ? state_dir_str : "");
    fs::path run_dir(run_dir_str   ? run_dir_str   : "");
    fs::path cache_dir(cache_dir_str ? cache_dir_str : "");
    std::string filter = id_filter ? id_filter : "";

    bool remove_mmio = (flags & HERA_REMOVE_MMIO) != 0;
    bool remove_beve = (flags & HERA_REMOVE_BEVE) != 0;
    bool force       = (flags & HERA_FORCE) != 0;

    if (state_dir.empty() || !fs::exists(state_dir)) return Result(0);

    // Collect candidate agents from state_dir (BEVE files are the anchor).
    std::vector<AgentFiles> candidates;

    for (const auto& entry : fs::directory_iterator(state_dir)) {
        if (entry.path().extension() != ".beve") continue;

        std::string stem = entry.path().stem().string();
        if (!filter.empty() && stem.find(filter) == std::string::npos) continue;

        AgentFiles af;
        af.stem = stem;
        af.beve = entry.path();

        // Derive base-36 UUID stem for PID and MMIO filenames.
        // The BEVE stem is the UUID in standard format (with dashes).
        std::string base36_stem = stem;
        auto uuid = uuid_ext::UUID::parse(stem);
        if (uuid.get() != 0) base36_stem = uuid.to_base_string(36);

        // Look up PID file.
        if (!run_dir.empty() && fs::exists(run_dir)) {
            auto pid_opt = find_pid_file(run_dir, stem);
            if (pid_opt) af.pid = *pid_opt;
        }

        // Look up MMIO file.
        if (!cache_dir.empty() && fs::exists(cache_dir)) {
            fs::path mmio_candidate = cache_dir / (base36_stem + ".mmio");
            if (fs::exists(mmio_candidate)) af.mmio = mmio_candidate;
        }

        // Check liveness.
        af.running = !af.pid.empty() && is_agent_running(run_dir, stem);

        if (af.running) continue;

        candidates.push_back(std::move(af));
    }

    if (candidates.empty()) return Result(0);

    // If a specific id was requested and matched a running daemon, that is an error.
    if (!filter.empty()) {
        for (const auto& af : candidates) {
            if (af.running) {
                ERROR_FMT_("Agent {} is still running. Stop it before cleaning up.", af.stem);
                return Result(1);
            }
        }
    }

    // Report and optionally remove.
    for (const auto& af : candidates) {
        std::cout << af.stem << "\n"
                  << "  " << _("BEVE file: ") << af.beve.string()  << "  [" << file_status(af.beve) << "]\n"
                  << "  " << _("MMIO file: ") << af.mmio.string()  << "  [" << file_status(af.mmio) << "]\n"
                  << "  " << _("PID file:  ") << af.pid.string()   << "  [" << file_status(af.pid)  << "]\n";

        if (!af.mmio.empty() && !remove_mmio && !remove_beve)
            USER_FMT_("MMIO state is available — run `hera rebuild {}` to recover.", af.stem);

        if (!force) USER_("Use --force to remove — agent state will be permanently deleted.");

        if (remove_beve) try_remove(af.beve, force);
        if (remove_mmio) try_remove(af.mmio, force);
        try_remove(af.pid, force);
    }

    return Result(0);
}

} // namespace hera

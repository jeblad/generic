/**
 * generic – the “list” subcommand implementation
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
#include <iomanip>
#include <iostream>
#include "generic/internal.hpp"
#include <rlog/rlog.hpp>
#include "hera/utility.hpp"
#include "sanitize/sanitize.hpp"

namespace hera {

struct ListEntry {
    std::string id;
    std::string pid;
    MetaData meta;

    bool operator<(const ListEntry& other) const {
        return id < other.id;
    }
};

Result list(
    const char * in_dir,
    const char * run_dir_str,
    const char * id_filter_str,
    const char * fields_str,
    int log_level,
    int flags
) {
    namespace fs = std::filesystem;
    if (!in_dir || !fs::exists(in_dir)) return Result(0);

    fs::path lib_dir(in_dir);
    fs::path run_dir(run_dir_str ? run_dir_str : "");
    std::string id_filter = id_filter_str ? id_filter_str : "";

    std::map<std::string, std::string> header_map = {
        {"id", _("ID")}, {"pid", _("PID")}, {"uuid", _("UUID")},
        {"callsign", _("Callsign")}, {"version", _("Version")},
        {"model", _("Model")}, {"epoch", _("Epoch")},
        {"license", _("License")}, {"provenance", _("Provenance")}
    };

    std::vector<ListEntry> entries;

    // Simple column handling
    std::cout << std::left << std::setw(20) << header_map["id"] << " " 
              << std::setw(20) << header_map["callsign"] << " "
              << std::setw(10) << header_map["pid"] << std::endl;
    std::cout << std::string(55, '-') << std::endl;

    for (const auto& entry : fs::directory_iterator(lib_dir)) {
        if (entry.path().extension() == ".beve") {
            std::string stem = entry.path().stem().string();
            if (!id_filter.empty() && stem.find(id_filter) == std::string::npos) continue;

            hera::AgentHeader header;
            std::string scratch;
            if (read_file(header, entry.path().string().c_str(), scratch)) continue;

            std::string pid_str = "";
            if (!run_dir.empty()) {
                fs::path pid_path = run_dir / (stem + ".pid");
                if (fs::exists(pid_path)) {
                    std::ifstream ifs_pid(pid_path);
                    PidFileContent info;
                    std::string pbuf((std::istreambuf_iterator<char>(ifs_pid)), std::istreambuf_iterator<char>());
                    if (!glz::read_json(info, pbuf)) pid_str = std::to_string(info.pid);
                }
            }

            entries.push_back(ListEntry{stem, pid_str, std::move(header.meta)});
        }
    }

    std::sort(entries.begin(), entries.end());

    for (const auto& e : entries) {
        std::cout << std::left << std::setw(20) << e.id << " "
                  << std::setw(20) << sanitize::untaint(e.meta.callsign.value_or(_("no callsign")), false) << " "
                  << std::setw(10) << e.pid << std::endl;
    }

    return Result(0);
}

} // namespace hera
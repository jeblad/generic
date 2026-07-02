/**
 * generic – the "list" subcommand implementation
 * Copyright © 2026 John Erling Blad. All Rights Reserved.
 * Protected under the Norwegian Copyright Act (Åndsverksloven).
 * See LEGAL_NOTICE.md for terms of use.
 **/

#include <filesystem>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <map>
#include <vector>
#include <string>
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

static void emit_text(const std::vector<ListEntry>& entries) {
    std::cout << std::left
              << std::setw(20) << _("ID") << " "
              << std::setw(20) << _("Nickname") << " "
              << std::setw(10) << _("PID") << "\n"
              << std::string(52, '-') << "\n";
    for (const auto& e : entries) {
        std::cout << std::left
                  << std::setw(20) << e.id << " "
                  << std::setw(20) << sanitize::untaint(e.meta.nickname.value_or(_("no nickname")), false) << " "
                  << std::setw(10) << e.pid << "\n";
    }
}

static void emit_json(const std::vector<ListEntry>& entries) {
    std::vector<std::map<std::string, std::string>> arr;
    for (const auto& e : entries) {
        std::map<std::string, std::string> obj;
        obj["id"] = e.id;
        obj["nickname"] = e.meta.nickname.value_or("");
        obj["pid"] = e.pid;
        if (e.meta.uuid)    obj["uuid"]    = *e.meta.uuid;
        if (e.meta.version) obj["version"] = *e.meta.version;
        if (e.meta.model)   obj["model"]   = *e.meta.model;
        arr.push_back(std::move(obj));
    }
    std::string out;
    if (glz::write_json(arr, out)) return;
    std::cout << out << "\n";
}

static void emit_yaml(const std::vector<ListEntry>& entries) {
    for (const auto& e : entries) {
        std::cout << "- id: " << e.id << "\n"
                  << "  nickname: " << e.meta.nickname.value_or("") << "\n"
                  << "  pid: " << e.pid << "\n";
        if (e.meta.uuid)    std::cout << "  uuid: "    << *e.meta.uuid    << "\n";
        if (e.meta.version) std::cout << "  version: " << *e.meta.version << "\n";
        if (e.meta.model)   std::cout << "  model: "   << *e.meta.model   << "\n";
    }
}

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

    std::vector<ListEntry> entries;

    for (const auto& entry : fs::directory_iterator(lib_dir)) {
        if (entry.path().extension() != ".beve") continue;
        std::string stem = entry.path().stem().string();
        if (!id_filter.empty() && stem.find(id_filter) == std::string::npos) continue;

        hera::AgentHeader header;
        std::string scratch;
        if (read_file(header, entry.path().string().c_str(), scratch)) continue;

        std::string pid_str;
        if (!run_dir.empty()) {
            fs::path pid_path = run_dir / (stem + ".pid");
            if (fs::exists(pid_path)) {
                std::ifstream ifs_pid(pid_path);
                PidFileContent info;
                std::string pbuf((std::istreambuf_iterator<char>(ifs_pid)),
                                  std::istreambuf_iterator<char>());
                if (!glz::read_json(info, pbuf) && ::kill(info.pid, 0) == 0)
                    pid_str = std::to_string(info.pid);
            }
        }

        entries.push_back(ListEntry{stem, pid_str, std::move(header.meta)});
    }

    std::sort(entries.begin(), entries.end());

    if (flags & HERA_FORMAT_JSON)
        emit_json(entries);
    else if (flags & HERA_FORMAT_YAML)
        emit_yaml(entries);
    else
        emit_text(entries);

    return Result(0);
}

} // namespace hera

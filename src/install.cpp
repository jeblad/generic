/**
 * generic – the “install” subcommand implementation
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
#include <chrono>
#include <format>
#include "generic/internal.hpp"
#include <rlog/rlog.hpp>

namespace hera {

Result install(
    const char * in_fn,
    const char * out_dir,
    const char * uuid_str,
    const char * nickname_str,
    int log_level,
    int flags
) {
    namespace fs = std::filesystem;
    FileClone doc;
    std::string full_buffer;

    fs::path install_path(out_dir);
    fs::path run_dir = (install_path.filename() == "lib") ? install_path.parent_path() / "run" : fs::path("/run") / PROJECT_NAME;

    if (uuid_str) {
        if (fs::exists(run_dir / (std::string(uuid_str) + ".pid"))) {
            ERROR_FMT_("Installation aborted: Agent {} is already running.", uuid_str);
            return Result(1);
        }
    }
    
    std::ifstream ifs(in_fn, std::ios::binary | std::ios::ate);
    if (!ifs) return Result(1);
    auto size = ifs.tellg();
    full_buffer.resize(static_cast<size_t>(size));
    ifs.seekg(0);
    if (!ifs.read(full_buffer.data(), size)) return Result(1);

    size_t offset = 0;
    if (!glz::read_beve_at(doc.meta, full_buffer, offset) || doc.meta.type != "metadata") return Result(1);

    if (offset < full_buffer.size()) {
        doc.remaining_parts.emplace_back(full_buffer.data() + offset, full_buffer.size() - offset);
    }

    if (uuid_str) doc.meta.uuid = std::string(uuid_str);
    const std::string uuid_val = doc.meta.uuid.value_or("");
    if (uuid_val.empty()) return Result(1);

    if (nickname_str) doc.meta.nickname = std::string(nickname_str);

    fs::path out_path = fs::path(out_dir) / (uuid_val + ".beve");
    if (!(flags & HERA_FORCE) && fs::exists(out_path)) {
        ERROR_FMT_("Target file already exists: {}", out_path.string());
        return Result(1);
    }

    auto now = std::chrono::system_clock::now();
    std::string timestamp = std::format("{:%FT%TZ}", std::chrono::floor<std::chrono::seconds>(now));
    if (!doc.meta.provenance) doc.meta.provenance.emplace();
    doc.meta.provenance->push_back({timestamp, "installed"});

    std::string out_buffer;
    if (glz::write_beve(doc.meta, out_buffer)) {
        return Result(1);
    }
    for (const auto& part : doc.remaining_parts) out_buffer.append(part);

    std::ofstream ofs(out_path, std::ios::binary);
    ofs.write(out_buffer.data(), static_cast<std::streamsize>(out_buffer.size()));
    if (!ofs) return Result(1);

    if (flags & HERA_REMOVE) {
        std::error_code ec;
        if (!fs::remove(in_fn, ec)) {
            ERROR_FMT_("Failed to remove source: {}", ec.message());
            return Result(1);
        }
    }

    NOTICE_FMT_("Installed agent {} ({}) from {}.",
        uuid_val, doc.meta.nickname.value_or(_("no nickname")), in_fn);

    return Result(0);
}

} // namespace hera
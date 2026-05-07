/**
 * generic – the “uninstall” subcommand implementation
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
#include "hera/utility.hpp"

namespace hera {

Result uninstall(
    const char * in_fn,
    const char * out_dir,
    const char * uuid_str,
    const char * callsign_str,
    int log_level,
    int flags
) {
    namespace fs = std::filesystem; // Use hera::MultipartAgentContent
    if (!in_fn || !fs::exists(in_fn)) return Result(1);

    hera::MultipartAgentContent doc;
    std::string full_buffer;
    std::ifstream ifs(in_fn, std::ios::binary | std::ios::ate);
    if (!ifs) return Result(1); // Error opening file
    auto size = ifs.tellg();
    if (size <= 0) return Result(1); // Empty or invalid file size
    full_buffer.resize(static_cast<size_t>(size));
    ifs.seekg(0);
    if (!ifs.read(full_buffer.data(), size)) return Result(1); // Error reading file

    size_t offset = 0;
    if (!glz::read_beve_at(doc.meta, full_buffer, offset) || doc.meta.type != "metadata") {
        return Result(1);
    }
    
    const std::string uuid_val = doc.meta.uuid.value_or("");
    fs::path source_path(in_fn);
    fs::path install_dir = source_path.parent_path();
    fs::path run_dir = (install_dir.filename() == "lib") ? install_dir.parent_path() / "run" : fs::path("/run") / PROJECT_NAME;

    if (fs::exists(run_dir / (uuid_val + ".pid"))) {
        ERROR_FMT_("Uninstallation aborted: Agent {} is running.", uuid_val);
        return Result(1);
    }

    fs::path target_path(out_dir ? out_dir : ".");
    if (fs::is_directory(target_path)) target_path /= source_path.filename();

    if (!(flags & HERA_FORCE) && fs::exists(target_path)) {
        ERROR_FMT_("Target file exists: {}", target_path.string());
        return Result(1);
    }

    auto now = std::chrono::system_clock::now();
    std::string timestamp = std::format("{:%FT%TZ}", std::chrono::floor<std::chrono::seconds>(now));
    if (!doc.meta.provenance) doc.meta.provenance.emplace();
    doc.meta.provenance->push_back({timestamp, "uninstalled"});

    std::string out_buffer;
    if (glz::write_beve(doc.meta, out_buffer)) { // Check for error
        return Result(1);
    }
    // Recreates the rest of the file
    size_t data_offset = 0;
    MetaData dummy;
    if (glz::read_beve_at(dummy, full_buffer, data_offset)) {
        out_buffer.append(full_buffer.data() + data_offset, full_buffer.size() - data_offset);
    }

    std::ofstream ofs_out(target_path, std::ios::binary);
    if (!ofs_out) {
        ERROR_FMT_("Failed to open output file {}: {}", target_path.string(), std::strerror(errno));
        return Result(1);
    }
    ofs_out.write(out_buffer.data(), static_cast<std::streamsize>(out_buffer.size()));
    if (!ofs_out) { // Check for error writing to output file
        ERROR_FMT_("Failed to write agent image to {}: {}", target_path.string(), std::strerror(errno));
        return Result(1);
    }

    if (flags & HERA_REMOVE) {
        std::error_code ec_rm;
        if (!fs::remove(source_path, ec_rm) && ec_rm) {
            ERROR_FMT_("Failed to remove source file after uninstall: {}.", ec_rm.message());
            return Result(1);
        }
    }

    NOTICE_FMT_("Uninstalled agent {} ({}) to {}.", 
        uuid_val, doc.meta.callsign.value_or(_("no callsign")), target_path.string());

    return Result(0);
}

} // namespace hera
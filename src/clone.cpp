/**
 * generic – the “clone” subcommand implementation
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

#include <fstream>
#include <chrono>
#include <format>
#include "generic/internal.hpp"
#include <rlog/rlog.hpp>
#include "hera/utility.hpp"

namespace hera {

Result clone(const char * in_fn, const char * out_fn, const char * uuid_str, const char * callsign_str, int flags) {
    hera::MultipartAgentContent doc;
    std::string full_buffer;
    
    std::ifstream ifs(in_fn, std::ios::binary | std::ios::ate);
    if (!ifs) {
        ERROR_FMT_("Failed to open input file: {}", in_fn);
        return Result(1);
    }
    
    auto size = ifs.tellg();
    if (size <= 0) return Result(1);

    full_buffer.resize(static_cast<size_t>(size));
    ifs.seekg(0);
    if (!ifs.read(full_buffer.data(), size)) return Result(1);

    size_t offset = 0;
    if (!glz::read_beve_at(doc.meta, full_buffer, offset) || doc.meta.type != "metadata") { // Use hera::MultipartAgentContent
        ERROR_FMT_("Invalid Multipart BEVE: Part 1 is not metadata in {}", in_fn);
        return Result(1);
    }
    
    if (offset < full_buffer.size()) {
        doc.remaining_parts.emplace_back(full_buffer.data() + offset, full_buffer.size() - offset);
    }

    if (uuid_str) doc.meta.uuid = std::string(uuid_str);
    if (callsign_str) doc.meta.callsign = std::string(callsign_str);

    auto now = std::chrono::system_clock::now();
    std::string timestamp = std::format("{:%FT%TZ}", std::chrono::floor<std::chrono::seconds>(now));
    if (!doc.meta.provenance) doc.meta.provenance.emplace();
    doc.meta.provenance->push_back({timestamp, "cloned"});

    std::string out_buffer;
    if (glz::write_beve(doc.meta, out_buffer)) return Result(1);
    for (const auto& part : doc.remaining_parts) out_buffer.append(part);

    std::ofstream ofs(out_fn, std::ios::binary);
    ofs.write(out_buffer.data(), static_cast<std::streamsize>(out_buffer.size()));
    if (!ofs) {
        ERROR_FMT_("Failed to write cloned image to {}: {}", out_fn, std::strerror(errno));
        return Result(1);
    }

    return Result(0);
}

} // namespace hera
/**
 * generic – the “clone” subcommand implementation
 * Copyright © 2026 John Erling Blad. All Rights Reserved.
 * Protected under the Norwegian Copyright Act (Åndsverksloven).
 * See LEGAL_NOTICE.md for terms of use.
 **/

#include <fstream>
#include <chrono>
#include <format>
#include "generic/internal.hpp"
#include <rlog/rlog.hpp>
#include "hera/utility.hpp"

namespace hera {

Result clone(const char * in_fn, const char * out_fn, const char * uuid_str, const char * nickname_str, int flags) {
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
    if (auto ec = hera::read_json_part(doc.meta, full_buffer, offset)) {
        ERROR_FMT_("Failed to read agent metadata from {}: {}", in_fn, glz::format_error(ec, full_buffer));
        return Result(1);
    }

    if (offset < full_buffer.size()) {
        doc.remaining_parts.emplace_back(full_buffer.data() + offset, full_buffer.size() - offset);
    }

    if (uuid_str) doc.meta.uuid = std::string(uuid_str);
    if (nickname_str) doc.meta.nickname = std::string(nickname_str);

    auto now = std::chrono::system_clock::now();
    std::string timestamp = std::format("{:%FT%TZ}", std::chrono::floor<std::chrono::seconds>(now));
    if (!doc.meta.provenance) doc.meta.provenance.emplace();
    doc.meta.provenance->push_back({timestamp, "cloned"});

    std::string out_buffer;
    if (auto ec = hera::write_json_part(doc.meta, out_buffer)) return Result(1);
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
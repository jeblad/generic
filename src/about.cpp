/**
 * generic – the "about" subcommand implementation
 * Copyright © 2026 John Erling Blad. All Rights Reserved.
 * Protected under the Norwegian Copyright Act (Åndsverksloven).
 * See LEGAL_NOTICE.md for terms of use.
 **/

#include <iostream>
#include <format>
#include <filesystem>
#include <cstring>
#include <map>
#include <vector>
#include <string>
#include <string_view>
#include <array>

#include "hera/config.h"
#include "generic/config.h"
#include "generic/internal.hpp"
#include <rlog/rlog.hpp>
#include "hera/utility.hpp"
#include "sanitize/sanitize.hpp"

namespace hera {

struct FieldEntry {
    std::string key;
    std::string label;
    std::string value;
    bool is_list{false};
};

static std::string join_list(std::string_view raw) {
    std::array<std::string_view, 32> items;
    size_t count = parse_list(raw, items.data(), items.size());
    std::string result;
    for (size_t i = 0; i < count; ++i) {
        if (i > 0) result += ", ";
        result += items[i];
    }
    return result;
}

static void emit_result(const std::vector<FieldEntry>& fields, int flags) {
    if (flags & HERA_FORMAT_JSON) {
        std::map<std::string, std::string> obj;
        for (const auto& f : fields) obj[f.key] = f.value;
        std::string out;
        if (glz::write_json(obj, out)) return;
        std::cout << out << "\n";
    } else if (flags & HERA_FORMAT_YAML) {
        for (const auto& f : fields)
            std::cout << f.key << ": " << f.value << "\n";
    } else {
        for (const auto& f : fields) {
            if (f.is_list) {
                std::array<std::string_view, 32> items;
                size_t count = parse_list(f.value, items.data(), items.size());
                if (count > 0) {
                    std::cout << f.label << "\n";
                    for (size_t i = 0; i < count; ++i)
                        std::cout << "\t" << items[i] << "\n";
                }
            } else {
                std::cout << f.label << "\n\t" << f.value << "\n";
            }
        }
    }
}

static bool want(bool all, int flags, int field_flag) {
    return all || (flags & field_flag);
}

Result about_plugin(int flags) {
    bool all = flags & HERA_REPORT_ABOUT;
    bool structured = flags & (HERA_FORMAT_JSON | HERA_FORMAT_YAML);
    std::string version = std::format("{}.{}.{}",
        GENERIC_VERSION_MAJOR, GENERIC_VERSION_MINOR, GENERIC_VERSION_PATCH);

    std::vector<FieldEntry> fields;
    if (want(all, flags, HERA_REPORT_DESCRIPTION) && *GENERIC_DESCRIPTION)
        fields.push_back({"description", _("Description:"), GENERIC_DESCRIPTION});
    if (want(all, flags, HERA_REPORT_HOMEPAGE) && *GENERIC_HOMEPAGE)
        fields.push_back({"homepage", _("Homepage:"), GENERIC_HOMEPAGE});
    if (want(all, flags, HERA_REPORT_VERSION))
        fields.push_back({"version", _("Version:"), version});
    if (want(all, flags, HERA_REPORT_API))
        fields.push_back({"api_level", _("API level:"), std::to_string(HERA_API_LEVEL)});
    if (want(all, flags, HERA_REPORT_LICENSE) && *GENERIC_LICENSE)
        fields.push_back({"license", _("License:"), GENERIC_LICENSE});
    if (want(all, flags, HERA_REPORT_COPYRIGHT) && *GENERIC_COPYRIGHT)
        fields.push_back({"copyright", _("Copyright:"), GENERIC_COPYRIGHT});
    if (want(all, flags, HERA_REPORT_CREATOR) && *GENERIC_CREATOR)
        fields.push_back({"creator", _("Creator:"), GENERIC_CREATOR});
    if (want(all, flags, HERA_REPORT_CONTRIBUTORS) && *GENERIC_CONTRIBUTORS)
        fields.push_back({"contributors", _("Contributors:"),
            structured ? join_list(GENERIC_CONTRIBUTORS) : GENERIC_CONTRIBUTORS, !structured});
    const char* credits = _("translator-credits");
    if (want(all, flags, HERA_REPORT_TRANSLATORS) && std::strcmp(credits, "translator-credits") != 0)
        fields.push_back({"translators", _("Translators:"),
            structured ? join_list(credits) : credits, !structured});

    emit_result(fields, flags);
    return Result(0);
}

Result about_system(int flags) {
    bool all = flags & HERA_REPORT_ABOUT;
    bool structured = flags & (HERA_FORMAT_JSON | HERA_FORMAT_YAML);

    std::vector<FieldEntry> fields;
    if (want(all, flags, HERA_REPORT_DESCRIPTION) && *HERA_DESCRIPTION)
        fields.push_back({"description", _("Description:"), HERA_DESCRIPTION});
    if (want(all, flags, HERA_REPORT_HOMEPAGE) && *HERA_HOMEPAGE)
        fields.push_back({"homepage", _("Homepage:"), HERA_HOMEPAGE});
    if (want(all, flags, HERA_REPORT_VERSION))
        fields.push_back({"version", _("Version:"), HERA_VERSION});
    if (want(all, flags, HERA_REPORT_API))
        fields.push_back({"api_level", _("API level:"), std::to_string(HERA_API_LEVEL)});
    if (want(all, flags, HERA_REPORT_LICENSE) && *HERA_LICENSE)
        fields.push_back({"license", _("License:"), HERA_LICENSE});
    if (want(all, flags, HERA_REPORT_COPYRIGHT) && *HERA_COPYRIGHT)
        fields.push_back({"copyright", _("Copyright:"), HERA_COPYRIGHT});
    if (want(all, flags, HERA_REPORT_CREATOR) && *HERA_CREATOR)
        fields.push_back({"creator", _("Creator:"), HERA_CREATOR});
    if (want(all, flags, HERA_REPORT_CONTRIBUTORS) && *HERA_CONTRIBUTORS)
        fields.push_back({"contributors", _("Contributors:"),
            structured ? join_list(HERA_CONTRIBUTORS) : HERA_CONTRIBUTORS, !structured});
    const char* credits = _("translator-credits");
    if (want(all, flags, HERA_REPORT_TRANSLATORS) && std::strcmp(credits, "translator-credits") != 0)
        fields.push_back({"translators", _("Translators:"),
            structured ? join_list(credits) : credits, !structured});

    emit_result(fields, flags);
    return Result(0);
}

Result about_module(const char* filename, int flags) {
    if (!filename || *filename == '\0' || !std::filesystem::exists(filename)) {
        ERROR_("Agent file not found for 'about' request.")
            .hint(_("Check that the correct mode is selected (--developer/--private/--system), or run `hera list` to see available agents."));
        return Result(1);
    }

    hera::AgentHeader header;
    std::string scratch;
    if (auto ec = read_file(header, filename, scratch)) {
        ERROR_FMT_("Failed to read agent metadata: {}", glz::format_error(ec, scratch))
            .hint(_("MMIO state is available — run `hera rebuild` to recover."));
        return Result(1);
    }

    bool all = flags & HERA_REPORT_ABOUT;

    std::vector<FieldEntry> fields;
    if (want(all, flags, HERA_REPORT_DESCRIPTION) && header.meta.description)
        fields.push_back({"description", _("Description:"), *header.meta.description});
    if (want(all, flags, HERA_REPORT_HOMEPAGE) && header.meta.homepage)
        fields.push_back({"homepage", _("Homepage:"), *header.meta.homepage});
    if (want(all, flags, HERA_REPORT_VERSION) && header.meta.version)
        fields.push_back({"version", _("Version:"), *header.meta.version});
    if (want(all, flags, HERA_REPORT_MODEL) && header.meta.model)
        fields.push_back({"model", _("Model:"), *header.meta.model});
    if (want(all, flags, HERA_REPORT_EPOCH) && header.meta.epoch)
        fields.push_back({"epoch", _("Epoch:"), std::to_string(*header.meta.epoch)});
    if (want(all, flags, HERA_REPORT_LICENSE) && header.meta.license)
        fields.push_back({"license", _("License:"), *header.meta.license});
    if (want(all, flags, HERA_REPORT_COPYRIGHT) && header.meta.copyright)
        fields.push_back({"copyright", _("Copyright:"), *header.meta.copyright});
    if (want(all, flags, HERA_REPORT_CREATOR) && header.meta.creator)
        fields.push_back({"creator", _("Creator:"), *header.meta.creator});
    if (want(all, flags, HERA_REPORT_CONTRIBUTORS) && header.meta.contributors) {
        std::string joined;
        for (size_t i = 0; i < header.meta.contributors->size(); ++i) {
            if (i > 0) joined += ", ";
            joined += (*header.meta.contributors)[i];
        }
        fields.push_back({"contributors", _("Contributors:"), joined});
    }
    if ((all || (flags & HERA_REPORT_PROVENANCE)) && header.meta.provenance
            && !header.meta.provenance->empty()) {
        std::string joined;
        for (size_t i = 0; i < header.meta.provenance->size(); ++i) {
            if (i > 0) joined += ", ";
            const auto& p = (*header.meta.provenance)[i];
            joined += p.timestamp + ": " + sanitize::untaint(p.action, false);
        }
        fields.push_back({"provenance", _("Provenance:"), joined});
    }

    emit_result(fields, flags);
    return Result(0);
}

} // namespace hera

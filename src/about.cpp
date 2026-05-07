/**
 * generic – the “about” subcommand implementation
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

#include <iostream>
#include <format>
#include <filesystem>
#include <cstring>

#include "hera/config.h"
#include "generic/config.h"
#include "generic/internal.hpp"
#include <rlog/rlog.hpp>
#include "hera/utility.hpp"
#include "sanitize/sanitize.hpp"

namespace hera {

static void do_report_field(bool all, int flags, int field_flag, const char* label, std::string_view val) {
    if ((all || (flags & field_flag)) && !val.empty()) {
        std::cout << label << std::endl << "\t" << val << std::endl;
    }
}

static void do_report_list_string(bool all, int flags, int field_flag, const char* s, const char* p, std::string_view raw) {
    if (!(all || (flags & field_flag)) || raw.empty()) return;
    std::array<std::string_view, 32> items;
    if (size_t count = parse_list(raw, items.data(), items.size()); count > 0) {
        std::cout << (count == 1 ? s : p) << std::endl;
        for (size_t i = 0; i < count; ++i) std::cout << "\t" << items[i] << std::endl;
    }
}

static void do_report_list_vector(bool all, int flags, int field_flag, const char* s, const char* p, const std::optional<std::vector<std::string>>& items) {
    if ((all || (flags & field_flag)) && items && !items->empty()) {
        std::cout << (items->size() == 1 ? s : p) << std::endl;
        for (const auto& item : *items) {
            std::cout << "\t" << item << std::endl;
        }
    }
}

Result about_plugin(int flags) {
    bool all = flags & HERA_REPORT_ABOUT;
    std::string version = std::format("{}.{}.{}", GENERIC_VERSION_MAJOR, GENERIC_VERSION_MINOR, GENERIC_VERSION_PATCH);

    do_report_field(all, flags, HERA_REPORT_DESCRIPTION, _("Description:"), GENERIC_DESCRIPTION);
    do_report_field(all, flags, HERA_REPORT_HOMEPAGE, _("Homepage:"), GENERIC_HOMEPAGE);
    do_report_field(all, flags, HERA_REPORT_VERSION, _("Version:"), version);
    do_report_field(all, flags, HERA_REPORT_API, _("API level:"), std::to_string(HERA_API_LEVEL));
    do_report_field(all, flags, HERA_REPORT_LICENSE, _("License:"), GENERIC_LICENSE);
    do_report_field(all, flags, HERA_REPORT_COPYRIGHT, _("Copyright:"), GENERIC_COPYRIGHT);
    do_report_field(all, flags, HERA_REPORT_CREATOR, _("Creator:"), GENERIC_CREATOR);

    do_report_list_string(all, flags, HERA_REPORT_CONTRIBUTORS, _("Contributor:"), _("Contributors:"), GENERIC_CONTRIBUTORS);

    const char* credits = _("translator-credits");
    if (std::strcmp(credits, "translator-credits") != 0) {
        do_report_list_string(all, flags, HERA_REPORT_TRANSLATORS, _("Translator:"), _("Translators:"), credits);
    }

    return Result(0);
}

Result about_system(int flags) {
    bool all = flags & HERA_REPORT_ABOUT;

    do_report_field(all, flags, HERA_REPORT_DESCRIPTION, _("Description:"), HERA_DESCRIPTION);
    do_report_field(all, flags, HERA_REPORT_HOMEPAGE, _("Homepage:"), HERA_HOMEPAGE);
    do_report_field(all, flags, HERA_REPORT_VERSION, _("Version:"), HERA_VERSION);
    do_report_field(all, flags, HERA_REPORT_API, _("API level:"), std::to_string(HERA_API_LEVEL));
    do_report_field(all, flags, HERA_REPORT_LICENSE, _("License:"), HERA_LICENSE);
    do_report_field(all, flags, HERA_REPORT_COPYRIGHT, _("Copyright:"), HERA_COPYRIGHT);
    do_report_field(all, flags, HERA_REPORT_CREATOR, _("Creator:"), HERA_CREATOR);

    do_report_list_string(all, flags, HERA_REPORT_CONTRIBUTORS, _("Contributor:"), _("Contributors:"), HERA_CONTRIBUTORS);

    const char* credits = _("translator-credits");
    if (std::strcmp(credits, "translator-credits") != 0) {
        do_report_list_string(all, flags, HERA_REPORT_TRANSLATORS, _("Translator:"), _("Translators:"), credits);
    }

    return Result(0);
}

Result about_module(const char* filename, int flags) {
    if (!filename || *filename == '\0' || !std::filesystem::exists(filename)) {
        ERROR_("Agent file not found for 'about' request.");
        return Result(1);
    }

    hera::AgentHeader header;
    std::string scratch;
    if (auto ec = read_file(header, filename, scratch)) {
        ERROR_FMT_("Failed to read agent metadata: {}", glz::format_error(ec, scratch));
        return Result(1);
    }

    bool all = flags & HERA_REPORT_ABOUT;
    do_report_field(all, flags, HERA_REPORT_DESCRIPTION, _("Description:"), header.meta.description.value_or(""));
    do_report_field(all, flags, HERA_REPORT_HOMEPAGE, _("Homepage:"), header.meta.homepage.value_or(""));
    do_report_field(all, flags, HERA_REPORT_VERSION, _("Version:"), header.meta.version.value_or(""));
    do_report_field(all, flags, HERA_REPORT_MODEL, _("Model:"), header.meta.model.value_or(""));
    do_report_field(all, flags, HERA_REPORT_EPOCH, _("Epoch:"), header.meta.epoch ? std::to_string(*header.meta.epoch) : "");
    do_report_field(all, flags, HERA_REPORT_LICENSE, _("License:"), header.meta.license.value_or(""));
    do_report_field(all, flags, HERA_REPORT_COPYRIGHT, _("Copyright:"), header.meta.copyright.value_or(""));
    do_report_field(all, flags, HERA_REPORT_CREATOR, _("Creator:"), header.meta.creator.value_or(""));

    do_report_list_vector(all, flags, HERA_REPORT_CONTRIBUTORS, _("Contributor:"), _("Contributors:"), header.meta.contributors);

    if ((all || (flags & HERA_REPORT_PROVENANCE)) && header.meta.provenance && !header.meta.provenance->empty()) {
        std::cout << _("Provenance:") << std::endl;
        for (const auto& p : *header.meta.provenance) {
            std::cout << "\t" << p.timestamp << ": " << sanitize::untaint(p.action, false) << std::endl;
        }
    }

    return Result(0);
}
} // namespace hera
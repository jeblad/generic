/**
The generic extension for Heuristic Reasoning Agent (hera)

This extension is an implementation of a simplified model that is
barely enough for verifying proper operation. It should be sufficient
for further work.

Copyrighted according to Norwegian Coyright Law (Åndsverksloven)

That basically means CC-by-NC-ND where ND is weak and you can make
derivatives if they add considerable own work, respects the original
work, and some other minor clauses. Minor changes to fix bugs should
be acceptable.

Copyright 2026 John Erling Blad
**/

#include <chrono>
#include <format>
#include <iostream>
#include <fstream>
#include <string>
#include <iomanip>
#include <optional>
#include <map>
#include <array>
#include <vector>
#include <string_view>
#include <sstream>
#include <filesystem>
#include <algorithm>
#include <cctype>
#include <cstring>

#include "hera/config.h"
#include "generic/config.h"
#include <rlog/rlog.hpp> // Use rlog for i18n and logging
#include "hera/hera.hpp"
#include "sanitize/sanitize.hpp"
//#include "uuid_ext/uuid.hpp"
//#include "uuid_ext/uuid_ext.hpp"
#include "glaze/glaze.hpp"
#include "glaze/core/istream_buffer.hpp"

namespace hera {

/**
 * @brief Root structure for cloning that preserves unknown 'huge' fields.
 */
struct FileClone {
    hera::MetaData meta{};
    // Glaze uses this name specifically to capture any unmapped fields
    std::map<std::string, glz::generic> extra;
};

// forward declaration
Result clone(
    const char * in_fn,
    const char * out_fn,
    const char * uuid_str,
    const char * callsign_str,
    int flags
);

Result build(
    const char * in_fn,
    const char * out_fn,
    const char * uuid_str,
    const char * callsign_str,
    int flags
);

Result install(
    const char * in_fn,
    const char * out_dir,
    const char * uuid_str,
    const char * callsign_str,
    int log_level,
    int flags
);

Result uninstall(
    const char * in_fn,
    const char * out_dir,
    const char * uuid_str,
    const char * callsign_str,
    int log_level,
    int flags
);

Result list(
    const char * in_fn,
    const char * out_fn,
    const char * uuid_str,
    const char * fields_str,
    int log_level,
    int flags
);

Result about_system(int flags);
Result about_plugin(int flags);
Result about_module(const char* filename, int flags);

// forward declaration
const char* get_plugin_info();

}  // namespace hera

template <>
struct glz::meta<hera::FileClone> {
    using T = hera::FileClone;
    static constexpr auto value = glz::object(
        "meta", &T::meta);
    static constexpr auto unknown_write{&T::extra};
    static constexpr auto unknown_read{&T::extra};
};

/**
 * @brief Default implementation of the clone operation.
 * logic: Create a new UUID and copy basic metadata.
 */
hera::Result hera::clone(
    const char * in_fn,
    const char * out_fn,
    const char * uuid_str,
    const char * callsign_str,
    int flags
) {
    FileClone doc;

    std::string buffer;
    auto ec_read = read_file(doc, in_fn, buffer);
    if (ec_read) {
        ERROR_FMT_("Failed to read agent image from {}: {}", 
            in_fn, glz::format_error(ec_read, buffer));
        return hera::Result(1);
    }

    std::string old_uuid = doc.meta.uuid.value_or("<unknown>");
    if (uuid_str && *uuid_str != '\0') doc.meta.uuid = uuid_str;
    if (callsign_str && *callsign_str != '\0') doc.meta.callsign = callsign_str;

    auto now = std::chrono::system_clock::now();
    std::string timestamp = std::format("{:%FT%TZ}",
        std::chrono::floor<std::chrono::seconds>(now));

    if (!doc.meta.provenance) {
        doc.meta.provenance.emplace();
    }
    doc.meta.provenance->push_back({
        timestamp,
        std::format("Cloned from {}", old_uuid)
    });

    auto ec_write = write_file(doc, out_fn);
    if (ec_write) {
        ERROR_FMT_("Failed to write agent image to {}: {}", 
            out_fn, glz::format_error(ec_write));
        return hera::Result(1);
    }

    return hera::Result(0);
}

/**
 * @brief Generic build implementation (not supported).
 */
hera::Result hera::build(
    const char * in_fn,
    const char * out_fn,
    const char * uuid_str,
    const char * callsign_str,
    int flags
) {
    ERROR_("Generic plugin does not support 'build' operations");
    return hera::Result(2);
}

/**
 * @brief Default implementation of the install operation.
 */
hera::Result hera::install(
    const char * in_fn,
    const char * out_dir,
    const char * uuid_str,
    const char * callsign_str,
    int log_level,
    int flags
) {
    hera::FileClone doc;
    std::string buffer;
    auto ec_read = hera::read_file(doc, in_fn, buffer);
    if (ec_read) {
        ERROR_FMT_("Failed to read agent image from {}: {}", in_fn, glz::format_error(ec_read, buffer));
        return hera::Result(1);
    }

    // Determine UUID from metadata or fallback to argument
    std::string uuid_val = doc.meta.uuid.value_or(uuid_str ? std::string(uuid_str) : "");
    if (uuid_val.empty()) {
        ERROR_("No UUID found in metadata or arguments. Cannot install.");
        return hera::Result(1);
    }
    hera::set_uuid(uuid_val);

    // Update callsign in metadata if provided
    if (callsign_str && *callsign_str != '\0') {  // Only update if callsign_str is not empty
        doc.meta.callsign = std::string(callsign_str);
    }

    // Determine the runtime directory for PID files
    // If the installation is in a "lib" subdirectory (Private/Develop mode),
    // we use a sibling "run" directory. Otherwise, we assume a system-wide
    // installation and use the standard /run location.
    std::filesystem::path run_dir;
    std::filesystem::path install_path(out_dir);
    if (install_path.filename() == "lib") {
        // Private mode: ~/.config/hera/lib -> ~/.config/hera/run
        run_dir = install_path.parent_path() / "run";
    } else {
        // System mode: /var/lib/hera -> /run/hera
        // This ensures root visibility for system-level agents.
        run_dir = std::filesystem::path("/run") / PROJECT_NAME;
    }

    std::filesystem::path pid_path = run_dir / (uuid_val + ".pid");

    // PID files are absolute blockers regardless of --force
    if (std::filesystem::exists(pid_path)) {
        ERROR_FMT_("Installation aborted: Agent {} is already running (found {})", 
            uuid_val, pid_path.string());
        return hera::Result(1);
    }

    // Construct output filename: <uuid>.beve
    std::filesystem::path out_path = std::filesystem::path(out_dir) / (uuid_val + ".beve");

    // Existence check for the image file
    if (!(flags & HERA_FORCE) && std::filesystem::exists(out_path)) {
        ERROR_FMT_("Output file already exists: {}. Use --force to overwrite.", 
            out_path.string());
        return hera::Result(1);
    }

    // Update provenance
    auto now = std::chrono::system_clock::now();
    std::string timestamp = std::format("{:%FT%TZ}", std::chrono::floor<std::chrono::seconds>(now));

    if (!doc.meta.provenance) {
        doc.meta.provenance.emplace();
    }
    doc.meta.provenance->push_back({timestamp, "installed"});

    // Write as .beve (write_file uses extension to decide format)
    std::string final_path = out_path.string();
    auto ec_write = hera::write_file(doc, final_path);
    if (ec_write) {
        ERROR_FMT_("Failed to write agent image to {}: {}", final_path, glz::format_error(ec_write));
        return hera::Result(1);
    }

    NOTICE_FMT_("Installed agent {} ({}) from {}.", 
        uuid_val, doc.meta.callsign.value_or(_("no callsign")), in_fn);
    
    if (flags & HERA_REMOVE) {
        std::error_code ec_rm;
        if (!std::filesystem::remove(in_fn, ec_rm)) {
            WARNING_FMT_("Failed to remove source file after installation: {}", ec_rm.message());
        }
    }

    return hera::Result(0);
}

/**
 * @brief Moves an installed agent back to the working directory and removes it from system/private lib.
 */
hera::Result hera::uninstall(
    const char * in_fn,
    const char * out_dir,
    const char * uuid_str,
    const char * callsign_str,
    int log_level,
    int flags
) {
    if (!in_fn || !std::filesystem::exists(in_fn)) {
        ERROR_("Source file not found in installation directory.");
        return hera::Result(1);
    }

    FileClone doc;
    std::string buffer;
    auto ec_read = hera::read_file(doc, in_fn, buffer);
    if (ec_read) {
        ERROR_FMT_("Failed to read agent image from {}: {}", in_fn, glz::format_error(ec_read, buffer));
        return hera::Result(1);
    }

    // Determine UUID for state checks
    std::string uuid_val = doc.meta.uuid.value_or(uuid_str ? std::string(uuid_str) : "");
    if (uuid_val.empty()) {
        ERROR_("No UUID found in image. Cannot verify state.");
        return hera::Result(1);
    }
    hera::set_uuid(uuid_val);

    // Find PID file based on source path location
    std::filesystem::path source_path(in_fn);
    std::filesystem::path install_dir = source_path.parent_path();
    std::filesystem::path run_dir;

    if (install_dir.filename() == "lib") {
        // Private mode: ~/.config/hera/lib -> ~/.config/hera/run
        run_dir = install_dir.parent_path() / "run";
    } else {
        // System mode: /var/lib/hera -> /run/hera
        run_dir = std::filesystem::path("/run") / PROJECT_NAME;
    }

    std::filesystem::path pid_path = run_dir / (uuid_val + ".pid");

    if (std::filesystem::exists(pid_path)) {
        ERROR_FMT_("Uninstallation aborted: Agent {} is currently running (found {})", 
            uuid_val, pid_path.string());
        return hera::Result(1);
    }

    // Hvis målet er en eksisterende katalog, beholder vi det opprinnelige filnavnet.
    // Ellers behandler vi out_dir som den fullstendige mål-stien.
    std::filesystem::path target_path(out_dir ? out_dir : ".");
    if (std::filesystem::is_directory(target_path)) {
        target_path /= source_path.filename();
    }

    if (!(flags & HERA_FORCE) && std::filesystem::exists(target_path)) {
        ERROR_FMT_("Uninstallation aborted: Target file already exists in working directory: {}. Use --force to overwrite.", 
            target_path.string());
        return hera::Result(1);
    }

    // Update provenance
    auto now = std::chrono::system_clock::now();
    std::string timestamp = std::format("{:%FT%TZ}", std::chrono::floor<std::chrono::seconds>(now));

    if (!doc.meta.provenance) {
        doc.meta.provenance.emplace();
    }
    doc.meta.provenance->push_back({timestamp, "uninstalled"});


    // Write copy to working directory
    auto ec_write = hera::write_file(doc, target_path.string());
    if (ec_write) {
        ERROR_FMT_("Failed to write agent image to {}: {}", target_path.string(), glz::format_error(ec_write));
        return hera::Result(1);
    }

    // Remove source file from installation folder
    std::error_code ec_rm;
    if (!std::filesystem::remove(source_path, ec_rm)) {
        // Return an error if the source file cannot be removed
        ERROR_FMT_("Failed to remove source file after copy: {}", ec_rm.message());
        return hera::Result(1);
    } 

    NOTICE_FMT_("Uninstalled agent {} ({}) to {}.", 
        uuid_val, doc.meta.callsign.value_or(_("no callsign")), target_path.string());
    return hera::Result(0);
}

/**
 * @brief Specialized structure for the list command to read ONLY metadata.
 * By avoiding glz::generic/extra maps, we ensure that huge binary blobs 
 * in .beve files are skipped efficiently rather than parsed.
 */
struct AgentHeader {
    hera::MetaData meta;
    struct glz {
        static constexpr auto error_on_unknown_keys = false;
    };
};

struct ListEntry {
    std::string id;
    std::string callsign;
    std::string pid;

    bool operator<(const ListEntry& other) const {
        return id < other.id;
    }
};

/**
 * @brief Lists agents by scanning a directory and reading their metadata.
 */
hera::Result hera::list(
    const char * in_dir,
    const char * run_dir_str,
    const char * id_filter_str,
    const char * fields_str,
    int /*log_level*/,
    int /*flags*/
) {
    namespace fs = std::filesystem;
    if (!in_dir || !fs::exists(in_dir)) {
        return hera::Result(0);
    }

    fs::path lib_dir(in_dir);
    fs::path run_dir(run_dir_str ? run_dir_str : "");
    std::string id_filter = id_filter_str ? id_filter_str : "";

    // Definer tilgjengelige felter og deres lokaliserte overskrifter
    std::map<std::string, std::string> header_map = {
        {"id",       _("ID")}, // Use rlog's _ macro
        {"pid",      _("PID")}, // Use rlog's _ macro
        {"uuid",     _("UUID")}, // Use rlog's _ macro
        {"callsign", _("CALLSIGN")}, // Use rlog's _ macro
        {"version",  _("VERSION")}, // Use rlog's _ macro
        {"model",    _("MODEL")}, // Use rlog's _ macro
        {"epoch",    _("EPOCH")}, // Use rlog's _ macro
        {"license",  _("LICENSE")}, // Use rlog's _ macro
        {"provenance", _("PROVENANCE")} // Use rlog's _ macro
    };

    // Bestem hvilke kolonner som skal vises
    std::vector<std::string> fields;
    if (fields_str && *fields_str != '\0') {
        std::stringstream ss(fields_str);
        std::string segment;
        while (std::getline(ss, segment, ',')) {
            // Trim og sjekk om feltet er gyldig (unntatt provenance)
            segment.erase(0, segment.find_first_not_of(" "));
            segment.erase(segment.find_last_not_of(" ") + 1);
            if (header_map.count(segment)) {
                fields.push_back(segment);
            }
        }
    }

    if (fields.empty()) {
        fields = {"id", "callsign", "pid"};
    }

    std::vector<ListEntry> entries;
    for (const auto& entry : fs::directory_iterator(lib_dir)) {
        if (entry.path().extension() == ".beve") {
            std::string stem = entry.path().stem().string();

            // Bruk identifier som filter-fragment hvis oppgitt
            if (!id_filter.empty() && stem.find(id_filter) == std::string::npos) {
                continue;
            }

            AgentHeader header;
            std::string scratch;
            auto ec = hera::read_file(header, entry.path().string(), scratch);

            if (ec.ec == glz::error_code::none) {
                ListEntry le{stem, header.meta.callsign.value_or(""), ""}; // Use rlog's WARNING_FMT_ macro

                // Sjekk PID hvis run_dir er tilgjengelig
                if (!run_dir.empty()) {
                    fs::path pid_path = run_dir / (stem + ".pid");
                    if (fs::exists(pid_path)) {
                        std::ifstream ifs(pid_path);
                        ifs >> le.pid;
                    }
                }
                entries.push_back(le);
            } else {
                // Logg feilen så brukeren forstår hvorfor metadata mangler
                WARNING_FMT_("Could not read metadata for agent {}: {}", stem, glz::format_error(ec, scratch));
            }
        }
    }

    std::sort(entries.begin(), entries.end());

    if (entries.empty()) {
        return hera::Result(0);
    }

    auto get_width = [](const std::string& f) { return (f == "uuid") ? 40 : 25; };

    // Skille mellom tabulære felt og komplekse felt (som provenance)
    std::vector<std::string> tabular_fields;
    bool show_provenance = false;
    for (const auto& f : fields) {
        if (f == "provenance") show_provenance = true;
        else tabular_fields.push_back(f);
    }

    // Hvis brukeren bare valgte komplekse felt (som provenance), legg til 'id' som fallback
    if (tabular_fields.empty() && show_provenance) {
        tabular_fields.push_back("id");
    }

    // Skriv ut overskrifter for tabulære felt
    size_t total_width = 0;
    for (const auto& f : tabular_fields) {
        int w = get_width(f);
        std::cout << std::left << std::setw(w) << header_map[f] << " "; // Use rlog's _ macro
        total_width += w + 1;
    }

    if (!tabular_fields.empty()) {
        std::cout << std::endl;
        for (size_t i = 0; i < total_width - 1; ++i) {
            std::cout << "─";
        }
        std::cout << std::endl;
    }

    for (const auto& e : entries) {
        // Vi må lese headeren på nytt for hver linje for å hente de spesifikke feltene
        AgentHeader header;
        std::string scratch;
        read_file(header, (lib_dir / (e.id + ".beve")).string(), scratch);

        std::vector<std::string> row_values;
        bool has_content = false;

        for (const auto& f : tabular_fields) {
            std::string val;
            if (f == "id") val = e.id;
            else if (f == "pid") val = e.pid;
            else if (f == "uuid") val = header.meta.uuid.value_or("");
            else if (f == "callsign") val = header.meta.callsign.value_or("");
            else if (f == "version") val = header.meta.version.value_or("");
            else if (f == "model") val = header.meta.model.value_or("");
            else if (f == "license") val = header.meta.license.value_or("");
            else if (f == "epoch") val = header.meta.epoch ? std::to_string(*header.meta.epoch) : "";

            if (!val.empty()) has_content = true;
            row_values.push_back(val);
        }

        // Sjekk om raden eller undertabellen har innhold
        bool has_provenance = show_provenance && header.meta.provenance && !header.meta.provenance->empty();

        if (has_content || has_provenance) {
            for (size_t i = 0; i < tabular_fields.size(); ++i) {
                std::cout << std::left << std::setw(get_width(tabular_fields[i]))
                          << sanitize::untaint(row_values[i], false) << " ";
            }
            if (!tabular_fields.empty()) std::cout << std::endl;

            if (has_provenance) {
                const auto& prov = *header.meta.provenance;
                for (size_t i = 0; i < prov.size(); ++i) {
                    const char* connector = (i == prov.size() - 1) ? "  └─ " : "  ├─ ";
                    std::cout << connector << prov[i].timestamp << ": "
                              << sanitize::untaint(prov[i].action, false) << std::endl;
                }
                std::cout << std::endl;
            }
        }
    }

    return hera::Result(0);
}


/**
 * Helper to parse a semicolon-separated list into a fixed-size buffer of string_views.
 * Trims leading and trailing whitespace/newlines.
 */
size_t parse_list(std::string_view raw, std::string_view* out, size_t max_size) {
    size_t count = 0;
    for (size_t start = 0, end = 0; start < raw.size() && count < max_size; start = end + 1) {
        end = raw.find(';', start);
        std::string_view part = raw.substr(start, end == std::string_view::npos ? std::string_view::npos : end - start);
        if (auto f = part.find_first_not_of(" \t\n\r"); f != std::string_view::npos) {
            auto l = part.find_last_not_of(" \t\n\r");
            out[count++] = part.substr(f, l - f + 1);
        }
        if (end == std::string_view::npos) break;
    }
    return count;
}

/**
 * Internal helpers for formatted metadata reporting.
 */
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

hera::Result hera::about_plugin(int flags) {
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

    return hera::Result(0);
}

/**
 * @brief Reports metadata about the generic plugin or the system.
 */
hera::Result hera::about_system(int flags) {
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

    return hera::Result(0);
}

/**
 * @brief Reports metadata from a specific agent module file.
 */
hera::Result hera::about_module(const char* filename, int flags) {
    if (!filename || *filename == '\0' || !std::filesystem::exists(filename)) {
        ERROR_("Agent file not found for 'about' request.");
        return hera::Result(1);
    }

    AgentHeader header;
    std::string scratch;
    auto ec = hera::read_file(header, filename, scratch);
    if (ec) {
        ERROR_FMT_("Failed to read agent metadata: {}", glz::format_error(ec, scratch));
        return hera::Result(1);
    }

    bool all = flags & HERA_REPORT_ABOUT;
    const auto& meta = header.meta;

    do_report_field(all, flags, HERA_REPORT_DESCRIPTION, _("Description:"), meta.description.value_or(""));
    do_report_field(all, flags, HERA_REPORT_HOMEPAGE, _("Homepage:"), meta.homepage.value_or(""));
    do_report_field(all, flags, HERA_REPORT_VERSION, _("Version:"), meta.version.value_or(""));
    do_report_field(all, flags, HERA_REPORT_MODEL, _("Model:"), meta.model.value_or(""));
    do_report_field(all, flags, HERA_REPORT_EPOCH, _("Epoch:"), meta.epoch ? std::to_string(*meta.epoch) : "");
    do_report_field(all, flags, HERA_REPORT_LICENSE, _("License:"), meta.license.value_or(""));
    do_report_field(all, flags, HERA_REPORT_COPYRIGHT, _("Copyright:"), meta.copyright.value_or(""));
    do_report_field(all, flags, HERA_REPORT_CREATOR, _("Creator:"), meta.creator.value_or(""));

    do_report_list_vector(all, flags, HERA_REPORT_CONTRIBUTORS, _("Contributor:"), _("Contributors:"), meta.contributors);

    if ((all || (flags & HERA_REPORT_PROVENANCE)) && meta.provenance && !meta.provenance->empty()) {
        std::cout << _("Provenance:") << std::endl;
        for (const auto& p : *meta.provenance) {
            std::cout << "\t" << p.timestamp << ": " << sanitize::untaint(p.action, false) << std::endl;
        }
    }

    return hera::Result(0);
}

/**
 * @brief Returns the plugin type.
 */
const char* hera::get_plugin_info() {
    return "Hera Generic/Fallback Provider v1.0";
}

extern "C" {
    HERA_EXPORT uint32_t hera_api() {
        // Current Hera API version
        return HERA_API_LEVEL;
    }

    HERA_EXPORT hera::Result hera_clone(
        const char * in_fn,
        const char * out_fn,
        const char * uuid_str,
        const char * callsign_str,
        int log_level,
        int flags
    ) {
        rlog::openreport(log_level);
        hera::set_uuid(uuid_str ? uuid_str : "");
        hera::set_command("clone");
        DEBUG_FMT_("Entered {}()", __func__);
        return hera::clone(in_fn, out_fn, uuid_str, callsign_str, flags);
    }

    HERA_EXPORT hera::Result hera_build(
        const char * in_fn,
        const char * out_fn,
        const char * uuid_str,
        const char * callsign_str,
        int log_level,
        int flags
    ) {
        rlog::openreport(log_level);
        hera::set_uuid(uuid_str ? uuid_str : "");
        hera::set_command("build");
        DEBUG_FMT_("Entered {}()", __func__);
        return hera::build(in_fn, out_fn, uuid_str, callsign_str, flags);
    }


    HERA_EXPORT hera::Result hera_install(
        const char * in_fn,
        const char * out_dir,
        const char * uuid_str,
        const char * callsign_str,
        int log_level,
        int flags
    ) {
        rlog::openreport(log_level);
        hera::set_uuid(uuid_str ? uuid_str : "");
        hera::set_command("install");
        DEBUG_FMT_("Entered {}()", __func__);
        return hera::install(in_fn, out_dir, uuid_str, callsign_str, log_level, flags);
    }

    HERA_EXPORT hera::Result hera_uninstall(
        const char * in_fn,
        const char * out_dir,
        const char * uuid_str,
        const char * callsign_str,
        int log_level,
        int flags
    ) {
        rlog::openreport(log_level);
        hera::set_uuid(uuid_str ? uuid_str : "");
        hera::set_command("uninstall");
        DEBUG_FMT_("Entered {}()", __func__);
        return hera::uninstall(in_fn, out_dir, uuid_str, callsign_str, log_level, flags);
    }

    HERA_EXPORT hera::Result hera_list(
        const char * in_fn,
        const char * out_fn,
        const char * uuid_str,
        const char * callsign_str,
        int log_level,
        int flags
    ) {
        rlog::openreport(log_level);
        hera::set_uuid(uuid_str ? uuid_str : "");
        hera::set_command("list");
        DEBUG_FMT_("Entered {}()", __func__);
        return hera::list(in_fn, out_fn, uuid_str, callsign_str, log_level, flags);
    }

    HERA_EXPORT hera::Result hera_about_system(
        const char * /*in_fn*/,
        const char * /*out_fn*/,
        const char * uuid_str,
        const char * /*callsign_str*/,
        int log_level,
        int flags
    ) {
        rlog::openreport(log_level);
        hera::set_uuid(uuid_str ? uuid_str : "");
        hera::set_command("about");
        return hera::about_system(flags);
    }

    HERA_EXPORT hera::Result hera_about_plugin(
        const char * /*in_fn*/,
        const char * /*out_fn*/,
        const char * uuid_str,
        const char * /*callsign_str*/,
        int log_level,
        int flags
    ) {
        rlog::openreport(log_level);
        hera::set_uuid(uuid_str ? uuid_str : "");
        hera::set_command("about");
        return hera::about_plugin(flags);
    }

    HERA_EXPORT hera::Result hera_about_module(
        const char * in_fn,
        const char * /*out_fn*/,
        const char * uuid_str,
        const char * /*callsign_str*/,
        int log_level,
        int flags
    ) {
        rlog::openreport(log_level);
        hera::set_uuid(uuid_str ? uuid_str : "");
        hera::set_command("about");
        return hera::about_module(in_fn, flags);
    }

}  // extern "C"

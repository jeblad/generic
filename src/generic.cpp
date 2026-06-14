/**
 * generic – the main plugin entry point
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

#include <chrono>
#include <format>
#include <string>

#include "hera/config.h"
#include <rlog/rlog.hpp> // Use rlog for i18n and logging
#include "hera/hera.hpp"
#include "generic/internal.hpp"

namespace hera {
    const char* get_plugin_info() { return "Hera Generic/Fallback Provider v1.0"; }
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
        const char * nickname_str,
        int log_level,
        int flags
    ) {
        rlog::openreport(log_level);
        hera::set_uuid(uuid_str ? uuid_str : "");
        hera::set_command("clone");
        DEBUG_FMT_("Entered {}()", __func__);
        return hera::clone(in_fn, out_fn, uuid_str, nickname_str, flags);
    }

    HERA_EXPORT hera::Result hera_build(
        const char * in_fn,
        const char * out_fn,
        const char * uuid_str,
        const char * nickname_str,
        int log_level,
        int flags
    ) {
        rlog::openreport(log_level);
        hera::set_uuid(uuid_str ? uuid_str : "");
        hera::set_command("build");
        DEBUG_FMT_("Entered {}()", __func__);
        return hera::build(in_fn, out_fn, uuid_str, nickname_str, flags);
    }


    HERA_EXPORT hera::Result hera_install(
        const char * in_fn,
        const char * out_dir,
        const char * uuid_str,
        const char * nickname_str,
        int log_level,
        int flags
    ) {
        rlog::openreport(log_level);
        hera::set_uuid(uuid_str ? uuid_str : "");
        hera::set_command("install");
        DEBUG_FMT_("Entered {}()", __func__);
        return hera::install(in_fn, out_dir, uuid_str, nickname_str, log_level, flags);
    }

    HERA_EXPORT hera::Result hera_uninstall(
        const char * in_fn,
        const char * out_dir,
        const char * uuid_str,
        const char * nickname_str,
        int log_level,
        int flags
    ) {
        rlog::openreport(log_level);
        hera::set_uuid(uuid_str ? uuid_str : "");
        hera::set_command("uninstall");
        DEBUG_FMT_("Entered {}()", __func__);
        return hera::uninstall(in_fn, out_dir, uuid_str, nickname_str, log_level, flags);
    }

    HERA_EXPORT hera::Result hera_signal(
        const char * in_dir,
        const char * run_dir,
        const char * id_filter,
        int log_level,
        int flags
    ) {
        rlog::openreport(log_level);
        hera::set_uuid(id_filter ? id_filter : "");
        hera::set_command("signal");
        DEBUG_FMT_("Entered {}()", __func__);
        return hera::signal(in_dir, run_dir, id_filter, log_level, flags);
    }

    HERA_EXPORT hera::Result hera_prune(
        const char * state_dir,
        const char * run_dir,
        const char * cache_dir,
        const char * id_filter,
        int log_level,
        int flags
    ) {
        rlog::openreport(log_level);
        hera::set_uuid("");
        hera::set_command("prune");
        DEBUG_FMT_("Entered {}()", __func__);
        return hera::prune(state_dir, run_dir, cache_dir, id_filter, log_level, flags);
    }

    HERA_EXPORT hera::Result hera_list(
        const char * in_dir,
        const char * run_dir,
        const char * id_filter,
        const char * fields,
        int log_level,
        int flags
    ) {
        rlog::openreport(log_level);
        hera::set_uuid(id_filter ? id_filter : "");
        hera::set_command("list");
        DEBUG_FMT_("Entered {}()", __func__);
        return hera::list(in_dir, run_dir, id_filter, fields, log_level, flags);
    }

    HERA_EXPORT hera::Result hera_import(
        const char * from_dir,
        const char * workspace_dir,
        const char * uuid_str,
        const char * nickname_str,
        int log_level,
        int flags
    ) {
        rlog::openreport(log_level);
        hera::set_uuid(uuid_str ? uuid_str : "");
        hera::set_command("import");
        DEBUG_FMT_("Entered {}()", __func__);
        return hera::agent_import(from_dir, workspace_dir, uuid_str, nickname_str, log_level, flags);
    }

    HERA_EXPORT hera::Result hera_export(
        const char * workspace_dir,
        const char * to_dir,
        const char * uuid_str,
        const char * nickname_str,
        int log_level,
        int flags
    ) {
        rlog::openreport(log_level);
        hera::set_uuid(uuid_str ? uuid_str : "");
        hera::set_command("export");
        DEBUG_FMT_("Entered {}()", __func__);
        return hera::agent_export(workspace_dir, to_dir, uuid_str, nickname_str, log_level, flags);
    }

    HERA_EXPORT hera::Result hera_about_system(
        const char * /*in_fn*/,
        const char * /*out_fn*/,
        const char * uuid_str,
        const char * /*nickname_str*/,
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
        const char * /*nickname_str*/,
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
        const char * /*nickname_str*/,
        int log_level,
        int flags
    ) {
        rlog::openreport(log_level);
        hera::set_uuid(uuid_str ? uuid_str : "");
        hera::set_command("about");
        return hera::about_module(in_fn, flags);
    }

}  // extern "C"

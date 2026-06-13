#pragma once
#include <string>
#include <vector>
#include "hera/hera.hpp"
#include "glaze/glaze.hpp"

namespace hera {

/**
 * @brief Root structure for cloning that preserves unknown 'huge' fields.
 */
struct FileClone {
    hera::MetaData meta;
    std::vector<std::string> remaining_parts;
};

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
    const char * run_dir_str,
    const char * id_filter_str,
    const char * fields_str,
    int log_level,
    int flags
);

Result signal(
    const char * in_dir,
    const char * run_dir,
    const char * id_filter,
    int log_level,
    int flags
);


Result prune(
    const char * state_dir,
    const char * run_dir,
    const char * cache_dir,
    const char * id_filter,
    int log_level,
    int flags
);

Result about_system(int flags);
Result about_plugin(int flags);
Result about_module(const char* filename, int flags);

} // namespace hera
cmake_minimum_required(VERSION 3.24)

file(READ "package.json" PACKAGE_JSON)
string(JSON PACKAGE_VERSION GET ${PACKAGE_JSON} "version")

project(generic
    VERSION ${PACKAGE_VERSION}
    LANGUAGES CXX
)

# Metadata for Config.h (Used by hera_about)
set(GENERIC_DESCRIPTION "An extension for the Heuristic Reasoning Agent (hera) framework providing model-specific functionality.")
set(GENERIC_LICENSE "SEE LICENSE IN LICENSE.txt")
set(GENERIC_COPYRIGHT "Copyright © 2026 John Erling Blad. All Rights Reserved.")
set(GENERIC_CREATOR "John Erling Blad")
set(GENERIC_HOMEPAGE "https://github.com/jeblad/generic")
set(GENERIC_CONTRIBUTORS "Assisted by Gemini AI (2.5, 3.0, 3.1)")

# 1. Project Standards
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)

# Hardening Flags (Matching Hera host for compatibility)
list(APPEND SECURITY_FLAGS "-Wall" "-Wformat" "-Wformat=2" "-Wconversion")
list(APPEND SECURITY_FLAGS "-Wimplicit-fallthrough" "-Wformat-security" "-Werror=format-security")
list(APPEND SECURITY_FLAGS "-U_FORTIFY_SOURCE -D_FORTIFY_SOURCE=3" "-D_GLIBCXX_ASSERTIONS")
list(APPEND SECURITY_FLAGS "-fstrict-flex-arrays=3" "-fstack-clash-protection" "-fstack-protector-strong")
list(APPEND SECURITY_FLAGS "-Wtrampolines" "-Wbidi-chars=any" "-fcf-protection=full")
list(APPEND SECURITY_FLAGS "-Werror" "-O2")
include(FetchContent)

list(JOIN SECURITY_FLAGS " " SECURITY_FLAGS_STR)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${SECURITY_FLAGS_STR}")

# Enable hardware-specific optimizations only if explicitly requested or on local dev
if(NOT DEFINED PORTABLE_BUILD)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -march=native")
endif()
# --- Dependencies ---

if(NOT TARGET glaze::glaze)
    FetchContent_Declare(
        glaze
        GIT_REPOSITORY https://github.com/stephenberry/glaze.git
        GIT_TAG        v7.5.0
        GIT_SHALLOW    TRUE
    )
endif()

# rlog (Combined gettext and syslog)
FetchContent_Declare(
    rlog
    GIT_REPOSITORY git@github.com:jeblad/rlog.git
    GIT_TAG        main
    GIT_SHALLOW    TRUE
)

if(NOT TARGET uuid_v4)
    FetchContent_Declare(
        uuid_v4
        GIT_REPOSITORY https://github.com/crashoz/uuid_v4.git
        GIT_TAG        master
        GIT_SHALLOW    TRUE
    )
endif()

# uuid_ext
FetchContent_Declare(
    uuid_ext
    GIT_REPOSITORY git@github.com:jeblad/uuid_ext.git
    GIT_TAG        main
    GIT_SHALLOW    TRUE
)

# Sanitize
FetchContent_Declare(
    sanitize
    GIT_REPOSITORY git@github.com:jeblad/sanitize.git
    GIT_TAG        main
    GIT_SHALLOW    TRUE
)
FetchContent_MakeAvailable(glaze rlog uuid_v4 uuid_ext sanitize)

# Generate local configuration header for generic metadata
configure_file(Config.h.in "${CMAKE_CURRENT_BINARY_DIR}/include/generic/config.h")

# Setup virtual include for uuid_v4 (matching hera structure)
set(UUID_V4_VIRTUAL_DIR "${CMAKE_BINARY_DIR}/virtual_includes")
file(MAKE_DIRECTORY "${UUID_V4_VIRTUAL_DIR}/uuid_v4")
if(NOT EXISTS "${UUID_V4_VIRTUAL_DIR}/uuid_v4/uuid_v4.h")
    file(CREATE_LINK 
      "${uuid_v4_SOURCE_DIR}/uuid_v4.h" 
      "${UUID_V4_VIRTUAL_DIR}/uuid_v4/uuid_v4.h" 
      SYMBOLIC)
endif()

# 3. Handle HERA (The Host System)
if(NOT TARGET hera::hera)
    set(HERA_SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/../hera" CACHE PATH "Path to hera core")
    if(EXISTS "${HERA_SOURCE_DIR}/include")
        add_library(hera_headers INTERFACE)
        add_library(hera::hera ALIAS hera_headers)
        target_include_directories(hera_headers INTERFACE "${HERA_SOURCE_DIR}/include")
        message(STATUS "Generic: Using local Hera headers from: ${HERA_SOURCE_DIR}")
    else()
        message(STATUS "Generic: Hera not found locally. Fetching via SSH...")
        FetchContent_Declare(
            hera
            GIT_REPOSITORY git@github.com:your-org/hera.git
            GIT_TAG main
        )
        FetchContent_MakeAvailable(hera)
    endif()
endif()

# --- Git Hooks ---
# Installs hooks from the /hooks folder to .git/hooks during configuration
if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/.git")
    file(GLOB HOOK_FILES "${CMAKE_CURRENT_SOURCE_DIR}/hooks/*")
    foreach(HOOK_FILE ${HOOK_FILES})
        get_filename_component(HOOK_NAME ${HOOK_FILE} NAME)
        file(COPY ${HOOK_FILE}
             DESTINATION "${CMAKE_CURRENT_SOURCE_DIR}/.git/hooks"
             FILE_PERMISSIONS OWNER_READ OWNER_WRITE OWNER_EXECUTE GROUP_READ GROUP_EXECUTE WORLD_READ WORLD_EXECUTE)
    endforeach()
endif()

# --- Translations (Gettext via Hera helpers) ---
if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/cmake/Gettext_helpers.cmake")
    include("${CMAKE_CURRENT_SOURCE_DIR}/cmake/Gettext_helpers.cmake")
    configure_gettext(
        ALL
        DOMAIN ${PROJECT_NAME}
        TARGET_NAME ${PROJECT_NAME}-translations
        SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/src/generic.cpp"
        POTFILE_DESTINATION "locale"
        LANGUAGES "nn;nb"
        XGETTEXT_ARGS 
            --language=C++
            --from-code=UTF-8
            --keyword=_ --keyword=N_ 
            --keyword=fmt_:1
            --keyword=n_:1,2
            --keyword=ERROR_FMT_:1 --keyword=WARN_FMT_:1 
            --keyword=INFO_FMT_:1 --keyword=DEBUG_FMT_:1
    )
endif()

add_library(generic MODULE 
    src/generic.cpp
    src/clone.cpp
    src/build.cpp
    src/install.cpp
    src/uninstall.cpp
    src/list.cpp
    src/signal.cpp
    src/prune.cpp
    src/about.cpp
)

target_link_libraries(generic 
    PRIVATE 
        glaze::glaze
        rlog
        uuid_v4
        uuid_ext
        sanitize
        hera::hera
)

target_include_directories(generic PRIVATE 
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    "src"
    "${CMAKE_CURRENT_SOURCE_DIR}/include"
    "${CMAKE_CURRENT_BINARY_DIR}/include"
    "${UUID_V4_VIRTUAL_DIR}"
)

target_compile_definitions(generic PRIVATE _GLIBCXX_HOSTED=1)

# Linker flags for security
if(NOT APPLE)
    target_link_options(generic PRIVATE
        "-Wl,-z,relro"
        "-Wl,-z,now"
        "-Wl,-z,noexecstack"
    )
endif()

# --- Testing Setup ---

option(BUILD_TESTING "Build unit tests for the plugin" ON)

if(BUILD_TESTING)
    enable_testing()
    
    add_executable(test_generic 
        tests/test_main.cpp
        tests/test_utility.cpp
        tests/test_about.cpp
        tests/test_clone.cpp
        tests/test_lifecycle.cpp
        tests/test_management.cpp
        src/generic.cpp
        src/clone.cpp
        src/build.cpp
        src/install.cpp
        src/uninstall.cpp
        src/list.cpp
        src/signal.cpp
        src/prune.cpp
        src/about.cpp
    )
    
    target_include_directories(test_generic PRIVATE 
        "src"
        "${CMAKE_CURRENT_SOURCE_DIR}/include"
        "${CMAKE_CURRENT_BINARY_DIR}/include"
        "${UUID_V4_VIRTUAL_DIR}"
    )
    
    target_link_libraries(test_generic PRIVATE 
        glaze::glaze
        rlog
        uuid_v4
        uuid_ext
        sanitize
        hera::hera
    )
    
    target_compile_definitions(test_generic PRIVATE _GLIBCXX_HOSTED=1)
    add_test(NAME generic_api_check COMMAND test_generic)
endif()

set_target_properties(generic PROPERTIES 
    OUTPUT_NAME "generic"
    PREFIX ""
    CXX_VISIBILITY_PRESET hidden
    VISIBILITY_INLINES_HIDDEN ON
)
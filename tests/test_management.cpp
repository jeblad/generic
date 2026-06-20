/**
 * generic – tests for agent management subcommands
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
#include <sstream>
#include <cassert>
#include <cerrno>
#include <csignal>
#include <filesystem>
#include <fstream>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include "test_generic.hpp"
#include "generic/internal.hpp"
#include "hera/metadata.hpp"

void test_hera_list_logic() {
    std::cout << "Testing hera_list_logic..." << std::endl;
    namespace fs = std::filesystem;
    ScopedTestDir env;
    fs::path lib_dir = env.path / "test_lib";
    fs::create_directories(lib_dir);
    
    // List should work even if the directory is empty
    assert(hera::list(lib_dir.string().c_str(), nullptr, nullptr, nullptr, 0, 0).code == 0);
}

void test_list_json_format() {
    std::cout << "Testing list JSON format output..." << std::endl;
    namespace fs = std::filesystem;
    ScopedTestDir env;
    fs::path lib_dir = env.path / "test_lib";
    fs::create_directories(lib_dir);

    // Create a minimal .beve agent file
    hera::MetaData meta;
    meta.type = "metadata";
    meta.nickname = "TestAgent";
    std::string buf;
    assert(!glz::write_beve(meta, buf));
    std::ofstream ofs(lib_dir / "test-agent.beve", std::ios::binary);
    ofs.write(buf.data(), static_cast<std::streamsize>(buf.size()));
    ofs.close();

    std::ostringstream captured;
    std::streambuf* old_buf = std::cout.rdbuf(captured.rdbuf());
    hera::list(lib_dir.string().c_str(), nullptr, nullptr, nullptr, 0, HERA_FORMAT_JSON);
    std::cout.rdbuf(old_buf);

    glz::generic parsed;
    auto ec = glz::read_json(parsed, captured.str());
    assert(!ec);
}

void test_list_yaml_format() {
    std::cout << "Testing list YAML format output..." << std::endl;
    namespace fs = std::filesystem;
    ScopedTestDir env;
    fs::path lib_dir = env.path / "test_lib";
    fs::create_directories(lib_dir);

    // Create a minimal .beve agent file
    hera::MetaData meta;
    meta.type = "metadata";
    meta.nickname = "TestAgent";
    std::string buf;
    assert(!glz::write_beve(meta, buf));
    std::ofstream ofs(lib_dir / "test-agent.beve", std::ios::binary);
    ofs.write(buf.data(), static_cast<std::streamsize>(buf.size()));
    ofs.close();

    std::ostringstream captured;
    std::streambuf* old_buf = std::cout.rdbuf(captured.rdbuf());
    hera::list(lib_dir.string().c_str(), nullptr, nullptr, nullptr, 0, HERA_FORMAT_YAML);
    std::cout.rdbuf(old_buf);

    // Every non-empty line must start with "- " or "  key: value"
    std::istringstream lines(captured.str());
    std::string line;
    while (std::getline(lines, line)) {
        if (line.empty()) continue;
        assert(line.substr(0, 2) == "- " || line.substr(0, 2) == "  ");
    }
}

void test_hera_prune_logic() {
    std::cout << "Testing hera_prune_logic..." << std::endl;
    namespace fs = std::filesystem;
    ScopedTestDir env;
    fs::path state_dir = env.path / "test_state";
    fs::path run_dir   = env.path / "test_run";
    fs::path cache_dir = env.path / "test_cache";
    fs::create_directories(state_dir);
    fs::create_directories(run_dir);
    fs::create_directories(cache_dir);
    // Empty state_dir → no BEVE anchor → prune returns success immediately
    assert(hera::prune(state_dir.string().c_str(), run_dir.string().c_str(), cache_dir.string().c_str(), nullptr, 0, 0).code == 0);
}

void test_hera_signal_logic() {
    std::cout << "Testing hera_signal_logic..." << std::endl;
    ScopedTestDir env;
    // This will fail without an actual running process, 
    // so we only test that it handles missing PID files correctly.
    assert(hera::signal(env.path.c_str(), env.path.c_str(), "non-existent", 0, HERA_SIGTERM).code == 1);
}


void test_hera_signal_stop_cont_logic() {
    std::cout << "Testing hera_signal_stop_cont_logic..." << std::endl;
    ScopedTestDir env;
    // Verifies flag handling
    assert(hera::signal(env.path.c_str(), env.path.c_str(), "none", 0, HERA_SIGSTOP).code == 1);
    assert(hera::signal(env.path.c_str(), env.path.c_str(), "none", 0, HERA_SIGCONT).code == 1);
}

void test_hera_down_logic() {
    std::cout << "Testing hera_down_logic..." << std::endl;
    namespace fs = std::filesystem;

    // Auto-reap children so kill(pid, 0) returns ESRCH once the child dies.
    // Without this, a terminated child becomes a zombie and kill(pid, 0) still
    // returns 0, making the polling loop in down() think the process is alive.
    signal(SIGCHLD, SIG_IGN);

    auto write_pid_file = [](const fs::path& run_dir, const std::string& stem, pid_t pid) {
        hera::PidFileContent info;
        info.pid       = static_cast<int>(pid);
        info.beve_path = (run_dir / (stem + ".beve")).string();
        info.mmio_path = (run_dir / (stem + ".mmio")).string();
        info.status    = "running";
        std::string json;
        assert(!glz::write_json(info, json));
        std::ofstream ofs(run_dir / (stem + ".pid"));
        ofs << json;
    };

    // Part 1: No PID file present → down() must fail immediately
    {
        ScopedTestDir env;
        fs::path run_dir = env.path / "run";
        fs::create_directories(run_dir);
        assert(hera::down("", run_dir.string().c_str(), "non-existent", 0, 0).code != 0);
    }

    // Part 2: Process exits gracefully on SIGTERM → code 0, PID file removed
    {
        ScopedTestDir env;
        fs::path run_dir = env.path / "run";
        fs::create_directories(run_dir);

        pid_t pid = fork();
        assert(pid >= 0);
        if (pid == 0) {
            // Child: block until a signal arrives; default SIGTERM handler terminates.
            pause();
            _exit(0);
        }
        write_pid_file(run_dir, "graceful-agent", pid);

        auto r = hera::down("", run_dir.string().c_str(), "graceful-agent", 0, 0);
        assert(r.code == 0);
        assert(!fs::exists(run_dir / "graceful-agent.pid"));
    }

    // Part 3: Process ignores SIGTERM, no --force → error after 5 s wait; PID file remains
    // NOTE: slow test — waits the full 5-second timeout in down()
    {
        ScopedTestDir env;
        fs::path run_dir = env.path / "run";
        fs::create_directories(run_dir);

        pid_t pid = fork();
        assert(pid >= 0);
        if (pid == 0) {
            signal(SIGTERM, SIG_IGN);
            pause(); // won't be interrupted by SIGTERM; only SIGKILL can stop us
            _exit(0);
        }
        write_pid_file(run_dir, "stubborn-agent", pid);

        auto r = hera::down("", run_dir.string().c_str(), "stubborn-agent", 0, 0);
        assert(r.code != 0);
        assert(fs::exists(run_dir / "stubborn-agent.pid"));
        ::kill(pid, SIGKILL); // clean up child
    }

    // Part 4: Process ignores SIGTERM, --force → SIGKILL, code 0, PID file removed
    // NOTE: slow test — waits the full 5-second timeout before escalating to SIGKILL
    {
        ScopedTestDir env;
        fs::path run_dir = env.path / "run";
        fs::create_directories(run_dir);

        pid_t pid = fork();
        assert(pid >= 0);
        if (pid == 0) {
            signal(SIGTERM, SIG_IGN);
            pause();
            _exit(0);
        }
        write_pid_file(run_dir, "stubborn-agent", pid);

        auto r = hera::down("", run_dir.string().c_str(), "stubborn-agent", 0, HERA_FORCE);
        assert(r.code == 0);
        assert(!fs::exists(run_dir / "stubborn-agent.pid"));
    }
}
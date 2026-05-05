/**
 * Unit tests for the generic plugin
 **/

#include <iostream>
#include <cassert>
#include <string>
#include <string_view>
#include <array>
#include <sstream>
#include <fstream>
#include <filesystem>
#include "hera/hera.hpp"
#include "generic/config.h"

namespace hera {
    // Prototypes for internal logic to bypass rlog wrappers
    Result about_system(int flags);
    Result about_plugin(int flags);
    Result about_module(const char* filename, int flags);
}

// Deklarasjon av hjelpefunksjon fra generic.cpp
size_t parse_list(std::string_view raw, std::string_view* out, size_t max_size);

extern "C" {
    uint32_t hera_api();
}

void test_hera_api_version() {
    std::cout << "Checking hera_api() version..." << std::endl;
    assert(hera_api() == HERA_API_LEVEL);
}

void test_parse_list() {
    std::cout << "Testing parse_list helper..." << std::endl;
    std::string_view raw = "  Alpha ; Beta;Gamma  ; Delta \n ; ";
    std::array<std::string_view, 10> out;
    size_t count = parse_list(raw, out.data(), out.size());
    assert(count == 4);
    assert(out[0] == "Alpha");
    assert(out[1] == "Beta");
    assert(out[2] == "Gamma");
    assert(out[3] == "Delta");
}

void test_hera_about_logic() {
    std::cout << "Testing hera_about_plugin output..." << std::endl;

    // Omdiriger stdout for å fange utskrift
    std::stringstream buffer;
    std::streambuf* old = std::cout.rdbuf(buffer.rdbuf());

    // Case 1: Full rapport (HERA_REPORT_ABOUT)
    auto result = hera::about_plugin(HERA_REPORT_ABOUT);
    assert(result.code == 0);
    std::string output = buffer.str();

    // Verifiser at metadata-makroene finnes i utskriften
    assert(output.find(GENERIC_DESCRIPTION) != std::string::npos);
    assert(output.find(GENERIC_CREATOR) != std::string::npos);
    assert(output.find(GENERIC_HOMEPAGE) != std::string::npos);

    // Sjekk versjonsnummer (bygget fra major/minor)
    std::string version_check = std::to_string(GENERIC_VERSION_MAJOR) + "." + std::to_string(GENERIC_VERSION_MINOR);
    assert(output.find(version_check) != std::string::npos);

    // Sjekk bidragsytere (vi vet at Gemini AI er i lista i CMakeLists.txt)
    assert(output.find("Gemini AI") != std::string::npos);

    // Case 2: Begrenset rapport (kun versjon)
    buffer.str("");
    buffer.clear();
    result = hera::about_plugin(HERA_REPORT_VERSION);
    assert(result.code == 0);
    output = buffer.str();

    // Skal inneholde versjon
    assert(output.find(version_check) != std::string::npos);
    // Skal IKKE inneholde beskrivelse eller skaper når flaggene mangler
    assert(output.find("Description:") == std::string::npos);
    assert(output.find("Creator:") == std::string::npos);
    assert(output.find("Contributors:") == std::string::npos);
    assert(output.find(GENERIC_DESCRIPTION) == std::string::npos);
    assert(output.find(GENERIC_CREATOR) == std::string::npos);

    // Case 3: System-rapport (HERA_REPORT_ABOUT for host)
    buffer.str("");
    buffer.clear();
    result = hera::about_system(HERA_REPORT_ABOUT);
    assert(result.code == 0);
    output = buffer.str();
    
    // Verifiser at system-metadata (Hera) dukker opp
    // Vi vet at HERA_DESCRIPTION er definert i hera/config.h
    assert(output.find("Description:") != std::string::npos);
    assert(output.find(HERA_DESCRIPTION) != std::string::npos);
    assert(output.find("Version:") != std::string::npos);
    assert(output.find(HERA_VERSION) != std::string::npos);
    assert(output.find("API level:") != std::string::npos);
    assert(output.find(std::to_string(HERA_API_LEVEL)) != std::string::npos);

    // Gjenopprett stdout
    std::cout.rdbuf(old);
}

void test_hera_about_module_logic() {
    std::cout << "Testing hera_about_module output..." << std::endl;

    const char* test_fn = "test_agent.json";
    {
        std::ofstream ofs(test_fn);
        ofs << R"({
            "meta": {
                "uuid": "module-uuid",
                "description": "Test module description",
                "model": "TestModel-v1",
                "epoch": 123,
                "contributors": ["Contributor A", "Contributor B"]
            }
        })";
    }

    std::stringstream buffer;
    std::streambuf* old = std::cout.rdbuf(buffer.rdbuf());

    // Kall om om modulen (agenthodet)
    auto result = hera::about_module(test_fn, HERA_REPORT_ABOUT);
    assert(result.code == 0);
    std::string output = buffer.str();

    // Verifiser innholdet
    assert(output.find("Description:") != std::string::npos);
    assert(output.find("Test module description") != std::string::npos);
    assert(output.find("Model:") != std::string::npos);
    assert(output.find("TestModel-v1") != std::string::npos);
    assert(output.find("Epoch:") != std::string::npos);
    assert(output.find("123") != std::string::npos);
    assert(output.find("Contributors:") != std::string::npos);
    assert(output.find("Contributor A") != std::string::npos);
    assert(output.find("Contributor B") != std::string::npos);

    std::cout.rdbuf(old);
    std::filesystem::remove(test_fn);
}

int main() {
    test_hera_api_version();
    test_parse_list();
    test_hera_about_logic();
    test_hera_about_module_logic();
    std::cout << "All generic API tests passed!" << std::endl;
    return 0;
}
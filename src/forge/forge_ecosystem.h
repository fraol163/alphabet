#ifndef ALPHABET_FORGE_ECOSYSTEM_H
#define ALPHABET_FORGE_ECOSYSTEM_H

#include "forge_spec.h"
#include <string>
#include <vector>

namespace alphabet {
namespace forge {

struct TestSummary {
    int total = 0;
    int passed = 0;
    int failed = 0;
    double duration_ms = 0.0;
    std::vector<std::string> failures;
};

class ForgeEcosystem {
public:
    // 1. VS Code Extension Generator
    static bool export_vscode_extension(const ForgeSpec& spec, const std::string& output_dir, std::string& out_error);

    // 2. Built-in Test Runner
    static TestSummary run_tests(const ForgeSpec& spec, const std::string& path);

    // 3. Source Formatter
    static std::string format_source(const std::string& source, const ForgeSpec& spec);
    static bool format_file(const std::string& filepath, const ForgeSpec& spec, std::string& out_error);

    // 4. Package Manager
    static bool pkg_init(const ForgeSpec& spec, const std::string& project_dir, std::string& out_error);
    static bool pkg_install(const ForgeSpec& spec, const std::string& package_name, std::string& out_error);
    static bool pkg_list(const ForgeSpec& spec, const std::string& project_dir);

    // 5. Language Project Scaffolding Wizard
    static bool init_project(const std::string& lang_name, const std::string& target_dir, const std::string& style, std::string& out_error);

    // 6. Distribution Bundle Exporter
    static bool bundle_distribution(const ForgeSpec& spec, const std::string& output_dir, std::string& out_error);
};

} // namespace forge
} // namespace alphabet

#endif // ALPHABET_FORGE_ECOSYSTEM_H

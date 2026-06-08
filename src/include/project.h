#pragma once

#include <map>
#include <string>
#include <vector>

namespace alphabet {

struct ProjectConfig {
    std::string name;
    std::string version;
    std::string language = "en";
    std::string entry = "main.abc";
    std::map<std::string, std::string> dependencies;
    std::vector<std::string> source_dirs;
    std::vector<std::string> test_dirs;
};

class ProjectManager {
  public:
    // Load project config from alphabet.toml
    static ProjectConfig load(const std::string& path = "alphabet.toml");

    // Check if alphabet.toml exists
    static bool exists(const std::string& path = "alphabet.toml");

    // Resolve dependency path
    static std::string resolve_dep(const std::string& name, const ProjectConfig& config);

    // Get all source files in project
    static std::vector<std::string> get_source_files(const ProjectConfig& config);

    // Get all test files in project
    static std::vector<std::string> get_test_files(const ProjectConfig& config);

    // Print project info
    static void print_info(const ProjectConfig& config);

  private:
    // Simple TOML parser (key = "value" format)
    static std::map<std::string, std::string> parse_toml(const std::string& content);

    // Parse section [name]
    static std::string parse_section(const std::string& line);
};

} // namespace alphabet

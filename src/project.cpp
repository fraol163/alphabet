#include "project.h"
#include <algorithm>
#include <dirent.h>
#include <fstream>
#include <iostream>
#include <sstream>
#include <sys/stat.h>

namespace alphabet {

ProjectConfig ProjectManager::load(const std::string& path) {
    ProjectConfig config;

    std::ifstream file(path);
    if (!file.good()) {
        return config;
    }

    std::ostringstream oss;
    oss << file.rdbuf();
    std::string content = oss.str();

    std::string current_section;
    std::istringstream stream(content);
    std::string line;

    while (std::getline(stream, line)) {
        // Trim whitespace
        line.erase(0, line.find_first_not_of(" \t"));
        line.erase(line.find_last_not_of(" \t\r\n") + 1);

        // Skip empty lines and comments
        if (line.empty() || line[0] == '#') {
            continue;
        }

        // Section header
        if (line[0] == '[' && line.back() == ']') {
            current_section = line.substr(1, line.size() - 2);
            continue;
        }

        // Key = value
        auto eq_pos = line.find('=');
        if (eq_pos == std::string::npos) {
            continue;
        }

        std::string key = line.substr(0, eq_pos);
        std::string value = line.substr(eq_pos + 1);

        // Trim key and value
        key.erase(0, key.find_first_not_of(" \t"));
        key.erase(key.find_last_not_of(" \t") + 1);
        value.erase(0, value.find_first_not_of(" \t"));
        value.erase(value.find_last_not_of(" \t\r\n") + 1);

        // Remove quotes from value
        if (value.size() >= 2 && value.front() == '"' && value.back() == '"') {
            value = value.substr(1, value.size() - 2);
        }

        if (current_section == "project") {
            if (key == "name")
                config.name = value;
            else if (key == "version")
                config.version = value;
            else if (key == "language")
                config.language = value;
            else if (key == "entry")
                config.entry = value;
        } else if (current_section == "dependencies") {
            config.dependencies[key] = value;
        } else if (current_section == "sources") {
            config.source_dirs.push_back(value);
        } else if (current_section == "tests") {
            config.test_dirs.push_back(value);
        }
    }

    // Defaults
    if (config.source_dirs.empty()) {
        config.source_dirs.push_back("src");
    }
    if (config.test_dirs.empty()) {
        config.test_dirs.push_back("tests");
    }

    return config;
}

bool ProjectManager::exists(const std::string& path) {
    std::ifstream file(path);
    return file.good();
}

std::string ProjectManager::resolve_dep(const std::string& name, const ProjectConfig& config) {
    auto it = config.dependencies.find(name);
    if (it != config.dependencies.end()) {
        return it->second;
    }

    // Try stdlib
    std::string stdlib_path = "stdlib/" + name + ".abc";
    std::ifstream f(stdlib_path);
    if (f.good()) {
        return stdlib_path;
    }

    return "";
}

std::vector<std::string> ProjectManager::get_source_files(const ProjectConfig& config) {
    std::vector<std::string> files;
    for (const auto& dir : config.source_dirs) {
        DIR* d = opendir(dir.c_str());
        if (d) {
            struct dirent* entry;
            while ((entry = readdir(d)) != nullptr) {
                std::string name = entry->d_name;
                if (name.size() > 4 && name.substr(name.size() - 4) == ".abc") {
                    files.push_back(dir + "/" + name);
                }
            }
            closedir(d);
        }
    }
    // Also check entry point
    if (!config.entry.empty()) {
        std::ifstream f(config.entry);
        if (f.good()) {
            files.push_back(config.entry);
        }
    }
    return files;
}

std::vector<std::string> ProjectManager::get_test_files(const ProjectConfig& config) {
    std::vector<std::string> files;
    for (const auto& dir : config.test_dirs) {
        DIR* d = opendir(dir.c_str());
        if (d) {
            struct dirent* entry;
            while ((entry = readdir(d)) != nullptr) {
                std::string name = entry->d_name;
                if (name.size() > 4 && name.substr(name.size() - 4) == ".abc") {
                    files.push_back(dir + "/" + name);
                }
            }
            closedir(d);
        }
    }
    return files;
}

void ProjectManager::print_info(const ProjectConfig& config) {
    std::cout << "Project: " << (config.name.empty() ? "(unnamed)" : config.name) << "\n";
    if (!config.version.empty()) {
        std::cout << "Version: " << config.version << "\n";
    }
    std::cout << "Language: " << config.language << "\n";
    std::cout << "Entry: " << config.entry << "\n";

    if (!config.dependencies.empty()) {
        std::cout << "Dependencies:\n";
        for (const auto& [name, path] : config.dependencies) {
            std::cout << "  " << name << " = " << path << "\n";
        }
    }

    auto sources = get_source_files(config);
    std::cout << "Source files: " << sources.size() << "\n";

    auto tests = get_test_files(config);
    std::cout << "Test files: " << tests.size() << "\n";
}

std::map<std::string, std::string> ProjectManager::parse_toml(const std::string& content) {
    std::map<std::string, std::string> result;
    // Simple implementation — handled in load()
    return result;
}

std::string ProjectManager::parse_section(const std::string& line) {
    if (line.size() >= 2 && line.front() == '[' && line.back() == ']') {
        return line.substr(1, line.size() - 2);
    }
    return "";
}

} // namespace alphabet

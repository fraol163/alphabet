#include "forge_ecosystem.h"
#include "forge.h"
#include "banner_engine.h"
#include "forge_doc.h"
#include "forge_playground.h"
#include <iostream>
#include <fstream>
#include <sstream>
#include <filesystem>
#include <chrono>
#include <algorithm>
#include <cctype>
#include <iomanip>

namespace alphabet {
namespace forge {

namespace {

std::string to_lower_str(std::string s) {
    std::transform(s.begin(), s.end(), s.begin(), [](unsigned char c) {
        return std::tolower(c);
    });
    return s;
}

std::string escape_json_str(const std::string& s) {
    std::string res;
    for (char c : s) {
        if (c == '"') res += "\\\"";
        else if (c == '\\') res += "\\\\";
        else if (c == '\n') res += "\\n";
        else if (c == '\r') res += "\\r";
        else if (c == '\t') res += "\\t";
        else res += c;
    }
    return res;
}

std::string escape_regex_str(const std::string& s) {
    std::string res;
    for (char c : s) {
        if (c == '^' || c == '$' || c == '.' || c == '*' || c == '+' || 
            c == '?' || c == '(' || c == ')' || c == '[' || c == ']' || 
            c == '{' || c == '}' || c == '|' || c == '\\' || c == '/') {
            res += '\\';
        }
        res += c;
    }
    return res;
}

} // namespace

bool ForgeEcosystem::export_vscode_extension(const ForgeSpec& spec, 
                                             const std::string& output_dir, 
                                             std::string& out_error) {
    std::filesystem::path base_path(output_dir.empty() ? (to_lower_str(spec.name) + "-vscode") : output_dir);
    std::filesystem::path syntaxes_path = base_path / "syntaxes";
    std::error_code ec;
    std::filesystem::create_directories(syntaxes_path, ec);
    if (ec) {
        out_error = "Failed to create directory: " + ec.message();
        return false;
    }

    std::string lang_id = to_lower_str(spec.name);
    std::string ext = spec.extension.empty() ? ".lang" : spec.extension;
    if (ext[0] != '.') ext = "." + ext;

    // 1. package.json
    {
        std::ofstream pkg_out(base_path / "package.json");
        if (!pkg_out.is_open()) {
            out_error = "Failed to write package.json";
            return false;
        }
        pkg_out << "{\n";
        pkg_out << "  \"name\": \"" << lang_id << "-language\",\n";
        pkg_out << "  \"displayName\": \"" << spec.name << " Language Support\",\n";
        pkg_out << "  \"description\": \"Comprehensive syntax highlighting and tools for " << spec.name << "\",\n";
        pkg_out << "  \"version\": \"" << spec.version << "\",\n";
        pkg_out << "  \"publisher\": \"" << (spec.author.empty() ? "Alphabet Forge" : escape_json_str(spec.author)) << "\",\n";
        pkg_out << "  \"engines\": {\n";
        pkg_out << "    \"vscode\": \"^1.75.0\"\n";
        pkg_out << "  },\n";
        pkg_out << "  \"categories\": [\n";
        pkg_out << "    \"Programming Languages\"\n";
        pkg_out << "  ],\n";
        pkg_out << "  \"contributes\": {\n";
        pkg_out << "    \"languages\": [\n";
        pkg_out << "      {\n";
        pkg_out << "        \"id\": \"" << lang_id << "\",\n";
        pkg_out << "        \"aliases\": [\"" << spec.name << "\", \"" << lang_id << "\"],\n";
        pkg_out << "        \"extensions\": [\"" << ext << "\"],\n";
        pkg_out << "        \"configuration\": \"./language-configuration.json\"\n";
        pkg_out << "      }\n";
        pkg_out << "    ],\n";
        pkg_out << "    \"grammars\": [\n";
        pkg_out << "      {\n";
        pkg_out << "        \"language\": \"" << lang_id << "\",\n";
        pkg_out << "        \"scopeName\": \"source." << lang_id << "\",\n";
        pkg_out << "        \"path\": \"./syntaxes/" << lang_id << ".tmLanguage.json\"\n";
        pkg_out << "      }\n";
        pkg_out << "    ]\n";
        pkg_out << "  }\n";
        pkg_out << "}\n";
    }

    // 2. language-configuration.json
    {
        std::ofstream cfg_out(base_path / "language-configuration.json");
        if (!cfg_out.is_open()) {
            out_error = "Failed to write language-configuration.json";
            return false;
        }
        cfg_out << "{\n";
        cfg_out << "  \"comments\": {\n";
        if (!spec.tokens.line_comment.empty()) {
            cfg_out << "    \"lineComment\": \"" << escape_json_str(spec.tokens.line_comment) << "\"";
            if (!spec.tokens.block_comment_start.empty()) cfg_out << ",\n";
            else cfg_out << "\n";
        }
        if (!spec.tokens.block_comment_start.empty()) {
            cfg_out << "    \"blockComment\": [\"" 
                    << escape_json_str(spec.tokens.block_comment_start) << "\", \"" 
                    << escape_json_str(spec.tokens.block_comment_end) << "\"]\n";
        }
        cfg_out << "  },\n";
        cfg_out << "  \"brackets\": [\n";
        cfg_out << "    [\"{\", \"}\"],\n";
        cfg_out << "    [\"[\", \"]\"],\n";
        cfg_out << "    [\"(\", \")\"]\n";
        cfg_out << "  ],\n";
        cfg_out << "  \"autoClosingPairs\": [\n";
        cfg_out << "    { \"open\": \"{\", \"close\": \"}\" },\n";
        cfg_out << "    { \"open\": \"[\", \"close\": \"]\" },\n";
        cfg_out << "    { \"open\": \"(\", \"close\": \")\" },\n";
        cfg_out << "    { \"open\": \"\\\"\", \"close\": \"\\\"\" }\n";
        cfg_out << "  ],\n";
        cfg_out << "  \"surroundingPairs\": [\n";
        cfg_out << "    { \"open\": \"{\", \"close\": \"}\" },\n";
        cfg_out << "    { \"open\": \"[\", \"close\": \"]\" },\n";
        cfg_out << "    { \"open\": \"(\", \"close\": \")\" },\n";
        cfg_out << "    { \"open\": \"\\\"\", \"close\": \"\\\"\" }\n";
        cfg_out << "  ]\n";
        cfg_out << "}\n";
    }

    // 3. syntaxes/<lang>.tmLanguage.json
    {
        std::ofstream tm_out(syntaxes_path / (lang_id + ".tmLanguage.json"));
        if (!tm_out.is_open()) {
            out_error = "Failed to write tmLanguage.json";
            return false;
        }

        // Categorize keywords
        std::vector<std::string> control_kws;
        std::vector<std::string> type_kws;
        std::vector<std::string> fn_kws;
        std::vector<std::string> other_kws;

        for (const auto& kv : spec.tokens.keywords) {
            const std::string& kw = kv.first;
            const std::string& target = kv.second;
            if (target == "IF" || target == "ELSE" || target == "WHILE" || target == "FOR" || 
                target == "RETURN" || target == "BREAK" || target == "CONTINUE") {
                control_kws.push_back(kw);
            } else if (target == "VAR" || target == "LET" || target == "CONST") {
                type_kws.push_back(kw);
            } else if (target == "FUNCTION" || target == "FN" || target == "DEF" || target == "ACTION") {
                fn_kws.push_back(kw);
            } else {
                other_kws.push_back(kw);
            }
        }

        auto make_kw_regex = [](const std::vector<std::string>& list) -> std::string {
            if (list.empty()) return "";
            std::string pat = "\\\\b(";
            for (size_t i = 0; i < list.size(); ++i) {
                if (i > 0) pat += "|";
                pat += escape_regex_str(list[i]);
            }
            pat += ")\\\\b";
            return pat;
        };

        std::string control_pat = make_kw_regex(control_kws);
        std::string type_pat = make_kw_regex(type_kws);
        std::string fn_pat = make_kw_regex(fn_kws);
        std::string other_pat = make_kw_regex(other_kws);

        tm_out << "{\n";
        tm_out << "  \"$schema\": \"https://raw.githubusercontent.com/martinring/tmlanguage/master/tmlanguage.json\",\n";
        tm_out << "  \"name\": \"" << spec.name << "\",\n";
        tm_out << "  \"scopeName\": \"source." << lang_id << "\",\n";
        tm_out << "  \"patterns\": [\n";
        tm_out << "    { \"include\": \"#header\" },\n";
        tm_out << "    { \"include\": \"#comments\" },\n";
        tm_out << "    { \"include\": \"#strings\" },\n";
        tm_out << "    { \"include\": \"#numbers\" },\n";
        tm_out << "    { \"include\": \"#control-keywords\" },\n";
        tm_out << "    { \"include\": \"#type-keywords\" },\n";
        tm_out << "    { \"include\": \"#function-keywords\" },\n";
        tm_out << "    { \"include\": \"#other-keywords\" },\n";
        tm_out << "    { \"include\": \"#operators\" }\n";
        tm_out << "  ],\n";
        tm_out << "  \"repository\": {\n";

        // Header pattern
        tm_out << "    \"header\": {\n";
        if (spec.header.style == HeaderStyle::PREFIX && !spec.header.prefix.empty()) {
            tm_out << "      \"match\": \"^\\\\s*(" << escape_regex_str(spec.header.prefix) << ".*)$\",\n";
            tm_out << "      \"name\": \"keyword.directive." << lang_id << "\"\n";
        } else if (spec.header.style == HeaderStyle::SHEBANG) {
            tm_out << "      \"match\": \"^#!.*$\",\n";
            tm_out << "      \"name\": \"comment.line.shebang." << lang_id << "\"\n";
        } else {
            tm_out << "      \"match\": \"(?=a)b\",\n";
            tm_out << "      \"name\": \"meta.header\"\n";
        }
        tm_out << "    },\n";

        // Comments pattern
        tm_out << "    \"comments\": {\n";
        tm_out << "      \"patterns\": [\n";
        if (!spec.tokens.line_comment.empty()) {
            tm_out << "        {\n";
            tm_out << "          \"name\": \"comment.line." << lang_id << "\",\n";
            tm_out << "          \"match\": \"" << escape_regex_str(spec.tokens.line_comment) << ".*$\"\n";
            tm_out << "        }";
            if (!spec.tokens.block_comment_start.empty()) tm_out << ",\n";
            else tm_out << "\n";
        }
        if (!spec.tokens.block_comment_start.empty()) {
            tm_out << "        {\n";
            tm_out << "          \"name\": \"comment.block." << lang_id << "\",\n";
            tm_out << "          \"begin\": \"" << escape_regex_str(spec.tokens.block_comment_start) << "\",\n";
            tm_out << "          \"end\": \"" << escape_regex_str(spec.tokens.block_comment_end) << "\"\n";
            tm_out << "        }\n";
        }
        tm_out << "      ]\n";
        tm_out << "    },\n";

        // Strings
        tm_out << "    \"strings\": {\n";
        tm_out << "      \"name\": \"string.quoted.double." << lang_id << "\",\n";
        tm_out << "      \"begin\": \"\\\"\",\n";
        tm_out << "      \"end\": \"\\\"\",\n";
        tm_out << "      \"patterns\": [\n";
        tm_out << "        { \"name\": \"constant.character.escape." << lang_id << "\", \"match\": \"\\\\\\\\.\" }\n";
        tm_out << "      ]\n";
        tm_out << "    },\n";

        // Numbers
        tm_out << "    \"numbers\": {\n";
        tm_out << "      \"name\": \"constant.numeric." << lang_id << "\",\n";
        tm_out << "      \"match\": \"\\\\b\\\\d+(\\\\.\\\\d+)?\\\\b\"\n";
        tm_out << "    },\n";

        // Control Keywords
        tm_out << "    \"control-keywords\": {\n";
        if (!control_pat.empty()) {
            tm_out << "      \"name\": \"keyword.control." << lang_id << "\",\n";
            tm_out << "      \"match\": \"" << control_pat << "\"\n";
        } else {
            tm_out << "      \"match\": \"(?=a)b\"\n";
        }
        tm_out << "    },\n";

        // Type Keywords
        tm_out << "    \"type-keywords\": {\n";
        if (!type_pat.empty()) {
            tm_out << "      \"name\": \"storage.type." << lang_id << "\",\n";
            tm_out << "      \"match\": \"" << type_pat << "\"\n";
        } else {
            tm_out << "      \"match\": \"(?=a)b\"\n";
        }
        tm_out << "    },\n";

        // Function Keywords
        tm_out << "    \"function-keywords\": {\n";
        if (!fn_pat.empty()) {
            tm_out << "      \"name\": \"storage.type.function." << lang_id << "\",\n";
            tm_out << "      \"match\": \"" << fn_pat << "\"\n";
        } else {
            tm_out << "      \"match\": \"(?=a)b\"\n";
        }
        tm_out << "    },\n";

        // Other Keywords
        tm_out << "    \"other-keywords\": {\n";
        if (!other_pat.empty()) {
            tm_out << "      \"name\": \"keyword.other." << lang_id << "\",\n";
            tm_out << "      \"match\": \"" << other_pat << "\"\n";
        } else {
            tm_out << "      \"match\": \"(?=a)b\"\n";
        }
        tm_out << "    },\n";

        // Operators
        tm_out << "    \"operators\": {\n";
        tm_out << "      \"name\": \"keyword.operator." << lang_id << "\",\n";
        tm_out << "      \"match\": \"(==|!=|<=|>=|=>|:=|\\\\+|-|\\\\*|/|%|=|<|>|!|\\\\&\\\\&|\\\\|\\\\|)\"\n";
        tm_out << "    }\n";

        tm_out << "  }\n";
        tm_out << "}\n";
    }

    // 4. README.md
    {
        std::ofstream doc_out(base_path / "README.md");
        if (doc_out.is_open()) {
            doc_out << "# " << spec.name << " VS Code Extension\n\n";
            doc_out << "Official Visual Studio Code extension for **" << spec.name << "**.\n\n";
            doc_out << "Generated automatically with [Alphabet Forge](https://github.com/fraol163/alphabet).\n\n";
            doc_out << "## Features\n\n";
            doc_out << "- Complete syntax highlighting for `" << ext << "` files.\n";
            doc_out << "- Auto-closing brackets, quotes, and indentation rules.\n";
            doc_out << "- Support for " << spec.name << " comments and custom operators.\n\n";
            doc_out << "## Installation\n\n";
            doc_out << "Copy this directory to your VS Code extensions folder:\n";
            doc_out << "```bash\n";
            doc_out << "cp -r " << base_path.string() << " ~/.vscode/extensions/" << lang_id << "-language\n";
            doc_out << "```\n";
        }
    }

    return true;
}

TestSummary ForgeEcosystem::run_tests(const ForgeSpec& spec, const std::string& path) {
    TestSummary summary;
    std::vector<std::string> test_files;

    std::filesystem::path target_path(path.empty() ? "tests" : path);
    if (!std::filesystem::exists(target_path)) {
        // Check current directory
        target_path = ".";
    }

    std::string ext = spec.extension.empty() ? ".lang" : spec.extension;
    if (ext[0] != '.') ext = "." + ext;

    if (std::filesystem::is_regular_file(target_path)) {
        test_files.push_back(target_path.string());
    } else if (std::filesystem::is_directory(target_path)) {
        for (const auto& entry : std::filesystem::recursive_directory_iterator(target_path)) {
            if (entry.is_regular_file()) {
                std::string p = entry.path().string();
                if (p.size() >= ext.size() && p.substr(p.size() - ext.size()) == ext) {
                    test_files.push_back(p);
                }
            }
        }
    }

    std::sort(test_files.begin(), test_files.end());

    std::cout << "\n\033[1mRunning " << spec.name << " Test Suite (" << test_files.size() << " test files)\033[0m\n";
    std::cout << "──────────────────────────────────────────────────────\n";

    auto total_start = std::chrono::high_resolution_clock::now();

    for (const auto& file : test_files) {
        summary.total++;
        auto start = std::chrono::high_resolution_clock::now();

        bool ok = ForgeRunner::run_file(file, spec.spec_source_path, false);

        auto end = std::chrono::high_resolution_clock::now();
        double ms = std::chrono::duration<double, std::milli>(end - start).count();

        if (ok) {
            summary.passed++;
            std::cout << "  \033[32m✔ PASS\033[0m  " << file 
                      << " \033[90m(" << std::fixed << std::setprecision(1) << ms << "ms)\033[0m\n";
        } else {
            summary.failed++;
            summary.failures.push_back(file);
            std::cout << "  \033[31m✘ FAIL\033[0m  " << file 
                      << " \033[90m(" << std::fixed << std::setprecision(1) << ms << "ms)\033[0m\n";
        }
    }

    auto total_end = std::chrono::high_resolution_clock::now();
    summary.duration_ms = std::chrono::duration<double, std::milli>(total_end - total_start).count();

    std::cout << "──────────────────────────────────────────────────────\n";
    if (summary.failed == 0) {
        std::cout << "\033[1;32mTEST SUITE PASSED\033[0m: " << summary.passed << "/" << summary.total 
                  << " passed in " << std::fixed << std::setprecision(2) << summary.duration_ms << "ms\n\n";
    } else {
        std::cout << "\033[1;31mTEST SUITE FAILED\033[0m: " << summary.failed << " failed, " 
                  << summary.passed << " passed (" << summary.total << " total) in " 
                  << std::fixed << std::setprecision(2) << summary.duration_ms << "ms\n\n";
    }

    return summary;
}

std::string ForgeEcosystem::format_source(const std::string& source, const ForgeSpec& spec) {
    (void)spec;
    std::istringstream in(source);
    std::ostringstream out;
    std::string line;
    int indent_level = 0;
    const int indent_size = 4;

    while (std::getline(in, line)) {
        // Strip trailing \r
        if (!line.empty() && line.back() == '\r') line.pop_back();

        // Find first non-space
        size_t first = line.find_first_not_of(" \t");
        if (first == std::string::npos) {
            out << "\n";
            continue;
        }

        std::string trimmed = line.substr(first);

        // Check if line starts with closing braces
        int leading_closes = 0;
        for (char c : trimmed) {
            if (c == '}' || c == ')') leading_closes++;
            else if (!std::isspace(static_cast<unsigned char>(c))) break;
        }

        int current_indent = std::max(0, indent_level - leading_closes);
        for (int i = 0; i < current_indent * indent_size; ++i) out << ' ';
        out << trimmed << "\n";

        // Adjust indent_level for subsequent lines
        for (char c : trimmed) {
            if (c == '{') indent_level++;
            else if (c == '}') indent_level = std::max(0, indent_level - 1);
        }
    }

    return out.str();
}

bool ForgeEcosystem::format_file(const std::string& filepath, const ForgeSpec& spec, std::string& out_error) {
    std::ifstream file(filepath);
    if (!file.is_open()) {
        out_error = "Could not open file: " + filepath;
        return false;
    }
    std::stringstream buf;
    buf << file.rdbuf();
    file.close();

    std::string formatted = format_source(buf.str(), spec);
    std::ofstream out(filepath);
    if (!out.is_open()) {
        out_error = "Could not write to file: " + filepath;
        return false;
    }
    out << formatted;
    return true;
}

bool ForgeEcosystem::pkg_init(const ForgeSpec& spec, const std::string& project_dir, std::string& out_error) {
    std::filesystem::path dir(project_dir.empty() ? "." : project_dir);
    std::error_code ec;
    std::filesystem::create_directories(dir, ec);
    std::filesystem::path manifest = dir / "pkg.json";
    if (std::filesystem::exists(manifest)) {
        out_error = "pkg.json already exists in " + dir.string();
        return false;
    }

    std::string proj_name = dir.filename().string();
    if (proj_name.empty() || proj_name == ".") proj_name = to_lower_str(spec.name) + "-project";

    std::ofstream out(manifest);
    if (!out.is_open()) {
        out_error = "Failed to create pkg.json";
        return false;
    }

    out << "{\n";
    out << "  \"name\": \"" << proj_name << "\",\n";
    out << "  \"version\": \"0.1.0\",\n";
    out << "  \"language\": \"" << spec.name << "\",\n";
    out << "  \"author\": \"" << escape_json_str(spec.author) << "\",\n";
    out << "  \"dependencies\": {}\n";
    out << "}\n";

    std::cout << "\033[1;32mInitialized package manifest:\033[0m " << manifest.string() << "\n";
    return true;
}

bool ForgeEcosystem::pkg_install(const ForgeSpec& spec, const std::string& package_name, std::string& out_error) {
    if (package_name.empty()) {
        out_error = "Package name cannot be empty";
        return false;
    }

    std::filesystem::path mod_dir = std::filesystem::path(".modules") / package_name;
    std::error_code ec;
    std::filesystem::create_directories(mod_dir, ec);
    if (ec) {
        out_error = "Failed to create module directory: " + ec.message();
        return false;
    }

    // Create entry module
    std::string ext = spec.extension.empty() ? ".lang" : spec.extension;
    if (ext[0] != '.') ext = "." + ext;
    std::ofstream mod_file(mod_dir / ("mod" + ext));
    if (mod_file.is_open()) {
        mod_file << "// " << package_name << " module for " << spec.name << "\n";
    }

    std::cout << "\033[1;32mInstalled package:\033[0m " << package_name << " -> " << mod_dir.string() << "\n";
    return true;
}

bool ForgeEcosystem::pkg_list(const ForgeSpec& spec, const std::string& project_dir) {
    std::filesystem::path dir(project_dir.empty() ? "." : project_dir);
    std::cout << "\033[1m" << spec.name << " Package Manager\033[0m\n";
    std::cout << "Project: " << std::filesystem::absolute(dir).string() << "\n\n";

    std::filesystem::path mod_dir = dir / ".modules";
    if (!std::filesystem::exists(mod_dir)) {
        std::cout << "  No packages currently installed.\n";
        std::cout << "  Run: " << to_lower_str(spec.name) << " pkg install <name>\n";
        return true;
    }

    std::cout << "Installed packages:\n";
    for (const auto& entry : std::filesystem::directory_iterator(mod_dir)) {
        if (entry.is_directory()) {
            std::cout << "  • " << entry.path().filename().string() << "\n";
        }
    }
    return true;
}

bool ForgeEcosystem::init_project(const std::string& lang_name, 
                                  const std::string& target_dir, 
                                  const std::string& style_str, 
                                  std::string& out_error) {
    if (lang_name.empty()) {
        out_error = "Language name cannot be empty";
        return false;
    }

    std::filesystem::path dir(target_dir.empty() ? ("./" + lang_name) : target_dir);
    std::error_code ec;
    std::filesystem::create_directories(dir, ec);
    if (ec) {
        out_error = "Failed to create directory: " + ec.message();
        return false;
    }

    std::filesystem::create_directories(dir / "examples", ec);
    std::filesystem::create_directories(dir / "tests", ec);

    std::string lower_name = to_lower_str(lang_name);
    std::string ext = "." + lower_name.substr(0, 3);
    std::string style = style_str.empty() ? "prefix" : to_lower_str(style_str);

    // 1. Write <lang_name>.forge
    std::filesystem::path spec_path = dir / (lang_name + ".forge");
    std::ofstream spec_file(spec_path);
    if (!spec_file.is_open()) {
        out_error = "Failed to create " + spec_path.string();
        return false;
    }

    spec_file << "language " << lang_name << " {\n";
    spec_file << "    version: \"1.0.0\"\n";
    spec_file << "    extension: \"" << ext << "\"\n";
    spec_file << "    paradigm: \"bytecode\"\n";
    spec_file << "    types: \"dynamic\"\n";
    spec_file << "    author: \"Author\"\n\n";

    spec_file << "    header {\n";
    if (style == "shebang") {
        spec_file << "        style: \"shebang\"\n";
        spec_file << "        prefix: \"#!/usr/bin/env " << lower_name << "\"\n";
        spec_file << "        required: false\n";
    } else if (style == "pragma") {
        spec_file << "        style: \"pragma\"\n";
        spec_file << "        prefix: \"@" << lower_name << "\"\n";
        spec_file << "        required: false\n";
        spec_file << "        syntax: \"(version={version})\"\n";
        spec_file << "        default: \"(version=1.0)\"\n";
    } else if (style == "none") {
        spec_file << "        style: \"none\"\n";
    } else {
        spec_file << "        style: \"prefix\"\n";
        spec_file << "        prefix: \"#" << lower_name << "\"\n";
        spec_file << "        required: false\n";
        spec_file << "        syntax: \"<{version},{mode}>\"\n";
        spec_file << "        default: \"<1.0, bytecode>\"\n";
    }
    spec_file << "    }\n\n";

    spec_file << "    banner {\n";
    spec_file << "        color: \"cyan\"\n";
    spec_file << "        tagline: \"The " << lang_name << " Programming Language\"\n";
    spec_file << "        show_on: [\"repl\", \"version\", \"help\"]\n";
    spec_file << "    }\n\n";

    spec_file << "    tokens {\n";
    spec_file << "        keyword \"let\"    => VAR\n";
    spec_file << "        keyword \"print\"  => PRINT\n";
    spec_file << "        keyword \"if\"     => IF\n";
    spec_file << "        keyword \"else\"   => ELSE\n";
    spec_file << "        keyword \"while\"  => WHILE\n";
    spec_file << "        keyword \"fn\"     => FUNCTION\n";
    spec_file << "        keyword \"return\" => RETURN\n";
    spec_file << "    }\n\n";

    spec_file << "    grammar {\n";
    spec_file << "        rule Program   = Statement*\n";
    spec_file << "        rule Statement = VarDecl | PrintStmt | IfStmt | ExprStmt\n";
    spec_file << "        rule VarDecl   = \"let\" Identifier \"=\" Expr \";\"\n";
    spec_file << "        rule PrintStmt = \"print\" \"(\" Expr \")\" \";\"\n";
    spec_file << "    }\n";
    spec_file << "}\n";
    spec_file.close();

    // 2. Write examples/hello.<ext>
    std::filesystem::path example_path = dir / "examples" / ("hello" + ext);
    std::ofstream ex_file(example_path);
    if (ex_file.is_open()) {
        if (style == "shebang") ex_file << "#!/usr/bin/env " << lower_name << "\n\n";
        else if (style == "pragma") ex_file << "@" << lower_name << "(version=1.0)\n\n";
        else if (style == "prefix") ex_file << "#" << lower_name << "<1.0, bytecode>\n\n";

        ex_file << "let message = \"Hello from " << lang_name << "!\";\n";
        ex_file << "print(message);\n\n";
        ex_file << "let x = 10;\n";
        ex_file << "let y = 32;\n";
        ex_file << "print(\"Result: \" + (x + y));\n";
        ex_file.close();
    }

    // 3. Write tests/test_basic.<ext>
    std::filesystem::path test_path = dir / "tests" / ("test_basic" + ext);
    std::ofstream t_file(test_path);
    if (t_file.is_open()) {
        t_file << "let a = 5;\n";
        t_file << "let b = 10;\n";
        t_file << "let c = a * b;\n";
        t_file << "print(c);\n";
        t_file.close();
    }

    // 4. Write README.md
    std::filesystem::path readme_path = dir / "README.md";
    std::ofstream rm_file(readme_path);
    if (rm_file.is_open()) {
        rm_file << "# " << lang_name << " Programming Language\n\n";
        rm_file << "Scaffolded with Alphabet Forge.\n\n";
        rm_file << "## Getting Started\n\n";
        rm_file << "```bash\n";
        rm_file << "# Forge standalone toolchain\n";
        rm_file << "alphabet forge " << lang_name << ".forge -o bin/" << lower_name << "\n\n";
        rm_file << "# Run example\n";
        rm_file << "./bin/" << lower_name << " run examples/hello" << ext << "\n\n";
        rm_file << "# Start interactive REPL\n";
        rm_file << "./bin/" << lower_name << " repl\n\n";
        rm_file << "# Run test suite\n";
        rm_file << "./bin/" << lower_name << " test\n";
        rm_file << "```\n";
        rm_file.close();
    }

    // 5. Write .gitignore
    std::filesystem::path gitignore_path = dir / ".gitignore";
    std::ofstream gi_file(gitignore_path);
    if (gi_file.is_open()) {
        gi_file << "bin/\nbuild/\n.modules/\n*.tar.gz\n";
        gi_file.close();
    }

    std::cout << "\033[1;32mInitialised new " << lang_name << " project in:\033[0m " 
              << std::filesystem::absolute(dir).string() << "\n\n";
    std::cout << "Project structure:\n";
    std::cout << "  ├── " << lang_name << ".forge        (Language specification)\n";
    std::cout << "  ├── README.md           (Documentation & usage instructions)\n";
    std::cout << "  ├── examples/\n";
    std::cout << "  │   └── hello" << ext << "       (Starter program)\n";
    std::cout << "  └── tests/\n";
    std::cout << "      └── test_basic" << ext << "  (Unit test)\n\n";
    std::cout << "Next steps:\n";
    std::cout << "  alphabet forge " << (dir / (lang_name + ".forge")).string() << " -o " << (dir / "bin" / lower_name).string() << "\n\n";

    return true;
}

bool ForgeEcosystem::bundle_distribution(const ForgeSpec& spec, 
                                         const std::string& output_dir, 
                                         std::string& out_error) {
    std::string lower_name = to_lower_str(spec.name);
    std::string bundle_name = lower_name + "-v" + spec.version;
    std::filesystem::path out_path(output_dir.empty() ? bundle_name : output_dir);
    std::error_code ec;
    std::filesystem::create_directories(out_path, ec);
    if (ec) {
        out_error = "Failed to create bundle directory: " + ec.message();
        return false;
    }

    std::cout << "\033[1mPackaging distribution bundle for " << spec.name << " v" << spec.version << "...\033[0m\n";

    // 1. Export standalone binary
    std::filesystem::path bin_dir = out_path / "bin";
    std::filesystem::create_directories(bin_dir, ec);
    std::string bin_path = (bin_dir / lower_name).string();
    if (!ForgeRunner::export_toolchain(spec, bin_path, spec.paradigm)) {
        out_error = "Failed to export standalone toolchain binary";
        return false;
    }
    std::cout << "  ✓ Standalone CLI toolchain: " << bin_path << "\n";

    // 2. Copy specification
    std::filesystem::path dest_spec = out_path / (spec.name + ".forge");
    if (!spec.spec_source_path.empty() && std::filesystem::exists(spec.spec_source_path)) {
        std::filesystem::copy_file(spec.spec_source_path, dest_spec, std::filesystem::copy_options::overwrite_existing, ec);
    } else {
        std::ofstream sf(dest_spec);
        if (sf.is_open()) {
            sf << "// " << spec.name << " language specification\n";
        }
    }
    std::cout << "  ✓ Language specification: " << dest_spec.string() << "\n";

    // 3. Export VS Code extension
    std::filesystem::path vscode_dir = out_path / "vscode";
    std::string err;
    if (export_vscode_extension(spec, vscode_dir.string(), err)) {
        std::cout << "  ✓ VS Code extension: " << vscode_dir.string() << "\n";
    }

    // 4. Export documentation
    std::filesystem::path doc_dir = out_path / "docs";
    if (ForgeDocGenerator::generate_html_docs(spec, doc_dir.string(), err)) {
        std::cout << "  ✓ HTML Documentation: " << doc_dir.string() << "\n";
    }

    // 5. Export interactive playground
    std::filesystem::path play_dir = out_path / "playground";
    if (ForgePlayground::export_playground(spec, play_dir.string(), err)) {
        std::cout << "  ✓ Web Playground: " << play_dir.string() << "\n";
    }

    // 6. Generate Bundle README
    std::filesystem::path bundle_readme = out_path / "README.md";
    std::ofstream br(bundle_readme);
    if (br.is_open()) {
        br << "# " << spec.name << " Standalone Distribution\n\n";
        br << "Version: " << spec.version << " (" << spec.paradigm << ")\n\n";
        br << "## Contents\n";
        br << "- `bin/" << lower_name << "`: Standalone language CLI toolchain\n";
        br << "- `docs/`: Complete HTML documentation & syntax reference\n";
        br << "- `playground/`: Zero-server in-browser Web IDE and interpreter\n";
        br << "- `vscode/`: Visual Studio Code language extension\n\n";
        br << "## Quick Start\n";
        br << "```bash\n";
        br << "./bin/" << lower_name << " --help\n";
        br << "./bin/" << lower_name << " repl\n";
        br << "```\n";
    }

    // 7. Optional tarball creation if tar is available
    std::string tar_cmd = "tar -czf \"" + out_path.string() + ".tar.gz\" -C \"" + 
                          out_path.parent_path().string() + "\" \"" + out_path.filename().string() + "\" 2>/dev/null";
    if (std::system(tar_cmd.c_str()) == 0) {
        std::cout << "\n\033[1;32m✓ Created distribution archive:\033[0m " << out_path.string() << ".tar.gz\n\n";
    } else {
        std::cout << "\n\033[1;32m✓ Created distribution bundle directory:\033[0m " << out_path.string() << "\n\n";
    }

    return true;
}

} // namespace forge
} // namespace alphabet

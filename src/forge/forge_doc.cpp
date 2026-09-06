#include "forge_doc.h"
#include "banner_engine.h"
#include <fstream>
#include <iostream>
#include <filesystem>

namespace alphabet {
namespace forge {

namespace {

std::string escape_html(const std::string& s) {
    std::string res;
    for (char c : s) {
        if (c == '<') res += "&lt;";
        else if (c == '>') res += "&gt;";
        else if (c == '&') res += "&amp;";
        else if (c == '"') res += "&quot;";
        else res += c;
    }
    return res;
}

} // namespace

bool ForgeDocGenerator::generate_html_docs(const ForgeSpec& spec, const std::string& output_dir, std::string& out_error) {
    std::filesystem::path dir(output_dir.empty() ? "docs" : output_dir);
    std::error_code ec;
    std::filesystem::create_directories(dir, ec);
    if (ec) {
        out_error = "Failed to create directory: " + ec.message();
        return false;
    }

    std::filesystem::path index_file = dir / "index.html";
    std::ofstream out(index_file);
    if (!out.is_open()) {
        out_error = "Failed to write documentation file: " + index_file.string();
        return false;
    }

    out << "<!DOCTYPE html>\n";
    out << "<html lang=\"en\">\n";
    out << "<head>\n";
    out << "  <meta charset=\"UTF-8\">\n";
    out << "  <meta name=\"viewport\" content=\"width=device-width, initial-scale=1.0\">\n";
    out << "  <title>" << escape_html(spec.name) << " Documentation</title>\n";
    out << "  <style>\n";
    out << "    :root {\n";
    out << "      --bg-color: #0d1117;\n";
    out << "      --card-bg: #161b22;\n";
    out << "      --border-color: #30363d;\n";
    out << "      --text-main: #c9d1d9;\n";
    out << "      --text-muted: #8b949e;\n";
    out << "      --accent: #58a6ff;\n";
    out << "      --accent-green: #3fb950;\n";
    out << "      --code-bg: #090d13;\n";
    out << "    }\n";
    out << "    body {\n";
    out << "      font-family: -apple-system, BlinkMacSystemFont, 'Segoe UI', Roboto, Helvetica, Arial, sans-serif;\n";
    out << "      background-color: var(--bg-color);\n";
    out << "      color: var(--text-main);\n";
    out << "      line-height: 1.6;\n";
    out << "      margin: 0;\n";
    out << "      padding: 0;\n";
    out << "    }\n";
    out << "    .container {\n";
    out << "      max-width: 1000px;\n";
    out << "      margin: 0 auto;\n";
    out << "      padding: 40px 20px;\n";
    out << "    }\n";
    out << "    .hero {\n";
    out << "      text-align: center;\n";
    out << "      border-bottom: 1px solid var(--border-color);\n";
    out << "      padding-bottom: 30px;\n";
    out << "      margin-bottom: 40px;\n";
    out << "    }\n";
    out << "    .banner-pre {\n";
    out << "      font-family: monospace;\n";
    out << "      color: var(--accent);\n";
    out << "      font-size: 14px;\n";
    out << "      line-height: 1.2;\n";
    out << "      display: inline-block;\n";
    out << "      text-align: left;\n";
    out << "      background: var(--code-bg);\n";
    out << "      padding: 20px 30px;\n";
    out << "      border-radius: 8px;\n";
    out << "      border: 1px solid var(--border-color);\n";
    out << "    }\n";
    out << "    .badges {\n";
    out << "      margin-top: 15px;\n";
    out << "    }\n";
    out << "    .badge {\n";
    out << "      display: inline-block;\n";
    out << "      background: var(--border-color);\n";
    out << "      color: var(--text-main);\n";
    out << "      padding: 4px 12px;\n";
    out << "      border-radius: 20px;\n";
    out << "      font-size: 13px;\n";
    out << "      font-weight: 600;\n";
    out << "      margin: 0 4px;\n";
    out << "    }\n";
    out << "    .badge.green { background: #238636; color: white; }\n";
    out << "    .badge.blue { background: #1f6feb; color: white; }\n";
    out << "    .card {\n";
    out << "      background: var(--card-bg);\n";
    out << "      border: 1px solid var(--border-color);\n";
    out << "      border-radius: 8px;\n";
    out << "      padding: 24px;\n";
    out << "      margin-bottom: 24px;\n";
    out << "    }\n";
    out << "    h1, h2, h3 { color: #fff; }\n";
    out << "    table {\n";
    out << "      width: 100%;\n";
    out << "      border-collapse: collapse;\n";
    out << "      margin-top: 15px;\n";
    out << "    }\n";
    out << "    th, td {\n";
    out << "      padding: 10px 14px;\n";
    out << "      text-align: left;\n";
    out << "      border-bottom: 1px solid var(--border-color);\n";
    out << "    }\n";
    out << "    th { background: #21262d; color: var(--accent); }\n";
    out << "    code {\n";
    out << "      font-family: 'SFMono-Regular', Consolas, 'Liberation Mono', Menlo, monospace;\n";
    out << "      background: var(--code-bg);\n";
    out << "      padding: 2px 6px;\n";
    out << "      border-radius: 4px;\n";
    out << "      font-size: 0.9em;\n";
    out << "      color: #79c0ff;\n";
    out << "    }\n";
    out << "    pre code {\n";
    out << "      display: block;\n";
    out << "      padding: 16px;\n";
    out << "      overflow-x: auto;\n";
    out << "      border: 1px solid var(--border-color);\n";
    out << "      border-radius: 6px;\n";
    out << "    }\n";
    out << "  </style>\n";
    out << "</head>\n";
    out << "<body>\n";
    out << "  <div class=\"container\">\n";
    out << "    <div class=\"hero\">\n";
    if (!spec.banner.art.empty()) {
        out << "      <pre class=\"banner-pre\">" << escape_html(spec.banner.art) << "</pre>\n";
    }
    out << "      <h1>" << escape_html(spec.name) << " Language</h1>\n";
    if (!spec.banner.tagline.empty()) {
        out << "      <p style=\"color: var(--text-muted); font-size: 1.1em;\">" << escape_html(spec.banner.tagline) << "</p>\n";
    }
    out << "      <div class=\"badges\">\n";
    out << "        <span class=\"badge green\">v" << escape_html(spec.version) << "</span>\n";
    out << "        <span class=\"badge blue\">" << escape_html(spec.paradigm) << "</span>\n";
    out << "        <span class=\"badge\">" << escape_html(spec.type_system) << " types</span>\n";
    out << "        <span class=\"badge\">" << escape_html(spec.extension) << "</span>\n";
    out << "      </div>\n";
    out << "    </div>\n";

    out << "    <div class=\"card\">\n";
    out << "      <h2>Overview</h2>\n";
    out << "      <p><strong>" << escape_html(spec.name) << "</strong> is an independent language created with <strong>Alphabet Forge</strong>.</p>\n";
    out << "      <ul>\n";
    out << "        <li><strong>Author:</strong> " << escape_html(spec.author) << "</li>\n";
    out << "        <li><strong>Primary File Extension:</strong> <code>" << escape_html(spec.extension) << "</code></li>\n";
    out << "        <li><strong>Execution Paradigm:</strong> " << escape_html(spec.paradigm) << "</li>\n";
    out << "      </ul>\n";
    out << "    </div>\n";

    out << "    <div class=\"card\">\n";
    out << "      <h2>Keywords & Syntax Tokens</h2>\n";
    out << "      <table>\n";
    out << "        <thead><tr><th>" << escape_html(spec.name) << " Keyword</th><th>Canonical Operation</th></tr></thead>\n";
    out << "        <tbody>\n";
    for (const auto& kv : spec.tokens.keywords) {
        out << "          <tr><td><code>" << escape_html(kv.first) << "</code></td><td>" << escape_html(kv.second) << "</td></tr>\n";
    }
    out << "        </tbody>\n";
    out << "      </table>\n";
    out << "    </div>\n";

    out << "    <div class=\"card\">\n";
    out << "      <h2>Grammar Rules (PEG)</h2>\n";
    out << "      <table>\n";
    out << "        <thead><tr><th>Rule Name</th></tr></thead>\n";
    out << "        <tbody>\n";
    for (const auto& rule : spec.rules) {
        out << "          <tr><td><code>" << escape_html(rule.name) << "</code></td></tr>\n";
    }
    out << "        </tbody>\n";
    out << "      </table>\n";
    out << "    </div>\n";

    out << "    <div class=\"card\">\n";
    out << "      <h2>CLI & Toolchain Usage</h2>\n";
    out << "      <pre><code>";
    out << "# Run a " << escape_html(spec.name) << " program\n";
    out << "./" << escape_html(spec.name) << " run program" << escape_html(spec.extension) << "\n\n";
    out << "# Start interactive REPL\n";
    out << "./" << escape_html(spec.name) << " repl\n\n";
    out << "# Run test suite\n";
    out << "./" << escape_html(spec.name) << " test ./tests/\n\n";
    out << "# Format code\n";
    out << "./" << escape_html(spec.name) << " fmt program" << escape_html(spec.extension) << "\n\n";
    out << "# Export VS Code extension\n";
    out << "./" << escape_html(spec.name) << " --export-vscode\n";
    out << "</code></pre>\n";
    out << "    </div>\n";

    out << "  </div>\n";
    out << "</body>\n";
    out << "</html>\n";
    out.close();

    std::cout << "\033[1;32mGenerated documentation site:\033[0m " << index_file.string() << "\n";
    return true;
}

} // namespace forge
} // namespace alphabet

#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include <cstring>
#include <cstdlib>
#include <unistd.h>
#include <sys/stat.h>

#include "lexer.h"
#include "parser.h"
#include "compiler.h"
#include "vm.h"
#include "type_system.h"
#include "ffi.h"
#include "lsp.h"

namespace {

constexpr const char* VERSION = "2.2.0";
constexpr const char* DEVELOPER = "Fraol Teshome (fraolteshome444@gmail.com)";

constexpr const char* LOGO = R"(
            d8b            d8b                 d8b
           88P            ?88                 ?88                d8P
          d88              88b                 88b            d888888P
 d888b8b  888  ?88,.d88b,  888888b  d888b8b    888888b  d8888b  ?88'
d8P' ?88  ?88  `?88'  ?88  88P `?8bd8P' ?88    88P `?8bd8b_,dP  88P
88b  ,88b  88b   88b  d8P d88   88P88b  ,88b  d88,  d8888b      88b
`?88P'`88b  88b  888888P'd88'   88b`?88P'`88bd88'`?88P'`?888P'  `?8b
                 88P'
                d88
                ?8P
)";

void print_version() {
    std::cout << "Alphabet " << VERSION << " (Native C++)\n";
    std::cout << "Developer: " << DEVELOPER << "\n";
    std::cout << "Compiled with C++17\n";
}

void print_help() {
    std::cout << "Usage: alphabet [options] [file]\n\n";
    std::cout << "Options:\n";
    std::cout << "  -v, --version     Show version information\n";
    std::cout << "  -h, --help        Show this help message\n";
    std::cout << "  -c, --compile     Compile only, don't run\n";
    std::cout << "  -o, --output      Output file for compiled bytecode\n";
    std::cout << "  --repl            Start interactive REPL\n";
    std::cout << "  --lsp             Start Language Server Protocol server\n\n";
    std::cout << "Examples:\n";
    std::cout << "  alphabet program.abc          Run a program\n";
    std::cout << "  alphabet -c program.abc       Compile only\n";
    std::cout << "  alphabet --repl               Interactive mode\n";
    std::cout << "  alphabet --lsp                LSP server for VS Code\n";
    std::cout << "  --debug              Run in debug mode\n";
    std::cout << "  --sandbox            Sandbox mode: block FFI and file access\n";
    std::cout << "  alphabet update      Self-update to latest version\n";
}

void run_source(const std::string& source, bool debug_mode = false, const std::string& source_dir = "", bool sandbox_mode = false) {
    try {
        alphabet::Lexer lexer(source);
        std::vector<alphabet::Token> tokens = lexer.scan_tokens();

        alphabet::Parser parser(tokens, source);
        std::vector<alphabet::StmtPtr> statements = parser.parse();
        
        if (parser.had_errors()) {
            std::string msg = parser.first_error().empty() ? "Syntax errors in source code" : parser.first_error();
            throw alphabet::ParseError(msg);
        }

        alphabet::Compiler compiler;
        if (!source_dir.empty()) {
            compiler.set_source_dir(source_dir);
        }
        alphabet::Program program = compiler.compile(statements);

        alphabet::VM vm(program);
        vm.set_debug_mode(debug_mode);
        vm.set_sandbox_mode(sandbox_mode);
        if (debug_mode) {
             std::cout << "DEBUG_READY" << std::endl;
        }
        vm.run();

    } catch (const alphabet::MissingLanguageHeader& e) {
        std::cerr << "Error: " << e.what() << "\n";
        std::cerr << "  Add '#alphabet<lang>' as the first line of your source file.\n";
    } catch (const alphabet::ParseError& e) {
        std::cerr << "Parse Error: " << e.what() << "\n";
    } catch (const alphabet::CompileError& e) {
        std::cerr << "Compile Error: " << e.what() << "\n";
    } catch (const alphabet::RuntimeError& e) {
        std::cerr << "Runtime Error: " << e.what() << "\n";
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << "\n";
    }
}

void start_repl() {
    std::cout << LOGO;
    std::cout << "Alphabet Language [v" << VERSION << " - Native C++]\n";
    std::cout << "Developed by " << DEVELOPER << "\n";
    std::cout << "Type 'q' to exit.\n\n";
    std::cout << "Multi-line mode: Type '{' to start a block, then continue on next lines.\n";
    std::cout << "Commands: 'history' (show past), '!!' (repeat last)\n";

    ffi_init();

    // Load history from file
    std::vector<std::string> history;
    const char* home = std::getenv("HOME");
    std::string history_path = home ? (std::string(home) + "/.alphabet_history") : ".alphabet_history";
    {
        std::ifstream hist_file(history_path);
        std::string hist_line;
        while (std::getline(hist_file, hist_line)) {
            if (!hist_line.empty()) history.push_back(hist_line);
        }
    }
    auto save_history = [&]() {
        std::ofstream hist_file(history_path);
        // Keep last 500 entries
        size_t start = history.size() > 500 ? history.size() - 500 : 0;
        for (size_t i = start; i < history.size(); ++i) {
            hist_file << history[i] << "\n";
        }
    };

    std::string line;
    std::string buffer;
    int brace_depth = 0;

    // REPL state: accumulate source and persist globals
    std::string all_source;  // All previously entered code
    std::unordered_map<std::string, alphabet::Value> saved_globals;

    while (true) {
        if (buffer.empty()) {
            std::cout << ">>> ";
        } else {
            std::cout << "... ";
        }
        std::cout.flush();

        if (!std::getline(std::cin, line)) {
            std::cout << "\n";
            break;
        }

        if (buffer.empty() && (line == "q" || line == "quit" || line == "exit")) {
            save_history();
            break;
        }

        // History commands
        if (buffer.empty() && line == "history") {
            size_t start = history.size() > 20 ? history.size() - 20 : 0;
            for (size_t i = start; i < history.size(); ++i) {
                std::cout << "  " << (i + 1) << "  " << history[i] << "\n";
            }
            continue;
        }
        if (buffer.empty() && line == "!!" && !history.empty()) {
            line = history.back();
            std::cout << line << "\n";
        }

        if (buffer.empty() && line.empty()) {
            continue;
        }

        // Save to history (only non-empty, non-command lines)
        if (buffer.empty() && line != "!!") {
            history.push_back(line);
        }

        for (char c : line) {
            if (c == '{') ++brace_depth;
            else if (c == '}') --brace_depth;
        }

        if (!buffer.empty()) buffer += '\n';
        buffer += line;

        if (brace_depth == 0) {
            // Append new code to accumulated source
            all_source += buffer + "\n";
            
            try {
                std::string full_source = "#alphabet<repl>\n" + all_source;
                alphabet::Lexer lexer(full_source);
                auto tokens = lexer.scan_tokens();
                alphabet::Parser parser(tokens, full_source);
                auto statements = parser.parse();
                
                if (parser.had_errors()) {
                    // Show error but don't clear state
                    if (!parser.first_error().empty()) {
                        std::cerr << parser.first_error() << "\n";
                    }
                    // Remove the bad line from accumulated source
                    size_t last_newline = all_source.rfind('\n', all_source.size() - 2);
                    if (last_newline != std::string::npos) {
                        all_source = all_source.substr(0, last_newline + 1);
                    } else {
                        all_source.clear();
                    }
                } else {
                    alphabet::Compiler compiler;
                    alphabet::Program program = compiler.compile(statements);
                    
                    alphabet::VM vm(program);
                    vm.set_globals(saved_globals);
                    vm.run();
                    saved_globals = vm.get_globals();
                }
            } catch (const alphabet::MissingLanguageHeader&) {
                std::cerr << "Error: Missing header\n";
            } catch (const alphabet::ParseError& e) {
                std::cerr << "Parse Error: " << e.what() << "\n";
            } catch (const alphabet::CompileError& e) {
                std::cerr << "Compile Error: " << e.what() << "\n";
            } catch (const alphabet::RuntimeError& e) {
                std::cerr << "Runtime Error: " << e.what() << "\n";
            }
            
            buffer.clear();
            brace_depth = 0;
        }
    }

    ffi_cleanup();
}

void do_update() {
    std::cout << "Checking for updates...\n";

    // Get latest release info from GitHub
    std::string api_cmd = "curl -s https://api.github.com/repos/fraol163/alphabet/releases/latest";
    FILE* pipe = popen(api_cmd.c_str(), "r");
    if (!pipe) {
        std::cerr << "Error: Failed to check for updates\n";
        return;
    }

    std::string response;
    char buffer[256];
    while (fgets(buffer, sizeof(buffer), pipe)) {
        response += buffer;
    }
    int status = pclose(pipe);

    if (status != 0) {
        std::cerr << "Error: Could not reach GitHub\n";
        return;
    }

    // Parse tag_name from JSON (simple extraction)
    std::string latest_version;
    size_t tag_pos = response.find("\"tag_name\"");
    if (tag_pos != std::string::npos) {
        size_t quote_start = response.find('"', tag_pos + 10);
        if (quote_start != std::string::npos) {
            quote_start++;
            // Skip whitespace and "v" prefix
            while (quote_start < response.size() && (response[quote_start] == '"' || response[quote_start] == 'v'))
                quote_start++;
            // Actually, find the value between quotes
            size_t val_start = response.find('"', tag_pos + 10);
            if (val_start != std::string::npos) {
                val_start++;
                size_t val_end = response.find('"', val_start);
                if (val_end != std::string::npos) {
                    latest_version = response.substr(val_start, val_end - val_start);
                    // Strip leading 'v'
                    if (!latest_version.empty() && latest_version[0] == 'v')
                        latest_version = latest_version.substr(1);
                }
            }
        }
    }

    if (latest_version.empty()) {
        std::cerr << "Error: Could not parse latest version\n";
        return;
    }

    std::cout << "Current version: " << VERSION << "\n";
    std::cout << "Latest version:  " << latest_version << "\n";

    // Simple version comparison: split on '.'
    auto version_tuple = [](const std::string& v) -> std::tuple<int,int,int> {
        int a = 0, b = 0, c = 0;
        sscanf(v.c_str(), "%d.%d.%d", &a, &b, &c);
        return {a, b, c};
    };

    if (version_tuple(latest_version) <= version_tuple(VERSION)) {
        std::cout << "Already up to date!\n";
        return;
    }

    // Determine OS and arch
    std::string os = "linux";
    std::string arch = "amd64";
#ifdef __APPLE__
    os = "macos";
    #ifdef __aarch64__
    arch = "arm64";
    #endif
#elif defined(__aarch64__)
    arch = "arm64";
#endif

    // Download URL
    std::string download_url =
        "https://github.com/fraol163/alphabet/releases/download/v" +
        latest_version + "/alphabet-" + os + "-" + arch;

    std::cout << "Downloading " << download_url << "...\n";

    // Get current binary path
    std::string self_path;
    char self_buf[4096];
    ssize_t len = readlink("/proc/self/exe", self_buf, sizeof(self_buf) - 1);
    if (len > 0) {
        self_buf[len] = '\0';
        self_path = self_buf;
    } else {
        self_path = "/usr/local/bin/alphabet";
    }

    std::string tmp_path = self_path + ".tmp";

    // Download to temp file
    std::string dl_cmd = "curl -sL -o \"" + tmp_path + "\" \"" + download_url + "\"";
    int dl_status = system(dl_cmd.c_str());
    if (dl_status != 0) {
        std::cerr << "Error: Download failed\n";
        unlink(tmp_path.c_str());
        return;
    }

    // Make executable
    chmod(tmp_path.c_str(), 0755);

    // Replace self (use a shell wrapper since we can't replace running binary)
    std::cout << "Installing update...\n";
    std::string mv_cmd = "mv \"" + tmp_path + "\" \"" + self_path + "\"";
    int mv_status = system(mv_cmd.c_str());
    if (mv_status != 0) {
        std::cerr << "Error: Could not install update (try running as root)\n";
        unlink(tmp_path.c_str());
        return;
    }

    std::cout << "Updated to v" << latest_version << "!\n";
}

std::string read_file(const std::string& path) {
    std::ifstream file(path);
    if (!file.is_open()) {
        throw std::runtime_error("Cannot open file: " + path);
    }
    
    std::ostringstream oss;
    oss << file.rdbuf();
    return oss.str();
}

}

int main(int argc, char* argv[]) {
    bool compile_only = false;
    bool repl_mode = false;
    bool lsp_mode = false;
    bool debug_mode = false;
    bool sandbox_mode = false;
    std::string output_file;
    std::string input_file;

    for (int i = 1; i < argc; ++i) {
        std::string arg = argv[i];

        if (arg == "-v" || arg == "--version") {
            print_version();
            return 0;
        }

        if (arg == "-h" || arg == "--help") {
            print_help();
            return 0;
        }

        if (arg == "-c" || arg == "--compile") {
            compile_only = true;
            continue;
        }

        if (arg == "--repl") {
            repl_mode = true;
            continue;
        }

        if (arg == "--lsp") {
            lsp_mode = true;
            continue;
        }

        if (arg == "--debug") {
            debug_mode = true;
            continue;
        }

        if (arg == "--sandbox") {
            sandbox_mode = true;
            continue;
        }

        if (arg == "-o" || arg == "--output") {
            if (i + 1 < argc) {
                output_file = argv[++i];
            } else {
                std::cerr << "Error: -o requires an output file argument\n";
                return 1;
            }
            continue;
        }

        if (arg[0] == '-') {
            std::cerr << "Unknown option: " << arg << "\n";
            std::cerr << "Use --help for usage information\n";
            return 1;
        }

        // Subcommands
        if (arg == "update") {
            do_update();
            return 0;
        }

        input_file = arg;
    }

    if (lsp_mode) {
        alphabet::lsp::LanguageServer server;
        server.run();
        return 0;
    }

    if (repl_mode || (input_file.empty() && !compile_only)) {
        start_repl();
        return 0;
    }

    if (input_file.empty()) {
        std::cerr << "Error: No input file specified\n";
        std::cerr << "Use --help for usage information\n";
        return 1;
    }

    try {
        std::string source = read_file(input_file);

        if (compile_only) {
            alphabet::Lexer lexer(source);
            std::vector<alphabet::Token> tokens = lexer.scan_tokens();

            alphabet::Parser parser(tokens, source);
            std::vector<alphabet::StmtPtr> statements = parser.parse();
            
            // Check for parse errors
            if (parser.had_errors()) {
                std::string msg = parser.first_error().empty() ? "Syntax errors in source code" : parser.first_error();
            throw alphabet::ParseError(msg);
            }

            alphabet::Compiler compiler;
            // Extract source file directory for resolving relative imports
            size_t last_sl = input_file.find_last_of("/\\");
            if (last_sl != std::string::npos) {
                compiler.set_source_dir(input_file.substr(0, last_sl));
            }
            alphabet::Program program = compiler.compile(statements);
            
            if (!output_file.empty()) {
                std::ofstream out(output_file, std::ios::binary);
                if (!out.is_open()) {
                    std::cerr << "Error: Cannot write to " << output_file << "\n";
                    return 1;
                }
                
                // Header: magic + version + instruction count
                const char magic[] = "ALPH";
                out.write(magic, 4);
                uint32_t version = 2;
                out.write(reinterpret_cast<const char*>(&version), sizeof(version));
                uint32_t count = static_cast<uint32_t>(program.main.size());
                out.write(reinterpret_cast<const char*>(&count), sizeof(count));
                
                // Write each instruction with its operand
                for (const auto& instr : program.main) {
                    uint8_t op = static_cast<uint8_t>(instr.op);
                    out.write(reinterpret_cast<const char*>(&op), sizeof(op));
                    
                    // Operand type tag
                    uint8_t tag;
                    if (std::holds_alternative<std::monostate>(instr.operand)) {
                        tag = 0;
                        out.write(reinterpret_cast<const char*>(&tag), sizeof(tag));
                    } else if (auto* i = std::get_if<int64_t>(&instr.operand)) {
                        tag = 1;
                        out.write(reinterpret_cast<const char*>(&tag), sizeof(tag));
                        out.write(reinterpret_cast<const char*>(i), sizeof(*i));
                    } else if (auto* d = std::get_if<double>(&instr.operand)) {
                        tag = 2;
                        out.write(reinterpret_cast<const char*>(&tag), sizeof(tag));
                        out.write(reinterpret_cast<const char*>(d), sizeof(*d));
                    } else if (auto* s = std::get_if<std::string>(&instr.operand)) {
                        tag = 3;
                        out.write(reinterpret_cast<const char*>(&tag), sizeof(tag));
                        uint32_t len = static_cast<uint32_t>(s->size());
                        out.write(reinterpret_cast<const char*>(&len), sizeof(len));
                        out.write(s->data(), len);
                    } else if (std::holds_alternative<std::nullptr_t>(instr.operand)) {
                        tag = 4;
                        out.write(reinterpret_cast<const char*>(&tag), sizeof(tag));
                    } else if (auto* p = std::get_if<std::pair<std::string, int>>(&instr.operand)) {
                        tag = 5;
                        out.write(reinterpret_cast<const char*>(&tag), sizeof(tag));
                        uint32_t len = static_cast<uint32_t>(p->first.size());
                        out.write(reinterpret_cast<const char*>(&len), sizeof(len));
                        out.write(p->first.data(), len);
                        int32_t second = p->second;
                        out.write(reinterpret_cast<const char*>(&second), sizeof(second));
                    } else {
                        tag = 0;
                        out.write(reinterpret_cast<const char*>(&tag), sizeof(tag));
                    }
                }
                
                std::cout << "Compiled " << program.main.size() 
                          << " instructions to " << output_file << "\n";
            } else {
                std::cout << "Compilation successful: " 
                          << program.main.size() << " instructions\n";
            }
        } else {
            // Extract source file directory for resolving relative imports
            std::string source_dir;
            size_t last_slash = input_file.find_last_of("/\\");
            if (last_slash != std::string::npos) {
                source_dir = input_file.substr(0, last_slash);
            }
            run_source(source, debug_mode, source_dir, sandbox_mode);
        }
        
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << "\n";
        return 1;
    }
    
    return 0;
}

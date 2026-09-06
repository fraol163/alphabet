#include "forge.h"
#include <fstream>
#include <sstream>
#include <iostream>
#include <filesystem>
#ifdef _WIN32
#include <io.h>
#define isatty _isatty
#define STDIN_FILENO 0
#else
#include <unistd.h>
#endif

namespace alphabet {
namespace forge {

static std::string read_all(const std::string& path) {
    std::ifstream file(path);
    if (!file.is_open()) return "";
    std::stringstream ss;
    ss << file.rdbuf();
    return ss.str();
}

bool ForgeRunner::compile_source(const std::string& source,
                                const ForgeSpec& spec,
                                alphabet::Program& out_program,
                                std::vector<DiagnosticError>& out_errors,
                                const std::string& filename) {
    // 1. Process headers
    HeaderParseResult hdr = HeaderEngine::parse(source, spec.header, spec.name);
    if (!hdr.valid) {
        DiagnosticError err;
        err.line = 1;
        err.column = 1;
        err.message = hdr.error;
        err.line_content = source.substr(0, source.find('\n'));
        err.hint = "add required header matching prefix '" + spec.header.prefix + "'";
        out_errors.push_back(err);
        return false;
    }

    // 2. Parse using PEG engine (with header_lines offset for precise source positions)
    PegEngine engine(spec);
    ParseResult pres = engine.parse(hdr.stripped_source, filename, hdr.header_lines);
    if (!pres.success) {
        out_errors = pres.errors;
        return false;
    }

    // 3. Compile AST to bytecode
    try {
        alphabet::Compiler compiler;
        out_program = compiler.compile(pres.statements);
        return true;
    } catch (const std::exception& e) {
        DiagnosticError err;
        err.line = 1;
        err.column = 1;
        err.message = std::string("Bytecode compilation error: ") + e.what();
        out_errors.push_back(err);
        return false;
    }
}

bool ForgeRunner::run_source(const std::string& source,
                            const ForgeSpec& spec,
                            const std::string& filename,
                            bool debug) {
    alphabet::Program prog;
    std::vector<DiagnosticError> errors;
    if (!compile_source(source, spec, prog, errors, filename)) {
        for (const auto& err : errors) {
            std::cerr << err.format(filename);
        }
        return false;
    }

    try {
        alphabet::VM vm(prog);
        vm.set_source(source);
        vm.set_debugger_prompt(spec.name + "-dbg");
        vm.set_debug_mode(debug);
        if (debug) {
            vm.set_pause_on_start(true);
            if (isatty(STDIN_FILENO)) {
                std::cout << "\033[1;36m[" << spec.name << " Interactive Debugger v" << spec.version << "]\033[0m\n";
                std::cout << "Type 'h' or 'help' for commands, 'c' to continue, 's' to step.\n\n";
            } else {
                std::cout << "DEBUG_READY" << std::endl;
            }
        }
        vm.run();
        return true;
    } catch (const std::exception& e) {
        std::cerr << "\033[1;31mRuntime error in " << spec.name << ": " << e.what() << "\033[0m\n";
        return false;
    }
}

bool ForgeRunner::run_file(const std::string& filepath,
                          const std::string& spec_filepath,
                          bool debug) {
    std::string source = read_all(filepath);
    if (source.empty()) {
        std::cerr << "Error: Cannot open or empty source file: " << filepath << "\n";
        return false;
    }

    ForgeSpec spec;
    std::string err;

    if (!spec_filepath.empty()) {
        if (!ForgeSpec::load_from_file(spec_filepath, spec, err)) {
            std::cerr << "Error: Failed to load language specification: " << err << "\n";
            return false;
        }
    } else {
        // Try auto-detection by file extension or header
        std::string ext;
        size_t dot = filepath.find_last_of('.');
        if (dot != std::string::npos) ext = filepath.substr(dot);

        // Check if there's a matching .forge in the directory
        size_t slash = filepath.find_last_of("/\\");
        std::string dir = (slash != std::string::npos) ? filepath.substr(0, slash) : ".";
        
        // Default built-in spec if matching nova.nv
        if (ext == ".nv" || filepath.find("nova") != std::string::npos) {
            // Check if nova.forge exists
            std::string candidate = dir + "/nova.forge";
            if (!ForgeSpec::load_from_file(candidate, spec, err)) {
                // Check current dir
                candidate = "nova.forge";
                if (!ForgeSpec::load_from_file(candidate, spec, err)) {
                    std::cerr << "Error: No .forge language specification found for " << filepath << "\n";
                    std::cerr << "Please specify with: alphabet run " << filepath << " --spec <spec.forge>\n";
                    return false;
                }
            }
        } else {
            std::cerr << "Error: Please specify the language specification: alphabet run " 
                      << filepath << " --spec <spec.forge>\n";
            return false;
        }
    }

    return run_source(source, spec, filepath, debug);
}

bool ForgeRunner::export_toolchain(const ForgeSpec& spec,
                                  const std::string& output_binary_path,
                                  const std::string& target_mode) {
    (void)target_mode;
    std::cout << "Forging standalone toolchain for " << spec.name << "...\n";
    BannerEngine::print_if_enabled(spec, "help", std::cout);
    std::cout << "Target: " << output_binary_path << "\n";

    // Ensure parent directory exists
    std::filesystem::path out_path(output_binary_path);
    if (out_path.has_parent_path()) {
        std::error_code ec;
        std::filesystem::create_directories(out_path.parent_path(), ec);
    }

    // Copy .forge file to output directory if possible
    std::filesystem::path spec_dest_dir = out_path.has_parent_path() ? out_path.parent_path() : std::filesystem::current_path();
    std::filesystem::path spec_dest = spec_dest_dir / (spec.name + ".forge");
    if (!spec.spec_source_path.empty() && std::filesystem::exists(spec.spec_source_path)) {
        std::error_code ec;
        std::filesystem::copy_file(spec.spec_source_path, spec_dest, std::filesystem::copy_options::overwrite_existing, ec);
    }

    // Resolve current alphabet executable path
    std::string current_exe = "alphabet";
    try {
        current_exe = std::filesystem::canonical("/proc/self/exe").string();
    } catch (...) {}

    // Generate standalone launcher
    std::string script_path = output_binary_path;
    std::ofstream out(script_path);
    if (!out.is_open()) {
        std::cerr << "Error: Could not write to " << output_binary_path << "\n";
        return false;
    }

    out << "#!/usr/bin/env bash\n";
    out << "# Standalone Toolchain for " << spec.name << " v" << spec.version << "\n";
    out << "# Generated by Alphabet Forge\n\n";
    out << "DIR=\"$(cd \"$(dirname \"${BASH_SOURCE[0]}\")\" && pwd)\"\n\n";

    out << "if [ -f \"$DIR/" << spec.name << ".forge\" ]; then\n";
    out << "    SPEC_FILE=\"$DIR/" << spec.name << ".forge\"\n";
    if (!spec.spec_source_path.empty()) {
        out << "elif [ -f \"" << spec.spec_source_path << "\" ]; then\n";
        out << "    SPEC_FILE=\"" << spec.spec_source_path << "\"\n";
    }
    out << "else\n";
    out << "    SPEC_FILE=\"" << spec.name << ".forge\"\n";
    out << "fi\n\n";

    out << "if [ -n \"$ALPHABET_BIN\" ]; then\n";
    out << "    ENGINE_BIN=\"$ALPHABET_BIN\"\n";
    out << "elif [ -x \"$DIR/alphabet\" ]; then\n";
    out << "    ENGINE_BIN=\"$DIR/alphabet\"\n";
    out << "elif [ -x \"" << current_exe << "\" ]; then\n";
    out << "    ENGINE_BIN=\"" << current_exe << "\"\n";
    out << "elif command -v alphabet >/dev/null 2>&1; then\n";
    out << "    ENGINE_BIN=\"alphabet\"\n";
    out << "else\n";
    out << "    ENGINE_BIN=\"alphabet\"\n";
    out << "fi\n\n";

    out << "if [ $# -eq 0 ]; then\n";
    out << "    exec \"$ENGINE_BIN\" forge --repl --spec \"$SPEC_FILE\"\n";
    out << "fi\n\n";

    out << "case \"$1\" in\n";
    out << "    --version|-v|version)\n";
    out << "        echo \"" << spec.name << " Language v" << spec.version << " (" << spec.paradigm << ")\"\n";
    out << "        exit 0\n";
    out << "        ;;\n";
    out << "    --help|-h|help)\n";
    out << "        echo \"" << spec.name << " Language v" << spec.version << "\"\n";
    out << "        echo \"Usage: " << spec.name << " [command] [options]\"\n";
    out << "        echo \"\"\n";
    out << "        echo \"Commands:\"\n";
    out << "        echo \"  run <file>            Run a " << spec.name << " program\"\n";
    out << "        echo \"  debug <file>          Interactively debug a " << spec.name << " program\"\n";
    out << "        echo \"  compile <file> [-o b] Compile to standalone native binary (AOT)\"\n";
    out << "        echo \"  repl                  Start interactive " << spec.name << " REPL\"\n";
    out << "        echo \"  test [dir]            Run " << spec.name << " test suite\"\n";
    out << "        echo \"  bench <file>          Benchmark " << spec.name << " program performance\"\n";
    out << "        echo \"  lint <file>           Lint " << spec.name << " source file for warnings\"\n";
    out << "        echo \"  lsp                   Start " << spec.name << " Language Server Protocol\"\n";
    out << "        echo \"  playground [dir]      Export interactive web browser playground\"\n";
    out << "        echo \"  doc [dir]             Generate interactive HTML documentation\"\n";
    out << "        echo \"  fmt <file>            Format " << spec.name << " source file\"\n";
    out << "        echo \"  pkg <init|install>    Manage packages for " << spec.name << "\"\n";
    out << "        echo \"  check                 Validate language specification & grammar\"\n";
    out << "        echo \"  bundle [dir]          Create complete standalone distribution archive\"\n";
    out << "        echo \"  --export-vscode [dir] Export VS Code syntax & extension\"\n";
    out << "        echo \"  version               Display version information\"\n";
    out << "        echo \"  help                  Display this help message\"\n";
    out << "        exit 0\n";
    out << "        ;;\n";
    out << "    check)\n";
    out << "        shift\n";
    out << "        exec \"$ENGINE_BIN\" forge \"$SPEC_FILE\" --check \"$@\"\n";
    out << "        ;;\n";
    out << "    bundle)\n";
    out << "        shift\n";
    out << "        exec \"$ENGINE_BIN\" forge \"$SPEC_FILE\" --bundle \"$@\"\n";
    out << "        ;;\n";
    out << "    repl)\n";
    out << "        exec \"$ENGINE_BIN\" forge --repl --spec \"$SPEC_FILE\"\n";
    out << "        ;;\n";
    out << "    debug)\n";
    out << "        shift\n";
    out << "        exec \"$ENGINE_BIN\" run \"$@\" --debug --spec \"$SPEC_FILE\"\n";
    out << "        ;;\n";
    out << "    compile)\n";
    out << "        shift\n";
    out << "        exec \"$ENGINE_BIN\" forge --compile \"$@\" --spec \"$SPEC_FILE\"\n";
    out << "        ;;\n";
    out << "    bench)\n";
    out << "        shift\n";
    out << "        exec \"$ENGINE_BIN\" forge --bench \"$@\" --spec \"$SPEC_FILE\"\n";
    out << "        ;;\n";
    out << "    lint)\n";
    out << "        shift\n";
    out << "        exec \"$ENGINE_BIN\" forge --lint \"$@\" --spec \"$SPEC_FILE\"\n";
    out << "        ;;\n";
    out << "    lsp)\n";
    out << "        shift\n";
    out << "        exec \"$ENGINE_BIN\" forge --lsp \"$@\" --spec \"$SPEC_FILE\"\n";
    out << "        ;;\n";
    out << "    playground)\n";
    out << "        shift\n";
    out << "        exec \"$ENGINE_BIN\" forge --playground \"$@\" --spec \"$SPEC_FILE\"\n";
    out << "        ;;\n";
    out << "    doc)\n";
    out << "        shift\n";
    out << "        exec \"$ENGINE_BIN\" forge --doc \"$@\" --spec \"$SPEC_FILE\"\n";
    out << "        ;;\n";
    out << "    test)\n";
    out << "        shift\n";
    out << "        exec \"$ENGINE_BIN\" forge --test \"$@\" --spec \"$SPEC_FILE\"\n";
    out << "        ;;\n";
    out << "    fmt)\n";
    out << "        shift\n";
    out << "        exec \"$ENGINE_BIN\" forge --fmt \"$@\" --spec \"$SPEC_FILE\"\n";
    out << "        ;;\n";
    out << "    pkg)\n";
    out << "        shift\n";
    out << "        exec \"$ENGINE_BIN\" forge --pkg \"$@\" --spec \"$SPEC_FILE\"\n";
    out << "        ;;\n";
    out << "    --export-vscode)\n";
    out << "        shift\n";
    out << "        exec \"$ENGINE_BIN\" forge --export-vscode \"$@\" --spec \"$SPEC_FILE\"\n";
    out << "        ;;\n";
    out << "    run)\n";
    out << "        shift\n";
    out << "        exec \"$ENGINE_BIN\" run \"$@\" --spec \"$SPEC_FILE\"\n";
    out << "        ;;\n";
    out << "    *)\n";
    out << "        exec \"$ENGINE_BIN\" run \"$@\" --spec \"$SPEC_FILE\"\n";
    out << "        ;;\n";
    out << "esac\n";
    out.close();

    // Make executable
    std::string chmod_cmd = "chmod +x \"" + script_path + "\"";
    int res = std::system(chmod_cmd.c_str());
    (void)res;
    std::cout << "\033[1;32mSuccessfully created standalone toolchain:\033[0m " << output_binary_path << "\n";
    return true;
}

void ForgeRunner::run_repl(const ForgeSpec& spec) {
    BannerEngine::print_if_enabled(spec, "repl", std::cout);
    std::cout << spec.name << " v" << spec.version << " Interactive REPL (" << spec.paradigm << ")\n";
    std::cout << "Type '.help' for help, '.vars' to list variables, '.exit' or 'exit' to quit.\n\n";

    std::string header_line = "";
    if (spec.header.style != HeaderStyle::NONE) {
        if (!spec.header.prefix.empty()) {
            header_line = spec.header.prefix + "<" + spec.version + ">\n";
        }
    }

    std::string print_kw = "display";
    for (const auto& [kw, target] : spec.tokens.keywords) {
        if (target == "PRINT") {
            print_kw = kw;
            break;
        }
    }

    std::string accumulated_code = header_line;
    std::unordered_map<std::string, alphabet::Value> saved_globals;
    std::string buffer;
    int brace_depth = 0;

    while (true) {
        if (buffer.empty()) {
            std::cout << spec.name << "> ";
        } else {
            std::cout << std::string(spec.name.size() + 1, '.') << " ";
        }
        std::cout.flush();

        std::string line;
        if (!std::getline(std::cin, line)) {
            std::cout << "\n";
            break;
        }

        if (buffer.empty()) {
            if (line == ".exit" || line == "exit" || line == ".quit" || line == "quit") {
                break;
            }
            if (line == ".help" || line == "help") {
                std::cout << "Commands:\n";
                std::cout << "  .vars, .globals   List all global variables currently defined\n";
                std::cout << "  .clear            Reset accumulated REPL session\n";
                std::cout << "  .exit, exit       Quit REPL\n";
                std::cout << "  .help, help       Show this help message\n\n";
                continue;
            }
            if (line == ".vars" || line == ".globals") {
                std::cout << "Variables in session:\n";
                if (saved_globals.empty()) {
                    std::cout << "  (none)\n";
                } else {
                    for (const auto& [k, v] : saved_globals) {
                        std::cout << "  " << k << " = " << alphabet::value_to_string(v) << "\n";
                    }
                }
                std::cout << "\n";
                continue;
            }
            if (line == ".clear") {
                accumulated_code = header_line;
                saved_globals.clear();
                std::cout << "Session cleared.\n\n";
                continue;
            }
        }

        for (char c : line) {
            if (c == '{') brace_depth++;
            else if (c == '}') { if (brace_depth > 0) brace_depth--; }
        }

        if (!buffer.empty()) buffer += "\n";
        buffer += line;

        if (brace_depth > 0) {
            continue;
        }

        if (buffer.empty()) continue;

        std::string trimmed = buffer;
        while (!trimmed.empty() && (trimmed.back() == ' ' || trimmed.back() == '\t' || trimmed.back() == '\r' || trimmed.back() == '\n' || trimmed.back() == ';')) {
            trimmed.pop_back();
        }

        bool is_decl_or_stmt = false;
        for (const auto& [kw, target] : spec.tokens.keywords) {
            if (target == "VAR" || target == "LET" || target == "IF" || 
                target == "WHILE" || target == "FUNCTION" || target == "RETURN" || target == "PRINT") {
                if (trimmed.rfind(kw + " ", 0) == 0 || trimmed.rfind(kw + "(", 0) == 0 || trimmed == kw) {
                    is_decl_or_stmt = true;
                    break;
                }
            }
        }
        if (trimmed.find('=') != std::string::npos && trimmed.find("==") == std::string::npos &&
            trimmed.find("!=") == std::string::npos && trimmed.find("<=") == std::string::npos &&
            trimmed.find(">=") == std::string::npos) {
            is_decl_or_stmt = true;
        }

        std::string try_code = accumulated_code + buffer + "\n";
        alphabet::Program prog;
        std::vector<DiagnosticError> errors;
        bool compiled = false;
        bool is_expr = false;

        if (!is_decl_or_stmt) {
            std::string expr_code = accumulated_code + print_kw + "(" + trimmed + ");\n";
            std::vector<DiagnosticError> expr_errors;
            if (compile_source(expr_code, spec, prog, expr_errors, "<repl>")) {
                try_code = expr_code;
                compiled = true;
                is_expr = true;
            }
        }

        if (!compiled) {
            compiled = compile_source(try_code, spec, prog, errors, "<repl>");
        }

        if (compiled) {
            try {
                alphabet::VM vm(prog);
                vm.set_globals(saved_globals);
                vm.run();
                saved_globals = vm.get_globals();
                if (!is_expr) {
                    accumulated_code = try_code;
                }
            } catch (const std::exception& e) {
                std::cerr << "\033[1;31mRuntime error: " << e.what() << "\033[0m\n";
            }
        } else {
            for (const auto& err : errors) {
                std::cerr << err.format("<repl>");
            }
        }

        buffer.clear();
        brace_depth = 0;
    }
}

} // namespace forge
} // namespace alphabet

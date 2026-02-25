#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include <cstring>

#include "lexer.h"
#include "parser.h"
#include "compiler.h"
#include "vm.h"
#include "type_system.h"
#include "ffi.h"
#include "lsp.h"

namespace {

constexpr const char* VERSION = "2.0.0";
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
}

void run_source(const std::string& source) {
    try {
        alphabet::Lexer lexer(source);
        std::vector<alphabet::Token> tokens = lexer.scan_tokens();

        alphabet::Parser parser(tokens);
        std::vector<alphabet::StmtPtr> statements = parser.parse();

        alphabet::Compiler compiler;
        alphabet::Program program = compiler.compile(statements);

        alphabet::VM vm(program);
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
    std::cout << "Example:\n";
    std::cout << "  >>> c MyClass {\n";
    std::cout << "  ...   v m 1 getValue() {\n";
    std::cout << "  ...     r 42\n";
    std::cout << "  ...   }\n";
    std::cout << "  ... }\n\n";

    alphabet::TypeManager type_manager;
    ffi_init();

    std::string line;
    std::string buffer;
    int brace_depth = 0;

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
            break;
        }

        if (buffer.empty() && line.empty()) {
            continue;
        }

        for (char c : line) {
            if (c == '{') ++brace_depth;
            else if (c == '}') --brace_depth;
        }

        if (!buffer.empty()) buffer += '\n';
        buffer += line;

        if (brace_depth == 0) {
            std::string full_source = "#alphabet<repl>\n" + buffer;
            run_source(full_source);
            buffer.clear();
            brace_depth = 0;
        }
    }

    ffi_cleanup();
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
            
            alphabet::Parser parser(tokens);
            std::vector<alphabet::StmtPtr> statements = parser.parse();
            
            alphabet::Compiler compiler;
            alphabet::Program program = compiler.compile(statements);
            
            if (!output_file.empty()) {
                std::ofstream out(output_file, std::ios::binary);
                if (!out.is_open()) {
                    std::cerr << "Error: Cannot write to " << output_file << "\n";
                    return 1;
                }

                const char magic[] = "ALPH";
                out.write(magic, 4);

                uint32_t count = static_cast<uint32_t>(program.main.size());
                out.write(reinterpret_cast<const char*>(&count), sizeof(count));

                for (const auto& instr : program.main) {
                    uint8_t op = static_cast<uint8_t>(instr.op);
                    out.write(reinterpret_cast<const char*>(&op), sizeof(op));
                }
                
                std::cout << "Compiled " << program.main.size() 
                          << " instructions to " << output_file << "\n";
            } else {
                std::cout << "Compilation successful: " 
                          << program.main.size() << " instructions\n";
            }
        } else {
            run_source(source);
        }
        
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << "\n";
        return 1;
    }
    
    return 0;
}

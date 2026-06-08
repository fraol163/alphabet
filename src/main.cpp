#include <algorithm>
#include <chrono>
#include <csignal>
#include <cstdlib>
#include <cstring>
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
#include <thread>
#include <vector>
#ifdef _WIN32
#include <io.h>
#include <sys/stat.h>
#include <windows.h>
#define popen _popen
#define pclose _pclose
#define unlink _unlink
#define chmod _chmod
#define access _access
#define stat _stat
#else
#include <dirent.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <unistd.h>
#endif

#include "compiler.h"
#include "ffi.h"
#include "lexer.h"
#include "linter.h"
#include "lsp.h"
#include "nl_to_code.h"
#include "parser.h"
#include "project.h"
#include "type_system.h"
#include "version.h"
#include "vm.h"
#include "voice.h"
#include "trace_utils.h"

#ifdef _WIN32
#define RED(x) x
#define YELLOW(x) x
#define GREEN(x) x
#define CYAN(x) x
#define BOLD(x) x
#define RESET ""
#else
#define RED(x) "\033[31m" x "\033[0m"
#define YELLOW(x) "\033[33m" x "\033[0m"
#define GREEN(x) "\033[32m" x "\033[0m"
#define CYAN(x) "\033[36m" x "\033[0m"
#define BOLD(x) "\033[1m" x "\033[0m"
#define RESET "\033[0m"
#endif

namespace {

constexpr const char* VERSION = ALPHABET_VERSION;
constexpr const char* DEVELOPER = "Fraol Teshome (fraolteshome444@gmail.com)";

static std::vector<std::string> list_dir_abc(const std::string& dir) {
    std::vector<std::string> files;
#ifdef _WIN32
    WIN32_FIND_DATAA findData;
    std::string pattern = dir + "\\*.abc";
    HANDLE hFind = FindFirstFileA(pattern.c_str(), &findData);
    if (hFind != INVALID_HANDLE_VALUE) {
        do {
            if (!(findData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)) {
                files.push_back(dir + "\\" + findData.cFileName);
            }
        } while (FindNextFileA(hFind, &findData));
        FindClose(hFind);
    }
#else
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
#endif
    return files;
}

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
    std::cout << "  --lsp             Start Language Server Protocol server\n";
    std::cout << "  --debug           Run in debug mode (breakpoints)\n";
    std::cout << "  --sandbox         Sandbox mode: block FFI and file access\n";
    std::cout << "  --dump-bytecode   Print compiled bytecode and exit\n\n";
    std::cout << "Subcommands:\n";
    std::cout << "  alphabet update   Self-update to latest version (alias: upgrade)\n";
    std::cout << "  alphabet update --force  Reinstall current version (fix bugs)\n";
    std::cout << "  alphabet pkg      Package manager (install, list, search)\n";
    std::cout << "  alphabet doc      Show builtin documentation (alphabet doc <name>)\n";
    std::cout << "  alphabet bench    Run VM benchmark suite\n";
    std::cout << "  alphabet examples List available examples\n";
    std::cout << "  alphabet tour     Interactive language tour\n";
    std::cout << "  alphabet run      Run a program (alphabet run <file.abc>)\n";
    std::cout << "  alphabet lint     Lint a file for warnings\n";
    std::cout << "  voice (in REPL)   Voice-to-text input\n";
    std::cout << "  alphabet voice-tutorial  Learn voice input\n";
    std::cout << "  alphabet setup-voice     Install voice dependencies\n";
    std::cout << "  alphabet init         Create new project\n";
    std::cout << "  alphabet test         Run test files\n";
    std::cout << "  alphabet info         Show project info\n\n";
    std::cout << "Examples:\n";
    std::cout << "  alphabet program.abc          Run a program\n";
    std::cout << "  alphabet -c program.abc       Compile only\n";
    std::cout << "  alphabet --repl               Interactive mode\n";
    std::cout << "  alphabet --lsp                LSP server for VS Code\n";
    std::cout << "  alphabet --dump-bytecode prog.abc  Inspect bytecode\n";
    std::cout << "  alphabet --debug program.abc  Debug with breakpoints\n";
}

void run_source(const std::string& source, bool debug_mode = false, const std::string& source_dir = "",
                bool sandbox_mode = false) {
    try {
        alphabet::Lexer lexer(source);
        std::vector<alphabet::Token> tokens = lexer.scan_tokens();

        alphabet::Parser parser(tokens, source);
        std::vector<alphabet::StmtPtr> statements = parser.parse();

        if (parser.had_errors()) {
            const auto& errors = parser.errors();
            if (errors.empty()) {
                throw alphabet::ParseError("Syntax errors in source code");
            }
            std::string msg;
            for (size_t i = 0; i < errors.size(); ++i) {
                if (i > 0)
                    msg += "\n";
                msg += errors[i];
            }
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
        std::cerr << RED("Error: ") << e.what() << "\n";
        std::cerr << "  Add '#alphabet<lang>' as the first line of your source file.\n";
    } catch (const alphabet::ParseError& e) {
        std::cerr << RED("Parse Error: ") << e.what() << "\n";
    } catch (const alphabet::CompileError& e) {
        std::cerr << RED("Compile Error: ") << e.what() << "\n";
    } catch (const alphabet::RuntimeError& e) {
        std::cerr << RED("Runtime Error: ") << e.what() << "\n";
    } catch (const std::exception& e) {
        std::cerr << RED("Error: ") << e.what() << "\n";
    }
}

namespace repl {
volatile std::sig_atomic_t g_interrupted = 0;

void sigint_handler(int) {
    g_interrupted = 1;
}

int count_braces_safe(const std::string& line) {
    int depth = 0;
    bool in_string = false;
    char quote_char = 0;

    for (size_t i = 0; i < line.size(); ++i) {
        char c = line[i];

        if (in_string) {
            if (c == '\\' && i + 1 < line.size()) {
                ++i;
                continue;
            }
            if (c == quote_char)
                in_string = false;
            continue;
        }

        if (c == '"' || c == '\'') {
            in_string = true;
            quote_char = c;
        } else if (c == '{') {
            ++depth;
        } else if (c == '}') {
            --depth;
        }
    }
    return depth;
}

bool is_command(const std::string& line) {
    if (line == "quit" || line == "exit" || line == "help" || line == "clear" || line == "reset" || line == "reload" ||
        line == "vars" || line == "history" || line == "keywords" || line == "builtins" || line == "voice" ||
        line == "lang" || line == "trace" || line == "trace on" || line == "trace off" || line == "trace slow" || line == "trace fast" || line == "!!" || (line.size() > 1 && line[0] == '!' && line[1] != '!'))
        return true;

    if (line.rfind("#alphabet<", 0) == 0 && line.size() > 10 && line.back() == '>')
        return true;
    return false;
}

} // namespace repl

static std::string format_operand(const alphabet::Operand& op) {
    return std::visit([](const auto& v) -> std::string {
        using T = std::decay_t<decltype(v)>;
        if constexpr (std::is_same_v<T, std::monostate>) return "";
        else if constexpr (std::is_same_v<T, int64_t>) return std::to_string(v);
        else if constexpr (std::is_same_v<T, double>) {
            std::string s = std::to_string(v);
            if (s.find('.') != std::string::npos && s.find_last_not_of('0') == s.find('.'))
                s.erase(s.find('.'));
            return s;
        }
        else if constexpr (std::is_same_v<T, std::string>) return "\"" + v + "\"";
        else if constexpr (std::is_same_v<T, std::nullptr_t>) return "null";
        else if constexpr (std::is_same_v<T, std::pair<std::string, int>>)
            return v.first + "@" + std::to_string(v.second);
        return "";
    }, op);
}

static std::string format_instruction(const alphabet::Instruction& instr, size_t index) {
    std::string result = std::to_string(index) + ": ";
    result += alphabet::opcode_to_string(instr.op);
    std::string operand = format_operand(instr.operand);
    if (!operand.empty()) result += " " + operand;
    return result;
}

static void slow_delay(bool slow) {
    if (slow) std::this_thread::sleep_for(std::chrono::milliseconds(80));
}

static void slow_delay_long(bool slow) {
    if (slow) std::this_thread::sleep_for(std::chrono::milliseconds(200));
}

void start_repl() {
#ifdef _WIN32
    // Enable ANSI escape codes on Windows 10+
    HANDLE hOut = GetStdHandle(STD_OUTPUT_HANDLE);
    if (hOut != INVALID_HANDLE_VALUE) {
        DWORD dwMode = 0;
        if (GetConsoleMode(hOut, &dwMode)) {
            dwMode |= ENABLE_VIRTUAL_TERMINAL_PROCESSING;
            SetConsoleMode(hOut, dwMode);
        }
    }
#endif

    std::cout << LOGO;
    std::cout << "Alphabet Language [v" << VERSION << " - Native C++]\n";
    std::cout << "Developed by " << DEVELOPER << "\n";
    std::cout << "\nType 'help' for commands, 'q' to exit.\n\n";

    std::signal(SIGINT, repl::sigint_handler);

    ffi_init();

    std::vector<std::string> history;
#ifdef _WIN32
    const char* home = std::getenv("USERPROFILE");
#else
    const char* home = std::getenv("HOME");
#endif
    std::string history_path = home ? (std::string(home) + "/.alphabet_history") : ".alphabet_history";
    {
        std::ifstream hist_file(history_path);
        std::string hist_line;
        while (std::getline(hist_file, hist_line)) {
            if (!hist_line.empty())
                history.push_back(hist_line);
        }
    }
    auto save_history = [&]() {
        std::ofstream hist_file(history_path);
        size_t start = history.size() > 500 ? history.size() - 500 : 0;
        for (size_t i = start; i < history.size(); ++i) {
            hist_file << history[i] << "\n";
        }
    };

    std::string line;
    std::string buffer;
    int brace_depth = 0;

    std::string all_source;
    std::string repl_lang = "en";
    bool trace_mode = true;
    bool trace_slow = false;
    std::unordered_map<std::string, alphabet::Value> saved_globals;
    alphabet::VM vm;
    size_t prev_bytecode_size = 0;

    while (true) {
        if (repl::g_interrupted) {
            repl::g_interrupted = 0;
            if (!buffer.empty()) {
                std::cout << "\n... cancelled\n";
                buffer.clear();
                brace_depth = 0;
            } else {
                std::cout << "\n";
            }
        }

        if (buffer.empty()) {
            std::cout << "\u256d\u2500[>>>] ";
        } else {
            std::string indent(brace_depth * 2, ' ');
            std::cout << "\u2570\u2500[...]" << indent;
        }
        std::cout.flush();

        if (!std::getline(std::cin, line)) {
            std::cout << "\n";
            break;
        }

        if (repl::g_interrupted) {
            repl::g_interrupted = 0;
            if (!buffer.empty()) {
                std::cout << "... cancelled\n";
                buffer.clear();
                brace_depth = 0;
            }
            continue;
        }

        if (buffer.empty() && (line == "quit" || line == "exit")) {
            save_history();
            break;
        }

        if (buffer.empty() && line == "help") {
            std::cout << "Commands:\n";
            std::cout << "  help          Show this message\n";
            std::cout << "  history       Show last 20 entries\n";
            std::cout << "  !N            Repeat history entry #N\n";
            std::cout << "  !!            Repeat last entry\n";
            std::cout << "  vars          Show defined variables\n";
            std::cout << "  lang          Show current language\n";
            std::cout << "  keywords      Show keywords for current language\n";
            std::cout << "  builtins      Show all available builtins\n";
            std::cout << "  voice         Voice-to-text input (checks deps first)\n";
            std::cout << "  trace         Show trace mode status\n";
            std::cout << "  trace on      Enable execution trace (on by default)\n";
            std::cout << "  trace off     Disable execution trace\n";
            std::cout << "  trace slow    Watch each step happen gradually\n";
            std::cout << "  trace fast    Instant results (default)\n";
            std::cout << "  clear         Clear screen (banner redraws)\n";
            std::cout << "  clear history Clear command history\n";
            std::cout << "  clear all     Clear screen + history\n";
            std::cout << "  reset         Clear REPL state (variables + code)\n";
            std::cout << "  reload        Clear variables, keep code\n";
            std::cout << "  quit/exit     Exit\n";
            std::cout << "\nLanguage: Type \"#alphabet<lang>\" as first line to switch.\n";
            std::cout << "  en=English  am=Amharic  es=Spanish  fr=French  de=German\n";
            std::cout << "  Current: " << repl_lang << "\n";
            std::cout << "\nMulti-line: Type '{' to start a block.\n";
            std::cout << "Expressions: Type any expression to evaluate.\n";
            std::cout << "Ctrl+C: Cancel current input line.\n";
            continue;
        }

        if (buffer.empty() && (line == "clear" || line == "clear history" || line == "clear all")) {
            if (line == "clear history" || line == "clear all") {
                history.clear();
                std::ofstream hist_file(history_path, std::ios::trunc);
                std::cout << "History cleared.\n";
            }
            if (line == "clear" || line == "clear all") {
#ifdef _WIN32
                int rc = system("cls");
#else
                int rc = system("clear");
#endif
                (void)rc;
                std::cout << LOGO;
                std::cout << "Alphabet Language [v" << VERSION << " - Native C++]\n";
                std::cout << "Developed by " << DEVELOPER << "\n";
                std::cout << "\nType 'help' for commands, 'q' to exit.\n\n";
            }
            continue;
        }

        if (buffer.empty() && line == "reset") {
            all_source.clear();
            saved_globals.clear();
            repl_lang = "en";
            buffer.clear();
            brace_depth = 0;
            prev_bytecode_size = 0;
            std::cout << "State cleared. Language reset to en.\n";
            continue;
        }

        if (buffer.empty() && line == "reload") {
            saved_globals.clear();
            prev_bytecode_size = 0;
            std::cout << "Variables cleared. Code preserved.\n";
            continue;
        }

        if (buffer.empty() && line == "lang") {
            static const std::unordered_map<std::string, std::string> lang_names = {{"en", "English"},
                                                                                    {"am", "Amharic (አማርኛ)"},
                                                                                    {"es", "Spanish (Español)"},
                                                                                    {"fr", "French (Français)"},
                                                                                    {"de", "German (Deutsch)"}};
            auto it = lang_names.find(repl_lang);
            std::string name = it != lang_names.end() ? it->second : repl_lang;
            std::cout << "Current language: " << repl_lang << " (" << name << ")\n";
            continue;
        }

        if (buffer.empty() && line == "trace on") {
            trace_mode = true;
            std::cout << "Trace mode: ON\n";
            continue;
        }
        if (buffer.empty() && line == "trace off") {
            trace_mode = false;
            std::cout << "Trace mode: OFF\n";
            continue;
        }
        if (buffer.empty() && line == "trace") {
            std::cout << "Trace mode: " << (trace_mode ? "ON" : "OFF");
            if (trace_mode) std::cout << " (" << (trace_slow ? "slow" : "fast") << ")";
            std::cout << "\n";
            std::cout << "Commands: trace on, trace off, trace slow, trace fast\n";
            continue;
        }
        if (buffer.empty() && line == "trace slow") {
            trace_slow = true;
            trace_mode = true;
            std::cout << "Trace mode: ON (slow - watch each step happen)\n";
            continue;
        }
        if (buffer.empty() && line == "trace fast") {
            trace_slow = false;
            std::cout << "Trace mode: " << (trace_mode ? "ON" : "OFF") << " (fast - instant results)\n";
            continue;
        }

        if (buffer.empty() && line == "vars") {
            std::cout << "  language: " << repl_lang << "\n";
            if (saved_globals.empty()) {
                std::cout << "  (no variables defined)\n";
            } else {
                for (const auto& [name, val] : saved_globals) {
                    std::cout << "  " << name << " = " << alphabet::value_to_string(val) << "\n";
                }
            }
            continue;
        }

        if (buffer.empty() && line == "keywords") {
            auto lang_it = alphabet::KEYWORD_MAPPINGS.find(repl_lang);
            if (lang_it == alphabet::KEYWORD_MAPPINGS.end()) {
                std::cerr << "No keyword mapping for language: " << repl_lang << "\n";
                continue;
            }
            const auto& kw = lang_it->second;

            static const std::unordered_map<std::string, std::string> lang_names = {{"en", "English"},
                                                                                    {"am", "Amharic (አማርኛ)"},
                                                                                    {"es", "Spanish (Español)"},
                                                                                    {"fr", "French (Français)"},
                                                                                    {"de", "German (Deutsch)"}};
            auto name_it = lang_names.find(repl_lang);
            std::string display_name = name_it != lang_names.end() ? name_it->second : repl_lang;

            std::unordered_map<std::string, std::vector<std::string>> reverse;
            for (const auto& [native, letter] : kw) {
                reverse[letter].push_back(native);
            }

            struct Cat {
                std::string label;
                std::vector<std::pair<std::string, std::string>> items;
            };
            std::vector<Cat> categories;

            Cat ctrl{"Control Flow", {}};
            for (auto& l : {"i", "e", "l", "b", "k", "r"}) {
                if (reverse.count(l)) {
                    for (auto& n : reverse[l])
                        ctrl.items.emplace_back(n, l);
                }
            }
            categories.push_back(ctrl);

            Cat oop{"OOP", {}};
            for (auto& l : {"c", "a", "j", "m", "n", "v", "p", "s", "^"}) {
                if (reverse.count(l)) {
                    for (auto& n : reverse[l])
                        oop.items.emplace_back(n, l);
                }
            }
            categories.push_back(oop);

            Cat err{"Error Handling", {}};
            for (auto& l : {"t", "h"}) {
                if (reverse.count(l)) {
                    for (auto& n : reverse[l])
                        err.items.emplace_back(n, l);
                }
            }
            categories.push_back(err);

            Cat io{"I/O & Modules", {}};
            for (auto& l : {"z", "z.i", "x", "q", "@"}) {
                if (reverse.count(l)) {
                    for (auto& n : reverse[l])
                        io.items.emplace_back(n, l);
                }
            }
            categories.push_back(io);

            Cat misc{"Other", {}};
            for (const auto& l : {"\x80"}) {
                if (reverse.count(l)) {
                    for (auto& n : reverse[l])
                        misc.items.emplace_back(n, "const");
                }
            }
            if (!misc.items.empty())
                categories.push_back(misc);

            std::cout << "Keywords: " << display_name << " (current: " << repl_lang << ")\n";
            std::cout << "─────────────────────────────────────\n";

            for (const auto& cat : categories) {
                std::cout << "\n  " << cat.label << ":\n";
                for (const auto& [native, letter] : cat.items) {
                    std::string padded = native;

                    size_t char_count =
                        std::count_if(native.begin(), native.end(), [](unsigned char c) { return (c & 0xC0) != 0x80; });

                    if (char_count < 16) {
                        padded += std::string(16 - char_count, ' ');
                    }
                    std::cout << "    " << padded << "→ " << letter << "\n";
                }
            }

            std::cout << "\n─────────────────────────────────────\n";
            std::cout << "  Special: @ = export, ^ = extends\n";
            std::cout << "  Type \"#alphabet<lang>\" to switch language.\n";
            continue;
        }

        if (buffer.empty() && line == "builtins") {
            std::cout << "Builtins (work with or without z. prefix):\n";
            std::cout << "─────────────────────────────────────────\n";
            std::cout << "  I/O:        o, i, f, fw, fa, exists, file_size, exec, system, exit, args\n";
            std::cout << "  Math:       sqrt, sin, cos, tan, abs, floor, ceil, round, pow, min, max\n";
            std::cout << "  String:     len, tostr, tonum, type, split, join, replace, trim, upper, lower\n";
            std::cout << "  String:     substr, find, count, chr, ord, starts_with, ends_with\n";
            std::cout << "  List:       append, pop_back, contains, reverse, sort, insert, remove, slice\n";
            std::cout << "  List:       swap, unique, flatten, flatten_str, zip, enumerate, sum, avg\n";
            std::cout << "  Set:        set, add, has, set_size\n";
            std::cout << "  Map:        keys, values\n";
            std::cout << "  Functional: map, filter, reduce, range\n";
            std::cout << "  Type:       is_null, is_empty, clamp\n";
            std::cout << "  JSON:       json_parse, json_stringify\n";
            std::cout << "  Network:    http_get, http_post\n";
            std::cout << "  System:     sleep, timestamp, env, rand, randint\n";
            std::cout << "  Assert:     assert, assert_eq\n";
            std::cout << "  Builder:    builder, append_str, build\n";
            std::cout << "─────────────────────────────────────────\n";
            std::cout << "  Use 'alphabet doc <name>' for details.\n";
            continue;
        }

        if (buffer.empty() && line == "voice") {
            // Check if voice server deps are available
            bool has_vosk = system("python3 -c 'import vosk' >/dev/null 2>&1") == 0;
            bool has_pyaudio = system("python3 -c 'import pyaudio' >/dev/null 2>&1") == 0;
            bool has_whisper = system("python3 -c 'import whisper' >/dev/null 2>&1") == 0;
            bool has_arecord = system("which arecord >/dev/null 2>&1") == 0;
            bool has_sox = system("which sox >/dev/null 2>&1") == 0;

            bool has_stt = has_vosk || has_whisper;
            bool has_audio = has_pyaudio || has_arecord || has_sox;

            if (!has_stt || !has_audio) {
                std::cout << "Voice input dependencies not found.\n\n";
                std::cout << "Status:\n";
                std::cout << "  " << (has_vosk ? "✅" : "❌") << " Vosk (STT for en/es/fr/de)\n";
                std::cout << "  " << (has_whisper ? "✅" : "❌") << " Whisper (STT for Amharic)\n";
                std::cout << "  " << (has_pyaudio ? "✅" : "❌") << " PyAudio (microphone)\n";
                std::cout << "  " << (has_arecord ? "✅" : "❌") << " arecord (Linux fallback)\n";
                std::cout << "  " << (has_sox ? "✅" : "❌") << " sox (cross-platform fallback)\n\n";
                std::cout << "Run 'setup-voice' to install dependencies.\n";
                continue;
            }

            alphabet::VoiceInput voice;
            std::cout << "Starting voice input...\n";

            if (!voice.start()) {
                std::cerr << "Failed to start voice server.\n";
                std::cerr << "Install dependencies:\n";
                std::cerr << "  pip install vosk pyaudio\n";
                std::cerr << "  pip install openai-whisper  (for Amharic)\n";
                continue;
            }

            // Initialize for current language
            if (!voice.init_language(repl_lang)) {
                std::cerr << "Failed to initialize voice for language: " << repl_lang << "\n";
                voice.stop();
                continue;
            }

            std::cout << "Listening... (speak your code in " << repl_lang << ")\n";
            std::cout << "Speak now...\n";

            std::string text = voice.listen(15);
            voice.stop();

            if (text.empty()) {
                std::cout << "No speech detected.\n";
            } else {
                std::cout << "Heard: " << text << "\n";

                // Convert NL to code
                alphabet::NLToCode converter;
                std::string code = converter.convert(text, repl_lang);

                if (code != text) {
                    std::cout << "Code: " << code << "\n";
                }

                // Fill buffer (don't auto-execute)
                std::cout << "Press Enter to run, or edit first.\n";
                buffer = code;
            }
            continue;
        }

        if (buffer.empty() && line.rfind("#alphabet<", 0) == 0 && line.size() > 10 && line.back() == '>') {
            std::string new_lang = line.substr(10, line.size() - 11);
            static const std::unordered_set<std::string> valid_langs = {"en", "am", "es", "fr", "de"};
            if (valid_langs.count(new_lang)) {
                repl_lang = new_lang;
                all_source.clear();
                saved_globals.clear();
                prev_bytecode_size = 0;
                std::cout << "Language set to " << repl_lang << ".\n";
            } else {
                std::cerr << "Unknown language: " << new_lang << ". Valid: en, am, es, fr, de\n";
            }
            continue;
        }

        if (buffer.empty() && line == "history") {
            size_t start = history.size() > 20 ? history.size() - 20 : 0;
            for (size_t i = start; i < history.size(); ++i) {
                std::cout << "  " << (i + 1) << "  " << history[i] << "\n";
            }
            continue;
        }

        if (buffer.empty() && line.size() > 1 && line[0] == '!' && line[1] != '!') {
            std::string num_str = line.substr(1);
            bool is_num = !num_str.empty() && std::all_of(num_str.begin(), num_str.end(), [](char c) {
                return std::isdigit(static_cast<unsigned char>(c));
            });
            if (is_num) {
                size_t idx = std::stoul(num_str);
                if (idx >= 1 && idx <= history.size()) {
                    line = history[idx - 1];
                    std::cout << line << "\n";
                } else {
                    std::cerr << "History index out of range (1-" << history.size() << ")\n";
                    continue;
                }
            } else {
                std::cerr << "Usage: !N (where N is a number)\n";
                continue;
            }
        }

        if (buffer.empty() && line == "!!") {
            if (!history.empty()) {
                line = history.back();
                std::cout << line << "\n";
            } else {
                std::cout << "(no history)\n";
                continue;
            }
        }

        if (buffer.empty() && line.empty()) {
            continue;
        }

        if (buffer.empty() && !repl::is_command(line)) {
            if (history.empty() || history.back() != line)
                history.push_back(line);
        }

        brace_depth += repl::count_braces_safe(line);

        if (!buffer.empty())
            buffer += '\n';
        buffer += line;

        if (brace_depth == 0) {
            all_source += buffer + "\n";

            auto rollback_last_line = [&all_source]() {
                size_t last_newline = all_source.rfind('\n', all_source.size() - 2);
                if (last_newline != std::string::npos) {
                    all_source.resize(last_newline + 1);
                } else {
                    all_source.clear();
                }
            };

            try {
                std::string full_source = "#alphabet<" + repl_lang + ">\n" + all_source;

                std::string new_input;
                {
                    std::istringstream iss(all_source);
                    std::string line;
                    while (std::getline(iss, line)) {
                        if (!line.empty()) new_input = line;
                    }
                }

                if (trace_mode && !new_input.empty()) {
                    alphabet::Lexer new_lexer(new_input, true);
                    auto new_tokens = new_lexer.scan_tokens();
                    std::cout << CYAN("┌─ Tokenizing ─────────────────────────────┐\n");
                    std::cout.flush();
                    slow_delay_long(trace_slow);
                    std::string line_buf;
                    int col = 0;
                    for (const auto& tok : new_tokens) {
                        if (tok.type == alphabet::TokenType::EOF_TOKEN) break;
                        std::string formatted = std::string(tok.lexeme);
                        if (col + (int)formatted.size() + 1 > 56) {
                            std::cout << CYAN("│ ") << line_buf << "\n";
                            std::cout.flush();
                            slow_delay(trace_slow);
                            line_buf.clear();
                            col = 0;
                        }
                        if (!line_buf.empty()) { line_buf += " "; col++; }
                        line_buf += formatted;
                        col += (int)formatted.size();
                        slow_delay(trace_slow);
                    }
                    if (!line_buf.empty()) {
                        std::cout << CYAN("│ ") << line_buf << "\n";
                        std::cout.flush();
                    }
                    slow_delay_long(trace_slow);
                    std::cout << CYAN("└──────────────────────────────────────────┘\n");
                    std::cout.flush();
                }

                alphabet::Lexer lexer(full_source);
                auto tokens = lexer.scan_tokens();

                alphabet::Parser parser(tokens, full_source);
                auto statements = parser.parse();

                if (parser.had_errors()) {
                    if (trace_mode) {
                        std::cout << RED("┌─ Errors ─────────────────────────────────┐\n");
                        for (const auto& err : parser.errors()) {
                            std::cout << RED("│ ") << err << "\n";
                        }
                        if (parser.errors().empty()) {
                            std::cout << RED("│ ") << "Syntax errors in source code\n";
                        }
                        std::cout << RED("└──────────────────────────────────────────┘\n");
                    } else {
                        for (const auto& err : parser.errors()) {
                            std::cerr << err << "\n";
                        }
                        if (parser.errors().empty()) {
                            std::cerr << "Syntax errors in source code\n";
                        }
                    }
                    rollback_last_line();
                } else {
                    if (trace_mode) {
                        std::cout << CYAN("┌─ Parsing ────────────────────────────────┐\n");
                        std::cout.flush();
                        slow_delay_long(trace_slow);
                        for (size_t i = 0; i < statements.size(); ++i) {
                            std::cout << CYAN("│ ") << alphabet::stmt_to_string(statements[i]) << "\n";
                            std::cout.flush();
                            slow_delay(trace_slow);
                        }
                        slow_delay_long(trace_slow);
                        std::cout << CYAN("└──────────────────────────────────────────┘\n");
                        std::cout.flush();
                    }

                    alphabet::Compiler compiler;
                    alphabet::Program program = compiler.compile(statements);

                    if (trace_mode) {
                        std::cout << CYAN("┌─ Compiling ──────────────────────────────┐\n");
                        std::cout.flush();
                        slow_delay_long(trace_slow);
                        size_t start = prev_bytecode_size;
                        for (size_t i = start; i < program.main.size(); ++i) {
                            std::cout << CYAN("│ ") << format_instruction(program.main[i], i - start) << "\n";
                            std::cout.flush();
                            slow_delay(trace_slow);
                        }
                        for (const auto& [id, cls] : program.classes) {
                            std::cout << CYAN("│ ") << "CLASS " << cls.name << "\n";
                            std::cout.flush();
                            slow_delay(trace_slow);
                            for (const auto& [mname, m] : cls.methods) {
                                std::cout << CYAN("│ ") << "  METHOD " << mname << " (" << m.bytecode.size() << " instructions)\n";
                                std::cout.flush();
                                slow_delay(trace_slow);
                            }
                        }
                        for (const auto& [fname, fn] : program.functions) {
                            std::cout << CYAN("│ ") << "FUNC " << fname << " (" << fn.bytecode.size() << " instructions)\n";
                            std::cout.flush();
                            slow_delay(trace_slow);
                        }
                        slow_delay_long(trace_slow);
                        std::cout << CYAN("└──────────────────────────────────────────┘\n");
                        std::cout.flush();
                    }

                    vm.init(program);
                    vm.set_globals(saved_globals);
                    vm.set_executed_up_to(prev_bytecode_size);
                    vm.clear_unhandled_error();

                    int saved_stderr = -1;
                    if (trace_mode) {
                        saved_stderr = dup(STDERR_FILENO);
                        freopen("/dev/null", "w", stderr);
                        std::cout << CYAN("┌─ Executing ──────────────────────────────┐\n");
                        std::cout.flush();
                        size_t exec_idx = 0;
                        size_t skip_up_to = prev_bytecode_size;
                        vm.set_trace_callback([&, exec_idx, skip_up_to](const alphabet::Instruction& instr, size_t stack_depth, const std::string&) mutable {
                            std::string output = vm.consume_output_buffer();
                            if (!output.empty()) {
                                std::cout << CYAN("│ ") << "→ " << output << "\n";
                                std::cout.flush();
                                slow_delay(trace_slow);
                            }
                            if (exec_idx >= skip_up_to) {
                                std::string line = format_instruction(instr, exec_idx - skip_up_to);
                                std::string padding;
                                if (line.size() < 56) padding = std::string(56 - line.size(), ' ');
                                std::cout << CYAN("│ ") << line << padding << CYAN("│ depth:") << stack_depth << "\n";
                                std::cout.flush();
                                slow_delay(trace_slow);
                            }
                            exec_idx++;
                        });
                    }

                    vm.run();

                    if (trace_mode) {
                        vm.set_trace_callback(nullptr);
                        std::cout << CYAN("└──────────────────────────────────────────┘\n");
                        std::cout.flush();
                    }

                    if (trace_mode && saved_stderr >= 0) {
                        fflush(stderr);
                        dup2(saved_stderr, STDERR_FILENO);
                        close(saved_stderr);
                        stderr = fdopen(STDERR_FILENO, "w");
                    }

                    if (!vm.get_unhandled_error().empty()) {
                        if (trace_mode) {
                            std::cout << RED("┌─ Errors ─────────────────────────────────┐\n");
                            std::cout << RED("│ ") << vm.get_unhandled_error() << "\n";
                            int err_line = vm.get_last_line();
                            if (err_line > 0) {
                                std::istringstream iss(all_source);
                                std::string src_line;
                                for (int i = 0; i < err_line && std::getline(iss, src_line); ++i) {
                                }
                                if (!src_line.empty()) {
                                    std::cout << RED("│ ") << "  " << err_line << " | " << src_line << "\n";
                                }
                            }
                            std::cout << RED("└──────────────────────────────────────────┘\n");
                        } else {
                            std::cerr << RED("Runtime Error: ") << vm.get_unhandled_error() << "\n";
                            int err_line = vm.get_last_line();
                            if (err_line > 0) {
                                std::istringstream iss(all_source);
                                std::string src_line;
                                for (int i = 0; i < err_line && std::getline(iss, src_line); ++i) {
                                }
                                if (!src_line.empty()) {
                                    std::cerr << "  " << err_line << " | " << src_line << "\n";
                                }
                            }
                        }
                    }
                    prev_bytecode_size = program.main.size();
                    saved_globals = vm.get_globals();
                }
            } catch (const alphabet::MissingLanguageHeader&) {
                if (trace_mode) {
                    std::cout << RED("┌─ Errors ─────────────────────────────────┐\n");
                    std::cout << RED("│ ") << "Missing language header\n";
                    std::cout << RED("└──────────────────────────────────────────┘\n");
                } else {
                    std::cerr << RED("Error: Missing header") << "\n";
                }
                rollback_last_line();
            } catch (const alphabet::ParseError& e) {
                if (trace_mode) {
                    std::cout << RED("┌─ Errors ─────────────────────────────────┐\n");
                    std::cout << RED("│ ") << e.what() << "\n";
                    std::cout << RED("└──────────────────────────────────────────┘\n");
                } else {
                    std::cerr << RED("Parse Error: ") << e.what() << "\n";
                }
                rollback_last_line();
            } catch (const alphabet::CompileError& e) {
                if (trace_mode) {
                    std::cout << RED("┌─ Errors ─────────────────────────────────┐\n");
                    std::cout << RED("│ ") << e.what() << "\n";
                    std::cout << RED("└──────────────────────────────────────────┘\n");
                } else {
                    std::cerr << RED("Compile Error: ") << e.what() << "\n";
                }
                rollback_last_line();
            } catch (const alphabet::RuntimeError& e) {
                if (trace_mode) {
                    std::cout << RED("┌─ Errors ─────────────────────────────────┐\n");
                    std::cout << RED("│ ") << e.what() << "\n";
                    int err_line = vm.get_last_line();
                    if (err_line > 0) {
                        std::istringstream iss(all_source);
                        std::string src_line;
                        for (int i = 0; i < err_line && std::getline(iss, src_line); ++i) {
                        }
                        if (!src_line.empty()) {
                            std::cout << RED("│ ") << "  " << err_line << " | " << src_line << "\n";
                        }
                    }
                    std::cout << RED("└──────────────────────────────────────────┘\n");
                } else {
                    std::cerr << RED("Runtime Error: ") << e.what() << "\n";
                    int err_line = vm.get_last_line();
                    if (err_line > 0) {
                        std::istringstream iss(all_source);
                        std::string src_line;
                        for (int i = 0; i < err_line && std::getline(iss, src_line); ++i) {
                        }
                        if (!src_line.empty()) {
                            std::cerr << "  " << err_line << " | " << src_line << "\n";
                        }
                    }
                }
            } catch (const std::exception& e) {
                if (trace_mode) {
                    std::cout << RED("┌─ Errors ─────────────────────────────────┐\n");
                    std::cout << RED("│ ") << e.what() << "\n";
                    std::cout << RED("└──────────────────────────────────────────┘\n");
                } else {
                    std::cerr << "Error: " << e.what() << "\n";
                }
                rollback_last_line();
            }

            buffer.clear();
            brace_depth = 0;
        }
    }

    ffi_cleanup();
}

bool is_safe_path(const std::string& path) {
    if (path.find("..") != std::string::npos)
        return false;
    return std::none_of(path.begin(), path.end(), [](char c) {
        return c == ';' || c == '|' || c == '&' || c == '$' || c == '`' || c == '(' || c == ')' || c == '{' ||
               c == '}' || c == '<' || c == '>' || c == '\n' || c == '\r' || c == '\\' || c == '\'' || c == '"' ||
               c == '*' || c == '?' || c == '[' || c == ']' || c == '#' || c == '~' || c == '%' || c == '\0';
    });
}

#ifndef _WIN32
int exec_curl(const std::string& url, const std::string& output_path) {
    pid_t pid = fork();
    if (pid < 0)
        return -1;
    if (pid == 0) {
        execlp("curl", "curl", "-fsSL", "-o", output_path.c_str(), url.c_str(), nullptr);
        _exit(127);
    }
    int status = 0;
    waitpid(pid, &status, 0);
    if (WIFEXITED(status))
        return WEXITSTATUS(status);
    return -1;
}

int exec_mv(const std::string& src, const std::string& dst) {
    pid_t pid = fork();
    if (pid < 0)
        return -1;
    if (pid == 0) {
        execlp("mv", "mv", "-f", src.c_str(), dst.c_str(), nullptr);
        _exit(127);
    }
    int status = 0;
    waitpid(pid, &status, 0);
    if (WIFEXITED(status))
        return WEXITSTATUS(status);
    return -1;
}

std::string compute_sha256(const std::string& path) {
    int pipefd[2];
    if (pipe(pipefd) != 0)
        return "";
    pid_t pid = fork();
    if (pid < 0) {
        close(pipefd[0]);
        close(pipefd[1]);
        return "";
    }
    if (pid == 0) {
        close(pipefd[0]);
        dup2(pipefd[1], STDOUT_FILENO);
        close(pipefd[1]);
        execlp("sha256sum", "sha256sum", path.c_str(), nullptr);
        _exit(127);
    }
    close(pipefd[1]);
    char buf[256];
    std::string result;
    ssize_t n;
    while ((n = read(pipefd[0], buf, sizeof(buf))) > 0) {
        result.append(buf, n);
    }
    close(pipefd[0]);
    waitpid(pid, nullptr, 0);
    size_t sp = result.find(' ');
    if (sp != std::string::npos)
        return result.substr(0, sp);
    return "";
}
#endif

void do_update(bool force = false) {
    std::cout << "Checking for updates...\n";

    std::string api_cmd = "curl -fsSL https://api.github.com/repos/fraol163/alphabet/releases/latest";
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

    if (status != 0 || response.empty()) {
        std::cerr << "Error: Could not reach GitHub\n";
        return;
    }

    std::string latest_version;
    size_t tag_pos = response.find("\"tag_name\"");
    if (tag_pos != std::string::npos) {
        size_t val_start = response.find('"', tag_pos + 10);
        if (val_start != std::string::npos) {
            ++val_start;
            size_t val_end = response.find('"', val_start);
            if (val_end != std::string::npos) {
                latest_version = response.substr(val_start, val_end - val_start);
                if (!latest_version.empty() && latest_version[0] == 'v') {
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

    auto version_tuple = [](const std::string& v) -> std::tuple<int, int, int> {
        int a = 0, b = 0, c = 0;
        sscanf(v.c_str(), "%d.%d.%d", &a, &b, &c);
        return {a, b, c};
    };

    if (version_tuple(latest_version) <= version_tuple(VERSION) && !force) {
        std::cout << "Already up to date!\n";
        std::cout << "Use 'alphabet update --force' to reinstall current version.\n";
        return;
    }

    std::string os = "linux";
    std::string arch = "amd64";
    std::string ext;
#ifdef _WIN32
    os = "windows";
    ext = ".exe";
#elif defined(__APPLE__)
    os = "macos";
#if defined(__aarch64__) || defined(__arm64__) || defined(_M_ARM64)
    arch = "arm64";
#else
    arch = "amd64";
#endif
#elif defined(__aarch64__) || defined(__arm64__)
    arch = "arm64";
#endif

    std::string asset_name = "alphabet-" + os + "-" + arch + ext;
    std::string download_url =
        "https://github.com/fraol163/alphabet/releases/download/v" + latest_version + "/" + asset_name;
    if (download_url.find("https://github.com/") != 0 || !is_safe_path(download_url)) {
        std::cerr << "Error: Invalid download URL\n";
        return;
    }

    std::string checksum_url = download_url + ".sha256";
    std::cout << "Downloading " << download_url << "...\n";

    std::string self_path;
    char self_buf[4096];
#ifdef _WIN32
    DWORD len = GetModuleFileName(NULL, self_buf, sizeof(self_buf) - 1);
    if (len > 0) {
        self_buf[len] = '\0';
        self_path = self_buf;
    } else {
        self_path = "C:\\Program Files\\alphabet\\alphabet.exe";
    }
#else
    ssize_t len = readlink("/proc/self/exe", self_buf, sizeof(self_buf) - 1);
    if (len > 0) {
        self_buf[len] = '\0';
        self_path = self_buf;
    } else {
        self_path = "/usr/local/bin/alphabet";
    }
#endif

    std::string tmp_path = self_path + ".tmp";

    if (!is_safe_path(self_path) || !is_safe_path(tmp_path)) {
        std::cerr << "Error: Invalid installation path\n";
        return;
    }

#ifdef _WIN32
    std::string dl_cmd = "curl -fsSL -o \"" + tmp_path + "\" \"" + download_url + "\"";
    int dl_status = system(dl_cmd.c_str());
#else
    int dl_status = exec_curl(download_url, tmp_path);
#endif
    if (dl_status != 0) {
        std::cerr << "Error: Download failed\n";
#ifdef _WIN32
        _unlink(tmp_path.c_str());
#else
        unlink(tmp_path.c_str());
#endif
        return;
    }

#ifndef _WIN32
    std::string expected_hash;
    int hash_dl = exec_curl(checksum_url, tmp_path + ".sha256");
    if (hash_dl == 0) {
        std::ifstream hf(tmp_path + ".sha256");
        if (hf.good())
            hf >> expected_hash;
        unlink((tmp_path + ".sha256").c_str());
    }
    if (!expected_hash.empty()) {
        std::string actual_hash = compute_sha256(tmp_path);
        if (actual_hash != expected_hash) {
            std::cerr << "Error: Checksum mismatch\n";
            unlink(tmp_path.c_str());
            return;
        }
    }

    chmod(tmp_path.c_str(), 0755);

    std::cout << "Installing update...\n";
    int mv_status = exec_mv(tmp_path, self_path);
    if (mv_status != 0) {
        std::cerr << "Error: Could not install update\n";
        unlink(tmp_path.c_str());
        return;
    }
#else
    std::cout << "Installing update...\n";
    std::string pid = std::to_string(GetCurrentProcessId());
    std::string ps_script = "while (Get-Process -Id " + pid +
                            " -ErrorAction SilentlyContinue) { Start-Sleep -Milliseconds 500 }; "
                            "Move-Item -Force '" +
                            tmp_path + "' '" + self_path + "'";
    std::string ps_cmd = "start \"\" /b powershell -NoProfile -WindowStyle Hidden -Command \"" + ps_script + "\"";
    int ps_status = system(ps_cmd.c_str());
    if (ps_status != 0) {
        std::cerr << "Error: Could not schedule update installation\n";
        _unlink(tmp_path.c_str());
        return;
    }
#endif

    std::cout << "Updated to v" << latest_version << "!\n";
}

std::string read_input(const std::string& path) {
    if (path == "-") {
        std::ostringstream oss;
        oss << std::cin.rdbuf();
        return oss.str();
    }

    std::ifstream file(path);
    if (!file.is_open()) {
        throw std::runtime_error("Cannot open file: " + path);
    }

    std::ostringstream oss;
    oss << file.rdbuf();
    return oss.str();
}

} // namespace

int main(int argc, char* argv[]) {
    bool compile_only = false;
    bool repl_mode = false;
    bool lsp_mode = false;
    bool debug_mode = false;
    bool sandbox_mode = false;
    bool dump_bytecode = false;
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

        if (arg == "--dump-bytecode") {
            dump_bytecode = true;
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

        if (arg[0] == '-' && arg.size() > 1) {
            std::cerr << "Unknown option: " << arg << "\n";
            std::cerr << "Use --help for usage information\n";
            return 1;
        }

        if (arg == "update" || arg == "upgrade") {
            bool force = false;
            if (i + 1 < argc && std::string(argv[i + 1]) == "--force") {
                force = true;
                ++i;
            }
            do_update(force);
            return 0;
        }

        if (arg == "setup-voice") {
            std::cout << "╔══════════════════════════════════════════════════════╗\n";
            std::cout << "║  ALPHABET VOICE INPUT SETUP                          ║\n";
            std::cout << "╚══════════════════════════════════════════════════════╝\n\n";

            // Detect OS
            std::string os_name = "unknown";
#ifdef __linux__
            os_name = "Linux";
#elif __APPLE__
            os_name = "macOS";
#elif _WIN32
            os_name = "Windows";
#endif
            std::cout << "OS: " << os_name << "\n\n";

            // Check Python
            std::cout << "Checking dependencies...\n\n";
            bool has_python = system("python3 --version >/dev/null 2>&1") == 0;
            if (!has_python) {
                has_python = system("python --version >/dev/null 2>&1") == 0;
            }
            std::string python_cmd = has_python ? "python3" : "python";

            if (!has_python) {
                std::cerr << "❌ Python not found.\n";
                std::cerr << "   Voice input requires Python 3.8+\n";
                std::cerr << "   Install: https://www.python.org/downloads/\n";
                return 1;
            }
            std::cout << "✅ Python found\n";

            // Check pip
            bool has_pip = system((python_cmd + " -m pip --version >/dev/null 2>&1").c_str()) == 0;
            if (!has_pip) {
                std::cerr << "❌ pip not found.\n";
                std::cerr << "   Install: " << python_cmd << " -m ensurepip\n";
                return 1;
            }
            std::cout << "✅ pip found\n";

            // Check audio
            bool has_audio = false;
            std::string audio_backend = "none";
#ifdef __linux__
            has_audio = system("which arecord >/dev/null 2>&1") == 0;
            if (has_audio)
                audio_backend = "arecord (ALSA)";
            if (!has_audio) {
                has_audio = system("which sox >/dev/null 2>&1") == 0;
                if (has_audio)
                    audio_backend = "sox";
            }
#elif __APPLE__
            has_audio = system("which sox >/dev/null 2>&1") == 0;
            if (has_audio)
                audio_backend = "sox";
#endif
            bool has_pyaudio = system((python_cmd + " -c 'import pyaudio' >/dev/null 2>&1").c_str()) == 0;
            if (has_pyaudio)
                audio_backend = "pyaudio";

            if (has_audio || has_pyaudio) {
                std::cout << "✅ Audio: " << audio_backend << "\n";
            } else {
                std::cout << "❌ Audio: not found\n";
            }

            // Check Vosk
            bool has_vosk = system((python_cmd + " -c 'import vosk' >/dev/null 2>&1").c_str()) == 0;
            std::cout << (has_vosk ? "✅" : "❌") << " Vosk (STT for en/es/fr/de)\n";

            // Check Whisper
            bool has_whisper = system((python_cmd + " -c 'import whisper' >/dev/null 2>&1").c_str()) == 0;
            std::cout << (has_whisper ? "✅" : "❌") << " Whisper (STT for Amharic)\n";

            // Check PyAudio
            std::cout << (has_pyaudio ? "✅" : "❌") << " PyAudio (microphone capture)\n";

            std::cout << "\n";

            // Determine what needs installing
            bool needs_install = !has_vosk || !has_whisper || !has_pyaudio;

            if (!needs_install) {
                std::cout << "✅ All voice dependencies installed!\n";
                std::cout << "   Use 'voice' in REPL to start voice input.\n";
                return 0;
            }

            // Show what's missing
            std::cout << "Missing packages:\n";
            if (!has_pyaudio)
                std::cout << "  • pyaudio — microphone capture\n";
            if (!has_vosk)
                std::cout << "  • vosk — speech-to-text (en/es/fr/de)\n";
            if (!has_whisper)
                std::cout << "  • whisper — speech-to-text (Amharic)\n";
            std::cout << "\n";

            // Ask user
            std::cout << "Install missing packages? (y/n): ";
            std::string answer;
            std::getline(std::cin, answer);

            if (answer != "y" && answer != "Y" && answer != "yes") {
                std::cout << "\nSkipped. Voice input will not work until packages are installed.\n";
                std::cout << "Manual install:\n";
                std::cout << "  " << python_cmd << " -m pip install vosk pyaudio openai-whisper\n";
                return 0;
            }

            // Install
            std::cout << "\nInstalling packages...\n\n";

            // Install system deps first
#ifdef __linux__
            std::cout << "Installing system audio libraries...\n";
            system("sudo apt-get update -qq && sudo apt-get install -y -qq portaudio19-dev libasound2-dev 2>/dev/null");
#elif __APPLE__
            std::cout << "Installing system audio libraries...\n";
            system("brew install portaudio 2>/dev/null");
#endif

            // Install PyAudio
            if (!has_pyaudio) {
                std::cout << "Installing pyaudio...\n";
                int ret = system((python_cmd + " -m pip install pyaudio 2>&1").c_str());
                if (ret != 0) {
                    std::cerr << "❌ Failed to install pyaudio\n";
                    std::cerr << "   Try manually: " << python_cmd << " -m pip install pyaudio\n";
                } else {
                    std::cout << "✅ pyaudio installed\n";
                }
            }

            // Install Vosk
            if (!has_vosk) {
                std::cout << "Installing vosk...\n";
                int ret = system((python_cmd + " -m pip install vosk 2>&1").c_str());
                if (ret != 0) {
                    std::cerr << "❌ Failed to install vosk\n";
                } else {
                    std::cout << "✅ vosk installed\n";
                }
            }

            // Install Whisper
            if (!has_whisper) {
                std::cout << "Installing openai-whisper (may take a few minutes)...\n";
                int ret = system((python_cmd + " -m pip install openai-whisper 2>&1").c_str());
                if (ret != 0) {
                    std::cerr << "❌ Failed to install whisper\n";
                } else {
                    std::cout << "✅ whisper installed\n";
                }
            }

            // Final check
            std::cout << "\n══════════════════════════════════════════════════════\n";
            std::cout << "Setup complete!\n\n";
            std::cout << "Voice input status:\n";
            bool vosk_ok = system((python_cmd + " -c 'import vosk' >/dev/null 2>&1").c_str()) == 0;
            bool pyaudio_ok = system((python_cmd + " -c 'import pyaudio' >/dev/null 2>&1").c_str()) == 0;
            bool whisper_ok = system((python_cmd + " -c 'import whisper' >/dev/null 2>&1").c_str()) == 0;

            std::cout << "  " << (vosk_ok ? "✅" : "❌") << " Vosk (en/es/fr/de)\n";
            std::cout << "  " << (whisper_ok ? "✅" : "❌") << " Whisper (Amharic)\n";
            std::cout << "  " << (pyaudio_ok ? "✅" : "❌") << " PyAudio (microphone)\n\n";

            if (vosk_ok || whisper_ok) {
                std::cout << "Use 'voice' in REPL to start voice input.\n";
                std::cout << "Use 'voice-tutorial' to learn how to speak code.\n";
            } else {
                std::cout << "Voice input not available. Install failed.\n";
                std::cout << "Try manually: " << python_cmd << " -m pip install vosk pyaudio openai-whisper\n";
            }

            return 0;
        }

        if (arg == "doc") {
            std::string topic = (i + 1 < argc) ? argv[++i] : "";
            if (topic.empty()) {
                std::cout << "Usage: alphabet doc <builtin>\n\n";
                std::cout << "Available builtins:\n";
                std::cout << "  I/O:       o, i, f, fw, fa, exists, exec, system, exit, args\n";
                std::cout << "  Math:      sqrt, sin, cos, tan, abs, floor, ceil, round, pow, min, max, log, log10\n";
                std::cout << "  String:    len, tostr, tonum, type, split, join, replace, trim, upper, lower, substr\n";
                std::cout << "  List:      append, pop_back, contains, reverse, sort, insert, remove, slice, swap\n";
                std::cout << "  Set:       set, add, has, set_size\n";
                std::cout << "  Aggregate: sum, avg, unique, flatten, flatten_str, zip, enumerate\n";
                std::cout << "  Map:       keys, values\n";
                std::cout << "  Type:      is_null, is_empty, clamp\n";
                std::cout << "  JSON:      json_parse, json_stringify\n";
                std::cout << "  Network:   http_get, http_post\n";
                std::cout << "  System:    sleep, timestamp, env, rand, randint\n";
                std::cout << "  Assert:    assert, assert_eq\n";
                std::cout << "\nUse 'alphabet doc <builtin>' for details.\n";
            } else {
                // Show docs for specific builtin
                struct BuiltinDoc {
                    const char* name;
                    const char* sig;
                    const char* desc;
                };
                static const BuiltinDoc docs[] = {
                    {"o", "o(val)", "Print val to stdout with newline. Alias: z.o()"},
                    {"i", "i()", "Read a line from stdin. Returns string or number if numeric."},
                    {"f", "f(path)", "Read file contents to string. Blocks '..' traversal."},
                    {"fw", "fw(path, content)", "Write string to file. Overwrites existing."},
                    {"fa", "fa(path, content)", "Append string to file."},
                    {"exists", "exists(path)", "Check if file exists. Returns 1.0 or 0.0."},
                    {"len", "len(val)", "Length of string (UTF-8 aware), list, or map."},
                    {"tostr", "tostr(val)", "Convert any type to string representation."},
                    {"tonum", "tonum(val)", "Convert string/bool to number."},
                    {"type", "type(val)", "Returns type name: null, number, string, list, map, object."},
                    {"split", "split(str, delim)", "Split string by delimiter into list."},
                    {"join", "join(list, sep)", "Join list elements into string with separator."},
                    {"replace", "replace(str, old, new)", "Replace all occurrences of old with new in string."},
                    {"trim", "trim(str)", "Strip leading/trailing whitespace."},
                    {"upper", "upper(str)", "Convert string to uppercase."},
                    {"lower", "lower(str)", "Convert string to lowercase."},
                    {"substr", "substr(str, start [, len])", "Extract substring from start position."},
                    {"find", "find(haystack, needle)", "Find index of needle in string/list. Returns -1 if not found."},
                    {"count", "count(haystack, needle)", "Count occurrences of needle in string/list."},
                    {"sqrt", "sqrt(x)", "Square root of x."},
                    {"abs", "abs(x)", "Absolute value of x."},
                    {"pow", "pow(base, exp)", "Raise base to exp power."},
                    {"min", "min(a, b)", "Smaller of two values."},
                    {"max", "max(a, b)", "Larger of two values."},
                    {"floor", "floor(x)", "Round down to nearest integer."},
                    {"ceil", "ceil(x)", "Round up to nearest integer."},
                    {"round", "round(x)", "Round to nearest integer."},
                    {"clamp", "clamp(val, min, max)", "Constrain val between min and max."},
                    {"append", "append(list, val)", "Add val to end of list. Returns list."},
                    {"pop_back", "pop_back(list)", "Remove and return last element of list."},
                    {"insert", "insert(list, idx, val)", "Insert val at index in list."},
                    {"remove", "remove(list, idx)", "Remove and return element at index."},
                    {"contains", "contains(collection, val)", "Check if list/string contains val. Returns 1.0 or 0.0."},
                    {"reverse", "reverse(list)", "Reverse list in place. Returns list."},
                    {"sort", "sort(list)", "Sort list in place. Returns list."},
                    {"slice", "slice(obj, start [, end])", "Extract sublist or substring. Supports negative indices."},
                    {"swap", "swap(list, i, j)", "Swap elements at indices i and j in place."},
                    {"unique", "unique(list)", "Remove duplicate elements. Returns new list."},
                    {"flatten", "flatten(nested_list)", "Recursively flatten nested lists."},
                    {"flatten_str", "flatten_str(nested_list)", "Flatten nested lists to concatenated string."},
                    {"zip", "zip(list_a, list_b)", "Pair elements from two lists: [[1,a],[2,b]]."},
                    {"enumerate", "enumerate(list)", "Create index-value pairs: [[0,x],[1,y]]."},
                    {"sum", "sum(list)", "Sum of all numbers in list."},
                    {"avg", "avg(list)", "Average of all numbers in list."},
                    {"range", "range(stop) | range(start, stop [, step])", "Generate list of numbers."},
                    {"keys", "keys(map)", "List of map keys."},
                    {"values", "values(map)", "List of map values."},
                    {"set", "set()", "Create empty set (list with uniqueness)."},
                    {"add", "add(set, val)", "Add val to set if not present."},
                    {"has", "has(set, val)", "Check if set contains val."},
                    {"is_null", "is_null(val)", "Check if val is null. Returns 1.0 or 0.0."},
                    {"is_empty", "is_empty(val)", "Check if list/string/map is empty."},
                    {"json_parse", "json_parse(str)", "Parse JSON string to Alphabet value."},
                    {"json_stringify", "json_stringify(val)", "Convert value to JSON string."},
                    {"http_get", "http_get(url)", "HTTP GET request. Returns response body string."},
                    {"http_post", "http_post(url, body)", "HTTP POST with JSON body. Returns response string."},
                    {"sleep", "sleep(ms)", "Sleep for milliseconds (max 300000)."},
                    {"timestamp", "timestamp()", "Current epoch time in milliseconds."},
                    {"env", "env(name)", "Get environment variable value."},
                    {"rand", "rand()", "Random float between 0.0 and 1.0."},
                    {"randint", "randint(min, max)", "Random integer between min and max."},
                    {"assert", "assert(cond [, msg])", "Throw error if condition is false."},
                    {"assert_eq", "assert_eq(a, b)", "Throw error if a != b."},
                    {"builder", "builder()", "Create string builder (fast concatenation)."},
                    {"append_str", "append_str(sb, text)", "Append text to string builder."},
                    {"build", "build(sb)", "Build final string from builder."},
                    {"chr", "chr(code)", "Character from Unicode codepoint."},
                    {"ord", "ord(char)", "Unicode codepoint from character."},
                    {"starts_with", "starts_with(str, prefix)", "Check if string starts with prefix."},
                    {"ends_with", "ends_with(str, suffix)", "Check if string ends with suffix."},
                    {"exec", "exec(cmd)", "Run shell command. Returns stdout string."},
                    {"system", "system(cmd)", "Run shell command. Returns exit code."},
                    {"exit", "exit(code)", "Exit program with given exit code."},
                    {"args", "args()", "Get command-line arguments as list."},
                };
                bool found = false;
                for (const auto& d : docs) {
                    if (topic == d.name) {
                        std::cout << "\033[1m" << d.name << "\033[0m" << "\n";
                        std::cout << "  Signature: " << d.sig << "\n";
                        std::cout << "  " << d.desc << "\n";
                        std::cout << "  All builtins work with or without z. prefix.\n";
                        found = true;
                        break;
                    }
                }
                if (!found) {
                    std::cerr << "Unknown builtin: " << topic << "\n";
                    std::cerr << "Use 'alphabet doc' to see all available builtins.\n";
                    return 1;
                }
            }
            return 0;
        }

        if (arg == "bench") {
            std::cout << "Running Alphabet VM benchmarks...\n\n";
            struct Bench {
                const char* name;
                const char* code;
            };
            static const Bench benches[] = {
                {"fibonacci", "#alphabet<en>\nm 5 fib(5 num) {\n  i (num <= 1) { r num }\n  r fib(num - 1) + fib(num - "
                              "2)\n}\no(fib(20))\n"},
                {"loop_sum", "#alphabet<en>\n5 total = 0\nl (5 i = 0 : i < 100000 : i = i + 1) {\n  total = total + "
                             "i\n}\no(total)\n"},
                {"string_concat", "#alphabet<en>\n5 sb = builder()\nl (5 i = 0 : i < 1000 : i = i + 1) {\n  "
                                  "append_str(sb, \"hello\")\n}\no(len(build(sb)))\n"},
                {"list_ops", "#alphabet<en>\n5 lst = []\nl (5 i = 0 : i < 1000 : i = i + 1) {\n  append(lst, "
                             "i)\n}\nreverse(lst)\no(len(lst))\n"},
                {"map_ops", "#alphabet<en>\n5 mp = {}\nl (5 i = 0 : i < 1000 : i = i + 1) {\n  mp[tostr(i)] = i * "
                            "2\n}\no(len(mp))\n"},
            };
            std::cout << "Benchmark         Time (ms)\n";
            std::cout << "--------------------------\n";
            for (const auto& b : benches) {
                auto start = std::chrono::high_resolution_clock::now();
                {
                    std::streambuf* old_cout = std::cout.rdbuf();
                    std::ostringstream null_stream;
                    std::cout.rdbuf(null_stream.rdbuf());
                    try {
                        run_source(b.code);
                    } catch (...) {
                    }
                    std::cout.rdbuf(old_cout);
                }
                auto end = std::chrono::high_resolution_clock::now();
                double ms = std::chrono::duration<double, std::milli>(end - start).count();
                printf("%-18s %.1f\n", b.name, ms);
            }
            return 0;
        }

        if (arg == "examples") {
            std::cout << "Available examples (run with: alphabet examples/<name>.abc):\n\n";
            std::cout << "  fibonacci.abc        Recursion and loops\n";
            std::cout << "  classes.abc          OOP, inheritance, this keyword\n";
            std::cout << "  functional.abc       Lambdas, map/filter/reduce\n";
            std::cout << "  json.abc             JSON parse and stringify\n";
            std::cout << "  amharic.abc          Multilingual (Amharic keywords)\n";
            std::cout << "  error_handling.abc   Try/handle, safe access\n";
            std::cout << "  pattern_matching.abc Match/case, type checking\n";
            std::cout << "  file_io.abc          File read/write/append\n";
            std::cout << "  data_processing.abc  Maps, lists, JSON, sorting\n";
            std::cout << "  comprehensive.abc    All features in one file\n";
            std::cout << "  hello_shebang.abc    Executable via shebang\n";
            std::cout << "\nRun: alphabet examples/<name>.abc\n";
            return 0;
        }

        if (arg == "tour") {
            std::string tour_path = "learn/tour.abc";
            std::ifstream tour_file(tour_path);
            if (!tour_file.good()) {
                // Try relative to binary location
                char bin_path[4096] = {};
#ifdef __linux__
                ssize_t len = readlink("/proc/self/exe", bin_path, sizeof(bin_path) - 1);
                if (len > 0) {
                    bin_path[len] = '\0';
                    std::string bin_dir(bin_path);
                    auto pos = bin_dir.rfind('/');
                    if (pos != std::string::npos) {
                        bin_dir = bin_dir.substr(0, pos);
                        // Try ../learn/tour.abc (from build/ to source/)
                        tour_path = bin_dir + "/../learn/tour.abc";
                        tour_file.open(tour_path);
                    }
                }
#endif
                if (!tour_file.good()) {
                    std::cerr << "Error: Tour file not found.\n";
                    std::cerr << "Run from the Alphabet source directory,\n";
                    std::cerr << "or ensure learn/tour.abc exists.\n";
                    return 1;
                }
            }
            std::ostringstream oss;
            oss << tour_file.rdbuf();
            std::string source = oss.str();
            run_source(source);
            return 0;
        }

        if (arg == "voice-tutorial") {
            std::string tutorial_path = "learn/voice_tutorial.abc";
            std::ifstream tutorial_file(tutorial_path);
            if (!tutorial_file.good()) {
                // Try relative to binary location
                char bin_path[4096] = {};
#ifdef __linux__
                ssize_t len = readlink("/proc/self/exe", bin_path, sizeof(bin_path) - 1);
                if (len > 0) {
                    bin_path[len] = '\0';
                    std::string bin_dir(bin_path);
                    auto pos = bin_dir.rfind('/');
                    if (pos != std::string::npos) {
                        bin_dir = bin_dir.substr(0, pos);
                        tutorial_path = bin_dir + "/../learn/voice_tutorial.abc";
                        tutorial_file.open(tutorial_path);
                    }
                }
#endif
                if (!tutorial_file.good()) {
                    std::cerr << "Error: Voice tutorial not found.\n";
                    return 1;
                }
            }
            std::ostringstream oss;
            oss << tutorial_file.rdbuf();
            std::string source = oss.str();
            run_source(source);
            return 0;
        }

        if (arg == "init") {
            std::string project_name = "my_project";
            if (i + 1 < argc) {
                project_name = argv[++i];
            }

            // Create project directory
            if (mkdir(project_name.c_str(), 0755) != 0 && errno != EEXIST) {
                std::cerr << "Error: Cannot create directory: " << project_name << "\n";
                return 1;
            }

            // Create main.abc
            std::string main_path = project_name + "/main.abc";
            std::ofstream main_file(main_path);
            if (main_file.good()) {
                main_file << "#alphabet<en>\n";
                main_file << "/// " << project_name << " — Alphabet project\n";
                main_file << "\n";
                main_file << "z.o(\"Hello from " << project_name << "!\")\n";
                main_file.close();
            }

            // Create alphabet.toml
            std::string toml_path = project_name + "/alphabet.toml";
            std::ofstream toml_file(toml_path);
            if (toml_file.good()) {
                toml_file << "[project]\n";
                toml_file << "name = \"" << project_name << "\"\n";
                toml_file << "version = \"0.1.0\"\n";
                toml_file << "language = \"en\"\n";
                toml_file << "entry = \"main.abc\"\n";
                toml_file << "\n";
                toml_file << "[dependencies]\n";
                toml_file << "# Add stdlib modules here\n";
                toml_file << "# math = \"stdlib/math.abc\"\n";
                toml_file.close();
            }

            // Create src/ directory
            std::string src_dir = project_name + "/src";
            mkdir(src_dir.c_str(), 0755);

            // Create tests/ directory
            std::string tests_dir = project_name + "/tests";
            mkdir(tests_dir.c_str(), 0755);

            std::cout << "Created project: " << project_name << "\n";
            std::cout << "\n";
            std::cout << "  " << project_name << "/\n";
            std::cout << "  ├── main.abc          Entry point\n";
            std::cout << "  ├── alphabet.toml     Project config\n";
            std::cout << "  ├── src/              Source files\n";
            std::cout << "  └── tests/            Test files\n";
            std::cout << "\n";
            std::cout << "Next steps:\n";
            std::cout << "  cd " << project_name << "\n";
            std::cout << "  alphabet run main.abc\n";
            return 0;
        }

        if (arg == "test") {
            std::string tests_dir = "tests";
            if (i + 1 < argc) {
                tests_dir = argv[++i];
            }

            // Check if tests/ directory exists
            if (access(tests_dir.c_str(), F_OK) != 0) {
                std::cerr << "Error: Directory not found: " << tests_dir << "\n";
                std::cerr << "Create tests/ directory with .abc test files.\n";
                return 1;
            }

            // Find all .abc files in tests/
            std::vector<std::string> test_files = list_dir_abc(tests_dir);

            if (test_files.empty()) {
                std::cout << "No test files found in " << tests_dir << "/\n";
                return 0;
            }

            std::sort(test_files.begin(), test_files.end());

            int passed = 0;
            int failed = 0;
            int total = test_files.size();

            std::cout << "Running " << total << " test(s)...\n\n";

            for (const auto& file : test_files) {
                std::ifstream f(file);
                if (!f.good()) {
                    std::cerr << "  SKIP: " << file << " (cannot read)\n";
                    continue;
                }

                std::ostringstream oss;
                oss << f.rdbuf();
                std::string source = oss.str();

                try {
                    alphabet::Lexer lexer(source);
                    auto tokens = lexer.scan_tokens();
                    alphabet::Parser parser(tokens, source);
                    auto stmts = parser.parse();

                    if (!parser.had_errors()) {
                        alphabet::Compiler compiler;
                        auto program = compiler.compile(stmts);
                        alphabet::VM vm(program);
                        vm.run();
                        std::cout << "  PASS: " << file << "\n";
                        passed++;
                    } else {
                        std::cout << "  FAIL: " << file << " (parse errors)\n";
                        failed++;
                    }
                } catch (const std::exception& e) {
                    std::cout << "  FAIL: " << file << " (" << e.what() << ")\n";
                    failed++;
                }
            }

            std::cout << "\n";
            std::cout << "Results: " << passed << "/" << total << " passed";
            if (failed > 0) {
                std::cout << ", " << failed << " failed";
            }
            std::cout << "\n";

            return failed > 0 ? 1 : 0;
        }

        if (arg == "update" || arg == "upgrade") {
            std::cout << "Alphabet Language v" << ALPHABET_VERSION << "\n";
            std::cout << "\n";
            std::cout << "To update:\n";
            std::cout << "  1. Pull latest source:\n";
            std::cout << "     cd /path/to/Alphabet_Language\n";
            std::cout << "     git pull\n";
            std::cout << "\n";
            std::cout << "  2. Rebuild:\n";
            std::cout << "     cd build && cmake .. && make -j$(nproc)\n";
            std::cout << "\n";
            std::cout << "  3. Install (optional):\n";
            std::cout << "     sudo make install\n";
            std::cout << "\n";
            std::cout << "For nightly builds:\n";
            std::cout << "  https://github.com/alphabet-lang/alphabet/releases\n";
            return 0;
        }

        if (arg == "info") {
            if (alphabet::ProjectManager::exists()) {
                auto config = alphabet::ProjectManager::load();
                alphabet::ProjectManager::print_info(config);
            } else {
                std::cout << "No alphabet.toml found in current directory.\n";
                std::cout << "Run 'alphabet init' to create a project.\n";
            }
            return 0;
        }

        if (arg == "pkg") {
            std::string pkg_cmd = (i + 1 < argc) ? argv[++i] : "";
            const char* home = getenv("HOME");
            std::string pkg_dir = std::string(home ? home : "/tmp") + "/.alphabet/packages";

            if (pkg_cmd == "install" || pkg_cmd == "add") {
                std::string pkg_name = (i + 1 < argc) ? argv[++i] : "";
                if (pkg_name.empty()) {
                    std::cerr << "Usage: alphabet pkg install <name>\n";
                    std::cerr << "Available packages: math, string_utils, json, collections, testing\n";
                    return 1;
                }
                std::string exe_dir;
#ifdef _WIN32
                char resolved[MAX_PATH] = {};
                DWORD len = GetModuleFileNameA(nullptr, resolved, MAX_PATH);
                if (len > 0) {
                    exe_dir = std::string(resolved);
                    size_t last_slash = exe_dir.rfind('\\');
                    if (last_slash != std::string::npos)
                        exe_dir = exe_dir.substr(0, last_slash);
                }
#else
                std::string exe_path = "/proc/self/exe";
                char resolved[4096];
                ssize_t len = readlink(exe_path.c_str(), resolved, sizeof(resolved) - 1);
                if (len > 0) {
                    resolved[len] = '\0';
                    exe_dir = std::string(resolved);
                    size_t last_slash = exe_dir.rfind('/');
                    if (last_slash != std::string::npos)
                        exe_dir = exe_dir.substr(0, last_slash);
                }
#endif
                std::string src = exe_dir + "/../stdlib/" + pkg_name + ".abc";
                struct stat st;
                if (stat(src.c_str(), &st) != 0) {
                    std::cerr << "Package '" << pkg_name << "' not found.\n";
                    std::cerr << "Available: math, string_utils, json, collections, testing\n";
                    return 1;
                }
                {
                    std::string cmd = "mkdir -p " + pkg_dir;
                    system(cmd.c_str());
                }
                std::string dst = pkg_dir + "/" + pkg_name + ".abc";
                std::ifstream in(src, std::ios::binary);
                std::ofstream out(dst, std::ios::binary);
                out << in.rdbuf();
                std::cout << "Installed '" << pkg_name << "' to " << dst << "\n";
                return 0;
            } else if (pkg_cmd == "list" || pkg_cmd == "ls") {
                struct stat st;
                if (stat(pkg_dir.c_str(), &st) != 0) {
                    std::cout << "No packages installed.\n";
                    std::cout << "Use 'alphabet pkg install <name>' to install.\n";
                    return 0;
                }
                std::cout << "Installed packages:\n";
                std::vector<std::string> installed = list_dir_abc(pkg_dir);
                for (const auto& f : installed) {
                    auto pos = f.find_last_of("/\\");
                    std::string name = (pos != std::string::npos) ? f.substr(pos + 1) : f;
                    if (name.size() > 4 && name.substr(name.size() - 4) == ".abc") {
                        std::cout << "  " << name.substr(0, name.size() - 4) << "\n";
                    }
                }
                return 0;
            } else if (pkg_cmd == "search") {
                std::string query = (i + 1 < argc) ? argv[++i] : "";
                std::cout << "Available packages:\n";
                std::cout << "  math          - Math functions (clamp, lerp, fibonacci, is_prime, gcd, lcm)\n";
                std::cout << "  string_utils  - String utilities (contains, repeat, reverse, capitalize, pad)\n";
                std::cout << "  json          - JSON utilities (json_parse, json_stringify)\n";
                std::cout << "  collections   - Collection helpers (flatten, chunk, zip_with, group_by)\n";
                std::cout << "  testing       - Test framework (assert, assert_eq, test_suite, test_case)\n";
                return 0;
            } else {
                std::cout << "Alphabet Package Manager\n\n";
                std::cout << "Usage:\n";
                std::cout << "  alphabet pkg install <name>  Install a package\n";
                std::cout << "  alphabet pkg list            List installed packages\n";
                std::cout << "  alphabet pkg search          Search available packages\n";
                return 0;
            }
        }

        if (arg == "run") {
            if (i + 1 < argc) {
                input_file = argv[++i];
            } else {
                std::cerr << "Error: 'alphabet run' requires a file argument\n";
                std::cerr << "Usage: alphabet run <file.abc>\n";
                return 1;
            }
            continue;
        }

        if (arg == "lint") {
            if (i + 1 < argc) {
                std::string file = argv[++i];
                std::string source = read_input(file);
                if (source.empty()) {
                    std::cerr << "Error: Cannot read file: " << file << "\n";
                    return 1;
                }
                try {
                    alphabet::Lexer lexer(source);
                    auto tokens = lexer.scan_tokens();
                    alphabet::Parser parser(tokens, source);
                    auto stmts = parser.parse();
                    if (parser.had_errors()) {
                        std::cerr << "Parse errors found. Fix parse errors before linting.\n";
                        for (const auto& err : parser.errors()) {
                            std::cerr << "  " << err << "\n";
                        }
                        return 1;
                    }
                    alphabet::LintVisitor linter(file);
                    auto warnings = linter.lint(stmts);
                    if (warnings.empty()) {
                        std::cout << GREEN("No warnings found.") << "\n";
                    } else {
                        for (const auto& w : warnings) {
                            std::cerr << "\033[33m" << w.format() << "\033[0m" << "\n";
                        }
                        std::cout << "\n" << warnings.size() << " warning(s) found.\n";
                    }
                } catch (const std::exception& e) {
                    std::cerr << "Error: " << e.what() << "\n";
                    return 1;
                }
            } else {
                std::cerr << "Error: 'alphabet lint' requires a file argument\n";
                std::cerr << "Usage: alphabet lint <file.abc>\n";
                return 1;
            }
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
        std::string source = read_input(input_file);

        if (compile_only) {
            alphabet::Lexer lexer(source);
            std::vector<alphabet::Token> tokens = lexer.scan_tokens();

            alphabet::Parser parser(tokens, source);
            std::vector<alphabet::StmtPtr> statements = parser.parse();

            if (parser.had_errors()) {
                std::string msg = parser.first_error().empty() ? "Syntax errors in source code" : parser.first_error();
                throw alphabet::ParseError(msg);
            }

            alphabet::Compiler compiler;

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

                const char magic[] = "ALPH";
                out.write(magic, 4);
                uint32_t version = 2;
                out.write(reinterpret_cast<const char*>(&version), sizeof(version));
                uint32_t count = static_cast<uint32_t>(program.main.size());
                out.write(reinterpret_cast<const char*>(&count), sizeof(count));

                for (const auto& instr : program.main) {
                    uint8_t op = static_cast<uint8_t>(instr.op);
                    out.write(reinterpret_cast<const char*>(&op), sizeof(op));

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

                std::cout << "Compiled " << program.main.size() << " instructions to " << output_file << "\n";
            } else {
                std::cout << "Compilation successful: " << program.main.size() << " instructions\n";
            }
        } else if (dump_bytecode) {
            alphabet::Lexer lexer(source);
            std::vector<alphabet::Token> tokens = lexer.scan_tokens();

            alphabet::Parser parser(tokens, source);
            std::vector<alphabet::StmtPtr> statements = parser.parse();

            if (parser.had_errors()) {
                std::string msg = parser.first_error().empty() ? "Syntax errors in source code" : parser.first_error();
                throw alphabet::ParseError(msg);
            }

            alphabet::Compiler compiler;
            size_t last_sl = input_file.find_last_of("/\\");
            if (last_sl != std::string::npos) {
                compiler.set_source_dir(input_file.substr(0, last_sl));
            }
            alphabet::Program program = compiler.compile(statements);
            std::cout << alphabet::Compiler::dump_program(program);
        } else {
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

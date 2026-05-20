#include <algorithm>
#include <csignal>
#include <cstdlib>
#include <cstring>
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>
#ifdef _WIN32
#include <io.h>
#include <windows.h>
#define popen _popen
#define pclose _pclose
#define unlink _unlink
#define chmod _chmod
#else
#include <sys/stat.h>
#include <sys/wait.h>
#include <unistd.h>
#endif

#include "lexer.h"
#include "parser.h"
#include "compiler.h"
#include "vm.h"
#include "ffi.h"
#include "lsp.h"
#include "type_system.h"
#include "version.h"

namespace {

constexpr const char *VERSION = ALPHABET_VERSION;
constexpr const char *DEVELOPER = "Fraol Teshome (fraolteshome444@gmail.com)";

constexpr const char *LOGO = R"(
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

void print_version()
{
    std::cout << "Alphabet " << VERSION << " (Native C++)\n";
    std::cout << "Developer: " << DEVELOPER << "\n";
    std::cout << "Compiled with C++17\n";
}

void print_help()
{
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
    std::cout << "  alphabet update   Self-update to latest version (alias: upgrade)\n\n";
    std::cout << "Examples:\n";
    std::cout << "  alphabet program.abc          Run a program\n";
    std::cout << "  alphabet -c program.abc       Compile only\n";
    std::cout << "  alphabet --repl               Interactive mode\n";
    std::cout << "  alphabet --lsp                LSP server for VS Code\n";
    std::cout << "  alphabet --dump-bytecode prog.abc  Inspect bytecode\n";
    std::cout << "  alphabet --debug program.abc  Debug with breakpoints\n";
}

void run_source(const std::string &source, bool debug_mode = false,
                const std::string &source_dir = "", bool sandbox_mode = false)
{
    try {
        alphabet::Lexer lexer(source);
        std::vector<alphabet::Token> tokens = lexer.scan_tokens();

        alphabet::Parser parser(tokens, source);
        std::vector<alphabet::StmtPtr> statements = parser.parse();

        if (parser.had_errors()) {
            const auto &errors = parser.errors();
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
    }
    catch (const alphabet::MissingLanguageHeader &e) {
        std::cerr << "Error: " << e.what() << "\n";
        std::cerr << "  Add '#alphabet<lang>' as the first line of your source file.\n";
    }
    catch (const alphabet::ParseError &e) {
        std::cerr << "Parse Error: " << e.what() << "\n";
    }
    catch (const alphabet::CompileError &e) {
        std::cerr << "Compile Error: " << e.what() << "\n";
    }
    catch (const alphabet::RuntimeError &e) {
        std::cerr << "Runtime Error: " << e.what() << "\n";
    }
    catch (const std::exception &e) {
        std::cerr << "Error: " << e.what() << "\n";
    }
}

namespace repl {
volatile std::sig_atomic_t g_interrupted = 0;

void sigint_handler(int)
{
    g_interrupted = 1;
}

int count_braces_safe(const std::string &line)
{
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
        }
        else if (c == '{') {
            ++depth;
        }
        else if (c == '}') {
            --depth;
        }
    }
    return depth;
}

bool is_command(const std::string &line)
{
    if (line == "q" || line == "quit" || line == "exit" || line == "help" || line == "clear" ||
        line == "reset" || line == "vars" || line == "history" || line == "keywords" ||
        line == "!!" || (line.size() > 1 && line[0] == '!' && line[1] != '!'))
        return true;

    if (line.rfind("#alphabet<", 0) == 0 && line.size() > 10 && line.back() == '>')
        return true;
    return false;
}

} // namespace repl

void start_repl()
{
    std::cout << LOGO;
    std::cout << "Alphabet Language [v" << VERSION << " - Native C++]\n";
    std::cout << "Developed by " << DEVELOPER << "\n";
    std::cout << "\nType 'help' for commands, 'q' to exit.\n\n";

    std::signal(SIGINT, repl::sigint_handler);

    ffi_init();

    std::vector<std::string> history;
#ifdef _WIN32
    const char *home = std::getenv("USERPROFILE");
#else
    const char *home = std::getenv("HOME");
#endif
    std::string history_path =
        home ? (std::string(home) + "/.alphabet_history") : ".alphabet_history";
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
    std::unordered_map<std::string, alphabet::Value> saved_globals;
    alphabet::VM vm;

    while (true) {

        if (repl::g_interrupted) {
            repl::g_interrupted = 0;
            if (!buffer.empty()) {
                std::cout << "\n... cancelled\n";
                buffer.clear();
                brace_depth = 0;
            }
            else {
                std::cout << "\n";
            }
        }

        if (buffer.empty()) {
            std::cout << "\u256d\u2500[>>>] ";
        }
        else {
            std::cout << "\u2570\u2500[...] ";
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

        if (buffer.empty() && (line == "q" || line == "quit" || line == "exit")) {
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
            std::cout << "  keywords      Show keywords for current language\n";
            std::cout << "  clear         Clear screen (banner redraws)\n";
            std::cout << "  clear history Clear command history\n";
            std::cout << "  clear all     Clear screen + history\n";
            std::cout << "  reset         Clear REPL state (variables + code)\n";
            std::cout << "  q / quit      Exit\n";
            std::cout << "\nLanguage: Type " << "\"#alphabet<lang>\" as first line to switch.\n";
            std::cout << "  en=English  am=Amharic  es=Spanish  fr=French  de=German\n";
            std::cout << "  Current: " << repl_lang << "\n";
            std::cout << "\nMulti-line: Type '{' to start a block.\n";
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
            std::cout << "State cleared. Language reset to en.\n";
            continue;
        }

        if (buffer.empty() && line == "vars") {
            std::cout << "  language: " << repl_lang << "\n";
            if (saved_globals.empty()) {
                std::cout << "  (no variables defined)\n";
            }
            else {
                for (const auto &[name, val] : saved_globals) {
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
            const auto &kw = lang_it->second;

            static const std::unordered_map<std::string, std::string> lang_names = {
                {"en", "English"},
                {"am", "Amharic (አማርኛ)"},
                {"es", "Spanish (Español)"},
                {"fr", "French (Français)"},
                {"de", "German (Deutsch)"}};
            auto name_it = lang_names.find(repl_lang);
            std::string display_name = name_it != lang_names.end() ? name_it->second : repl_lang;

            std::unordered_map<std::string, std::vector<std::string>> reverse;
            for (const auto &[native, letter] : kw) {
                reverse[letter].push_back(native);
            }

            struct Cat
            {
                std::string label;
                std::vector<std::pair<std::string, std::string>> items;
            };
            std::vector<Cat> categories;

            Cat ctrl{"Control Flow", {}};
            for (auto &l : {"i", "e", "l", "b", "k", "r"}) {
                if (reverse.count(l)) {
                    for (auto &n : reverse[l])
                        ctrl.items.emplace_back(n, l);
                }
            }
            categories.push_back(ctrl);

            Cat oop{"OOP", {}};
            for (auto &l : {"c", "a", "j", "m", "n", "v", "p", "s", "^"}) {
                if (reverse.count(l)) {
                    for (auto &n : reverse[l])
                        oop.items.emplace_back(n, l);
                }
            }
            categories.push_back(oop);

            Cat err{"Error Handling", {}};
            for (auto &l : {"t", "h"}) {
                if (reverse.count(l)) {
                    for (auto &n : reverse[l])
                        err.items.emplace_back(n, l);
                }
            }
            categories.push_back(err);

            Cat io{"I/O & Modules", {}};
            for (auto &l : {"z", "z.i", "x", "q", "@"}) {
                if (reverse.count(l)) {
                    for (auto &n : reverse[l])
                        io.items.emplace_back(n, l);
                }
            }
            categories.push_back(io);

            Cat misc{"Other", {}};
            for (const auto &l : {"\x80"}) {
                if (reverse.count(l)) {
                    for (auto &n : reverse[l])
                        misc.items.emplace_back(n, "const");
                }
            }
            if (!misc.items.empty())
                categories.push_back(misc);

            std::cout << "Keywords: " << display_name << " (current: " << repl_lang << ")\n";
            std::cout << "─────────────────────────────────────\n";

            for (const auto &cat : categories) {
                std::cout << "\n  " << cat.label << ":\n";
                for (const auto &[native, letter] : cat.items) {

                    std::string padded = native;

                    size_t char_count =
                        std::count_if(native.begin(), native.end(),
                                      [](unsigned char c) { return (c & 0xC0) != 0x80; });

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

        if (buffer.empty() && line.rfind("#alphabet<", 0) == 0 && line.size() > 10 &&
            line.back() == '>') {
            std::string new_lang = line.substr(10, line.size() - 11);
            static const std::unordered_set<std::string> valid_langs = {"en", "am", "es", "fr",
                                                                        "de"};
            if (valid_langs.count(new_lang)) {
                repl_lang = new_lang;
                all_source.clear();
                saved_globals.clear();
                std::cout << "Language set to " << repl_lang << ".\n";
            }
            else {
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
            bool is_num =
                !num_str.empty() && std::all_of(num_str.begin(), num_str.end(), [](char c) {
                    return std::isdigit(static_cast<unsigned char>(c));
                });
            if (is_num) {
                size_t idx = std::stoul(num_str);
                if (idx >= 1 && idx <= history.size()) {
                    line = history[idx - 1];
                    std::cout << line << "\n";
                }
                else {
                    std::cerr << "History index out of range (1-" << history.size() << ")\n";
                    continue;
                }
            }
            else {
                std::cerr << "Usage: !N (where N is a number)\n";
                continue;
            }
        }

        if (buffer.empty() && line == "!!") {
            if (!history.empty()) {
                line = history.back();
                std::cout << line << "\n";
            }
            else {
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
                }
                else {
                    all_source.clear();
                }
            };

            try {
                std::string full_source = "#alphabet<" + repl_lang + ">\n" + all_source;
                alphabet::Lexer lexer(full_source);
                auto tokens = lexer.scan_tokens();
                alphabet::Parser parser(tokens, full_source);
                auto statements = parser.parse();

                if (parser.had_errors()) {
                    for (const auto &err : parser.errors()) {
                        std::cerr << err << "\n";
                    }
                    if (parser.errors().empty()) {
                        std::cerr << "Syntax errors in source code\n";
                    }
                    rollback_last_line();
                }
                else {
                    alphabet::Compiler compiler;
                    alphabet::Program program = compiler.compile(statements);

                    vm.init(program);
                    vm.set_globals(saved_globals);
                    vm.run();
                    saved_globals = vm.get_globals();
                }
            }
            catch (const alphabet::MissingLanguageHeader &) {
                std::cerr << "Error: Missing header\n";
                rollback_last_line();
            }
            catch (const alphabet::ParseError &e) {
                std::cerr << "Parse Error: " << e.what() << "\n";
                rollback_last_line();
            }
            catch (const alphabet::CompileError &e) {
                std::cerr << "Compile Error: " << e.what() << "\n";
                rollback_last_line();
            }
            catch (const alphabet::RuntimeError &e) {
                std::cerr << "Runtime Error: " << e.what() << "\n";
            }
            catch (const std::exception &e) {
                std::cerr << "Error: " << e.what() << "\n";
                rollback_last_line();
            }

            buffer.clear();
            brace_depth = 0;
        }
    }

    ffi_cleanup();
}

bool is_safe_path(const std::string &path)
{
    return std::none_of(path.begin(), path.end(), [](char c) {
        return c == ';' || c == '|' || c == '&' || c == '$' || c == '`' || c == '(' || c == ')' ||
               c == '{' || c == '}' || c == '<' || c == '>' || c == '\n' || c == '\r' ||
               c == '\\' || c == '\'' || c == '"' || c == '*' || c == '?' || c == '[' || c == ']' ||
               c == '#' || c == '~' || c == '%' || c == '\0';
    });
}

#ifndef _WIN32
int exec_curl(const std::string &url, const std::string &output_path)
{
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

int exec_mv(const std::string &src, const std::string &dst)
{
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

std::string compute_sha256(const std::string &path)
{
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

void do_update()
{
    std::cout << "Checking for updates...\n";

    std::string api_cmd =
        "curl -fsSL https://api.github.com/repos/fraol163/alphabet/releases/latest";
    FILE *pipe = popen(api_cmd.c_str(), "r");
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

    auto version_tuple = [](const std::string &v) -> std::tuple<int, int, int> {
        int a = 0, b = 0, c = 0;
        sscanf(v.c_str(), "%d.%d.%d", &a, &b, &c);
        return {a, b, c};
    };

    if (version_tuple(latest_version) <= version_tuple(VERSION)) {
        std::cout << "Already up to date!\n";
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
    std::string download_url = "https://github.com/fraol163/alphabet/releases/download/v" +
                               latest_version + "/" + asset_name;
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
    }
    else {
        self_path = "C:\\Program Files\\alphabet\\alphabet.exe";
    }
#else
    ssize_t len = readlink("/proc/self/exe", self_buf, sizeof(self_buf) - 1);
    if (len > 0) {
        self_buf[len] = '\0';
        self_path = self_buf;
    }
    else {
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
    std::string ps_cmd =
        "start \"\" /b powershell -NoProfile -WindowStyle Hidden -Command \"" + ps_script + "\"";
    int ps_status = system(ps_cmd.c_str());
    if (ps_status != 0) {
        std::cerr << "Error: Could not schedule update installation\n";
        _unlink(tmp_path.c_str());
        return;
    }
#endif

    std::cout << "Updated to v" << latest_version << "!\n";
}

std::string read_input(const std::string &path)
{
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

int main(int argc, char *argv[])
{
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
            }
            else {
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
        std::string source = read_input(input_file);

        if (compile_only) {
            alphabet::Lexer lexer(source);
            std::vector<alphabet::Token> tokens = lexer.scan_tokens();

            alphabet::Parser parser(tokens, source);
            std::vector<alphabet::StmtPtr> statements = parser.parse();

            if (parser.had_errors()) {
                std::string msg = parser.first_error().empty() ? "Syntax errors in source code"
                                                               : parser.first_error();
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
                out.write(reinterpret_cast<const char *>(&version), sizeof(version));
                uint32_t count = static_cast<uint32_t>(program.main.size());
                out.write(reinterpret_cast<const char *>(&count), sizeof(count));

                for (const auto &instr : program.main) {
                    uint8_t op = static_cast<uint8_t>(instr.op);
                    out.write(reinterpret_cast<const char *>(&op), sizeof(op));

                    uint8_t tag;
                    if (std::holds_alternative<std::monostate>(instr.operand)) {
                        tag = 0;
                        out.write(reinterpret_cast<const char *>(&tag), sizeof(tag));
                    }
                    else if (auto *i = std::get_if<int64_t>(&instr.operand)) {
                        tag = 1;
                        out.write(reinterpret_cast<const char *>(&tag), sizeof(tag));
                        out.write(reinterpret_cast<const char *>(i), sizeof(*i));
                    }
                    else if (auto *d = std::get_if<double>(&instr.operand)) {
                        tag = 2;
                        out.write(reinterpret_cast<const char *>(&tag), sizeof(tag));
                        out.write(reinterpret_cast<const char *>(d), sizeof(*d));
                    }
                    else if (auto *s = std::get_if<std::string>(&instr.operand)) {
                        tag = 3;
                        out.write(reinterpret_cast<const char *>(&tag), sizeof(tag));
                        uint32_t len = static_cast<uint32_t>(s->size());
                        out.write(reinterpret_cast<const char *>(&len), sizeof(len));
                        out.write(s->data(), len);
                    }
                    else if (std::holds_alternative<std::nullptr_t>(instr.operand)) {
                        tag = 4;
                        out.write(reinterpret_cast<const char *>(&tag), sizeof(tag));
                    }
                    else if (auto *p = std::get_if<std::pair<std::string, int>>(&instr.operand)) {
                        tag = 5;
                        out.write(reinterpret_cast<const char *>(&tag), sizeof(tag));
                        uint32_t len = static_cast<uint32_t>(p->first.size());
                        out.write(reinterpret_cast<const char *>(&len), sizeof(len));
                        out.write(p->first.data(), len);
                        int32_t second = p->second;
                        out.write(reinterpret_cast<const char *>(&second), sizeof(second));
                    }
                    else {
                        tag = 0;
                        out.write(reinterpret_cast<const char *>(&tag), sizeof(tag));
                    }
                }

                std::cout << "Compiled " << program.main.size() << " instructions to "
                          << output_file << "\n";
            }
            else {
                std::cout << "Compilation successful: " << program.main.size() << " instructions\n";
            }
        }
        else if (dump_bytecode) {
            alphabet::Lexer lexer(source);
            std::vector<alphabet::Token> tokens = lexer.scan_tokens();

            alphabet::Parser parser(tokens, source);
            std::vector<alphabet::StmtPtr> statements = parser.parse();

            if (parser.had_errors()) {
                std::string msg = parser.first_error().empty() ? "Syntax errors in source code"
                                                               : parser.first_error();
                throw alphabet::ParseError(msg);
            }

            alphabet::Compiler compiler;
            size_t last_sl = input_file.find_last_of("/\\");
            if (last_sl != std::string::npos) {
                compiler.set_source_dir(input_file.substr(0, last_sl));
            }
            alphabet::Program program = compiler.compile(statements);
            std::cout << alphabet::Compiler::dump_program(program);
        }
        else {

            std::string source_dir;
            size_t last_slash = input_file.find_last_of("/\\");
            if (last_slash != std::string::npos) {
                source_dir = input_file.substr(0, last_slash);
            }
            run_source(source, debug_mode, source_dir, sandbox_mode);
        }
    }
    catch (const std::exception &e) {
        std::cerr << "Error: " << e.what() << "\n";
        return 1;
    }

    return 0;
}

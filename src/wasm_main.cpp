#include "compiler.h"
#include "lexer.h"
#include "parser.h"
#include "vm.h"
#include <cstdlib>
#include <cstring>
#include <sstream>
#include <string>
#include <streambuf>

#ifndef ALPHABET_VERSION
#define ALPHABET_VERSION "2.3.5"
#endif

#ifdef __EMSCRIPTEN__
#include <emscripten/emscripten.h>
#else
#define EMSCRIPTEN_KEEPALIVE
#endif

static std::string last_output;

class StringBuf : public std::streambuf {
    std::string& target;
public:
    explicit StringBuf(std::string& s) : target(s) {}
protected:
    int overflow(int c) override {
        if (c != EOF) target += static_cast<char>(c);
        return c;
    }
};

extern "C" {

EMSCRIPTEN_KEEPALIVE
const char* alphabet_run(const char* code) {
    if (!code) return "";

    last_output.clear();

    try {
        std::string source(code);

        if (source.find("#alphabet<") == std::string::npos) {
            source = "#alphabet<en>\n" + source;
        }

        std::stringbuf cout_buf(last_output);
        std::streambuf* old_cout = std::cout.rdbuf(&cout_buf);

        alphabet::Lexer lexer(source);
        auto tokens = lexer.scan_tokens();
        alphabet::Parser parser(tokens, source);
        auto stmts = parser.parse();

        if (parser.had_errors()) {
            std::cout.rdbuf(old_cout);
            last_output = "Parse Error: " + parser.first_error();
            return last_output.c_str();
        }

        alphabet::Compiler compiler;
        auto program = compiler.compile(stmts);
        alphabet::VM vm(program);
        vm.run();

        std::cout.rdbuf(old_cout);

        if (last_output.empty()) {
            last_output = "(no output)";
        }
    } catch (const std::exception& e) {
        std::cout.rdbuf(nullptr);
        last_output = std::string("Error: ") + e.what();
    }

    return last_output.c_str();
}

EMSCRIPTEN_KEEPALIVE
const char* alphabet_eval(const char* expression) {
    if (!expression) return "";
    std::string code = std::string("#alphabet<en>\nz.o(") + expression + ")";
    return alphabet_run(code.c_str());
}

EMSCRIPTEN_KEEPALIVE
const char* alphabet_version() {
    return ALPHABET_VERSION;
}

}

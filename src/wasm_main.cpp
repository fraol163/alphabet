#include "compiler.h"
#include "lexer.h"
#include "parser.h"
#include "vm.h"
#include <algorithm>
#include <cstdlib>
#include <cstring>
#include <sstream>
#include <streambuf>
#include <string>

#ifndef ALPHABET_VERSION
#define ALPHABET_VERSION "2.3.6"
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
        if (c != EOF)
            target += static_cast<char>(c);
        return c;
    }
};

// WASM functions return const char* pointing into a rotating buffer.
// Callers MUST copy the result before the next call. The rotation
// ensures that two consecutive calls don't dangle — but if a caller
// holds the pointer across calls, the second call's pointer is
// independent. (The JS wrapper reads the string immediately, so this
// is safe in practice; this is here for C++ embedders that might
// hold the result across calls.)
static char result_buffer[2][8192];
static int result_slot = 0;
static const char* copy_to_result_buffer(const std::string& s) {
    int slot = result_slot;
    result_slot = (result_slot + 1) % 2;
    size_t n = std::min(s.size(), sizeof(result_buffer[slot]) - 1);
    std::memcpy(result_buffer[slot], s.c_str(), n);
    result_buffer[slot][n] = '\0';
    return result_buffer[slot];
}

extern "C" {

EMSCRIPTEN_KEEPALIVE
const char* alphabet_run(const char* code) {
    if (!code)
        return "";

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
            return copy_to_result_buffer(last_output);
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
        std::cout.rdbuf(old_cout);
        last_output = std::string("Error: ") + e.what();
    }

    return copy_to_result_buffer(last_output);
}

EMSCRIPTEN_KEEPALIVE
const char* alphabet_eval(const char* expression) {
    if (!expression)
        return "";
    std::string code = std::string("#alphabet<en>\nz.o(") + expression + ")";
    return alphabet_run(code.c_str());
}

EMSCRIPTEN_KEEPALIVE
const char* alphabet_version() {
    return ALPHABET_VERSION;
}
}

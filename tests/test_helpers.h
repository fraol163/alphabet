#ifndef ALPHABET_TEST_HELPERS_H
#define ALPHABET_TEST_HELPERS_H

#include <string>
#include <vector>
#include <sstream>
#include <iostream>
#include "lexer.h"
#include "parser.h"
#include "compiler.h"
#include "vm.h"

namespace alphabet {
namespace test {

// Helper: lex source, return tokens
inline std::vector<Token> lex(const std::string& source) {
    Lexer lexer(source);
    return lexer.scan_tokens();
}

// Helper: parse source, return statements
inline std::vector<StmtPtr> parse(const std::string& source) {
    Lexer lexer(source);
    auto tokens = lexer.scan_tokens();
    Parser parser(tokens, source);
    return parser.parse();
}

// Helper: compile and run source
inline void run(const std::string& source) {
    Lexer lexer(source);
    auto tokens = lexer.scan_tokens();
    Parser parser(tokens, source);
    auto stmts = parser.parse();
    Compiler compiler;
    auto program = compiler.compile(stmts);
    VM vm(program);
    vm.run();
}

// Helper: compile and run source, capture stdout
inline std::string run_capture(const std::string& source) {
    std::ostringstream oss;
    std::streambuf* old = std::cout.rdbuf(oss.rdbuf());
    try {
        Lexer lexer(source);
        auto tokens = lexer.scan_tokens();
        Parser parser(tokens, source);
        auto stmts = parser.parse();
        Compiler compiler;
        auto program = compiler.compile(stmts);
        VM vm(program);
        vm.run();
    } catch (...) {
        std::cout.rdbuf(old);
        throw;
    }
    std::cout.rdbuf(old);
    return oss.str();
}

// Helper: compile and run, return globals
inline std::unordered_map<std::string, Value> run_get_globals(const std::string& source) {
    Lexer lexer(source);
    auto tokens = lexer.scan_tokens();
    Parser parser(tokens, source);
    auto stmts = parser.parse();
    Compiler compiler;
    auto program = compiler.compile(stmts);
    VM vm(program);
    vm.run();
    return vm.get_globals();
}

}  // namespace test
}  // namespace alphabet

#endif

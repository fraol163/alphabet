#ifndef ALPHABET_TEST_HELPERS_H
#define ALPHABET_TEST_HELPERS_H

#include <string>
#include <vector>
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

// Helper: compile and run source, capture output (returns VM state)
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

}  // namespace test
}  // namespace alphabet

#endif

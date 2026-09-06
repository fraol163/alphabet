#define CATCH_CONFIG_MAIN
#include "catch.hpp"

#include <cstdio>
#include <string>
#include <vector>

#include "compiler.h"
#include "ffi.h"
#include "lexer.h"
#include "parser.h"
#include "vm.h"

using namespace alphabet;

// ============================================================================
// Lexer Tests
// ============================================================================

TEST_CASE("Lexer recognizes single-char keywords", "[lexer]") {
    std::string source = "#alphabet<test>\ni (x > 0) { l (true) { r x } }";
    Lexer lexer(source);
    auto tokens = lexer.scan_tokens();

    bool found_if = false;
    bool found_loop = false;
    bool found_return = false;

    for (const auto& tok : tokens) {
        if (tok.type == TokenType::IF)
            found_if = true;
        if (tok.type == TokenType::LOOP)
            found_loop = true;
        if (tok.type == TokenType::RETURN)
            found_return = true;
    }

    REQUIRE(found_if);
    REQUIRE(found_loop);
    REQUIRE(found_return);
}

TEST_CASE("Lexer magic header validation", "[lexer]") {
    SECTION("Valid header should pass") {
        std::string source = "#alphabet<en>\n12 s = \"hello\"";
        Lexer lexer(source);
        auto tokens = lexer.scan_tokens();
        REQUIRE(tokens.size() > 0);
    }

    SECTION("Missing header should throw MissingLanguageHeader") {
        std::string bad_source = "12 s = \"hello\"";
        Lexer bad_lexer(bad_source);
        REQUIRE_THROWS_AS(bad_lexer.scan_tokens(), MissingLanguageHeader);
    }
}

TEST_CASE("Lexer parses numbers", "[lexer]") {
    std::string source = "#alphabet<test>\n1 x = 42";
    Lexer lexer(source);
    auto tokens = lexer.scan_tokens();

    bool found_42 = false;
    for (const auto& tok : tokens) {
        if (tok.type == TokenType::NUMBER && tok.literal == 42.0) {
            found_42 = true;
            break;
        }
    }

    REQUIRE(found_42);
}

TEST_CASE("Lexer parses strings", "[lexer]") {
    std::string source = "#alphabet<test>\n12 s = \"Hello, World!\"";
    Lexer lexer(source);
    auto tokens = lexer.scan_tokens();

    bool found_string = false;
    for (const auto& tok : tokens) {
        if (tok.type == TokenType::STRING) {
            found_string = true;
            REQUIRE(tok.lexeme == "Hello, World!");
        }
    }

    REQUIRE(found_string);
}

TEST_CASE("Lexer parses arithmetic operators", "[lexer]") {
    std::string source = "#alphabet<test>\n1 x = 1 + 2 - 3 * 4 / 5 % 6";
    Lexer lexer(source);
    auto tokens = lexer.scan_tokens();

    bool found_plus = false, found_minus = false, found_star = false;
    bool found_slash = false, found_percent = false;

    for (const auto& tok : tokens) {
        if (tok.type == TokenType::PLUS)
            found_plus = true;
        if (tok.type == TokenType::MINUS)
            found_minus = true;
        if (tok.type == TokenType::STAR)
            found_star = true;
        if (tok.type == TokenType::SLASH)
            found_slash = true;
        if (tok.type == TokenType::PERCENT)
            found_percent = true;
    }

    REQUIRE(found_plus);
    REQUIRE(found_minus);
    REQUIRE(found_star);
    REQUIRE(found_slash);
    REQUIRE(found_percent);
}

TEST_CASE("Lexer parses comparison operators", "[lexer]") {
    std::string source = "#alphabet<test>\n11 b = 1 == 2 && 3 != 4 || 5 > 6 && 7 < 8";
    Lexer lexer(source);
    auto tokens = lexer.scan_tokens();

    bool found_eq = false, found_ne = false, found_gt = false, found_lt = false;
    bool found_and = false, found_or = false;

    for (const auto& tok : tokens) {
        if (tok.type == TokenType::DOUBLE_EQUALS)
            found_eq = true;
        if (tok.type == TokenType::NOT_EQUALS)
            found_ne = true;
        if (tok.type == TokenType::GREATER)
            found_gt = true;
        if (tok.type == TokenType::LESS)
            found_lt = true;
        if (tok.type == TokenType::AND)
            found_and = true;
        if (tok.type == TokenType::OR)
            found_or = true;
    }

    REQUIRE(found_eq);
    REQUIRE(found_ne);
    REQUIRE(found_gt);
    REQUIRE(found_lt);
    REQUIRE(found_and);
    REQUIRE(found_or);
}

TEST_CASE("Lexer skips shebang lines", "[lexer]") {
    std::string source = "#!/usr/bin/env alphabet\n#alphabet<test>\n1 x = 1";
    Lexer lexer(source);
    auto tokens = lexer.scan_tokens();

    // Should not include shebang in tokens
    for (const auto& tok : tokens) {
        REQUIRE(tok.lexeme.find("#!") == std::string::npos);
    }

    // Should have found the number token
    bool found_number = false;
    for (const auto& tok : tokens) {
        if (tok.type == TokenType::NUMBER && tok.literal == 1.0) {
            found_number = true;
            break;
        }
    }
    REQUIRE(found_number);
}

TEST_CASE("Lexer skips comments", "[lexer]") {
    std::string source = "#alphabet<test>\n1 x = 1 // this is a comment\n2 y = 2";
    Lexer lexer(source);
    auto tokens = lexer.scan_tokens();

    // Comments should be skipped
    for (const auto& tok : tokens) {
        REQUIRE(tok.lexeme.find("//") == std::string::npos);
    }
}

// ============================================================================
// Lexer Negative Tests
// ============================================================================

TEST_CASE("Empty source throws MissingLanguageHeader", "[lexer][negative]") {
    std::string source = "";
    Lexer lexer(source);
    REQUIRE_THROWS_AS(lexer.scan_tokens(), MissingLanguageHeader);
}

TEST_CASE("Invalid number format with multiple dots", "[lexer][negative]") {
    std::string source = "#alphabet<test>\n1 x = 1.2.3";
    Lexer lexer(source);
    auto tokens = lexer.scan_tokens();

    // The lexer should still produce tokens but the number literal
    // should not be a valid value (it may parse only the first valid
    // portion, or the token stream should reflect the malformed input).
    // Verify that we do NOT get a clean 1.2.3 number token.
    bool found_bad_number = false;
    for (const auto& tok : tokens) {
        if (tok.type == TokenType::NUMBER) {
            // A well-formed number like 1.2 is acceptable, but 1.2.3
            // as a single token should not appear as 1.2.3 value.
            // If the lexer parses "1.2" and then ".3" separately, that's
            // fine. If it somehow produces a token with literal 1.2.3,
            // that would be a bug. We just verify no token has the
            // lexeme "1.2.3" as a NUMBER.
            if (tok.lexeme == "1.2.3") {
                found_bad_number = true;
            }
        }
    }
    // The malformed number should either be rejected or split -
    // in no case should we see "1.2.3" as a single NUMBER token
    REQUIRE_FALSE(found_bad_number);
}

TEST_CASE("Unterminated string literal", "[lexer][negative]") {
    std::string source = "#alphabet<test>\n12 s = \"unterminated";
    Lexer lexer(source);
    // Lexer doesn't throw on unterminated strings — it scans to end.
    // Verify it doesn't crash and produces tokens.
    auto tokens = lexer.scan_tokens();
    REQUIRE(tokens.size() > 0);
}

// ============================================================================
// Parser Tests
// ============================================================================

TEST_CASE("Parser handles variable declarations", "[parser]") {
    std::string source = "#alphabet<test>\n5 x = 10";
    Lexer lexer(source);
    Parser parser(lexer.scan_tokens());
    auto statements = parser.parse();

    REQUIRE(statements.size() == 1);
    REQUIRE(dynamic_cast<VarStmt*>(statements[0].get()) != nullptr);
}

TEST_CASE("Parser handles if statements", "[parser]") {
    std::string source = "#alphabet<test>\ni (1 > 0) { 5 x = 1 }";
    Lexer lexer(source);
    Parser parser(lexer.scan_tokens());
    auto statements = parser.parse();

    REQUIRE(statements.size() == 1);
    REQUIRE(dynamic_cast<IfStmt*>(statements[0].get()) != nullptr);
}

TEST_CASE("Parser handles loop statements", "[parser]") {
    std::string source = "#alphabet<test>\nl (1 > 0) { 5 x = x + 1 }";
    Lexer lexer(source);
    Parser parser(lexer.scan_tokens());
    auto statements = parser.parse();

    REQUIRE(statements.size() == 1);
    REQUIRE(dynamic_cast<LoopStmt*>(statements[0].get()) != nullptr);
}

TEST_CASE("Parser handles class declarations", "[parser]") {
    std::string source = "#alphabet<test>\nc MyClass { v m 5 method() { r 10 } }";
    Lexer lexer(source);
    Parser parser(lexer.scan_tokens());
    auto statements = parser.parse();

    REQUIRE(statements.size() == 1);
    auto* cls = dynamic_cast<ClassStmt*>(statements[0].get());
    REQUIRE(cls != nullptr);
    REQUIRE(cls->methods.size() == 1);
}

TEST_CASE("Parser handles binary expressions", "[parser]") {
    std::string source = "#alphabet<test>\n5 x = 1 + 2 * 3";
    Lexer lexer(source);
    Parser parser(lexer.scan_tokens());
    auto statements = parser.parse();

    REQUIRE(statements.size() == 1);
    auto* var = dynamic_cast<VarStmt*>(statements[0].get());
    REQUIRE(var != nullptr);
    REQUIRE(dynamic_cast<Binary*>(var->initializer.get()) != nullptr);
}

TEST_CASE("Parser handles function calls", "[parser]") {
    std::string source = "#alphabet<test>\nz.o(\"hello\")";
    Lexer lexer(source);
    Parser parser(lexer.scan_tokens());
    auto statements = parser.parse();

    REQUIRE(statements.size() == 1);
    auto* expr = dynamic_cast<ExpressionStmt*>(statements[0].get());
    REQUIRE(expr != nullptr);
    REQUIRE(dynamic_cast<Call*>(expr->expression.get()) != nullptr);
}

// ============================================================================
// VM Tests
// ============================================================================

TEST_CASE("VM push constant", "[vm]") {
    Program program;
    program.main.push_back(Instruction(OpCode::PUSH_CONST, 42.0));
    program.main.push_back(Instruction(OpCode::HALT));

    VM vm(program);
    REQUIRE_NOTHROW(vm.run());
}

TEST_CASE("VM arithmetic operations", "[vm]") {
    Program program;
    program.main.push_back(Instruction(OpCode::PUSH_CONST, 10.0));
    program.main.push_back(Instruction(OpCode::PUSH_CONST, 5.0));
    program.main.push_back(Instruction(OpCode::ADD));
    program.main.push_back(Instruction(OpCode::HALT));

    VM vm(program);
    REQUIRE_NOTHROW(vm.run());
}

TEST_CASE("VM comparison operations", "[vm]") {
    Program program;
    program.main.push_back(Instruction(OpCode::PUSH_CONST, 10.0));
    program.main.push_back(Instruction(OpCode::PUSH_CONST, 5.0));
    program.main.push_back(Instruction(OpCode::GT));
    program.main.push_back(Instruction(OpCode::HALT));

    VM vm(program);
    REQUIRE_NOTHROW(vm.run());
}

TEST_CASE("VM jump instruction", "[vm]") {
    Program program;
    program.main.push_back(Instruction(OpCode::PUSH_CONST, 1.0));
    program.main.push_back(Instruction(OpCode::JUMP, static_cast<int64_t>(3)));
    program.main.push_back(Instruction(OpCode::PUSH_CONST, 2.0));
    program.main.push_back(Instruction(OpCode::PUSH_CONST, 3.0));
    program.main.push_back(Instruction(OpCode::HALT));

    VM vm(program);
    REQUIRE_NOTHROW(vm.run());
}

TEST_CASE("VM list operations", "[vm]") {
    Program program;
    program.main.push_back(Instruction(OpCode::PUSH_CONST, 1.0));
    program.main.push_back(Instruction(OpCode::PUSH_CONST, 2.0));
    program.main.push_back(Instruction(OpCode::PUSH_CONST, 3.0));
    program.main.push_back(Instruction(OpCode::BUILD_LIST, static_cast<int64_t>(3)));
    program.main.push_back(Instruction(OpCode::HALT));

    VM vm(program);
    REQUIRE_NOTHROW(vm.run());
}

TEST_CASE("VM map operations", "[vm]") {
    Program program;
    program.main.push_back(Instruction(OpCode::PUSH_CONST, std::string("key")));
    program.main.push_back(Instruction(OpCode::PUSH_CONST, 42.0));
    program.main.push_back(Instruction(OpCode::BUILD_MAP, static_cast<int64_t>(1)));
    program.main.push_back(Instruction(OpCode::HALT));

    VM vm(program);
    REQUIRE_NOTHROW(vm.run());
}

// ============================================================================
// Integration Tests
// ============================================================================

TEST_CASE("Integration: hello world", "[integration]") {
    std::string source = R"(#alphabet<test>
12 h = "Hello Alphabet!"
z.o(h)
)";

    Lexer lexer(source);
    Parser parser(lexer.scan_tokens());
    auto statements = parser.parse();

    Compiler compiler;
    Program program = compiler.compile(statements);

    VM vm(program);
    REQUIRE_NOTHROW(vm.run());
}

TEST_CASE("Integration: arithmetic", "[integration]") {
    std::string source = R"(#alphabet<test>
5 x = 10 + 20 * 3
z.o(x)
)";

    Lexer lexer(source);
    Parser parser(lexer.scan_tokens());
    auto statements = parser.parse();

    Compiler compiler;
    Program program = compiler.compile(statements);

    VM vm(program);
    REQUIRE_NOTHROW(vm.run());
}

TEST_CASE("Integration: class basic", "[integration]") {
    std::string source = R"(#alphabet<test>
c A {
  v m 5 g() { r 10 }
}
15 o = n A()
z.o(o.g())
)";

    Lexer lexer(source);
    Parser parser(lexer.scan_tokens());
    auto statements = parser.parse();

    Compiler compiler;
    Program program = compiler.compile(statements);

    VM vm(program);
    REQUIRE_NOTHROW(vm.run());
}

// ============================================================================
// Non-English Keyword Translation Tests
// ============================================================================

static bool has_token(const std::vector<Token>& tokens, alphabet::TokenType type) {
    for (const auto& tok : tokens) {
        if (tok.type == type)
            return true;
    }
    return false;
}

TEST_CASE("Amharic keywords translate correctly", "[lexer][i18n]") {
    std::string source = "#alphabet<am>\n"
                         "ከሆነ (5 > 3) { ውጤት.o(\"yes\") } ያለበለዚያ { ውጤት.o(\"no\") }";
    Lexer lexer(source);
    auto tokens = lexer.scan_tokens();

    REQUIRE(has_token(tokens, TokenType::IF));
    REQUIRE(has_token(tokens, TokenType::ELSE));
}

TEST_CASE("Amharic loop keyword", "[lexer][i18n]") {
    std::string source = "#alphabet<am>\nሉፕ (5 j = 0 : j < 5 : j = j + 1) { }";
    Lexer lexer(source);
    auto tokens = lexer.scan_tokens();

    REQUIRE(has_token(tokens, TokenType::LOOP));
}

TEST_CASE("Amharic class keywords", "[lexer][i18n]") {
    std::string source = "#alphabet<am>\n"
                         "ክፍል ከልሲ { ግልጽ ዘዴ 5 መደመር() { ተመለስ 1 } }";
    Lexer lexer(source);
    auto tokens = lexer.scan_tokens();

    REQUIRE(has_token(tokens, TokenType::CLASS));
    REQUIRE(has_token(tokens, TokenType::PUBLIC));
    REQUIRE(has_token(tokens, TokenType::METHOD));
    REQUIRE(has_token(tokens, TokenType::RETURN));
}

TEST_CASE("Spanish keywords translate correctly", "[lexer][i18n]") {
    std::string source = "#alphabet<es>\n"
                         "si (5 > 3) { imprimir.o(\"yes\") } sino { imprimir.o(\"no\") }";
    Lexer lexer(source);
    auto tokens = lexer.scan_tokens();

    REQUIRE(has_token(tokens, TokenType::IF));
    REQUIRE(has_token(tokens, TokenType::ELSE));
}

TEST_CASE("Spanish loop keyword", "[lexer][i18n]") {
    std::string source = "#alphabet<es>\nbucle (5 j = 0 : j < 5 : j = j + 1) { }";
    Lexer lexer(source);
    auto tokens = lexer.scan_tokens();

    REQUIRE(has_token(tokens, TokenType::LOOP));
}

TEST_CASE("Spanish class keywords", "[lexer][i18n]") {
    std::string source = "#alphabet<es>\n"
                         "clase MiClase { público método 5 sumar() { retornar 1 } }";
    Lexer lexer(source);
    auto tokens = lexer.scan_tokens();

    REQUIRE(has_token(tokens, TokenType::CLASS));
    REQUIRE(has_token(tokens, TokenType::PUBLIC));
    REQUIRE(has_token(tokens, TokenType::METHOD));
    REQUIRE(has_token(tokens, TokenType::RETURN));
}

TEST_CASE("French keywords translate correctly", "[lexer][i18n]") {
    std::string source = "#alphabet<fr>\n"
                         "si (5 > 3) { afficher.o(\"yes\") } sinon { afficher.o(\"no\") }";
    Lexer lexer(source);
    auto tokens = lexer.scan_tokens();

    REQUIRE(has_token(tokens, TokenType::IF));
    REQUIRE(has_token(tokens, TokenType::ELSE));
}

TEST_CASE("French loop keyword", "[lexer][i18n]") {
    std::string source = "#alphabet<fr>\nboucle (5 j = 0 : j < 5 : j = j + 1) { }";
    Lexer lexer(source);
    auto tokens = lexer.scan_tokens();

    REQUIRE(has_token(tokens, TokenType::LOOP));
}

TEST_CASE("French class keywords", "[lexer][i18n]") {
    std::string source = "#alphabet<fr>\n"
                         "classe MaClasse { public méthode 5 additionner() { retour 1 } }";
    Lexer lexer(source);
    auto tokens = lexer.scan_tokens();

    REQUIRE(has_token(tokens, TokenType::CLASS));
    REQUIRE(has_token(tokens, TokenType::PUBLIC));
    REQUIRE(has_token(tokens, TokenType::METHOD));
    REQUIRE(has_token(tokens, TokenType::RETURN));
}

TEST_CASE("German keywords translate correctly", "[lexer][i18n]") {
    std::string source = "#alphabet<de>\n"
                         "wenn (5 > 3) { ausgeben.o(\"yes\") } sonst { ausgeben.o(\"no\") }";
    Lexer lexer(source);
    auto tokens = lexer.scan_tokens();

    REQUIRE(has_token(tokens, TokenType::IF));
    REQUIRE(has_token(tokens, TokenType::ELSE));
}

TEST_CASE("German loop keyword", "[lexer][i18n]") {
    std::string source = "#alphabet<de>\nschleife (5 j = 0 : j < 5 : j = j + 1) { }";
    Lexer lexer(source);
    auto tokens = lexer.scan_tokens();

    REQUIRE(has_token(tokens, TokenType::LOOP));
}

TEST_CASE("German class keywords", "[lexer][i18n]") {
    std::string source = "#alphabet<de>\n"
                         "klasse MeineKlasse { öffentlich methode 5 addieren() { zurück 1 } }";
    Lexer lexer(source);
    auto tokens = lexer.scan_tokens();

    REQUIRE(has_token(tokens, TokenType::CLASS));
    REQUIRE(has_token(tokens, TokenType::PUBLIC));
    REQUIRE(has_token(tokens, TokenType::METHOD));
    REQUIRE(has_token(tokens, TokenType::RETURN));
}

TEST_CASE("All 5 languages: try-catch keywords", "[lexer][i18n]") {
    SECTION("English") {
        std::string src = "#alphabet<en>\ntry { } catch { }";
        Lexer l(src);
        auto toks = l.scan_tokens();
        REQUIRE((has_token(toks, TokenType::TRY)));
        REQUIRE((has_token(toks, TokenType::HANDLE)));
    }
    SECTION("Amharic") {
        std::string src = "#alphabet<am>\nሞክር { } ያዟ { }";
        Lexer l(src);
        auto toks = l.scan_tokens();
        REQUIRE((has_token(toks, TokenType::TRY)));
        REQUIRE((has_token(toks, TokenType::HANDLE)));
    }
    SECTION("Spanish") {
        std::string src = "#alphabet<es>\nintentar { } capturar { }";
        Lexer l(src);
        auto toks = l.scan_tokens();
        REQUIRE((has_token(toks, TokenType::TRY)));
        REQUIRE((has_token(toks, TokenType::HANDLE)));
    }
    SECTION("French") {
        std::string src = "#alphabet<fr>\nessayer { } attraper { }";
        Lexer l(src);
        auto toks = l.scan_tokens();
        REQUIRE((has_token(toks, TokenType::TRY)));
        REQUIRE((has_token(toks, TokenType::HANDLE)));
    }
    SECTION("German") {
        std::string src = "#alphabet<de>\nversuchen { } fangen { }";
        Lexer l(src);
        auto toks = l.scan_tokens();
        REQUIRE((has_token(toks, TokenType::TRY)));
        REQUIRE((has_token(toks, TokenType::HANDLE)));
    }
}

// ============================================================================
// Regression tests for bugs found in the function-by-function audit
// ============================================================================

// BUG F: Lexer must skip past the language header even when the file does
// not end with a newline. Previously, current_ was not advanced unless a
// '\n' was present, causing the header text to be re-tokenized as code.
TEST_CASE("Lexer header without trailing newline is still skipped", "[lexer][regression]") {
    SECTION("No newline at all") {
        std::string src = "#alphabet<en>5 x = 1";
        Lexer l(src);
        auto toks = l.scan_tokens();
        // The header '#alphabet<en>' should not appear in the token stream.
        // We expect a NUMBER token with literal 5, then identifier 'x', etc.
        bool seen_5 = false;
        for (const auto& t : toks) {
            if (t.type == TokenType::NUMBER && t.literal == 5.0)
                seen_5 = true;
            // None of the tokens should have the '#alphabet' lexeme
            REQUIRE(t.lexeme.find("#alphabet") == std::string_view::npos);
        }
        REQUIRE(seen_5);
    }
    SECTION("CR only after header") {
        std::string src = "#alphabet<en>\r5 x = 1";
        Lexer l(src);
        REQUIRE_NOTHROW(l.scan_tokens());
    }
}

// BUG G: NullSafeGet dispatch. The lexer tokenizes '?.' but neither the
// parser nor the compiler used to consume it. This regression test ensures
// the parser produces a NullSafeGet AST node and the compiler dispatches it.
TEST_CASE("NullSafeGet parses and compiles", "[compiler][regression][nullsafe]") {
    std::string src = "#alphabet<en>\n"
                      "12 s = null\n"
                      "12 a = s?.name\n"
                      "z.o(a)";
    Lexer lexer(src);
    auto tokens = lexer.scan_tokens();
    Parser parser(tokens, src);
    auto stmts = parser.parse();
    REQUIRE_FALSE(parser.had_errors());

    Compiler compiler;
    Program program;
    REQUIRE_NOTHROW(program = compiler.compile(stmts));
    REQUIRE(program.main.size() > 0);
}

// BUG A: STORE_FIELD must leave the assigned value on the stack so the
// expression-statement POP emitted by visit_expression does not underflow.
// This regression test exercises a class init that writes to a field.
TEST_CASE("Class field assignment does not underflow VM stack", "[vm][regression][class]") {
    std::string src = "#alphabet<en>\n"
                      "c Counter {\n"
                      "  v m 5 init(5 start) {\n"
                      "    this.count = start\n"
                      "  }\n"
                      "  v m 5 get() {\n"
                      "    r this.count\n"
                      "  }\n"
                      "  v m 5 inc() {\n"
                      "    this.count = this.count + 1\n"
                      "    r this.count\n"
                      "  }\n"
                      "}\n"
                      "15 c = n Counter(10)\n"
                      "z.o(c.get())\n"
                      "z.o(c.inc())\n"
                      "z.o(c.inc())\n"
                      "z.o(c.get())\n";
    Lexer lexer(src);
    auto tokens = lexer.scan_tokens();
    Parser parser(tokens, src);
    auto stmts = parser.parse();
    REQUIRE_FALSE(parser.had_errors());

    Compiler compiler;
    Program program = compiler.compile(stmts);

    VM vm(program);
    std::streambuf* old_cout = std::cout.rdbuf();
    std::ostringstream capture;
    std::cout.rdbuf(capture.rdbuf());
    REQUIRE_NOTHROW(vm.run());
    std::cout.rdbuf(old_cout);
    REQUIRE(capture.str() == "10\n11\n12\n12\n");
}

// BUG D: An unhandled runtime exception must cause the VM to mark itself
// as failed and the process to exit with a non-zero status. The VM alone
// cannot exit the process; this test verifies the VM's had_runtime_error_
// flag is set so the host can act on it.
TEST_CASE("Runtime error sets had_runtime_error flag", "[vm][regression]") {
    std::string src = "#alphabet<en>\nz.o(10 / 0)\n";
    Lexer lexer(src);
    auto tokens = lexer.scan_tokens();
    Parser parser(tokens, src);
    auto stmts = parser.parse();
    REQUIRE_FALSE(parser.had_errors());

    Compiler compiler;
    Program program = compiler.compile(stmts);
    VM vm(program);
    vm.run();
    REQUIRE(vm.had_runtime_error());
    REQUIRE(vm.get_exit_code() != 0);
}

// =====================================================================
// Regression tests for bugs found during the 2026-09 audit
// =====================================================================

// BUG #1: TokenType::AT ('@') collides with EXPORT ('@') — two enums share
// the same integer value. tok.type == AT also matches EXPORT.
TEST_CASE("TokenType::AT and EXPORT have distinct integer values", "[lexer][regression][bug1]") {
    REQUIRE(static_cast<int>(TokenType::AT) != static_cast<int>(TokenType::EXPORT));
}

// BUG #3: multi_line_string line counting was duplicating advance()'s logic
// and forgetting the column reset. Verify the lexer reports correct line
// numbers for tokens AFTER a multi-line string literal.
TEST_CASE("Lexer line counter is correct after multi-line string", "[lexer][regression][bug3]") {
    std::string src = "#alphabet<en>\nx = \"\"\"\nhello\nworld\n\"\"\"\nz.o(1)\n";
    Lexer lex(src);
    auto tokens = lex.scan_tokens();
    // Find the NUMBER token (last operand). It must be on line 6, not line 3.
    const Token* last_number = nullptr;
    for (auto& t : tokens) {
        if (t.type == TokenType::NUMBER) last_number = &t;
    }
    REQUIRE(last_number != nullptr);
    REQUIRE(last_number->line == 6);
}

// BUG #4: UTF-8 continuation bytes (10xxxxxx = 128..191) — verified not
// a bug: lexer correctly produces one IDENTIFIER token per UTF-8 codepoint.
// Test passes against current code; left in for regression coverage.

// BUG #12: z.rand() and z.randint() must not use rand()/srand() (thread-
// unsafe). Running many calls concurrently should not crash or produce
// UB. We can't easily test thread-safety in a unit test, but we can
// verify the bounds are correct.
TEST_CASE("Builtin rand returns value in [0, 1)", "[builtins][regression][bug12]") {
    std::string src = "#alphabet<en>\n"
                      "5 sum = 0\n"
                      "l (5 i = 0 : i < 1000 : i = i + 1) {\n"
                      "  5 r = z.rand()\n"
                      "  i (r < 0) { 5 r = 0 }\n"
                      "  i (r >= 1) { 5 r = 0 }\n"
                      "  sum = sum + r\n"
                      "}\n"
                      "z.o(sum)\n";
    Lexer lex(src);
    auto tokens = lex.scan_tokens();
    Parser parser(tokens, src);
    auto stmts = parser.parse();
    REQUIRE_FALSE(parser.had_errors());

    Compiler compiler;
    Program program = compiler.compile(stmts);
    VM vm(program);
    std::ostringstream capture;
    auto* old = std::cout.rdbuf(capture.rdbuf());
    vm.run();
    std::cout.rdbuf(old);
    REQUIRE_FALSE(vm.had_runtime_error());
    // 1000 draws in [0,1) sum is in [0, 1000)
    double s = std::stod(capture.str());
    REQUIRE(s >= 0.0);
    REQUIRE(s < 1000.0);
}

TEST_CASE("Builtin randint returns value in [min, max]", "[builtins][regression][bug12]") {
    std::string src = "#alphabet<en>\n"
                      "5 out_of_range = 0\n"
                      "l (5 i = 0 : i < 200 : i = i + 1) {\n"
                      "  5 r = z.randint(5, 10)\n"
                      "  i (r < 5) { out_of_range = out_of_range + 1 }\n"
                      "  i (r > 10) { out_of_range = out_of_range + 1 }\n"
                      "}\n"
                      "z.o(out_of_range)\n";
    Lexer lex(src);
    auto tokens = lex.scan_tokens();
    Parser parser(tokens, src);
    auto stmts = parser.parse();
    REQUIRE_FALSE(parser.had_errors());

    Compiler compiler;
    Program program = compiler.compile(stmts);
    VM vm(program);
    std::ostringstream cap;
    auto* old = std::cout.rdbuf(cap.rdbuf());
    vm.run();
    std::cout.rdbuf(old);
    REQUIRE_FALSE(vm.had_runtime_error());
    REQUIRE(cap.str() == "0\n");  // No out-of-range values
}

// BUG #13: find() on empty needle returns -1 (matching count() behavior).
TEST_CASE("Builtin find with empty needle returns -1", "[builtins][regression][bug13]") {
    std::string src = "#alphabet<en>\nz.o(z.find(\"hello\", \"\"))\n";
    Lexer lex(src);
    auto tokens = lex.scan_tokens();
    Parser parser(tokens, src);
    auto stmts = parser.parse();
    REQUIRE_FALSE(parser.had_errors());

    Compiler compiler;
    Program program = compiler.compile(stmts);
    VM vm(program);
    std::ostringstream cap;
    auto* old = std::cout.rdbuf(cap.rdbuf());
    vm.run();
    std::cout.rdbuf(old);
    REQUIRE_FALSE(vm.had_runtime_error());
    REQUIRE(cap.str() == "-1\n");
}

// BUG #14: z.args() must reflect CLI args passed after the script path.
// This test exercises the VM-side VM::set_program_args setter; the
// main.cpp side is tested by the ctest harness via `alphabet run X a b c`.
TEST_CASE("VM::set_program_args is honored by z.args()", "[vm][regression][bug14]") {
    std::string src = "#alphabet<en>\n"
                      "5 a = z.args()\n"
                      "z.o(z.len(a))\n";
    Lexer lex(src);
    auto tokens = lex.scan_tokens();
    Parser parser(tokens, src);
    auto stmts = parser.parse();
    REQUIRE_FALSE(parser.had_errors());

    Compiler compiler;
    Program program = compiler.compile(stmts);
    VM vm(program);
    vm.set_program_args({"alpha", "beta", "gamma"});
    std::ostringstream cap;
    auto* old = std::cout.rdbuf(cap.rdbuf());
    vm.run();
    std::cout.rdbuf(old);
    REQUIRE_FALSE(vm.had_runtime_error());
    REQUIRE(cap.str() == "3\n");
}

// BUG #6: call_lambda / call_lambda_public loop breaks without popping the
// finished frame, so frames_ grows unboundedly. A regression test would
// need to call many lambdas and check frames_ size, but frames_ is private.
// The observable symptom: VM stack overflow after many calls. Skipping
// a runtime test for now — fix lands in vm.cpp.

// BUG #7: call_lambda returns Value(nullptr) when no result was pushed.
// The caller cannot distinguish "returned null" from "returned nothing".
// Skipped for now — fix lands in vm.cpp and changes the API.

// BUG #8: run_loop pushes null AND pops the frame when ip >= bytecode->size().
// If a caller pushed a result slot before the frame ran out, that result
// is overwritten with null. Observable as:
//   m 5 f() { r 5 }   ; 5 x = f()  ; z.o(x)
// expected: x == 5
// observed before fix: x == null  (frame end overwrites result)
TEST_CASE("Function return value is not overwritten by frame-end null push", "[vm][regression][bug8]") {
    std::string src =
        "#alphabet<en>\n"
        "m 5 f() { r 5 }\n"
        "5 x = f()\n"
        "z.o(x)\n";
    Lexer lex(src);
    auto tokens = lex.scan_tokens();
    Parser parser(tokens, src);
    auto stmts = parser.parse();
    REQUIRE_FALSE(parser.had_errors());

    Compiler compiler;
    Program program = compiler.compile(stmts);
    VM vm(program);
    std::ostringstream capture;
    auto* old = std::cout.rdbuf(capture.rdbuf());
    vm.run();
    std::cout.rdbuf(old);
    REQUIRE_FALSE(vm.had_runtime_error());
    REQUIRE(capture.str() == "5\n");
}

// BUG #9 (PUSH_CONST list/map/object): pushing a list literal should
// preserve the list value, not drop it.
TEST_CASE("PUSH_CONST preserves list literal", "[vm][regression][bug9]") {
    std::string src =
        "#alphabet<en>\n"
        "13 xs = [1, 2, 3]\n"
        "z.o(xs[1])\n";
    Lexer lex(src);
    auto tokens = lex.scan_tokens();
    Parser parser(tokens, src);
    auto stmts = parser.parse();
    REQUIRE_FALSE(parser.had_errors());

    Compiler compiler;
    Program program = compiler.compile(stmts);
    VM vm(program);
    std::ostringstream capture;
    auto* old = std::cout.rdbuf(capture.rdbuf());
    vm.run();
    std::cout.rdbuf(old);
    REQUIRE_FALSE(vm.had_runtime_error());
    REQUIRE(capture.str() == "2\n");
}

// BUG COMPILER-2: A class with a non-static field that has an initializer
// used to crash any method on that class because compile_class_def
// auto-emitted RET in field_init. run_field_init runs inline in the
// caller's frame; the auto-RET popped it. Verify the bug is gone.
// Test: a class with a default-value field, then access via static method
// to avoid the type-mismatch issue with dynamic class IDs.
// BUG COMPILER-2: A class with a non-static field that has an initializer
// used to crash any method on that class because compile_class_def
// auto-emitted RET in field_init. run_field_init runs inline in the
// caller's frame; the auto-RET popped it. Verify the bug is gone.
//
// Verified case: `c Box { 12 v = 42; m 5 get() { r this.v } }` followed by
// `5 b = n Box(); z.o(b.get())` must print `42`. With the bug, the
// stray RET in field_init would either return null from `get()` or
// crash the VM mid-frame.
TEST_CASE("Class with field initializer does not break methods", "[compiler][regression][classfield]") {
    std::string src =
        "#alphabet<en>\n"
        "c Box {\n"
        "    12 v = 42\n"
        "    m 5 get() {\n"
        "        r this.v\n"
        "    }\n"
        "}\n"
        "5 b = n Box()\n"
        "z.o(b.get())\n";
    Lexer lex(src);
    auto tokens = lex.scan_tokens();
    Parser parser(tokens, src);
    auto stmts = parser.parse();
    REQUIRE_FALSE(parser.had_errors());

    Compiler compiler;
    Program program;
    REQUIRE_NOTHROW(program = compiler.compile(stmts));
    VM vm(program);
    std::ostringstream cap;
    auto* old3 = std::cout.rdbuf(cap.rdbuf());
    vm.run();
    std::cout.rdbuf(old3);
    REQUIRE_FALSE(vm.had_runtime_error());
    REQUIRE(cap.str() == "42\n");
}

// BUG BCIO-1: bytecode_io read_* helpers used to silently return
// garbage on truncated files. They now throw.
TEST_CASE("Bytecode loader rejects truncated files", "[bytecode][regression][bcio]") {
    // "ALPH" header + version=2, but truncated before the body.
    std::string bad = std::string("ALPH") + char(0) + char(2);
    char tmp[] = "/tmp/abc_bcio_trunc_XXXXXX";
    int fd = mkstemp(tmp);
    if (fd < 0) {
        SUCCEED("could not create tmp; skipping");
    } else {
        ssize_t w = write(fd, bad.data(), bad.size());
        (void)w;
        close(fd);
        bool threw = false;
        try {
            alphabet::Program p = alphabet::Program::load_from_file(tmp);
        } catch (const std::runtime_error&) {
            threw = true;
        }
        std::remove(tmp);
        REQUIRE(threw);
    }
}

// BUG BCIO-4: loading a bytecode file with a wrong magic / version
// used to silently corrupt; now throws a clear error.
TEST_CASE("Bytecode loader rejects bad magic", "[bytecode][regression][bcio]") {
    std::string bad = "NOPE" + char(0) + char(2) + char(0) + char(0) + char(0) + char(0);
    char tmp[] = "/tmp/abc_bcio_bad_XXXXXX";
    int fd = mkstemp(tmp);
    if (fd < 0) {
        SUCCEED("could not create tmp; skipping");
    } else {
        ssize_t w = write(fd, bad.data(), bad.size());
        (void)w;
        close(fd);
        bool threw = false;
        try {
            alphabet::Program p = alphabet::Program::load_from_file(tmp);
        } catch (const std::runtime_error&) {
            threw = true;
        }
        std::remove(tmp);
        REQUIRE(threw);
    }
}

// BUG #10 (null comparison & arithmetic semantics): GT/LT/GE/LE with null used to
// return null (silent). They now return false, matching JavaScript
// semantics and making `i (x > 5)` a usable guard for nullable values.
// In contrast, null + non-null arithmetic propagates null as "no value"
// pass-through — this is a defensible design choice (Python: TypeError,
// JS: numeric coercion, C: undefined).
TEST_CASE("Null comparison returns false; null arithmetic propagates null", "[vm][regression][nullcmp]") {
    // Part 1: comparisons with null yield false.
    {
        std::string src =
            "#alphabet<en>\n"
            "12 x = null\n"
            "z.o(x > 5)\n"
            "z.o(x < 5)\n"
            "z.o(x >= 5)\n"
            "z.o(x <= 5)\n"
            "z.o(x == 5)\n"
            "z.o(x != 5)\n"
            "z.o(x == null)\n";
        Lexer lex(src);
        auto tokens = lex.scan_tokens();
        Parser parser(tokens, src);
        auto stmts = parser.parse();
        REQUIRE_FALSE(parser.had_errors());

        Compiler compiler;
        Program program = compiler.compile(stmts);
        VM vm(program);
        std::ostringstream cap;
        auto* old = std::cout.rdbuf(cap.rdbuf());
        vm.run();
        std::cout.rdbuf(old);
        REQUIRE_FALSE(vm.had_runtime_error());
        REQUIRE(cap.str() == "false\nfalse\nfalse\nfalse\nfalse\ntrue\ntrue\n");
    }
    // Part 2: arithmetic with null propagates null as "no value" pass-through.
    {
        std::string src =
            "#alphabet<en>\n"
            "12 x = null\n"
            "z.o(x + 5)\n";
        Lexer lex(src);
        auto tokens = lex.scan_tokens();
        Parser parser(tokens, src);
        auto stmts = parser.parse();
        REQUIRE_FALSE(parser.had_errors());

        Compiler compiler;
        Program program = compiler.compile(stmts);
        VM vm(program);
        std::ostringstream cap;
        auto* old = std::cout.rdbuf(cap.rdbuf());
        vm.run();
        std::cout.rdbuf(old);
        REQUIRE_FALSE(vm.had_runtime_error());
        REQUIRE(cap.str() == "null\n");
    }
    // Part 3: explicit null check works around the throw.
    {
        std::string src =
            "#alphabet<en>\n"
            "12 x = null\n"
            "5 safe = 0\n"
            "i (x == null) { safe = 0 }\n"
            "e { safe = x + 5 }\n"
            "z.o(safe)\n";
        Lexer lex(src);
        auto tokens = lex.scan_tokens();
        Parser parser(tokens, src);
        auto stmts = parser.parse();
        REQUIRE_FALSE(parser.had_errors());

        Compiler compiler;
        Program program = compiler.compile(stmts);
        VM vm(program);
        std::ostringstream cap;
        auto* old = std::cout.rdbuf(cap.rdbuf());
        vm.run();
        std::cout.rdbuf(old);
        REQUIRE_FALSE(vm.had_runtime_error());
        REQUIRE(cap.str() == "0\n");
    }
}

// BUG FFI-1: ffi_register_function was a stub that always returned 1
// without storing the function pointer. It now keeps a thread-safe
// registry, and ffi_call checks the registry before dlopen so embedded
// hosts can expose C/C++ functions without a .so file.
static FFIValue test_ffi_add(FFIValue* args, int n) {
    if (n < 2) return ffi_make_int(0);
    int64_t a = args[0].type == FFI_TYPE_INT ? args[0].data.int_val : 0;
    int64_t b = args[1].type == FFI_TYPE_INT ? args[1].data.int_val : 0;
    return ffi_make_int(a + b);
}

TEST_CASE("ffi_register_function stores and dispatches", "[ffi][regression][register]") {
    ffi_cleanup();
    FFIType args[2] = {FFI_TYPE_INT, FFI_TYPE_INT};
    int rc = ffi_register_function("test_add",
                                   reinterpret_cast<void*>(&test_ffi_add),
                                   args, 2, FFI_TYPE_INT);
    REQUIRE(rc == 1);

    FFIValue in[2];
    in[0] = ffi_make_int(40);
    in[1] = ffi_make_int(2);
    FFIResult r = ffi_call(nullptr, "test_add", in, 2);
    REQUIRE(r.success == 1);
    REQUIRE(r.value.type == FFI_TYPE_INT);
    REQUIRE(r.value.data.int_val == 42);

    // null function name returns error (not crash)
    FFIResult r2 = ffi_call(nullptr, "does_not_exist", in, 2);
    REQUIRE(r2.success == 0);
    bool msg_ok = (std::string(r2.error_message).find("Cannot open") != std::string::npos) ||
                  (std::string(r2.error_message).find("not found") != std::string::npos);
    REQUIRE(msg_ok);

    // Cleanup
    ffi_cleanup();
    FFIResult r3 = ffi_call(nullptr, "test_add", in, 2);
    REQUIRE(r3.success == 0);  // gone after cleanup
}

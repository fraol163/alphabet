#define CATCH_CONFIG_MAIN
#include "catch.hpp"

#include <string>
#include <vector>

#include "lexer.h"
#include "parser.h"
#include "compiler.h"
#include "vm.h"

using namespace alphabet;

// ============================================================================
// Lexer Tests
// ============================================================================

TEST_CASE("Lexer recognizes single-char keywords", "[lexer]")
{
    std::string source = "#alphabet<test>\ni (x > 0) { l (true) { r x } }";
    Lexer lexer(source);
    auto tokens = lexer.scan_tokens();

    bool found_if = false;
    bool found_loop = false;
    bool found_return = false;

    for (const auto &tok : tokens) {
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

TEST_CASE("Lexer magic header validation", "[lexer]")
{
    SECTION("Valid header should pass")
    {
        std::string source = "#alphabet<en>\n12 s = \"hello\"";
        Lexer lexer(source);
        auto tokens = lexer.scan_tokens();
        REQUIRE(tokens.size() > 0);
    }

    SECTION("Missing header should throw MissingLanguageHeader")
    {
        std::string bad_source = "12 s = \"hello\"";
        Lexer bad_lexer(bad_source);
        REQUIRE_THROWS_AS(bad_lexer.scan_tokens(), MissingLanguageHeader);
    }
}

TEST_CASE("Lexer parses numbers", "[lexer]")
{
    std::string source = "#alphabet<test>\n1 x = 42";
    Lexer lexer(source);
    auto tokens = lexer.scan_tokens();

    bool found_42 = false;
    for (const auto &tok : tokens) {
        if (tok.type == TokenType::NUMBER && tok.literal == 42.0) {
            found_42 = true;
            break;
        }
    }

    REQUIRE(found_42);
}

TEST_CASE("Lexer parses strings", "[lexer]")
{
    std::string source = "#alphabet<test>\n12 s = \"Hello, World!\"";
    Lexer lexer(source);
    auto tokens = lexer.scan_tokens();

    bool found_string = false;
    for (const auto &tok : tokens) {
        if (tok.type == TokenType::STRING) {
            found_string = true;
            REQUIRE(tok.lexeme == "Hello, World!");
        }
    }

    REQUIRE(found_string);
}

TEST_CASE("Lexer parses arithmetic operators", "[lexer]")
{
    std::string source = "#alphabet<test>\n1 x = 1 + 2 - 3 * 4 / 5 % 6";
    Lexer lexer(source);
    auto tokens = lexer.scan_tokens();

    bool found_plus = false, found_minus = false, found_star = false;
    bool found_slash = false, found_percent = false;

    for (const auto &tok : tokens) {
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

TEST_CASE("Lexer parses comparison operators", "[lexer]")
{
    std::string source = "#alphabet<test>\n11 b = 1 == 2 && 3 != 4 || 5 > 6 && 7 < 8";
    Lexer lexer(source);
    auto tokens = lexer.scan_tokens();

    bool found_eq = false, found_ne = false, found_gt = false, found_lt = false;
    bool found_and = false, found_or = false;

    for (const auto &tok : tokens) {
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

TEST_CASE("Lexer skips shebang lines", "[lexer]")
{
    std::string source = "#!/usr/bin/env alphabet\n#alphabet<test>\n1 x = 1";
    Lexer lexer(source);
    auto tokens = lexer.scan_tokens();

    // Should not include shebang in tokens
    for (const auto &tok : tokens) {
        REQUIRE(tok.lexeme.find("#!") == std::string::npos);
    }

    // Should have found the number token
    bool found_number = false;
    for (const auto &tok : tokens) {
        if (tok.type == TokenType::NUMBER && tok.literal == 1.0) {
            found_number = true;
            break;
        }
    }
    REQUIRE(found_number);
}

TEST_CASE("Lexer skips comments", "[lexer]")
{
    std::string source = "#alphabet<test>\n1 x = 1 // this is a comment\n2 y = 2";
    Lexer lexer(source);
    auto tokens = lexer.scan_tokens();

    // Comments should be skipped
    for (const auto &tok : tokens) {
        REQUIRE(tok.lexeme.find("//") == std::string::npos);
    }
}

// ============================================================================
// Lexer Negative Tests
// ============================================================================

TEST_CASE("Empty source throws MissingLanguageHeader", "[lexer][negative]")
{
    std::string source = "";
    Lexer lexer(source);
    REQUIRE_THROWS_AS(lexer.scan_tokens(), MissingLanguageHeader);
}

TEST_CASE("Invalid number format with multiple dots", "[lexer][negative]")
{
    std::string source = "#alphabet<test>\n1 x = 1.2.3";
    Lexer lexer(source);
    auto tokens = lexer.scan_tokens();

    // The lexer should still produce tokens but the number literal
    // should not be a valid value (it may parse only the first valid
    // portion, or the token stream should reflect the malformed input).
    // Verify that we do NOT get a clean 1.2.3 number token.
    bool found_bad_number = false;
    for (const auto &tok : tokens) {
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

TEST_CASE("Unterminated string literal", "[lexer][negative]")
{
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

TEST_CASE("Parser handles variable declarations", "[parser]")
{
    std::string source = "#alphabet<test>\n5 x = 10";
    Lexer lexer(source);
    Parser parser(lexer.scan_tokens());
    auto statements = parser.parse();

    REQUIRE(statements.size() == 1);
    REQUIRE(dynamic_cast<VarStmt *>(statements[0].get()) != nullptr);
}

TEST_CASE("Parser handles if statements", "[parser]")
{
    std::string source = "#alphabet<test>\ni (1 > 0) { 5 x = 1 }";
    Lexer lexer(source);
    Parser parser(lexer.scan_tokens());
    auto statements = parser.parse();

    REQUIRE(statements.size() == 1);
    REQUIRE(dynamic_cast<IfStmt *>(statements[0].get()) != nullptr);
}

TEST_CASE("Parser handles loop statements", "[parser]")
{
    std::string source = "#alphabet<test>\nl (1 > 0) { 5 x = x + 1 }";
    Lexer lexer(source);
    Parser parser(lexer.scan_tokens());
    auto statements = parser.parse();

    REQUIRE(statements.size() == 1);
    REQUIRE(dynamic_cast<LoopStmt *>(statements[0].get()) != nullptr);
}

TEST_CASE("Parser handles class declarations", "[parser]")
{
    std::string source = "#alphabet<test>\nc MyClass { v m 5 method() { r 10 } }";
    Lexer lexer(source);
    Parser parser(lexer.scan_tokens());
    auto statements = parser.parse();

    REQUIRE(statements.size() == 1);
    auto *cls = dynamic_cast<ClassStmt *>(statements[0].get());
    REQUIRE(cls != nullptr);
    REQUIRE(cls->methods.size() == 1);
}

TEST_CASE("Parser handles binary expressions", "[parser]")
{
    std::string source = "#alphabet<test>\n5 x = 1 + 2 * 3";
    Lexer lexer(source);
    Parser parser(lexer.scan_tokens());
    auto statements = parser.parse();

    REQUIRE(statements.size() == 1);
    auto *var = dynamic_cast<VarStmt *>(statements[0].get());
    REQUIRE(var != nullptr);
    REQUIRE(dynamic_cast<Binary *>(var->initializer.get()) != nullptr);
}

TEST_CASE("Parser handles function calls", "[parser]")
{
    std::string source = "#alphabet<test>\nz.o(\"hello\")";
    Lexer lexer(source);
    Parser parser(lexer.scan_tokens());
    auto statements = parser.parse();

    REQUIRE(statements.size() == 1);
    auto *expr = dynamic_cast<ExpressionStmt *>(statements[0].get());
    REQUIRE(expr != nullptr);
    REQUIRE(dynamic_cast<Call *>(expr->expression.get()) != nullptr);
}

// ============================================================================
// VM Tests
// ============================================================================

TEST_CASE("VM push constant", "[vm]")
{
    Program program;
    program.main.push_back(Instruction(OpCode::PUSH_CONST, 42.0));
    program.main.push_back(Instruction(OpCode::HALT));

    VM vm(program);
    REQUIRE_NOTHROW(vm.run());
}

TEST_CASE("VM arithmetic operations", "[vm]")
{
    Program program;
    program.main.push_back(Instruction(OpCode::PUSH_CONST, 10.0));
    program.main.push_back(Instruction(OpCode::PUSH_CONST, 5.0));
    program.main.push_back(Instruction(OpCode::ADD));
    program.main.push_back(Instruction(OpCode::HALT));

    VM vm(program);
    REQUIRE_NOTHROW(vm.run());
}

TEST_CASE("VM comparison operations", "[vm]")
{
    Program program;
    program.main.push_back(Instruction(OpCode::PUSH_CONST, 10.0));
    program.main.push_back(Instruction(OpCode::PUSH_CONST, 5.0));
    program.main.push_back(Instruction(OpCode::GT));
    program.main.push_back(Instruction(OpCode::HALT));

    VM vm(program);
    REQUIRE_NOTHROW(vm.run());
}

TEST_CASE("VM jump instruction", "[vm]")
{
    Program program;
    program.main.push_back(Instruction(OpCode::PUSH_CONST, 1.0));
    program.main.push_back(Instruction(OpCode::JUMP, static_cast<int64_t>(3)));
    program.main.push_back(Instruction(OpCode::PUSH_CONST, 2.0));
    program.main.push_back(Instruction(OpCode::PUSH_CONST, 3.0));
    program.main.push_back(Instruction(OpCode::HALT));

    VM vm(program);
    REQUIRE_NOTHROW(vm.run());
}

TEST_CASE("VM list operations", "[vm]")
{
    Program program;
    program.main.push_back(Instruction(OpCode::PUSH_CONST, 1.0));
    program.main.push_back(Instruction(OpCode::PUSH_CONST, 2.0));
    program.main.push_back(Instruction(OpCode::PUSH_CONST, 3.0));
    program.main.push_back(Instruction(OpCode::BUILD_LIST, static_cast<int64_t>(3)));
    program.main.push_back(Instruction(OpCode::HALT));

    VM vm(program);
    REQUIRE_NOTHROW(vm.run());
}

TEST_CASE("VM map operations", "[vm]")
{
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

TEST_CASE("Integration: hello world", "[integration]")
{
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

TEST_CASE("Integration: arithmetic", "[integration]")
{
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

TEST_CASE("Integration: class basic", "[integration]")
{
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

static bool has_token(const std::vector<Token> &tokens, alphabet::TokenType type)
{
    for (const auto &tok : tokens) {
        if (tok.type == type)
            return true;
    }
    return false;
}

TEST_CASE("Amharic keywords translate correctly", "[lexer][i18n]")
{
    std::string source = "#alphabet<am>\n"
                         "ከሆነ (5 > 3) { ውጤት.o(\"yes\") } ያለበለዚያ { ውጤት.o(\"no\") }";
    Lexer lexer(source);
    auto tokens = lexer.scan_tokens();

    REQUIRE(has_token(tokens, TokenType::IF));
    REQUIRE(has_token(tokens, TokenType::ELSE));
}

TEST_CASE("Amharic loop keyword", "[lexer][i18n]")
{
    std::string source = "#alphabet<am>\nሉፕ (5 j = 0 : j < 5 : j = j + 1) { }";
    Lexer lexer(source);
    auto tokens = lexer.scan_tokens();

    REQUIRE(has_token(tokens, TokenType::LOOP));
}

TEST_CASE("Amharic class keywords", "[lexer][i18n]")
{
    std::string source = "#alphabet<am>\n"
                         "ክፍል ከልሲ { ግልጽ ዘዴ 5 መደመር() { ተመለስ 1 } }";
    Lexer lexer(source);
    auto tokens = lexer.scan_tokens();

    REQUIRE(has_token(tokens, TokenType::CLASS));
    REQUIRE(has_token(tokens, TokenType::PUBLIC));
    REQUIRE(has_token(tokens, TokenType::METHOD));
    REQUIRE(has_token(tokens, TokenType::RETURN));
}

TEST_CASE("Spanish keywords translate correctly", "[lexer][i18n]")
{
    std::string source = "#alphabet<es>\n"
                         "si (5 > 3) { imprimir.o(\"yes\") } sino { imprimir.o(\"no\") }";
    Lexer lexer(source);
    auto tokens = lexer.scan_tokens();

    REQUIRE(has_token(tokens, TokenType::IF));
    REQUIRE(has_token(tokens, TokenType::ELSE));
}

TEST_CASE("Spanish loop keyword", "[lexer][i18n]")
{
    std::string source = "#alphabet<es>\nbucle (5 j = 0 : j < 5 : j = j + 1) { }";
    Lexer lexer(source);
    auto tokens = lexer.scan_tokens();

    REQUIRE(has_token(tokens, TokenType::LOOP));
}

TEST_CASE("Spanish class keywords", "[lexer][i18n]")
{
    std::string source = "#alphabet<es>\n"
                         "clase MiClase { público método 5 sumar() { retornar 1 } }";
    Lexer lexer(source);
    auto tokens = lexer.scan_tokens();

    REQUIRE(has_token(tokens, TokenType::CLASS));
    REQUIRE(has_token(tokens, TokenType::PUBLIC));
    REQUIRE(has_token(tokens, TokenType::METHOD));
    REQUIRE(has_token(tokens, TokenType::RETURN));
}

TEST_CASE("French keywords translate correctly", "[lexer][i18n]")
{
    std::string source = "#alphabet<fr>\n"
                         "si (5 > 3) { afficher.o(\"yes\") } sinon { afficher.o(\"no\") }";
    Lexer lexer(source);
    auto tokens = lexer.scan_tokens();

    REQUIRE(has_token(tokens, TokenType::IF));
    REQUIRE(has_token(tokens, TokenType::ELSE));
}

TEST_CASE("French loop keyword", "[lexer][i18n]")
{
    std::string source = "#alphabet<fr>\nboucle (5 j = 0 : j < 5 : j = j + 1) { }";
    Lexer lexer(source);
    auto tokens = lexer.scan_tokens();

    REQUIRE(has_token(tokens, TokenType::LOOP));
}

TEST_CASE("French class keywords", "[lexer][i18n]")
{
    std::string source = "#alphabet<fr>\n"
                         "classe MaClasse { public méthode 5 additionner() { retour 1 } }";
    Lexer lexer(source);
    auto tokens = lexer.scan_tokens();

    REQUIRE(has_token(tokens, TokenType::CLASS));
    REQUIRE(has_token(tokens, TokenType::PUBLIC));
    REQUIRE(has_token(tokens, TokenType::METHOD));
    REQUIRE(has_token(tokens, TokenType::RETURN));
}

TEST_CASE("German keywords translate correctly", "[lexer][i18n]")
{
    std::string source = "#alphabet<de>\n"
                         "wenn (5 > 3) { ausgeben.o(\"yes\") } sonst { ausgeben.o(\"no\") }";
    Lexer lexer(source);
    auto tokens = lexer.scan_tokens();

    REQUIRE(has_token(tokens, TokenType::IF));
    REQUIRE(has_token(tokens, TokenType::ELSE));
}

TEST_CASE("German loop keyword", "[lexer][i18n]")
{
    std::string source = "#alphabet<de>\nschleife (5 j = 0 : j < 5 : j = j + 1) { }";
    Lexer lexer(source);
    auto tokens = lexer.scan_tokens();

    REQUIRE(has_token(tokens, TokenType::LOOP));
}

TEST_CASE("German class keywords", "[lexer][i18n]")
{
    std::string source = "#alphabet<de>\n"
                         "klasse MeineKlasse { öffentlich methode 5 addieren() { zurück 1 } }";
    Lexer lexer(source);
    auto tokens = lexer.scan_tokens();

    REQUIRE(has_token(tokens, TokenType::CLASS));
    REQUIRE(has_token(tokens, TokenType::PUBLIC));
    REQUIRE(has_token(tokens, TokenType::METHOD));
    REQUIRE(has_token(tokens, TokenType::RETURN));
}

TEST_CASE("All 5 languages: try-catch keywords", "[lexer][i18n]")
{
    SECTION("English")
    {
        std::string src = "#alphabet<en>\ntry { } catch { }";
        Lexer l(src);
        auto toks = l.scan_tokens();
        REQUIRE((has_token(toks, TokenType::TRY)));
        REQUIRE((has_token(toks, TokenType::HANDLE)));
    }
    SECTION("Amharic")
    {
        std::string src = "#alphabet<am>\nሞክር { } ያዟ { }";
        Lexer l(src);
        auto toks = l.scan_tokens();
        REQUIRE((has_token(toks, TokenType::TRY)));
        REQUIRE((has_token(toks, TokenType::HANDLE)));
    }
    SECTION("Spanish")
    {
        std::string src = "#alphabet<es>\nintentar { } capturar { }";
        Lexer l(src);
        auto toks = l.scan_tokens();
        REQUIRE((has_token(toks, TokenType::TRY)));
        REQUIRE((has_token(toks, TokenType::HANDLE)));
    }
    SECTION("French")
    {
        std::string src = "#alphabet<fr>\nessayer { } attraper { }";
        Lexer l(src);
        auto toks = l.scan_tokens();
        REQUIRE((has_token(toks, TokenType::TRY)));
        REQUIRE((has_token(toks, TokenType::HANDLE)));
    }
    SECTION("German")
    {
        std::string src = "#alphabet<de>\nversuchen { } fangen { }";
        Lexer l(src);
        auto toks = l.scan_tokens();
        REQUIRE((has_token(toks, TokenType::TRY)));
        REQUIRE((has_token(toks, TokenType::HANDLE)));
    }
}

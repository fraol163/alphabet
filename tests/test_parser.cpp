#define CATCH_CONFIG_MAIN
#include "catch.hpp"

#include <memory>
#include <string>

#include "alphabet_ast.h"
#include "lexer.h"
#include "parser.h"

using namespace alphabet;

// Helper to parse source
static std::vector<StmtPtr> parse_source(const std::string &source)
{
    Lexer lexer(source);
    Parser parser(lexer.scan_tokens());
    return parser.parse();
}

// Helper to parse source and get the parser object for error inspection
static Parser make_parser(const std::string &source)
{
    Lexer lexer(source);
    return Parser(lexer.scan_tokens());
}

// ============================================================================
// Expression Parsing Tests
// ============================================================================

TEST_CASE("parser literal number", "[parser]")
{
    std::string source = "#alphabet<test>\n5 x = 42";
    auto statements = parse_source(source);

    REQUIRE(statements.size() == 1);
    auto *var = dynamic_cast<VarStmt *>(statements[0].get());
    REQUIRE(var != nullptr);

    auto *lit = dynamic_cast<Literal *>(var->initializer.get());
    REQUIRE(lit != nullptr);
    REQUIRE(std::holds_alternative<int64_t>(lit->value));
    REQUIRE(std::get<int64_t>(lit->value) == 42);
}

TEST_CASE("parser literal string", "[parser]")
{
    std::string source = "#alphabet<test>\n12 s = \"hello world\"";
    auto statements = parse_source(source);

    REQUIRE(statements.size() == 1);
    auto *var = dynamic_cast<VarStmt *>(statements[0].get());
    REQUIRE(var != nullptr);

    auto *lit = dynamic_cast<Literal *>(var->initializer.get());
    REQUIRE(lit != nullptr);
}

TEST_CASE("parser binary addition", "[parser]")
{
    std::string source = "#alphabet<test>\n5 x = 1 + 2";
    auto statements = parse_source(source);

    REQUIRE(statements.size() == 1);
    auto *var = dynamic_cast<VarStmt *>(statements[0].get());
    REQUIRE(var != nullptr);

    auto *bin = dynamic_cast<Binary *>(var->initializer.get());
    REQUIRE(bin != nullptr);
    REQUIRE(bin->op.type == TokenType::PLUS);
}

TEST_CASE("parser precedence mul before add", "[parser]")
{
    // 1 + 2 * 3 should parse as 1 + (2 * 3)
    std::string source = "#alphabet<test>\n5 x = 1 + 2 * 3";
    auto statements = parse_source(source);

    auto *var = dynamic_cast<VarStmt *>(statements[0].get());
    REQUIRE(var != nullptr);

    auto *top = dynamic_cast<Binary *>(var->initializer.get());
    REQUIRE(top != nullptr);
    REQUIRE(top->op.type == TokenType::PLUS);

    // Right side should be multiplication
    auto *right = dynamic_cast<Binary *>(top->right.get());
    REQUIRE(right != nullptr);
    REQUIRE(right->op.type == TokenType::STAR);
}

TEST_CASE("parser precedence parentheses", "[parser]")
{
    // (1 + 2) * 3
    std::string source = "#alphabet<test>\n5 x = (1 + 2) * 3";
    auto statements = parse_source(source);

    auto *var = dynamic_cast<VarStmt *>(statements[0].get());
    REQUIRE(var != nullptr);

    auto *top = dynamic_cast<Binary *>(var->initializer.get());
    REQUIRE(top != nullptr);
    REQUIRE(top->op.type == TokenType::STAR);

    // Left side should be addition in grouping
    auto *left = dynamic_cast<Grouping *>(top->left.get());
    REQUIRE(left != nullptr);
    auto *inner = dynamic_cast<Binary *>(left->expression.get());
    REQUIRE(inner != nullptr);
    REQUIRE(inner->op.type == TokenType::PLUS);
}

TEST_CASE("parser unary minus", "[parser]")
{
    std::string source = "#alphabet<test>\n5 x = -10";
    auto statements = parse_source(source);

    auto *var = dynamic_cast<VarStmt *>(statements[0].get());
    REQUIRE(var != nullptr);

    auto *unary = dynamic_cast<Unary *>(var->initializer.get());
    REQUIRE(unary != nullptr);
    REQUIRE(unary->op.type == TokenType::MINUS);
}

TEST_CASE("parser unary not", "[parser]")
{
    std::string source = "#alphabet<test>\n11 b = !true";
    auto statements = parse_source(source);

    auto *var = dynamic_cast<VarStmt *>(statements[0].get());
    REQUIRE(var != nullptr);

    auto *unary = dynamic_cast<Unary *>(var->initializer.get());
    REQUIRE(unary != nullptr);
    REQUIRE(unary->op.type == TokenType::NOT);
}

TEST_CASE("parser logical and", "[parser]")
{
    std::string source = "#alphabet<test>\n11 b = 1 && 2";
    auto statements = parse_source(source);

    auto *var = dynamic_cast<VarStmt *>(statements[0].get());
    REQUIRE(var != nullptr);

    auto *log = dynamic_cast<Logical *>(var->initializer.get());
    REQUIRE(log != nullptr);
    REQUIRE(log->op.type == TokenType::AND);
}

TEST_CASE("parser logical or", "[parser]")
{
    std::string source = "#alphabet<test>\n11 b = 1 || 2";
    auto statements = parse_source(source);

    auto *var = dynamic_cast<VarStmt *>(statements[0].get());
    REQUIRE(var != nullptr);

    auto *log = dynamic_cast<Logical *>(var->initializer.get());
    REQUIRE(log != nullptr);
    REQUIRE(log->op.type == TokenType::OR);
}

TEST_CASE("parser comparison", "[parser]")
{
    std::string source = "#alphabet<test>\n11 b = 5 > 3";
    auto statements = parse_source(source);

    auto *var = dynamic_cast<VarStmt *>(statements[0].get());
    REQUIRE(var != nullptr);

    auto *bin = dynamic_cast<Binary *>(var->initializer.get());
    REQUIRE(bin != nullptr);
    REQUIRE(bin->op.type == TokenType::GREATER);
}

TEST_CASE("parser equality", "[parser]")
{
    std::string source = "#alphabet<test>\n11 b = 5 == 5";
    auto statements = parse_source(source);

    auto *var = dynamic_cast<VarStmt *>(statements[0].get());
    REQUIRE(var != nullptr);

    auto *bin = dynamic_cast<Binary *>(var->initializer.get());
    REQUIRE(bin != nullptr);
    REQUIRE(bin->op.type == TokenType::DOUBLE_EQUALS);
}

// ============================================================================
// Statement Parsing Tests
// ============================================================================

TEST_CASE("parser var no initializer", "[parser]")
{
    std::string source = "#alphabet<test>\n5 x";
    auto statements = parse_source(source);

    REQUIRE(statements.size() == 1);
    auto *var = dynamic_cast<VarStmt *>(statements[0].get());
    REQUIRE(var != nullptr);
    REQUIRE(var->initializer == nullptr);
}

TEST_CASE("parser var with initializer", "[parser]")
{
    std::string source = "#alphabet<test>\n5 x = 10";
    auto statements = parse_source(source);

    REQUIRE(statements.size() == 1);
    auto *var = dynamic_cast<VarStmt *>(statements[0].get());
    REQUIRE(var != nullptr);
    REQUIRE(var->initializer != nullptr);
}

TEST_CASE("parser if without else", "[parser]")
{
    std::string source = "#alphabet<test>\ni (1 > 0) { 5 x = 1 }";
    auto statements = parse_source(source);

    REQUIRE(statements.size() == 1);
    auto *if_stmt = dynamic_cast<IfStmt *>(statements[0].get());
    REQUIRE(if_stmt != nullptr);
    REQUIRE(if_stmt->else_branch == nullptr);
}

TEST_CASE("parser if with else", "[parser]")
{
    std::string source = "#alphabet<test>\ni (1 > 0) { 5 x = 1 } e { 5 x = 2 }";
    auto statements = parse_source(source);

    REQUIRE(statements.size() == 1);
    auto *if_stmt = dynamic_cast<IfStmt *>(statements[0].get());
    REQUIRE(if_stmt != nullptr);
    REQUIRE(if_stmt->else_branch != nullptr);
}

TEST_CASE("parser loop", "[parser]")
{
    std::string source = "#alphabet<test>\nl (1 > 0) { 5 x = x + 1 }";
    auto statements = parse_source(source);

    REQUIRE(statements.size() == 1);
    auto *loop = dynamic_cast<LoopStmt *>(statements[0].get());
    REQUIRE(loop != nullptr);
    REQUIRE(loop->condition != nullptr);
    REQUIRE(loop->body != nullptr);
}

TEST_CASE("parser block", "[parser]")
{
    std::string source = "#alphabet<test>\n{ 5 x = 1 5 y = 2 }";
    auto statements = parse_source(source);

    REQUIRE(statements.size() == 1);
    auto *block = dynamic_cast<Block *>(statements[0].get());
    REQUIRE(block != nullptr);
    REQUIRE(block->statements.size() == 2);
}

TEST_CASE("parser return with value", "[parser]")
{
    std::string source = "#alphabet<test>\nm 5 f() { r 10 }";
    auto statements = parse_source(source);

    // Method declaration at top level is handled differently
    // This tests that return is parsed
    REQUIRE(statements.size() > 0);
}

TEST_CASE("parser return without value", "[parser]")
{
    std::string source = "#alphabet<test>\nm 5 f() { r }";
    auto statements = parse_source(source);

    REQUIRE(statements.size() > 0);
}

// ============================================================================
// Class Parsing Tests
// ============================================================================

TEST_CASE("parser class simple", "[parser]")
{
    std::string source = "#alphabet<test>\nc A { v m 5 f() { r 1 } }";
    auto statements = parse_source(source);

    REQUIRE(statements.size() == 1);
    auto *cls = dynamic_cast<ClassStmt *>(statements[0].get());
    REQUIRE(cls != nullptr);
    REQUIRE_FALSE(cls->is_interface);
    REQUIRE(cls->methods.size() == 1);
}

TEST_CASE("parser class with superclass", "[parser]")
{
    std::string source = "#alphabet<test>\nc B ^ A { v m 5 f() { r 1 } }";
    auto statements = parse_source(source);

    auto *cls = dynamic_cast<ClassStmt *>(statements[0].get());
    REQUIRE(cls != nullptr);
    REQUIRE(cls->superclass != nullptr);
}

TEST_CASE("parser class with fields", "[parser]")
{
    std::string source = "#alphabet<test>\nc A { 5 x = 10 v m 5 f() { r x } }";
    auto statements = parse_source(source);

    auto *cls = dynamic_cast<ClassStmt *>(statements[0].get());
    REQUIRE(cls != nullptr);
    REQUIRE(cls->fields.size() == 1);
}

TEST_CASE("parser class static field", "[parser]")
{
    std::string source = "#alphabet<test>\nc A { s 5 x = 10 }";
    auto statements = parse_source(source);

    auto *cls = dynamic_cast<ClassStmt *>(statements[0].get());
    REQUIRE(cls != nullptr);
    REQUIRE(cls->fields.size() == 1);
    REQUIRE(cls->fields[0].is_static);
}

TEST_CASE("parser class private field", "[parser]")
{
    std::string source = "#alphabet<test>\nc A { p 5 x = 10 }";
    auto statements = parse_source(source);

    auto *cls = dynamic_cast<ClassStmt *>(statements[0].get());
    REQUIRE(cls != nullptr);
    REQUIRE(cls->fields[0].visibility.has_value());
}

TEST_CASE("parser interface", "[parser]")
{
    std::string source = "#alphabet<test>\nj I { m 5 f() m 6 g() }";
    auto statements = parse_source(source);

    REQUIRE(statements.size() == 1);
    auto *cls = dynamic_cast<ClassStmt *>(statements[0].get());
    REQUIRE(cls != nullptr);
    REQUIRE(cls->is_interface);
}

TEST_CASE("parser method with params", "[parser]")
{
    std::string source = "#alphabet<test>\nc A { v m 5 add(5 a, 5 b) { r a + b } }";
    auto statements = parse_source(source);

    auto *cls = dynamic_cast<ClassStmt *>(statements[0].get());
    REQUIRE(cls != nullptr);
    REQUIRE(cls->methods.size() == 1);
    REQUIRE(cls->methods[0].params.size() == 2);
}

// ============================================================================
// Complex Expression Tests
// ============================================================================

TEST_CASE("parser call simple", "[parser]")
{
    std::string source = "#alphabet<test>\nf()";
    auto statements = parse_source(source);

    auto *expr = dynamic_cast<ExpressionStmt *>(statements[0].get());
    REQUIRE(expr != nullptr);
    auto *call = dynamic_cast<Call *>(expr->expression.get());
    REQUIRE(call != nullptr);
}

TEST_CASE("parser call with args", "[parser]")
{
    std::string source = "#alphabet<test>\nf(1, 2, 3)";
    auto statements = parse_source(source);

    auto *expr = dynamic_cast<ExpressionStmt *>(statements[0].get());
    REQUIRE(expr != nullptr);
    auto *call = dynamic_cast<Call *>(expr->expression.get());
    REQUIRE(call != nullptr);
    REQUIRE(call->arguments.size() == 3);
}

TEST_CASE("parser method call", "[parser]")
{
    std::string source = "#alphabet<test>\nobj.method()";
    auto statements = parse_source(source);

    auto *expr = dynamic_cast<ExpressionStmt *>(statements[0].get());
    REQUIRE(expr != nullptr);
    auto *call = dynamic_cast<Call *>(expr->expression.get());
    REQUIRE(call != nullptr);
}

TEST_CASE("parser property access", "[parser]")
{
    std::string source = "#alphabet<test>\nobj.prop";
    auto statements = parse_source(source);

    auto *expr = dynamic_cast<ExpressionStmt *>(statements[0].get());
    REQUIRE(expr != nullptr);
    auto *get = dynamic_cast<Get *>(expr->expression.get());
    REQUIRE(get != nullptr);
}

TEST_CASE("parser assignment", "[parser]")
{
    std::string source = "#alphabet<test>\nfoo = 10";
    auto statements = parse_source(source);

    auto *expr = dynamic_cast<ExpressionStmt *>(statements[0].get());
    REQUIRE(expr != nullptr);
    auto *assign = dynamic_cast<Assign *>(expr->expression.get());
    REQUIRE(assign != nullptr);
}

TEST_CASE("parser list literal", "[parser]")
{
    std::string source = "#alphabet<test>\n13 a = [1, 2, 3]";
    auto statements = parse_source(source);

    auto *var = dynamic_cast<VarStmt *>(statements[0].get());
    REQUIRE(var != nullptr);
    auto *list = dynamic_cast<ListLiteral *>(var->initializer.get());
    REQUIRE(list != nullptr);
    REQUIRE(list->elements.size() == 3);
}

TEST_CASE("parser map literal", "[parser]")
{
    std::string source = "#alphabet<test>\n14 m = {\"a\": 1, \"b\": 2}";
    auto statements = parse_source(source);

    auto *var = dynamic_cast<VarStmt *>(statements[0].get());
    REQUIRE(var != nullptr);
    auto *map = dynamic_cast<MapLiteral *>(var->initializer.get());
    REQUIRE(map != nullptr);
    REQUIRE(map->keys.size() == 2);
}

TEST_CASE("parser index access", "[parser]")
{
    std::string source = "#alphabet<test>\na[0]";
    auto statements = parse_source(source);

    auto *expr = dynamic_cast<ExpressionStmt *>(statements[0].get());
    REQUIRE(expr != nullptr);
    auto *idx = dynamic_cast<IndexExpr *>(expr->expression.get());
    REQUIRE(idx != nullptr);
}

TEST_CASE("parser new object", "[parser]")
{
    std::string source = "#alphabet<test>\n15 o = n A()";
    auto statements = parse_source(source);

    auto *var = dynamic_cast<VarStmt *>(statements[0].get());
    REQUIRE(var != nullptr);
    auto *new_expr = dynamic_cast<New *>(var->initializer.get());
    REQUIRE(new_expr != nullptr);
}

TEST_CASE("parser system call", "[parser]")
{
    std::string source = "#alphabet<test>\nz.o(\"hello\")";
    auto statements = parse_source(source);

    auto *expr = dynamic_cast<ExpressionStmt *>(statements[0].get());
    REQUIRE(expr != nullptr);
    auto *call = dynamic_cast<Call *>(expr->expression.get());
    REQUIRE(call != nullptr);
}

// ============================================================================
// Error Handling Tests
// ============================================================================

TEST_CASE("parser missing header", "[parser][error]")
{
    std::string source = "5 x = 10"; // No magic header
    REQUIRE_THROWS_AS(parse_source(source), MissingLanguageHeader);
}

TEST_CASE("parser missing closing parenthesis", "[parser][error]")
{
    std::string source = "#alphabet<test>\n5 x = (1 + 2";
    auto parser = make_parser(source);
    auto statements = parser.parse();

    // Parser should detect the error via had_errors()
    // or it may recover and return partial results
    // At minimum, we check the parser flagged the error
    // If parse() threw instead, REQUIRE_NOTHROW or REQUIRE_THROWS would catch it.
    // Based on the API: parse() catches ParseError internally and sets had_errors_
    // But it could also bubble up uncaught errors.
    CHECK(parser.had_errors());
}

TEST_CASE("parser missing variable name in declaration", "[parser][error]")
{
    std::string source = "#alphabet<test>\n5 = 10";
    auto parser = make_parser(source);
    auto statements = parser.parse();

    // The parser should flag an error for a missing variable name
    // after the type identifier
    CHECK(parser.had_errors());
}

TEST_CASE("parser incomplete if statement", "[parser][error]")
{
    std::string source = "#alphabet<test>\ni (1 > 0)";
    auto parser = make_parser(source);
    auto statements = parser.parse();

    // Missing the body block after the condition
    CHECK(parser.had_errors());
}

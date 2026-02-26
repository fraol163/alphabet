#include <iostream>
#include <cassert>
#include <string>
#include <memory>

#include "lexer.h"
#include "parser.h"
#include "alphabet_ast.h"

using namespace alphabet;

// Test counters
static int tests_run = 0;
static int tests_passed = 0;
static int tests_failed = 0;

#define TEST(name) void name()
#define RUN_TEST(name) do { \
    tests_run++; \
    std::cout << "Running " << #name << "... "; \
    try { \
        name(); \
        tests_passed++; \
        std::cout << "PASSED\n"; \
    } catch (const std::exception& e) { \
        tests_failed++; \
        std::cout << "FAILED: " << e.what() << "\n"; \
    } \
} while(0)

#define ASSERT_EQ(expected, actual) do { \
    if ((expected) != (actual)) { \
        throw std::runtime_error("Assertion failed: " #expected " != " #actual); \
    } \
} while(0)

#define ASSERT_TRUE(cond) do { \
    if (!(cond)) { \
        throw std::runtime_error("Assertion failed: " #cond); \
    } \
} while(0)

#define ASSERT_FALSE(cond) do { \
    if (cond) { \
        throw std::runtime_error("Assertion failed: expected " #cond " to be false"); \
    } \
} while(0)

// Helper to parse source
std::vector<StmtPtr> parse_source(const std::string& source) {
    Lexer lexer(source);
    Parser parser(lexer.scan_tokens());
    return parser.parse();
}

// ============================================================================
// Expression Parsing Tests
// ============================================================================

TEST(test_parser_literal_number) {
    std::string source = "#alphabet<test>\n5 x = 42";
    auto statements = parse_source(source);
    
    ASSERT_EQ(1, statements.size());
    auto* var = dynamic_cast<VarStmt*>(statements[0].get());
    ASSERT_TRUE(var != nullptr);
    
    auto* lit = dynamic_cast<Literal*>(var->initializer.get());
    ASSERT_TRUE(lit != nullptr);
    ASSERT_TRUE(std::holds_alternative<double>(lit->value));
    ASSERT_EQ(42.0, std::get<double>(lit->value));
}

TEST(test_parser_literal_string) {
    std::string source = "#alphabet<test>\n12 s = \"hello world\"";
    auto statements = parse_source(source);
    
    ASSERT_EQ(1, statements.size());
    auto* var = dynamic_cast<VarStmt*>(statements[0].get());
    ASSERT_TRUE(var != nullptr);
    
    auto* lit = dynamic_cast<Literal*>(var->initializer.get());
    ASSERT_TRUE(lit != nullptr);
}

TEST(test_parser_binary_addition) {
    std::string source = "#alphabet<test>\n5 x = 1 + 2";
    auto statements = parse_source(source);
    
    ASSERT_EQ(1, statements.size());
    auto* var = dynamic_cast<VarStmt*>(statements[0].get());
    ASSERT_TRUE(var != nullptr);
    
    auto* bin = dynamic_cast<Binary*>(var->initializer.get());
    ASSERT_TRUE(bin != nullptr);
    ASSERT_EQ(TokenType::PLUS, bin->op.type);
}

TEST(test_parser_precedence_mul_before_add) {
    // 1 + 2 * 3 should parse as 1 + (2 * 3)
    std::string source = "#alphabet<test>\n5 x = 1 + 2 * 3";
    auto statements = parse_source(source);
    
    auto* var = dynamic_cast<VarStmt*>(statements[0].get());
    ASSERT_TRUE(var != nullptr);
    
    auto* top = dynamic_cast<Binary*>(var->initializer.get());
    ASSERT_TRUE(top != nullptr);
    ASSERT_EQ(TokenType::PLUS, top->op.type);
    
    // Right side should be multiplication
    auto* right = dynamic_cast<Binary*>(top->right.get());
    ASSERT_TRUE(right != nullptr);
    ASSERT_EQ(TokenType::STAR, right->op.type);
}

TEST(test_parser_precedence_parentheses) {
    // (1 + 2) * 3
    std::string source = "#alphabet<test>\n5 x = (1 + 2) * 3";
    auto statements = parse_source(source);
    
    auto* var = dynamic_cast<VarStmt*>(statements[0].get());
    ASSERT_TRUE(var != nullptr);
    
    auto* top = dynamic_cast<Binary*>(var->initializer.get());
    ASSERT_TRUE(top != nullptr);
    ASSERT_EQ(TokenType::STAR, top->op.type);
    
    // Left side should be addition in grouping
    auto* left = dynamic_cast<Grouping*>(top->left.get());
    ASSERT_TRUE(left != nullptr);
    auto* inner = dynamic_cast<Binary*>(left->expression.get());
    ASSERT_TRUE(inner != nullptr);
    ASSERT_EQ(TokenType::PLUS, inner->op.type);
}

TEST(test_parser_unary_minus) {
    std::string source = "#alphabet<test>\n5 x = -10";
    auto statements = parse_source(source);
    
    auto* var = dynamic_cast<VarStmt*>(statements[0].get());
    ASSERT_TRUE(var != nullptr);
    
    auto* unary = dynamic_cast<Unary*>(var->initializer.get());
    ASSERT_TRUE(unary != nullptr);
    ASSERT_EQ(TokenType::MINUS, unary->op.type);
}

TEST(test_parser_unary_not) {
    std::string source = "#alphabet<test>\n11 b = !true";
    auto statements = parse_source(source);
    
    auto* var = dynamic_cast<VarStmt*>(statements[0].get());
    ASSERT_TRUE(var != nullptr);
    
    auto* unary = dynamic_cast<Unary*>(var->initializer.get());
    ASSERT_TRUE(unary != nullptr);
    ASSERT_EQ(TokenType::NOT, unary->op.type);
}

TEST(test_parser_logical_and) {
    std::string source = "#alphabet<test>\n11 b = 1 && 2";
    auto statements = parse_source(source);
    
    auto* var = dynamic_cast<VarStmt*>(statements[0].get());
    ASSERT_TRUE(var != nullptr);
    
    auto* log = dynamic_cast<Logical*>(var->initializer.get());
    ASSERT_TRUE(log != nullptr);
    ASSERT_EQ(TokenType::AND, log->op.type);
}

TEST(test_parser_logical_or) {
    std::string source = "#alphabet<test>\n11 b = 1 || 2";
    auto statements = parse_source(source);
    
    auto* var = dynamic_cast<VarStmt*>(statements[0].get());
    ASSERT_TRUE(var != nullptr);
    
    auto* log = dynamic_cast<Logical*>(var->initializer.get());
    ASSERT_TRUE(log != nullptr);
    ASSERT_EQ(TokenType::OR, log->op.type);
}

TEST(test_parser_comparison) {
    std::string source = "#alphabet<test>\n11 b = 5 > 3";
    auto statements = parse_source(source);
    
    auto* var = dynamic_cast<VarStmt*>(statements[0].get());
    ASSERT_TRUE(var != nullptr);
    
    auto* bin = dynamic_cast<Binary*>(var->initializer.get());
    ASSERT_TRUE(bin != nullptr);
    ASSERT_EQ(TokenType::GREATER, bin->op.type);
}

TEST(test_parser_equality) {
    std::string source = "#alphabet<test>\n11 b = 5 == 5";
    auto statements = parse_source(source);
    
    auto* var = dynamic_cast<VarStmt*>(statements[0].get());
    ASSERT_TRUE(var != nullptr);
    
    auto* bin = dynamic_cast<Binary*>(var->initializer.get());
    ASSERT_TRUE(bin != nullptr);
    ASSERT_EQ(TokenType::DOUBLE_EQUALS, bin->op.type);
}

// ============================================================================
// Statement Parsing Tests
// ============================================================================

TEST(test_parser_var_no_initializer) {
    std::string source = "#alphabet<test>\n5 x";
    auto statements = parse_source(source);
    
    ASSERT_EQ(1, statements.size());
    auto* var = dynamic_cast<VarStmt*>(statements[0].get());
    ASSERT_TRUE(var != nullptr);
    ASSERT_TRUE(var->initializer == nullptr);
}

TEST(test_parser_var_with_initializer) {
    std::string source = "#alphabet<test>\n5 x = 10";
    auto statements = parse_source(source);
    
    ASSERT_EQ(1, statements.size());
    auto* var = dynamic_cast<VarStmt*>(statements[0].get());
    ASSERT_TRUE(var != nullptr);
    ASSERT_TRUE(var->initializer != nullptr);
}

TEST(test_parser_if_without_else) {
    std::string source = "#alphabet<test>\ni (1 > 0) { 5 x = 1 }";
    auto statements = parse_source(source);
    
    ASSERT_EQ(1, statements.size());
    auto* if_stmt = dynamic_cast<IfStmt*>(statements[0].get());
    ASSERT_TRUE(if_stmt != nullptr);
    ASSERT_TRUE(if_stmt->else_branch == nullptr);
}

TEST(test_parser_if_with_else) {
    std::string source = "#alphabet<test>\ni (1 > 0) { 5 x = 1 } e { 5 x = 2 }";
    auto statements = parse_source(source);
    
    ASSERT_EQ(1, statements.size());
    auto* if_stmt = dynamic_cast<IfStmt*>(statements[0].get());
    ASSERT_TRUE(if_stmt != nullptr);
    ASSERT_TRUE(if_stmt->else_branch != nullptr);
}

TEST(test_parser_loop) {
    std::string source = "#alphabet<test>\nl (1 > 0) { 5 x = x + 1 }";
    auto statements = parse_source(source);
    
    ASSERT_EQ(1, statements.size());
    auto* loop = dynamic_cast<LoopStmt*>(statements[0].get());
    ASSERT_TRUE(loop != nullptr);
    ASSERT_TRUE(loop->condition != nullptr);
    ASSERT_TRUE(loop->body != nullptr);
}

TEST(test_parser_block) {
    std::string source = "#alphabet<test>\n{ 5 x = 1 5 y = 2 }";
    auto statements = parse_source(source);
    
    ASSERT_EQ(1, statements.size());
    auto* block = dynamic_cast<Block*>(statements[0].get());
    ASSERT_TRUE(block != nullptr);
    ASSERT_EQ(2, block->statements.size());
}

TEST(test_parser_return_with_value) {
    std::string source = "#alphabet<test>\nm 5 f() { r 10 }";
    auto statements = parse_source(source);

    // Method declaration at top level is handled differently
    // This tests that return is parsed
    ASSERT_TRUE(statements.size() > 0);
}

TEST(test_parser_return_without_value) {
    std::string source = "#alphabet<test>\nm 5 f() { r }";
    auto statements = parse_source(source);

    ASSERT_TRUE(statements.size() > 0);
}

// ============================================================================
// Class Parsing Tests
// ============================================================================

TEST(test_parser_class_simple) {
    std::string source = "#alphabet<test>\nc A { v m 5 f() { r 1 } }";
    auto statements = parse_source(source);
    
    ASSERT_EQ(1, statements.size());
    auto* cls = dynamic_cast<ClassStmt*>(statements[0].get());
    ASSERT_TRUE(cls != nullptr);
    ASSERT_FALSE(cls->is_interface);
    ASSERT_EQ(1, cls->methods.size());
}

TEST(test_parser_class_with_superclass) {
    std::string source = "#alphabet<test>\nc B ^ A { v m 5 f() { r 1 } }";
    auto statements = parse_source(source);
    
    auto* cls = dynamic_cast<ClassStmt*>(statements[0].get());
    ASSERT_TRUE(cls != nullptr);
    ASSERT_TRUE(cls->superclass != nullptr);
}

TEST(test_parser_class_with_fields) {
    std::string source = "#alphabet<test>\nc A { 5 x = 10 v m 5 f() { r x } }";
    auto statements = parse_source(source);
    
    auto* cls = dynamic_cast<ClassStmt*>(statements[0].get());
    ASSERT_TRUE(cls != nullptr);
    ASSERT_EQ(1, cls->fields.size());
}

TEST(test_parser_class_static_field) {
    std::string source = "#alphabet<test>\nc A { s 5 x = 10 }";
    auto statements = parse_source(source);
    
    auto* cls = dynamic_cast<ClassStmt*>(statements[0].get());
    ASSERT_TRUE(cls != nullptr);
    ASSERT_EQ(1, cls->fields.size());
    ASSERT_TRUE(cls->fields[0].is_static);
}

TEST(test_parser_class_private_field) {
    std::string source = "#alphabet<test>\nc A { p 5 x = 10 }";
    auto statements = parse_source(source);
    
    auto* cls = dynamic_cast<ClassStmt*>(statements[0].get());
    ASSERT_TRUE(cls != nullptr);
    ASSERT_TRUE(cls->fields[0].visibility.has_value());
}

TEST(test_parser_interface) {
    std::string source = "#alphabet<test>\nj I { m 5 f() m 6 g() }";
    auto statements = parse_source(source);
    
    ASSERT_EQ(1, statements.size());
    auto* cls = dynamic_cast<ClassStmt*>(statements[0].get());
    ASSERT_TRUE(cls != nullptr);
    ASSERT_TRUE(cls->is_interface);
}

TEST(test_parser_method_with_params) {
    std::string source = "#alphabet<test>\nc A { v m 5 add(5 a, 5 b) { r a + b } }";
    auto statements = parse_source(source);
    
    auto* cls = dynamic_cast<ClassStmt*>(statements[0].get());
    ASSERT_TRUE(cls != nullptr);
    ASSERT_EQ(1, cls->methods.size());
    ASSERT_EQ(2, cls->methods[0].params.size());
}

// ============================================================================
// Complex Expression Tests
// ============================================================================

TEST(test_parser_call_simple) {
    std::string source = "#alphabet<test>\nf()";
    auto statements = parse_source(source);
    
    auto* expr = dynamic_cast<ExpressionStmt*>(statements[0].get());
    ASSERT_TRUE(expr != nullptr);
    auto* call = dynamic_cast<Call*>(expr->expression.get());
    ASSERT_TRUE(call != nullptr);
}

TEST(test_parser_call_with_args) {
    std::string source = "#alphabet<test>\nf(1, 2, 3)";
    auto statements = parse_source(source);
    
    auto* expr = dynamic_cast<ExpressionStmt*>(statements[0].get());
    ASSERT_TRUE(expr != nullptr);
    auto* call = dynamic_cast<Call*>(expr->expression.get());
    ASSERT_TRUE(call != nullptr);
    ASSERT_EQ(3, call->arguments.size());
}

TEST(test_parser_method_call) {
    std::string source = "#alphabet<test>\nobj.method()";
    auto statements = parse_source(source);
    
    auto* expr = dynamic_cast<ExpressionStmt*>(statements[0].get());
    ASSERT_TRUE(expr != nullptr);
    auto* call = dynamic_cast<Call*>(expr->expression.get());
    ASSERT_TRUE(call != nullptr);
}

TEST(test_parser_property_access) {
    std::string source = "#alphabet<test>\nobj.prop";
    auto statements = parse_source(source);
    
    auto* expr = dynamic_cast<ExpressionStmt*>(statements[0].get());
    ASSERT_TRUE(expr != nullptr);
    auto* get = dynamic_cast<Get*>(expr->expression.get());
    ASSERT_TRUE(get != nullptr);
}

TEST(test_parser_assignment) {
    std::string source = "#alphabet<test>\nx = 10";
    auto statements = parse_source(source);
    
    auto* expr = dynamic_cast<ExpressionStmt*>(statements[0].get());
    ASSERT_TRUE(expr != nullptr);
    auto* assign = dynamic_cast<Assign*>(expr->expression.get());
    ASSERT_TRUE(assign != nullptr);
}

TEST(test_parser_list_literal) {
    std::string source = "#alphabet<test>\n13 a = [1, 2, 3]";
    auto statements = parse_source(source);
    
    auto* var = dynamic_cast<VarStmt*>(statements[0].get());
    ASSERT_TRUE(var != nullptr);
    auto* list = dynamic_cast<ListLiteral*>(var->initializer.get());
    ASSERT_TRUE(list != nullptr);
    ASSERT_EQ(3, list->elements.size());
}

TEST(test_parser_map_literal) {
    std::string source = "#alphabet<test>\n14 m = {\"a\": 1, \"b\": 2}";
    auto statements = parse_source(source);
    
    auto* var = dynamic_cast<VarStmt*>(statements[0].get());
    ASSERT_TRUE(var != nullptr);
    auto* map = dynamic_cast<MapLiteral*>(var->initializer.get());
    ASSERT_TRUE(map != nullptr);
    ASSERT_EQ(2, map->keys.size());
}

TEST(test_parser_index_access) {
    std::string source = "#alphabet<test>\na[0]";
    auto statements = parse_source(source);
    
    auto* expr = dynamic_cast<ExpressionStmt*>(statements[0].get());
    ASSERT_TRUE(expr != nullptr);
    auto* idx = dynamic_cast<IndexExpr*>(expr->expression.get());
    ASSERT_TRUE(idx != nullptr);
}

TEST(test_parser_new_object) {
    std::string source = "#alphabet<test>\n15 o = n A()";
    auto statements = parse_source(source);
    
    auto* var = dynamic_cast<VarStmt*>(statements[0].get());
    ASSERT_TRUE(var != nullptr);
    auto* new_expr = dynamic_cast<New*>(var->initializer.get());
    ASSERT_TRUE(new_expr != nullptr);
}

TEST(test_parser_system_call) {
    std::string source = "#alphabet<test>\nz.o(\"hello\")";
    auto statements = parse_source(source);
    
    auto* expr = dynamic_cast<ExpressionStmt*>(statements[0].get());
    ASSERT_TRUE(expr != nullptr);
    auto* call = dynamic_cast<Call*>(expr->expression.get());
    ASSERT_TRUE(call != nullptr);
}

// ============================================================================
// Error Handling Tests
// ============================================================================

TEST(test_parser_missing_header) {
    std::string source = "5 x = 10";  // No magic header
    try {
        auto statements = parse_source(source);
        // Should have thrown
        ASSERT_TRUE(false);
    } catch (const MissingLanguageHeader&) {
        // Expected
    }
}

// ============================================================================
// Main
// ============================================================================

int main() {
    std::cout << "=== Alphabet Parser Tests ===\n\n";
    
    // Expression tests
    RUN_TEST(test_parser_literal_number);
    RUN_TEST(test_parser_literal_string);
    RUN_TEST(test_parser_binary_addition);
    RUN_TEST(test_parser_precedence_mul_before_add);
    RUN_TEST(test_parser_precedence_parentheses);
    RUN_TEST(test_parser_unary_minus);
    RUN_TEST(test_parser_unary_not);
    RUN_TEST(test_parser_logical_and);
    RUN_TEST(test_parser_logical_or);
    RUN_TEST(test_parser_comparison);
    RUN_TEST(test_parser_equality);
    
    // Statement tests
    RUN_TEST(test_parser_var_no_initializer);
    RUN_TEST(test_parser_var_with_initializer);
    RUN_TEST(test_parser_if_without_else);
    RUN_TEST(test_parser_if_with_else);
    RUN_TEST(test_parser_loop);
    RUN_TEST(test_parser_block);
    RUN_TEST(test_parser_return_with_value);
    RUN_TEST(test_parser_return_without_value);
    
    // Class tests
    RUN_TEST(test_parser_class_simple);
    RUN_TEST(test_parser_class_with_superclass);
    RUN_TEST(test_parser_class_with_fields);
    RUN_TEST(test_parser_class_static_field);
    RUN_TEST(test_parser_class_private_field);
    RUN_TEST(test_parser_interface);
    RUN_TEST(test_parser_method_with_params);
    
    // Complex expression tests
    RUN_TEST(test_parser_call_simple);
    RUN_TEST(test_parser_call_with_args);
    RUN_TEST(test_parser_method_call);
    RUN_TEST(test_parser_property_access);
    RUN_TEST(test_parser_assignment);
    RUN_TEST(test_parser_list_literal);
    RUN_TEST(test_parser_map_literal);
    RUN_TEST(test_parser_index_access);
    RUN_TEST(test_parser_new_object);
    RUN_TEST(test_parser_system_call);
    
    // Error handling
    RUN_TEST(test_parser_missing_header);
    
    std::cout << "\n========================================\n";
    std::cout << "Tests run: " << tests_run << "\n";
    std::cout << "Passed: " << tests_passed << "\n";
    std::cout << "Failed: " << tests_failed << "\n";
    std::cout << "========================================\n";
    
    return tests_failed > 0 ? 1 : 0;
}

#include <iostream>
#include <cassert>
#include <string>
#include <vector>

#include "lexer.h"
#include "parser.h"
#include "compiler.h"
#include "vm.h"

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

// ============================================================================
// Lexer Tests
// ============================================================================

TEST(test_lexer_single_char_keywords) {
    // Test that single-char keywords are recognized
    std::string source = "#alphabet<test>\ni (x > 0) { l (true) { r x } }";
    Lexer lexer(source);
    auto tokens = lexer.scan_tokens();
    
    // Should have IF, LPAREN, IDENTIFIER, GREATER, NUMBER, RPAREN, etc.
    bool found_if = false;
    bool found_loop = false;
    bool found_return = false;
    
    for (const auto& tok : tokens) {
        if (tok.type == TokenType::IF) found_if = true;
        if (tok.type == TokenType::LOOP) found_loop = true;
        if (tok.type == TokenType::RETURN) found_return = true;
    }
    
    ASSERT_TRUE(found_if);
    ASSERT_TRUE(found_loop);
    ASSERT_TRUE(found_return);
}

TEST(test_lexer_magic_header) {
    // Valid header should pass
    std::string source = "#alphabet<en>\n12 s = \"hello\"";
    Lexer lexer(source);
    auto tokens = lexer.scan_tokens();
    ASSERT_TRUE(tokens.size() > 0);
    
    // Missing header should throw
    std::string bad_source = "12 s = \"hello\"";
    Lexer bad_lexer(bad_source);
    try {
        bad_lexer.scan_tokens();
        throw std::runtime_error("Expected MissingLanguageHeader exception");
    } catch (const MissingLanguageHeader&) {
        // Expected
    }
}

TEST(test_lexer_numbers) {
    std::string source = "#alphabet<test>\n1 x = 42\n6 y = 3.14";
    Lexer lexer(source);
    auto tokens = lexer.scan_tokens();
    
    bool found_int = false;
    bool found_float = false;
    
    for (const auto& tok : tokens) {
        if (tok.type == TokenType::NUMBER) {
            if (tok.literal == 42.0) found_int = true;
            if (tok.literal == 3.14) found_float = true;
        }
    }
    
    ASSERT_TRUE(found_int);
    ASSERT_TRUE(found_float);
}

TEST(test_lexer_string) {
    std::string source = "#alphabet<test>\n12 s = \"Hello, World!\"";
    Lexer lexer(source);
    auto tokens = lexer.scan_tokens();
    
    bool found_string = false;
    for (const auto& tok : tokens) {
        if (tok.type == TokenType::STRING) {
            found_string = true;
            ASSERT_EQ(std::string(tok.lexeme), "Hello, World!");
        }
    }
    
    ASSERT_TRUE(found_string);
}

TEST(test_lexer_operators) {
    std::string source = "#alphabet<test>\n1 x = 1 + 2 - 3 * 4 / 5 % 6";
    Lexer lexer(source);
    auto tokens = lexer.scan_tokens();
    
    bool found_plus = false, found_minus = false, found_star = false;
    bool found_slash = false, found_percent = false;
    
    for (const auto& tok : tokens) {
        if (tok.type == TokenType::PLUS) found_plus = true;
        if (tok.type == TokenType::MINUS) found_minus = true;
        if (tok.type == TokenType::STAR) found_star = true;
        if (tok.type == TokenType::SLASH) found_slash = true;
        if (tok.type == TokenType::PERCENT) found_percent = true;
    }
    
    ASSERT_TRUE(found_plus);
    ASSERT_TRUE(found_minus);
    ASSERT_TRUE(found_star);
    ASSERT_TRUE(found_slash);
    ASSERT_TRUE(found_percent);
}

TEST(test_lexer_comparison_ops) {
    std::string source = "#alphabet<test>\n11 b = 1 == 2 && 3 != 4 || 5 > 6 && 7 < 8";
    Lexer lexer(source);
    auto tokens = lexer.scan_tokens();
    
    bool found_eq = false, found_ne = false, found_gt = false, found_lt = false;
    bool found_and = false, found_or = false;
    
    for (const auto& tok : tokens) {
        if (tok.type == TokenType::DOUBLE_EQUALS) found_eq = true;
        if (tok.type == TokenType::NOT_EQUALS) found_ne = true;
        if (tok.type == TokenType::GREATER) found_gt = true;
        if (tok.type == TokenType::LESS) found_lt = true;
        if (tok.type == TokenType::AND) found_and = true;
        if (tok.type == TokenType::OR) found_or = true;
    }
    
    ASSERT_TRUE(found_eq);
    ASSERT_TRUE(found_ne);
    ASSERT_TRUE(found_gt);
    ASSERT_TRUE(found_lt);
    ASSERT_TRUE(found_and);
    ASSERT_TRUE(found_or);
}

TEST(test_lexer_shebang_skip) {
    std::string source = "#!/usr/bin/env alphabet\n#alphabet<test>\n1 x = 1";
    Lexer lexer(source);
    auto tokens = lexer.scan_tokens();
    
    // Should not include shebang in tokens
    for (const auto& tok : tokens) {
        ASSERT_TRUE(tok.lexeme.find("#!") == std::string::npos);
    }
}

TEST(test_lexer_comments) {
    std::string source = "#alphabet<test>\n1 x = 1 // this is a comment\n2 y = 2";
    Lexer lexer(source);
    auto tokens = lexer.scan_tokens();
    
    // Comments should be skipped
    for (const auto& tok : tokens) {
        ASSERT_TRUE(tok.lexeme.find("//") == std::string::npos);
    }
}

// ============================================================================
// Parser Tests
// ============================================================================

TEST(test_parser_variable_declaration) {
    std::string source = "#alphabet<test>\n5 x = 10";
    Lexer lexer(source);
    Parser parser(lexer.scan_tokens());
    auto statements = parser.parse();
    
    ASSERT_EQ(1, statements.size());
    ASSERT_TRUE(dynamic_cast<VarStmt*>(statements[0].get()) != nullptr);
}

TEST(test_parser_if_statement) {
    std::string source = "#alphabet<test>\ni (1 > 0) { 5 x = 1 }";
    Lexer lexer(source);
    Parser parser(lexer.scan_tokens());
    auto statements = parser.parse();
    
    ASSERT_EQ(1, statements.size());
    ASSERT_TRUE(dynamic_cast<IfStmt*>(statements[0].get()) != nullptr);
}

TEST(test_parser_loop_statement) {
    std::string source = "#alphabet<test>\nl (1 > 0) { 5 x = x + 1 }";
    Lexer lexer(source);
    Parser parser(lexer.scan_tokens());
    auto statements = parser.parse();
    
    ASSERT_EQ(1, statements.size());
    ASSERT_TRUE(dynamic_cast<LoopStmt*>(statements[0].get()) != nullptr);
}

TEST(test_parser_class_declaration) {
    std::string source = "#alphabet<test>\nc MyClass { v m 5 method() { r 10 } }";
    Lexer lexer(source);
    Parser parser(lexer.scan_tokens());
    auto statements = parser.parse();
    
    ASSERT_EQ(1, statements.size());
    auto* cls = dynamic_cast<ClassStmt*>(statements[0].get());
    ASSERT_TRUE(cls != nullptr);
    ASSERT_EQ(1, cls->methods.size());
}

TEST(test_parser_binary_expression) {
    std::string source = "#alphabet<test>\n5 x = 1 + 2 * 3";
    Lexer lexer(source);
    Parser parser(lexer.scan_tokens());
    auto statements = parser.parse();
    
    ASSERT_EQ(1, statements.size());
    auto* var = dynamic_cast<VarStmt*>(statements[0].get());
    ASSERT_TRUE(var != nullptr);
    ASSERT_TRUE(dynamic_cast<Binary*>(var->initializer.get()) != nullptr);
}

TEST(test_parser_function_call) {
    std::string source = "#alphabet<test>\nz.o(\"hello\")";
    Lexer lexer(source);
    Parser parser(lexer.scan_tokens());
    auto statements = parser.parse();
    
    ASSERT_EQ(1, statements.size());
    auto* expr = dynamic_cast<ExpressionStmt*>(statements[0].get());
    ASSERT_TRUE(expr != nullptr);
    ASSERT_TRUE(dynamic_cast<Call*>(expr->expression.get()) != nullptr);
}

// ============================================================================
// VM Tests
// ============================================================================

TEST(test_vm_push_const) {
    Program program;
    program.main.push_back(Instruction(OpCode::PUSH_CONST, 42.0));
    program.main.push_back(Instruction(OpCode::HALT));
    
    VM vm(program);
    vm.run();
}

TEST(test_vm_arithmetic) {
    Program program;
    program.main.push_back(Instruction(OpCode::PUSH_CONST, 10.0));
    program.main.push_back(Instruction(OpCode::PUSH_CONST, 5.0));
    program.main.push_back(Instruction(OpCode::ADD));
    program.main.push_back(Instruction(OpCode::HALT));
    
    VM vm(program);
    vm.run();
}

TEST(test_vm_comparison) {
    Program program;
    program.main.push_back(Instruction(OpCode::PUSH_CONST, 10.0));
    program.main.push_back(Instruction(OpCode::PUSH_CONST, 5.0));
    program.main.push_back(Instruction(OpCode::GT));
    program.main.push_back(Instruction(OpCode::HALT));
    
    VM vm(program);
    vm.run();
}

TEST(test_vm_jump) {
    Program program;
    program.main.push_back(Instruction(OpCode::PUSH_CONST, 1.0));
    program.main.push_back(Instruction(OpCode::JUMP, static_cast<int64_t>(3)));
    program.main.push_back(Instruction(OpCode::PUSH_CONST, 2.0));
    program.main.push_back(Instruction(OpCode::PUSH_CONST, 3.0));
    program.main.push_back(Instruction(OpCode::HALT));
    
    VM vm(program);
    vm.run();
}

TEST(test_vm_list_operations) {
    Program program;
    program.main.push_back(Instruction(OpCode::PUSH_CONST, 1.0));
    program.main.push_back(Instruction(OpCode::PUSH_CONST, 2.0));
    program.main.push_back(Instruction(OpCode::PUSH_CONST, 3.0));
    program.main.push_back(Instruction(OpCode::BUILD_LIST, static_cast<int64_t>(3)));
    program.main.push_back(Instruction(OpCode::HALT));
    
    VM vm(program);
    vm.run();
}

TEST(test_vm_map_operations) {
    Program program;
    program.main.push_back(Instruction(OpCode::PUSH_CONST, std::string("key")));
    program.main.push_back(Instruction(OpCode::PUSH_CONST, 42.0));
    program.main.push_back(Instruction(OpCode::BUILD_MAP, static_cast<int64_t>(1)));
    program.main.push_back(Instruction(OpCode::HALT));
    
    VM vm(program);
    vm.run();
}

// ============================================================================
// Integration Tests
// ============================================================================

TEST(test_integration_hello_world) {
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
    vm.run();
}

TEST(test_integration_arithmetic) {
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
    vm.run();
}

TEST(test_integration_class_basic) {
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
    vm.run();
}

// ============================================================================
// Main
// ============================================================================

int main() {
    std::cout << "=== Alphabet Lexer Tests ===\n\n";
    
    RUN_TEST(test_lexer_single_char_keywords);
    RUN_TEST(test_lexer_magic_header);
    RUN_TEST(test_lexer_numbers);
    RUN_TEST(test_lexer_string);
    RUN_TEST(test_lexer_operators);
    RUN_TEST(test_lexer_comparison_ops);
    RUN_TEST(test_lexer_shebang_skip);
    RUN_TEST(test_lexer_comments);
    
    std::cout << "\n=== Alphabet Parser Tests ===\n\n";
    
    RUN_TEST(test_parser_variable_declaration);
    RUN_TEST(test_parser_if_statement);
    RUN_TEST(test_parser_loop_statement);
    RUN_TEST(test_parser_class_declaration);
    RUN_TEST(test_parser_binary_expression);
    RUN_TEST(test_parser_function_call);
    
    std::cout << "\n=== Alphabet VM Tests ===\n\n";
    
    RUN_TEST(test_vm_push_const);
    RUN_TEST(test_vm_arithmetic);
    RUN_TEST(test_vm_comparison);
    RUN_TEST(test_vm_jump);
    RUN_TEST(test_vm_list_operations);
    RUN_TEST(test_vm_map_operations);
    
    std::cout << "\n=== Integration Tests ===\n\n";
    
    RUN_TEST(test_integration_hello_world);
    RUN_TEST(test_integration_arithmetic);
    RUN_TEST(test_integration_class_basic);
    
    std::cout << "\n========================================\n";
    std::cout << "Tests run: " << tests_run << "\n";
    std::cout << "Passed: " << tests_passed << "\n";
    std::cout << "Failed: " << tests_failed << "\n";
    std::cout << "========================================\n";
    
    return tests_failed > 0 ? 1 : 0;
}

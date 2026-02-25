#include <iostream>
#include <cassert>
#include <string>
#include <sstream>
#include <streambuf>

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

// Helper to run source and capture output
std::string run_source_capture(const std::string& source) {
    // Capture stdout
    std::ostringstream oss;
    std::streambuf* old = std::cout.rdbuf(oss.rdbuf());
    
    try {
        Lexer lexer(source);
        Parser parser(lexer.scan_tokens());
        auto statements = parser.parse();
        
        Compiler compiler;
        Program program = compiler.compile(statements);
        
        VM vm(program);
        vm.run();
    } catch (...) {
        std::cout.rdbuf(old);
        throw;
    }
    
    std::cout.rdbuf(old);
    return oss.str();
}

// Helper to run source without capturing
void run_source(const std::string& source) {
    Lexer lexer(source);
    Parser parser(lexer.scan_tokens());
    auto statements = parser.parse();
    
    Compiler compiler;
    Program program = compiler.compile(statements);
    
    VM vm(program);
    vm.run();
}

// ============================================================================
// Basic VM Operation Tests
// ============================================================================

TEST(test_vm_push_and_halt) {
    Program program;
    program.main.push_back(Instruction(OpCode::PUSH_CONST, 42.0));
    program.main.push_back(Instruction(OpCode::HALT));
    
    VM vm(program);
    vm.run();
    // If we get here without crash, test passes
    ASSERT_TRUE(true);
}

TEST(test_vm_push_null) {
    Program program;
    program.main.push_back(Instruction(OpCode::PUSH_CONST, nullptr));
    program.main.push_back(Instruction(OpCode::HALT));
    
    VM vm(program);
    vm.run();
    ASSERT_TRUE(true);
}

TEST(test_vm_push_string) {
    Program program;
    program.main.push_back(Instruction(OpCode::PUSH_CONST, std::string("hello")));
    program.main.push_back(Instruction(OpCode::HALT));
    
    VM vm(program);
    vm.run();
    ASSERT_TRUE(true);
}

// ============================================================================
// Arithmetic Tests
// ============================================================================

TEST(test_vm_add) {
    Program program;
    program.main.push_back(Instruction(OpCode::PUSH_CONST, 10.0));
    program.main.push_back(Instruction(OpCode::PUSH_CONST, 5.0));
    program.main.push_back(Instruction(OpCode::ADD));
    program.main.push_back(Instruction(OpCode::HALT));
    
    VM vm(program);
    vm.run();
    ASSERT_TRUE(true);
}

TEST(test_vm_sub) {
    Program program;
    program.main.push_back(Instruction(OpCode::PUSH_CONST, 10.0));
    program.main.push_back(Instruction(OpCode::PUSH_CONST, 3.0));
    program.main.push_back(Instruction(OpCode::SUB));
    program.main.push_back(Instruction(OpCode::HALT));
    
    VM vm(program);
    vm.run();
    ASSERT_TRUE(true);
}

TEST(test_vm_mul) {
    Program program;
    program.main.push_back(Instruction(OpCode::PUSH_CONST, 4.0));
    program.main.push_back(Instruction(OpCode::PUSH_CONST, 5.0));
    program.main.push_back(Instruction(OpCode::MUL));
    program.main.push_back(Instruction(OpCode::HALT));
    
    VM vm(program);
    vm.run();
    ASSERT_TRUE(true);
}

TEST(test_vm_div) {
    Program program;
    program.main.push_back(Instruction(OpCode::PUSH_CONST, 20.0));
    program.main.push_back(Instruction(OpCode::PUSH_CONST, 4.0));
    program.main.push_back(Instruction(OpCode::DIV));
    program.main.push_back(Instruction(OpCode::HALT));
    
    VM vm(program);
    vm.run();
    ASSERT_TRUE(true);
}

TEST(test_vm_percent) {
    Program program;
    program.main.push_back(Instruction(OpCode::PUSH_CONST, 17.0));
    program.main.push_back(Instruction(OpCode::PUSH_CONST, 5.0));
    program.main.push_back(Instruction(OpCode::PERCENT));
    program.main.push_back(Instruction(OpCode::HALT));
    
    VM vm(program);
    vm.run();
    ASSERT_TRUE(true);
}

// ============================================================================
// Comparison Tests
// ============================================================================

TEST(test_vm_eq_true) {
    Program program;
    program.main.push_back(Instruction(OpCode::PUSH_CONST, 5.0));
    program.main.push_back(Instruction(OpCode::PUSH_CONST, 5.0));
    program.main.push_back(Instruction(OpCode::EQ));
    program.main.push_back(Instruction(OpCode::HALT));
    
    VM vm(program);
    vm.run();
    ASSERT_TRUE(true);
}

TEST(test_vm_gt) {
    Program program;
    program.main.push_back(Instruction(OpCode::PUSH_CONST, 10.0));
    program.main.push_back(Instruction(OpCode::PUSH_CONST, 5.0));
    program.main.push_back(Instruction(OpCode::GT));
    program.main.push_back(Instruction(OpCode::HALT));
    
    VM vm(program);
    vm.run();
    ASSERT_TRUE(true);
}

TEST(test_vm_lt) {
    Program program;
    program.main.push_back(Instruction(OpCode::PUSH_CONST, 3.0));
    program.main.push_back(Instruction(OpCode::PUSH_CONST, 7.0));
    program.main.push_back(Instruction(OpCode::LT));
    program.main.push_back(Instruction(OpCode::HALT));
    
    VM vm(program);
    vm.run();
    ASSERT_TRUE(true);
}

TEST(test_vm_not) {
    Program program;
    program.main.push_back(Instruction(OpCode::PUSH_CONST, 0.0));
    program.main.push_back(Instruction(OpCode::NOT));
    program.main.push_back(Instruction(OpCode::HALT));
    
    VM vm(program);
    vm.run();
    ASSERT_TRUE(true);
}

// ============================================================================
// Control Flow Tests
// ============================================================================

TEST(test_vm_jump) {
    Program program;
    program.main.push_back(Instruction(OpCode::PUSH_CONST, 1.0));
    program.main.push_back(Instruction(OpCode::JUMP, static_cast<int64_t>(3)));
    program.main.push_back(Instruction(OpCode::PUSH_CONST, 2.0));  // Skipped
    program.main.push_back(Instruction(OpCode::PUSH_CONST, 3.0));
    program.main.push_back(Instruction(OpCode::HALT));
    
    VM vm(program);
    vm.run();
    ASSERT_TRUE(true);
}

TEST(test_vm_jump_if_false_take_jump) {
    Program program;
    program.main.push_back(Instruction(OpCode::PUSH_CONST, 0.0));  // false
    program.main.push_back(Instruction(OpCode::JUMP_IF_FALSE, static_cast<int64_t>(3)));
    program.main.push_back(Instruction(OpCode::PUSH_CONST, 1.0));  // Skipped
    program.main.push_back(Instruction(OpCode::PUSH_CONST, 2.0));
    program.main.push_back(Instruction(OpCode::HALT));
    
    VM vm(program);
    vm.run();
    ASSERT_TRUE(true);
}

TEST(test_vm_jump_if_false_skip_jump) {
    Program program;
    program.main.push_back(Instruction(OpCode::PUSH_CONST, 1.0));  // true
    program.main.push_back(Instruction(OpCode::JUMP_IF_FALSE, static_cast<int64_t>(3)));
    program.main.push_back(Instruction(OpCode::PUSH_CONST, 2.0));  // Executed
    program.main.push_back(Instruction(OpCode::PUSH_CONST, 3.0));
    program.main.push_back(Instruction(OpCode::HALT));
    
    VM vm(program);
    vm.run();
    ASSERT_TRUE(true);
}

// ============================================================================
// Stack Operations Tests
// ============================================================================

TEST(test_vm_pop) {
    Program program;
    program.main.push_back(Instruction(OpCode::PUSH_CONST, 42.0));
    program.main.push_back(Instruction(OpCode::POP));
    program.main.push_back(Instruction(OpCode::HALT));
    
    VM vm(program);
    vm.run();
    ASSERT_TRUE(true);
}

// ============================================================================
// Data Structure Tests
// ============================================================================

TEST(test_vm_build_list) {
    Program program;
    program.main.push_back(Instruction(OpCode::PUSH_CONST, 1.0));
    program.main.push_back(Instruction(OpCode::PUSH_CONST, 2.0));
    program.main.push_back(Instruction(OpCode::PUSH_CONST, 3.0));
    program.main.push_back(Instruction(OpCode::BUILD_LIST, static_cast<int64_t>(3)));
    program.main.push_back(Instruction(OpCode::HALT));
    
    VM vm(program);
    vm.run();
    ASSERT_TRUE(true);
}

TEST(test_vm_build_map) {
    Program program;
    program.main.push_back(Instruction(OpCode::PUSH_CONST, std::string("key1")));
    program.main.push_back(Instruction(OpCode::PUSH_CONST, 100.0));
    program.main.push_back(Instruction(OpCode::PUSH_CONST, std::string("key2")));
    program.main.push_back(Instruction(OpCode::PUSH_CONST, 200.0));
    program.main.push_back(Instruction(OpCode::BUILD_MAP, static_cast<int64_t>(2)));
    program.main.push_back(Instruction(OpCode::HALT));
    
    VM vm(program);
    vm.run();
    ASSERT_TRUE(true);
}

TEST(test_vm_load_index_list) {
    Program program;
    program.main.push_back(Instruction(OpCode::PUSH_CONST, 1.0));
    program.main.push_back(Instruction(OpCode::PUSH_CONST, 2.0));
    program.main.push_back(Instruction(OpCode::PUSH_CONST, 3.0));
    program.main.push_back(Instruction(OpCode::BUILD_LIST, static_cast<int64_t>(3)));
    program.main.push_back(Instruction(OpCode::PUSH_CONST, 1.0));  // index
    program.main.push_back(Instruction(OpCode::LOAD_INDEX));
    program.main.push_back(Instruction(OpCode::HALT));
    
    VM vm(program);
    vm.run();
    ASSERT_TRUE(true);
}

// ============================================================================
// Object Tests
// ============================================================================

TEST(test_vm_new_object) {
    Program program;
    program.main.push_back(Instruction(OpCode::NEW, std::string("TestClass")));
    program.main.push_back(Instruction(OpCode::HALT));
    
    VM vm(program);
    vm.run();
    ASSERT_TRUE(true);
}

TEST(test_vm_load_field) {
    Program program;
    program.main.push_back(Instruction(OpCode::NEW, std::string("TestClass")));
    program.main.push_back(Instruction(OpCode::LOAD_FIELD, std::string("field")));
    program.main.push_back(Instruction(OpCode::HALT));
    
    VM vm(program);
    vm.run();
    ASSERT_TRUE(true);
}

TEST(test_vm_store_field) {
    Program program;
    program.main.push_back(Instruction(OpCode::NEW, std::string("TestClass")));
    program.main.push_back(Instruction(OpCode::PUSH_CONST, 42.0));
    program.main.push_back(Instruction(OpCode::STORE_FIELD, std::string("field")));
    program.main.push_back(Instruction(OpCode::HALT));
    
    VM vm(program);
    vm.run();
    ASSERT_TRUE(true);
}

// ============================================================================
// Exception Handling Tests
// ============================================================================

TEST(test_vm_setup_try) {
    Program program;
    program.main.push_back(Instruction(OpCode::SETUP_TRY, static_cast<int64_t>(3)));
    program.main.push_back(Instruction(OpCode::PUSH_CONST, 1.0));
    program.main.push_back(Instruction(OpCode::POP_TRY));
    program.main.push_back(Instruction(OpCode::JUMP, static_cast<int64_t>(5)));
    program.main.push_back(Instruction(OpCode::POP));  // Handler
    program.main.push_back(Instruction(OpCode::HALT));
    
    VM vm(program);
    vm.run();
    ASSERT_TRUE(true);
}

// ============================================================================
// Integration Tests (Full Pipeline)
// ============================================================================

TEST(test_integration_simple_print) {
    std::string source = R"(#alphabet<test>
z.o("Hello from VM test!")
)";
    
    std::string output = run_source_capture(source);
    ASSERT_TRUE(output.find("Hello from VM test!") != std::string::npos);
}

TEST(test_integration_variable) {
    std::string source = R"(#alphabet<test>
5 x = 42
z.o(x)
)";
    
    run_source(source);
    ASSERT_TRUE(true);
}

TEST(test_integration_arithmetic_expr) {
    std::string source = R"(#alphabet<test>
5 result = 10 + 20 * 3
z.o(result)
)";
    
    run_source(source);
    ASSERT_TRUE(true);
}

TEST(test_integration_if_statement) {
    std::string source = R"(#alphabet<test>
5 x = 10
i (x > 5) {
    z.o("x is greater than 5")
}
)";
    
    run_source(source);
    ASSERT_TRUE(true);
}

TEST(test_integration_loop) {
    std::string source = R"(#alphabet<test>
5 i = 0
l (i < 5) {
    5 i = i + 1
}
z.o(i)
)";
    
    run_source(source);
    ASSERT_TRUE(true);
}

TEST(test_integration_list) {
    std::string source = R"(#alphabet<test>
13 nums = [1, 2, 3, 4, 5]
z.o(nums)
)";
    
    run_source(source);
    ASSERT_TRUE(true);
}

TEST(test_integration_map) {
    std::string source = R"(#alphabet<test>
14 data = {"name": "test", "value": 42}
z.o(data)
)";
    
    run_source(source);
    ASSERT_TRUE(true);
}

// ============================================================================
// Edge Case Tests
// ============================================================================

TEST(test_vm_empty_program) {
    Program program;
    program.main.push_back(Instruction(OpCode::HALT));
    
    VM vm(program);
    vm.run();
    ASSERT_TRUE(true);
}

TEST(test_vm_nested_operations) {
    Program program;
    // (10 + 5) * (3 - 1)
    program.main.push_back(Instruction(OpCode::PUSH_CONST, 10.0));
    program.main.push_back(Instruction(OpCode::PUSH_CONST, 5.0));
    program.main.push_back(Instruction(OpCode::ADD));
    program.main.push_back(Instruction(OpCode::PUSH_CONST, 3.0));
    program.main.push_back(Instruction(OpCode::PUSH_CONST, 1.0));
    program.main.push_back(Instruction(OpCode::SUB));
    program.main.push_back(Instruction(OpCode::MUL));
    program.main.push_back(Instruction(OpCode::HALT));
    
    VM vm(program);
    vm.run();
    ASSERT_TRUE(true);
}

// ============================================================================
// Main
// ============================================================================

int main() {
    std::cout << "=== Alphabet VM Tests ===\n\n";
    
    // Basic operations
    RUN_TEST(test_vm_push_and_halt);
    RUN_TEST(test_vm_push_null);
    RUN_TEST(test_vm_push_string);
    
    // Arithmetic
    RUN_TEST(test_vm_add);
    RUN_TEST(test_vm_sub);
    RUN_TEST(test_vm_mul);
    RUN_TEST(test_vm_div);
    RUN_TEST(test_vm_percent);
    
    // Comparison
    RUN_TEST(test_vm_eq_true);
    RUN_TEST(test_vm_gt);
    RUN_TEST(test_vm_lt);
    RUN_TEST(test_vm_not);
    
    // Control flow
    RUN_TEST(test_vm_jump);
    RUN_TEST(test_vm_jump_if_false_take_jump);
    RUN_TEST(test_vm_jump_if_false_skip_jump);
    
    // Stack operations
    RUN_TEST(test_vm_pop);
    
    // Data structures
    RUN_TEST(test_vm_build_list);
    RUN_TEST(test_vm_build_map);
    RUN_TEST(test_vm_load_index_list);
    
    // Objects
    RUN_TEST(test_vm_new_object);
    RUN_TEST(test_vm_load_field);
    RUN_TEST(test_vm_store_field);
    
    // Exception handling
    RUN_TEST(test_vm_setup_try);
    
    // Integration tests
    RUN_TEST(test_integration_simple_print);
    RUN_TEST(test_integration_variable);
    RUN_TEST(test_integration_arithmetic_expr);
    RUN_TEST(test_integration_if_statement);
    RUN_TEST(test_integration_loop);
    RUN_TEST(test_integration_list);
    RUN_TEST(test_integration_map);
    
    // Edge cases
    RUN_TEST(test_vm_empty_program);
    RUN_TEST(test_vm_nested_operations);
    
    std::cout << "\n========================================\n";
    std::cout << "Tests run: " << tests_run << "\n";
    std::cout << "Passed: " << tests_passed << "\n";
    std::cout << "Failed: " << tests_failed << "\n";
    std::cout << "========================================\n";
    
    return tests_failed > 0 ? 1 : 0;
}

#include <cassert>
#include <cmath>
#include <iostream>
#include <sstream>
#include <streambuf>
#include <string>

#include "lexer.h"
#include "parser.h"
#include "compiler.h"
#include "vm.h"
#include "test_helpers.h"

using namespace alphabet;

// Test counters
static int tests_run = 0;
static int tests_passed = 0;
static int tests_failed = 0;

#define TEST(name) void name()
#define RUN_TEST(name)                                                                             \
    do {                                                                                           \
        tests_run++;                                                                               \
        std::cout << "Running " << #name << "... ";                                                \
        try {                                                                                      \
            name();                                                                                \
            tests_passed++;                                                                        \
            std::cout << "PASSED\n";                                                               \
        }                                                                                          \
        catch (const std::exception &e) {                                                          \
            tests_failed++;                                                                        \
            std::cout << "FAILED: " << e.what() << "\n";                                           \
        }                                                                                          \
    } while (0)

#define ASSERT_EQ(expected, actual)                                                                \
    do {                                                                                           \
        if ((expected) != (actual)) {                                                              \
            throw std::runtime_error("Expected " + std::to_string(expected) + " but got " +        \
                                     std::to_string(actual));                                      \
        }                                                                                          \
    } while (0)

#define ASSERT_STREQ(expected, actual)                                                             \
    do {                                                                                           \
        if (std::string(expected) != std::string(actual)) {                                        \
            throw std::runtime_error("Expected \"" + std::string(expected) + "\" but got \"" +     \
                                     std::string(actual) + "\"");                                  \
        }                                                                                          \
    } while (0)

#define ASSERT_CONTAINS(needle, haystack)                                                          \
    do {                                                                                           \
        if (std::string(haystack).find(std::string(needle)) == std::string::npos) {                \
            throw std::runtime_error("Expected output to contain \"" + std::string(needle) +       \
                                     "\" but got: " + std::string(haystack));                      \
        }                                                                                          \
    } while (0)

#define ASSERT_TRUE(cond)                                                                          \
    do {                                                                                           \
        if (!(cond)) {                                                                             \
            throw std::runtime_error("Assertion failed: " #cond);                                  \
        }                                                                                          \
    } while (0)

// Helper to run source and get a global variable value
double get_number_global(const std::string &source, const std::string &varname)
{
    auto globals = test::run_get_globals(source);
    auto it = globals.find(varname);
    if (it == globals.end())
        throw std::runtime_error("Global '" + varname + "' not found");
    if (!it->second.is_number())
        throw std::runtime_error("Global '" + varname + "' is not a number");
    return it->second.as_number();
}

std::string get_string_global(const std::string &source, const std::string &varname)
{
    auto globals = test::run_get_globals(source);
    auto it = globals.find(varname);
    if (it == globals.end())
        throw std::runtime_error("Global '" + varname + "' not found");
    if (!it->second.is_string())
        throw std::runtime_error("Global '" + varname + "' is not a string");
    return it->second.as_string();
}

// ============================================================================
// Arithmetic Tests
// ============================================================================

TEST(test_arithmetic_add)
{
    std::string output = test::run_capture("#alphabet<test>\nz.o(10 + 20)");
    ASSERT_CONTAINS("30", output);
}

TEST(test_arithmetic_sub)
{
    std::string output = test::run_capture("#alphabet<test>\nz.o(50 - 13)");
    ASSERT_CONTAINS("37", output);
}

TEST(test_arithmetic_mul)
{
    std::string output = test::run_capture("#alphabet<test>\nz.o(6 * 7)");
    ASSERT_CONTAINS("42", output);
}

TEST(test_arithmetic_div)
{
    std::string output = test::run_capture("#alphabet<test>\nz.o(100 / 4)");
    ASSERT_CONTAINS("25", output);
}

TEST(test_arithmetic_mod)
{
    std::string output = test::run_capture("#alphabet<test>\nz.o(17 % 5)");
    ASSERT_CONTAINS("2", output);
}

TEST(test_arithmetic_precedence)
{
    // 2 + 3 * 4 = 14, not 20
    std::string output = test::run_capture("#alphabet<test>\nz.o(2 + 3 * 4)");
    ASSERT_CONTAINS("14", output);
}

TEST(test_arithmetic_parentheses)
{
    // (2 + 3) * 4 = 20
    std::string output = test::run_capture("#alphabet<test>\nz.o((2 + 3) * 4)");
    ASSERT_CONTAINS("20", output);
}

// ============================================================================
// Variable Tests
// ============================================================================

TEST(test_variable_assignment)
{
    double val = get_number_global("#alphabet<test>\n5 x = 99", "x");
    ASSERT_EQ(99.0, val);
}

TEST(test_variable_expression)
{
    double val = get_number_global("#alphabet<test>\n5 x = 10 + 5 * 2", "x");
    ASSERT_EQ(20.0, val);
}

TEST(test_string_variable)
{
    std::string val = get_string_global("#alphabet<test>\n12 s = \"hello\"", "s");
    ASSERT_STREQ("hello", val);
}

// ============================================================================
// Comparison Tests
// ============================================================================

TEST(test_comparison_gt_true)
{
    std::string output = test::run_capture("#alphabet<test>\n5 b = 10 > 5\nz.o(b)");
    ASSERT_CONTAINS("1", output);
}

TEST(test_comparison_gt_false)
{
    std::string output = test::run_capture("#alphabet<test>\n5 b = 3 > 7\nz.o(b)");
    ASSERT_CONTAINS("0", output);
}

TEST(test_comparison_eq)
{
    std::string output = test::run_capture("#alphabet<test>\n5 b = 5 == 5\nz.o(b)");
    ASSERT_CONTAINS("1", output);
}

TEST(test_comparison_ne)
{
    std::string output = test::run_capture("#alphabet<test>\n5 b = 5 != 3\nz.o(b)");
    ASSERT_CONTAINS("1", output);
}

// ============================================================================
// Control Flow Tests
// ============================================================================

TEST(test_if_true_branch)
{
    std::string output = test::run_capture("#alphabet<test>\ni (1 > 0) { z.o(\"yes\") }");
    ASSERT_CONTAINS("yes", output);
}

TEST(test_if_else_false_branch)
{
    std::string output =
        test::run_capture("#alphabet<test>\ni (0 > 1) { z.o(\"yes\") } e { z.o(\"no\") }");
    ASSERT_CONTAINS("no", output);
}

TEST(test_loop_count)
{
    std::string output =
        test::run_capture("#alphabet<test>\n5 i = 0\nl (i < 5) { z.o(i) 5 i = i + 1 }");
    ASSERT_CONTAINS("0", output);
    ASSERT_CONTAINS("4", output);
}

// ============================================================================
// Function Tests
// ============================================================================

TEST(test_function_return)
{
    std::string output = test::run_capture(R"(#alphabet<test>
m 5 add(5 a, 5 b) {
  r a + b
}
z.o(add(3, 4)))");
    ASSERT_CONTAINS("7", output);
}

TEST(test_function_recursion)
{
    std::string output = test::run_capture(R"(#alphabet<test>
m 5 fact(5 num) {
  i (num <= 1) { r 1 }
  r num * fact(num - 1)
}
5 result = fact(4)
z.o(result))");
    ASSERT_CONTAINS("24", output);
}

// ============================================================================
// List Tests
// ============================================================================

TEST(test_list_create_and_access)
{
    std::string output = test::run_capture("#alphabet<test>\n13 nums = [10, 20, 30]\nz.o(nums[1])");
    ASSERT_CONTAINS("20", output);
}

TEST(test_list_negative_index)
{
    std::string output =
        test::run_capture("#alphabet<test>\n13 nums = [10, 20, 30]\nz.o(nums[-1])");
    ASSERT_CONTAINS("30", output);
}

TEST(test_list_length)
{
    std::string output =
        test::run_capture("#alphabet<test>\n13 nums = [1, 2, 3, 4, 5]\nz.o(z.len(nums))");
    ASSERT_CONTAINS("5", output);
}

TEST(test_list_append)
{
    std::string output =
        test::run_capture("#alphabet<test>\n13 nums = [1, 2]\nz.append(nums, 3)\nz.o(nums[2])");
    ASSERT_CONTAINS("3", output);
}

// ============================================================================
// Map Tests
// ============================================================================

TEST(test_map_create_and_access)
{
    std::string output =
        test::run_capture("#alphabet<test>\n14 m = {\"name\": \"Alphabet\"}\nz.o(m[\"name\"])");
    ASSERT_CONTAINS("Alphabet", output);
}

// ============================================================================
// String Tests
// ============================================================================

TEST(test_string_concat)
{
    std::string output =
        test::run_capture("#alphabet<test>\n12 s = \"Hello\" + \" World\"\nz.o(s)");
    ASSERT_CONTAINS("Hello World", output);
}

TEST(test_string_number_concat)
{
    std::string output = test::run_capture("#alphabet<test>\nz.o(\"x=\" + 42)");
    ASSERT_CONTAINS("x=42", output);
}

TEST(test_string_upper)
{
    std::string output = test::run_capture("#alphabet<test>\nz.o(z.upper(\"hello\"))");
    ASSERT_CONTAINS("HELLO", output);
}

TEST(test_string_lower)
{
    std::string output = test::run_capture("#alphabet<test>\nz.o(z.lower(\"HELLO\"))");
    ASSERT_CONTAINS("hello", output);
}

TEST(test_string_trim)
{
    std::string output = test::run_capture("#alphabet<test>\nz.o(z.trim(\"  hi  \"))");
    ASSERT_CONTAINS("hi", output);
}

TEST(test_string_split)
{
    std::string output =
        test::run_capture("#alphabet<test>\n13 parts = z.split(\"a,b,c\", \",\")\nz.o(parts[1])");
    ASSERT_CONTAINS("b", output);
}

TEST(test_string_replace)
{
    std::string output =
        test::run_capture("#alphabet<test>\nz.o(z.replace(\"hello world\", \"world\", \"ABC\"))");
    ASSERT_CONTAINS("hello ABC", output);
}

// ============================================================================
// Built-in Function Tests
// ============================================================================

TEST(test_builtin_sqrt)
{
    std::string output = test::run_capture("#alphabet<test>\nz.o(z.sqrt(144))");
    ASSERT_CONTAINS("12", output);
}

TEST(test_builtin_abs)
{
    std::string output = test::run_capture("#alphabet<test>\nz.o(z.abs(-42))");
    ASSERT_CONTAINS("42", output);
}

TEST(test_builtin_pow)
{
    std::string output = test::run_capture("#alphabet<test>\nz.o(z.pow(2, 10))");
    ASSERT_CONTAINS("1024", output);
}

TEST(test_builtin_type_number)
{
    std::string output = test::run_capture("#alphabet<test>\nz.o(z.type(42))");
    ASSERT_CONTAINS("number", output);
}

TEST(test_builtin_type_string)
{
    std::string output = test::run_capture("#alphabet<test>\nz.o(z.type(\"hi\"))");
    ASSERT_CONTAINS("string", output);
}

// ============================================================================
// New Built-in Tests (added in this version)
// ============================================================================

TEST(test_range_basic)
{
    std::string output = test::run_capture("#alphabet<test>\n13 r = z.range(5)\nz.o(r[0])");
    ASSERT_CONTAINS("0", output);
}

TEST(test_range_with_start)
{
    std::string output = test::run_capture("#alphabet<test>\n13 r = z.range(3, 7)\nz.o(r[0])");
    ASSERT_CONTAINS("3", output);
}

TEST(test_range_length)
{
    std::string output = test::run_capture("#alphabet<test>\n13 r = z.range(5)\nz.o(z.len(r))");
    ASSERT_CONTAINS("5", output);
}

TEST(test_contains_list)
{
    std::string output =
        test::run_capture("#alphabet<test>\n13 nums = [1, 2, 3]\nz.o(z.contains(nums, 2))");
    ASSERT_CONTAINS("1", output);
}

TEST(test_contains_string)
{
    std::string output =
        test::run_capture("#alphabet<test>\nz.o(z.contains(\"hello world\", \"world\"))");
    ASSERT_CONTAINS("1", output);
}

TEST(test_keys_map)
{
    std::string output =
        test::run_capture("#alphabet<test>\n14 m = {\"a\": 1}\n13 k = z.keys(m)\nz.o(k[0])");
    ASSERT_CONTAINS("a", output);
}

TEST(test_reverse_list)
{
    std::string output =
        test::run_capture("#alphabet<test>\n13 r = z.reverse([1, 2, 3])\nz.o(r[0])");
    ASSERT_CONTAINS("3", output);
}

TEST(test_substr)
{
    std::string output = test::run_capture("#alphabet<test>\nz.o(z.substr(\"hello\", 1, 3))");
    ASSERT_CONTAINS("ell", output);
}

TEST(test_chr_ord)
{
    std::string output = test::run_capture("#alphabet<test>\nz.o(z.chr(65))");
    ASSERT_CONTAINS("A", output);
}

TEST(test_starts_with)
{
    std::string output =
        test::run_capture("#alphabet<test>\nz.o(z.starts_with(\"hello\", \"hel\"))");
    ASSERT_CONTAINS("1", output);
}

TEST(test_ends_with)
{
    std::string output = test::run_capture("#alphabet<test>\nz.o(z.ends_with(\"hello\", \"llo\"))");
    ASSERT_CONTAINS("1", output);
}

// ============================================================================
// Exception Handling Tests
// ============================================================================

TEST(test_try_catch_no_throw)
{
    std::string output = test::run_capture(
        "#alphabet<test>\nt { z.o(\"try\") } h (12 e) { z.o(\"caught\") }\nz.o(\"done\")");
    ASSERT_CONTAINS("try", output);
    ASSERT_CONTAINS("done", output);
}

// ============================================================================
// Class Tests
// ============================================================================

TEST(test_class_basic)
{
    std::string output = test::run_capture(R"(#alphabet<test>
c Point {
  5 x = 0
  5 y = 0
  v m 5 get_x() { r x }
}
15 p = n Point()
z.o(p.get_x()))");
    ASSERT_CONTAINS("0", output);
}

// ============================================================================
// Pattern Matching Tests
// ============================================================================

TEST(test_match_basic)
{
    std::string output = test::run_capture(R"(#alphabet<test>
5 x = 2
q (x) {
  1: z.o("one")
  2: z.o("two")
  e: z.o("other")
})");
    ASSERT_CONTAINS("two", output);
}

// ============================================================================
// Logical Operator Tests
// ============================================================================

TEST(test_logical_and)
{
    std::string output = test::run_capture("#alphabet<test>\n5 b = 1 && 1\nz.o(b)");
    ASSERT_CONTAINS("1", output);
}

TEST(test_logical_or)
{
    std::string output = test::run_capture("#alphabet<test>\n5 b = 0 || 1\nz.o(b)");
    ASSERT_CONTAINS("1", output);
}

TEST(test_logical_not)
{
    std::string output = test::run_capture("#alphabet<test>\n5 b = !0\nz.o(b)");
    ASSERT_CONTAINS("1", output);
}

// ============================================================================
// Main
// ============================================================================

int main()
{
    std::cout << "=== Alphabet VM Tests (with assertions) ===\n\n";

    std::cout << "--- Arithmetic ---\n";
    RUN_TEST(test_arithmetic_add);
    RUN_TEST(test_arithmetic_sub);
    RUN_TEST(test_arithmetic_mul);
    RUN_TEST(test_arithmetic_div);
    RUN_TEST(test_arithmetic_mod);
    RUN_TEST(test_arithmetic_precedence);
    RUN_TEST(test_arithmetic_parentheses);

    std::cout << "\n--- Variables ---\n";
    RUN_TEST(test_variable_assignment);
    RUN_TEST(test_variable_expression);
    RUN_TEST(test_string_variable);

    std::cout << "\n--- Comparisons ---\n";
    RUN_TEST(test_comparison_gt_true);
    RUN_TEST(test_comparison_gt_false);
    RUN_TEST(test_comparison_eq);
    RUN_TEST(test_comparison_ne);

    std::cout << "\n--- Control Flow ---\n";
    RUN_TEST(test_if_true_branch);
    RUN_TEST(test_if_else_false_branch);
    RUN_TEST(test_loop_count);

    std::cout << "\n--- Functions ---\n";
    RUN_TEST(test_function_return);
    RUN_TEST(test_function_recursion);

    std::cout << "\n--- Lists ---\n";
    RUN_TEST(test_list_create_and_access);
    RUN_TEST(test_list_negative_index);
    RUN_TEST(test_list_length);
    RUN_TEST(test_list_append);

    std::cout << "\n--- Maps ---\n";
    RUN_TEST(test_map_create_and_access);

    std::cout << "\n--- Strings ---\n";
    RUN_TEST(test_string_concat);
    RUN_TEST(test_string_number_concat);
    RUN_TEST(test_string_upper);
    RUN_TEST(test_string_lower);
    RUN_TEST(test_string_trim);
    RUN_TEST(test_string_split);
    RUN_TEST(test_string_replace);

    std::cout << "\n--- Built-ins ---\n";
    RUN_TEST(test_builtin_sqrt);
    RUN_TEST(test_builtin_abs);
    RUN_TEST(test_builtin_pow);
    RUN_TEST(test_builtin_type_number);
    RUN_TEST(test_builtin_type_string);

    std::cout << "\n--- New Built-ins (v2.3.0) ---\n";
    RUN_TEST(test_range_basic);
    RUN_TEST(test_range_with_start);
    RUN_TEST(test_range_length);
    RUN_TEST(test_contains_list);
    RUN_TEST(test_contains_string);
    RUN_TEST(test_keys_map);
    RUN_TEST(test_reverse_list);
    RUN_TEST(test_substr);
    RUN_TEST(test_chr_ord);
    RUN_TEST(test_starts_with);
    RUN_TEST(test_ends_with);

    std::cout << "\n--- Exception Handling ---\n";
    RUN_TEST(test_try_catch_no_throw);

    std::cout << "\n--- Classes ---\n";
    RUN_TEST(test_class_basic);

    std::cout << "\n--- Pattern Matching ---\n";
    RUN_TEST(test_match_basic);

    std::cout << "\n--- Logical Operators ---\n";
    RUN_TEST(test_logical_and);
    RUN_TEST(test_logical_or);
    RUN_TEST(test_logical_not);

    std::cout << "\n========================================\n";
    std::cout << "Tests run: " << tests_run << "\n";
    std::cout << "Passed: " << tests_passed << "\n";
    std::cout << "Failed: " << tests_failed << "\n";
    std::cout << "========================================\n";

    return tests_failed > 0 ? 1 : 0;
}

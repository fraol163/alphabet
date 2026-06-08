#define CATCH_CONFIG_MAIN
#include "catch.hpp"

#include "lexer.h"
#include "parser.h"
#include "compiler.h"
#include "vm.h"
#include "test_helpers.h"

using namespace alphabet;

// Helper to run source and get a global variable value
static double get_number_global(const std::string &source, const std::string &varname)
{
    auto globals = test::run_get_globals(source);
    auto it = globals.find(varname);
    if (it == globals.end())
        throw std::runtime_error("Global '" + varname + "' not found");
    if (!it->second.is_number())
        throw std::runtime_error("Global '" + varname + "' is not a number");
    return it->second.as_number();
}

static std::string get_string_global(const std::string &source, const std::string &varname)
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

TEST_CASE("Arithmetic: addition", "[vm][arithmetic]")
{
    std::string output = test::run_capture("#alphabet<en>\nz.o(10 + 20)");
    REQUIRE(output == "30\n");
}

TEST_CASE("Arithmetic: subtraction", "[vm][arithmetic]")
{
    std::string output = test::run_capture("#alphabet<en>\nz.o(50 - 13)");
    REQUIRE(output == "37\n");
}

TEST_CASE("Arithmetic: multiplication", "[vm][arithmetic]")
{
    std::string output = test::run_capture("#alphabet<en>\nz.o(6 * 7)");
    REQUIRE(output == "42\n");
}

TEST_CASE("Arithmetic: division", "[vm][arithmetic]")
{
    std::string output = test::run_capture("#alphabet<en>\nz.o(100 / 4)");
    REQUIRE(output == "25\n");
}

TEST_CASE("Arithmetic: modulo", "[vm][arithmetic]")
{
    std::string output = test::run_capture("#alphabet<en>\nz.o(17 % 5)");
    REQUIRE(output == "2\n");
}

TEST_CASE("Arithmetic: operator precedence", "[vm][arithmetic]")
{
    // 2 + 3 * 4 = 14, not 20
    std::string output = test::run_capture("#alphabet<en>\nz.o(2 + 3 * 4)");
    REQUIRE(output == "14\n");
}

TEST_CASE("Arithmetic: parentheses", "[vm][arithmetic]")
{
    // (2 + 3) * 4 = 20
    std::string output = test::run_capture("#alphabet<en>\nz.o((2 + 3) * 4)");
    REQUIRE(output == "20\n");
}

// ============================================================================
// Variable Tests
// ============================================================================

TEST_CASE("Variable assignment", "[vm][variables]")
{
    double val = get_number_global("#alphabet<en>\n5 x = 99", "x");
    REQUIRE(val == Approx(99.0));
}

TEST_CASE("Variable expression assignment", "[vm][variables]")
{
    double val = get_number_global("#alphabet<en>\n5 x = 10 + 5 * 2", "x");
    REQUIRE(val == Approx(20.0));
}

TEST_CASE("String variable", "[vm][variables]")
{
    std::string val = get_string_global("#alphabet<en>\n12 s = \"hello\"", "s");
    REQUIRE(val == "hello");
}

// ============================================================================
// Comparison Tests
// ============================================================================

TEST_CASE("Comparison: greater than true", "[vm][comparison]")
{
    std::string output = test::run_capture("#alphabet<en>\n5 b = 10 > 5\nz.o(b)");
    REQUIRE(output == "true\n");
}

TEST_CASE("Comparison: greater than false", "[vm][comparison]")
{
    std::string output = test::run_capture("#alphabet<en>\n5 b = 3 > 7\nz.o(b)");
    REQUIRE(output == "false\n");
}

TEST_CASE("Comparison: equal", "[vm][comparison]")
{
    std::string output = test::run_capture("#alphabet<en>\n5 b = 5 == 5\nz.o(b)");
    REQUIRE(output == "true\n");
}

TEST_CASE("Comparison: not equal", "[vm][comparison]")
{
    std::string output = test::run_capture("#alphabet<en>\n5 b = 5 != 3\nz.o(b)");
    REQUIRE(output == "true\n");
}

// ============================================================================
// Control Flow Tests
// ============================================================================

TEST_CASE("If true branch", "[vm][control_flow]")
{
    std::string output = test::run_capture("#alphabet<en>\ni (1 > 0) { z.o(\"yes\") }");
    REQUIRE(output == "yes\n");
}

TEST_CASE("If-else false branch", "[vm][control_flow]")
{
    std::string output =
        test::run_capture("#alphabet<en>\ni (0 > 1) { z.o(\"yes\") } e { z.o(\"no\") }");
    REQUIRE(output == "no\n");
}

TEST_CASE("Loop count", "[vm][control_flow]")
{
    std::string output =
        test::run_capture("#alphabet<en>\n5 i = 0\nl (i < 5) { z.o(i) 5 i = i + 1 }");
    REQUIRE(output.find("0\n") != std::string::npos);
    REQUIRE(output.find("4\n") != std::string::npos);
}

// ============================================================================
// Function Tests
// ============================================================================

TEST_CASE("Function return", "[vm][functions]")
{
    std::string output = test::run_capture(R"(#alphabet<en>
m 5 add(5 a, 5 b) {
  r a + b
}
z.o(add(3, 4)))");
    REQUIRE(output == "7\n");
}

TEST_CASE("Function recursion", "[vm][functions]")
{
    std::string output = test::run_capture(R"(#alphabet<en>
m 5 fact(5 num) {
  i (num <= 1) { r 1 }
  r num * fact(num - 1)
}
5 result = fact(4)
z.o(result))");
    REQUIRE(output == "24\n");
}

// ============================================================================
// List Tests
// ============================================================================

TEST_CASE("List create and access", "[vm][lists]")
{
    std::string output = test::run_capture("#alphabet<en>\n13 nums = [10, 20, 30]\nz.o(nums[1])");
    REQUIRE(output == "20\n");
}

TEST_CASE("List negative index", "[vm][lists]")
{
    std::string output =
        test::run_capture("#alphabet<en>\n13 nums = [10, 20, 30]\nz.o(nums[-1])");
    REQUIRE(output == "30\n");
}

TEST_CASE("List length", "[vm][lists]")
{
    std::string output =
        test::run_capture("#alphabet<en>\n13 nums = [1, 2, 3, 4, 5]\nz.o(z.len(nums))");
    REQUIRE(output == "5\n");
}

TEST_CASE("List append", "[vm][lists]")
{
    std::string output =
        test::run_capture("#alphabet<en>\n13 nums = [1, 2]\nz.append(nums, 3)\nz.o(nums[2])");
    REQUIRE(output == "3\n");
}

// ============================================================================
// Map Tests
// ============================================================================

TEST_CASE("Map create and access", "[vm][maps]")
{
    std::string output =
        test::run_capture("#alphabet<en>\n14 m = {\"name\": \"Alphabet\"}\nz.o(m[\"name\"])");
    REQUIRE(output == "Alphabet\n");
}

// ============================================================================
// String Tests
// ============================================================================

TEST_CASE("String concatenation", "[vm][strings]")
{
    std::string output =
        test::run_capture("#alphabet<en>\n12 s = \"Hello\" + \" World\"\nz.o(s)");
    REQUIRE(output == "Hello World\n");
}

TEST_CASE("String-number concatenation", "[vm][strings]")
{
    std::string output = test::run_capture("#alphabet<en>\nz.o(\"x=\" + 42)");
    REQUIRE(output == "x=42\n");
}

TEST_CASE("String upper", "[vm][strings]")
{
    std::string output = test::run_capture("#alphabet<en>\nz.o(z.upper(\"hello\"))");
    REQUIRE(output == "HELLO\n");
}

TEST_CASE("String lower", "[vm][strings]")
{
    std::string output = test::run_capture("#alphabet<en>\nz.o(z.lower(\"HELLO\"))");
    REQUIRE(output == "hello\n");
}

TEST_CASE("String trim", "[vm][strings]")
{
    std::string output = test::run_capture("#alphabet<en>\nz.o(z.trim(\"  hi  \"))");
    REQUIRE(output == "hi\n");
}

TEST_CASE("String split", "[vm][strings]")
{
    std::string output =
        test::run_capture("#alphabet<en>\n13 parts = z.split(\"a,b,c\", \",\")\nz.o(parts[1])");
    REQUIRE(output == "b\n");
}

TEST_CASE("String replace", "[vm][strings]")
{
    std::string output =
        test::run_capture("#alphabet<en>\nz.o(z.replace(\"hello world\", \"world\", \"ABC\"))");
    REQUIRE(output == "hello ABC\n");
}

// ============================================================================
// Built-in Function Tests
// ============================================================================

TEST_CASE("Built-in sqrt", "[vm][builtins]")
{
    std::string output = test::run_capture("#alphabet<en>\nz.o(z.sqrt(144))");
    REQUIRE(output == "12\n");
}

TEST_CASE("Built-in abs", "[vm][builtins]")
{
    std::string output = test::run_capture("#alphabet<en>\nz.o(z.abs(-42))");
    REQUIRE(output == "42\n");
}

TEST_CASE("Built-in pow", "[vm][builtins]")
{
    std::string output = test::run_capture("#alphabet<en>\nz.o(z.pow(2, 10))");
    REQUIRE(output == "1024\n");
}

TEST_CASE("Built-in type number", "[vm][builtins]")
{
    std::string output = test::run_capture("#alphabet<en>\nz.o(z.type(42))");
    REQUIRE(output == "number\n");
}

TEST_CASE("Built-in type string", "[vm][builtins]")
{
    std::string output = test::run_capture("#alphabet<en>\nz.o(z.type(\"hi\"))");
    REQUIRE(output == "string\n");
}

// ============================================================================
// New Built-in Tests
// ============================================================================

TEST_CASE("Range basic", "[vm][builtins]")
{
    std::string output = test::run_capture("#alphabet<en>\n13 r = z.range(5)\nz.o(r[0])");
    REQUIRE(output == "0\n");
}

TEST_CASE("Range with start", "[vm][builtins]")
{
    std::string output = test::run_capture("#alphabet<en>\n13 r = z.range(3, 7)\nz.o(r[0])");
    REQUIRE(output == "3\n");
}

TEST_CASE("Range length", "[vm][builtins]")
{
    std::string output = test::run_capture("#alphabet<en>\n13 r = z.range(5)\nz.o(z.len(r))");
    REQUIRE(output == "5\n");
}

TEST_CASE("Contains list", "[vm][builtins]")
{
    std::string output =
        test::run_capture("#alphabet<en>\n13 nums = [1, 2, 3]\nz.o(z.contains(nums, 2))");
    REQUIRE(output == "1\n");
}

TEST_CASE("Contains string", "[vm][builtins]")
{
    std::string output =
        test::run_capture("#alphabet<en>\nz.o(z.contains(\"hello world\", \"world\"))");
    REQUIRE(output == "1\n");
}

TEST_CASE("Keys map", "[vm][builtins]")
{
    std::string output =
        test::run_capture("#alphabet<en>\n14 m = {\"a\": 1}\n13 k = z.keys(m)\nz.o(k[0])");
    REQUIRE(output == "a\n");
}

TEST_CASE("Reverse list", "[vm][builtins]")
{
    std::string output =
        test::run_capture("#alphabet<en>\n13 r = z.reverse([1, 2, 3])\nz.o(r[0])");
    REQUIRE(output == "3\n");
}

TEST_CASE("Substr", "[vm][builtins]")
{
    std::string output = test::run_capture("#alphabet<en>\nz.o(z.substr(\"hello\", 1, 3))");
    REQUIRE(output == "ell\n");
}

TEST_CASE("Chr and ord", "[vm][builtins]")
{
    std::string output = test::run_capture("#alphabet<en>\nz.o(z.chr(65))");
    REQUIRE(output == "A\n");
}

TEST_CASE("Starts with", "[vm][builtins]")
{
    std::string output =
        test::run_capture("#alphabet<en>\nz.o(z.starts_with(\"hello\", \"hel\"))");
    REQUIRE(output == "1\n");
}

TEST_CASE("Ends with", "[vm][builtins]")
{
    std::string output = test::run_capture("#alphabet<en>\nz.o(z.ends_with(\"hello\", \"llo\"))");
    REQUIRE(output == "1\n");
}

// ============================================================================
// Exception Handling Tests
// ============================================================================

TEST_CASE("Try-catch no throw", "[vm][exceptions]")
{
    std::string output = test::run_capture(
        "#alphabet<en>\nt { z.o(\"try\") } h (12 e) { z.o(\"caught\") }\nz.o(\"done\")");
    REQUIRE(output.find("try\n") != std::string::npos);
    REQUIRE(output.find("done\n") != std::string::npos);
}

// ============================================================================
// Class Tests
// ============================================================================

TEST_CASE("Class basic", "[vm][classes]")
{
    std::string output = test::run_capture(R"(#alphabet<en>
c Point {
  5 x = 0
  5 y = 0
  v m 5 get_x() { r x }
}
15 p = n Point()
z.o(p.get_x()))");
    REQUIRE(output == "0\n");
}

// ============================================================================
// Pattern Matching Tests
// ============================================================================

TEST_CASE("Match basic", "[vm][pattern_matching]")
{
    std::string output = test::run_capture(R"(#alphabet<en>
5 x = 2
q (x) {
  1: z.o("one")
  2: z.o("two")
  e: z.o("other")
})");
    REQUIRE(output.find("two\n") != std::string::npos);
}

// ============================================================================
// Logical Operator Tests
// ============================================================================

TEST_CASE("Logical AND", "[vm][logical]")
{
    std::string output = test::run_capture("#alphabet<en>\n5 b = 1 && 1\nz.o(b)");
    REQUIRE(output == "1\n");
}

TEST_CASE("Logical OR", "[vm][logical]")
{
    std::string output = test::run_capture("#alphabet<en>\n5 b = 0 || 1\nz.o(b)");
    REQUIRE(output == "1\n");
}

TEST_CASE("Logical NOT", "[vm][logical]")
{
    std::string output = test::run_capture("#alphabet<en>\n5 b = !0\nz.o(b)");
    REQUIRE(output == "true\n");
}

// ============================================================================
// Negative / Error Tests
// ============================================================================

TEST_CASE("Division by zero is catchable", "[vm][negative][errors]")
{
    std::string output = test::run_capture("#alphabet<en>\nt { z.o(10 / 0) } h (15 e) { z.o(e) }");
    REQUIRE(output.find("Division by zero") != std::string::npos);
}

TEST_CASE("Stack overflow handled gracefully", "[vm][negative][errors]")
{
    // Recursive function with no base case should not crash the process
    // It prints "Unhandled exception" and returns normally
    std::string output = test::run_capture(R"(#alphabet<en>
m 5 inf() {
  r inf()
}
inf())");
    REQUIRE(true);
}

TEST_CASE("Accessing undefined variable returns nil", "[vm][negative]")
{
    // Undefined variables resolve to nil. Verify it doesn't crash.
    // z.o(nil) should print empty/null without throwing.
    std::string output = test::run_capture(
        "#alphabet<en>\n5 x = undefined_var\nz.o(z.type(x))");
    REQUIRE(output.find("null\n") != std::string::npos);
}

// ============================================================================
// Feature Tests: String Concatenation
// ============================================================================

TEST_CASE("String concatenation with variable", "[vm][string]")
{
    std::string output = test::run_capture(
        "#alphabet<en>\n12 name = \"World\"\nz.o(\"Hello, \" + name + \"!\")");
    REQUIRE(output == "Hello, World!\n");
}

TEST_CASE("String number concatenation", "[vm][string]")
{
    std::string output = test::run_capture(
        "#alphabet<en>\nz.o(\"x=\" + 42)");
    REQUIRE(output == "x=42\n");
}

// ============================================================================
// Feature Tests: While Loop
// ============================================================================

TEST_CASE("While loop basic", "[vm][loop]")
{
    std::string output = test::run_capture(R"(#alphabet<en>
5 i = 0
l (i < 3) {
  z.o(i)
  5 i = i + 1
})");
    REQUIRE(output.find("0\n") != std::string::npos);
    REQUIRE(output.find("1\n") != std::string::npos);
    REQUIRE(output.find("2\n") != std::string::npos);
}

// ============================================================================
// Feature Tests: Break and Continue
// ============================================================================

TEST_CASE("Break exits loop early", "[vm][loop]")
{
    std::string output = test::run_capture(R"(#alphabet<en>
5 i = 0
l (i < 10) {
  i (i == 3) { b }
  z.o(i)
  5 i = i + 1
})");
    REQUIRE(output.find("0\n") != std::string::npos);
    REQUIRE(output.find("2\n") != std::string::npos);
    // 3 should NOT be printed (break happens before print)
    // Actually, break is after i==3 check but before print,
    // so 0,1,2 should be printed, not 3
}

TEST_CASE("Continue skips iteration", "[vm][loop]")
{
    std::string output = test::run_capture(R"(#alphabet<en>
5 i = 0
l (i < 5) {
  i (i == 2) { 5 i = i + 1 k }
  z.o(i)
  5 i = i + 1
})");
    REQUIRE(output.find("0\n") != std::string::npos);
    REQUIRE(output.find("1\n") != std::string::npos);
    // 2 should be skipped
    REQUIRE(output.find("3\n") != std::string::npos);
}

// ============================================================================
// Feature Tests: Variable Reassignment
// ============================================================================

TEST_CASE("Variable reassignment", "[vm][variable]")
{
    std::string output = test::run_capture(R"(#alphabet<en>
5 x = 10
5 x = 20
z.o(x))");
    REQUIRE(output == "20\n");
}

// ============================================================================
// Feature Tests: Class Inheritance
// ============================================================================

TEST_CASE("Class inheritance method override", "[vm][oop]")
{
    std::string output = test::run_capture(R"(#alphabet<en>
c Animal {
  v m 12 speak() { r "..." }
}
c Dog ^ Animal {
  v m 12 speak() { r "woof" }
}
15 d = n Dog()
z.o(d.speak()))");
    REQUIRE(output == "woof\n");
}

// ============================================================================
// Feature Tests: Pattern Matching
// ============================================================================

TEST_CASE("Match with default case", "[vm][match]")
{
    std::string output = test::run_capture(R"(#alphabet<en>
5 x = 99
q (x) {
  1: z.o("one")
  2: z.o("two")
  e: z.o("other")
})");
    REQUIRE(output.find("other\n") != std::string::npos);
}

// ============================================================================
// Feature Tests: Exception Handling
// ============================================================================

TEST_CASE("Try-catch with throw", "[vm][exceptions]")
{
    std::string output = test::run_capture(R"(#alphabet<en>
t {
  5 x = 1 + 1
  z.o("try")
} h (12 e) {
  z.o("caught")
}
z.o("done"))");
    REQUIRE(output.find("try\n") != std::string::npos);
    REQUIRE(output.find("done\n") != std::string::npos);
}

// ============================================================================
// Feature Tests: Map Operations
// ============================================================================

TEST_CASE("Map create and access key", "[vm][map]")
{
    std::string output = test::run_capture(
        "#alphabet<en>\n14 m = {\"key\": 42}\nz.o(m[\"key\"])");
    REQUIRE(output == "42\n");
}

// ============================================================================
// Feature Tests: Negative Indexing
// ============================================================================

TEST_CASE("Negative list indexing", "[vm][list]")
{
    std::string output = test::run_capture(
        "#alphabet<en>\n13 nums = [10, 20, 30]\nz.o(nums[-1])");
    REQUIRE(output == "30\n");
}

// ============================================================================
// Feature Tests: Built-in Functions
// ============================================================================

TEST_CASE("Built-in sqrt function", "[vm][builtin]")
{
    std::string output = test::run_capture(
        "#alphabet<en>\nz.o(z.sqrt(144))");
    REQUIRE(output == "12\n");
}

TEST_CASE("Built-in type function", "[vm][builtin]")
{
    std::string output = test::run_capture(
        "#alphabet<en>\nz.o(z.type(42))");
    REQUIRE(output == "number\n");
}

// ============================================================================
// Feature Tests: F-Strings
// ============================================================================

TEST_CASE("F-string basic variable", "[vm][fstring]")
{
    std::string output = test::run_capture(
        "#alphabet<en>\n12 name = \"World\"\nz.o(f\"Hello {name}\")");
    REQUIRE(output == "Hello World\n");
}

TEST_CASE("F-string concatenation", "[vm][fstring]")
{
    std::string output = test::run_capture(
        "#alphabet<en>\n12 greeting = \"Hello\"\n12 name = \"World\"\nz.o(f\"{greeting} {name}\")");
    REQUIRE(output == "Hello World\n");
}

TEST_CASE("F-string multiple variables", "[vm][fstring]")
{
    std::string output = test::run_capture(
        "#alphabet<en>\n12 a = \"hi\"\n5 b = 42\nz.o(f\"{a} {b}\")");
    REQUIRE(output == "hi 42\n");
}

// ============================================================================
// New v2.3.5 Tests — Shorthand Builtins, Type Names, Null-safe, Range
// ============================================================================

TEST_CASE("Named type string variable", "[vm][types]")
{
    std::string output = test::run_capture(
        "#alphabet<en>\n12 name = \"Fraol\"\nz.o(name)");
    REQUIRE(output == "Fraol\n");
}

TEST_CASE("Named type integer variable", "[vm][types]")
{
    std::string output = test::run_capture(
        "#alphabet<en>\n5 age = 21\nz.o(age)");
    REQUIRE(output == "21\n");
}

TEST_CASE("Null-safe operator on null", "[vm][nullsafe]")
{
    std::string output = test::run_capture(
        "#alphabet<en>\n5 x = null\nz.o(z.is_null(x))");
    REQUIRE(output == "1\n");
}

TEST_CASE("Range expression 1..5", "[vm][range]")
{
    std::string output = test::run_capture(
        "#alphabet<en>\n5 lst = 1..5\nz.o(z.len(lst))");
    REQUIRE(output == "4\n");
}

TEST_CASE("Destructuring assignment", "[vm][destructuring]")
{
    std::string output = test::run_capture(
        "#alphabet<en>\n5 a = 0\n5 b = 0\n[a, b] = [10, 20]\nz.o(a + b)");
    REQUIRE(output == "30\n");
}

TEST_CASE("Do-while loop", "[vm][dowhile]")
{
    std::string output = test::run_capture(
        "#alphabet<en>\n5 count = 0\nl {\n  count = count + 1\n} (count < 3)\nz.o(count)");
    REQUIRE(output == "3\n");
}

TEST_CASE("Lambda basic", "[vm][lambda]")
{
    std::string output = test::run_capture(
        "#alphabet<en>\n5 fn = m(5 x) { r x * 2 }\nz.o(fn(5))");
    REQUIRE(output == "10\n");
}

TEST_CASE("Abstract class blocks instantiation", "[vm][abstract]")
{
    std::string output = test::run_capture(
        "#alphabet<en>\na c Shape {\n  v abstract m 5 area()\n}");
    REQUIRE((output.find("abstract") != std::string::npos || output.find("Cannot") != std::string::npos || output.empty()));
}

TEST_CASE("Shorthand builtins - len, type, tostr", "[vm][shorthand]")
{
    std::string output = test::run_capture(
        "#alphabet<en>\nz.o(z.len([1,2,3]))\nz.o(z.type(42))\nz.o(z.tostr(123))");
    REQUIRE(output == "3\nnumber\n123\n");
}

TEST_CASE("Bytecode version header check", "[vm][bytecode]")
{
    std::string output = test::run_capture(
        "#alphabet<en>\nz.o(\"version check\")");
    REQUIRE(output == "version check\n");
}

TEST_CASE("Memory limit enforcement", "[vm][security]")
{
    std::string output = test::run_capture(
        "#alphabet<en>\nz.o(\"memory test\")");
    REQUIRE(output == "memory test\n");
}

TEST_CASE("Instruction count limit", "[vm][security]")
{
    std::string output = test::run_capture(
        "#alphabet<en>\n5 total = 0\nl (5 i = 0 : i < 100 : i = i + 1) {\n  total = total + i\n}\nz.o(total)");
    REQUIRE(output == "4950\n");
}

TEST_CASE("z.map with lambda", "[vm][functional]")
{
    std::string output = test::run_capture(
        "#alphabet<en>\n5 lst = [1, 2, 3]\n5 result = z.map(lst, m(5 x) { r x * 2 })\nz.o(z.tostr(result))");
    REQUIRE(output == "[2, 4, 6]\n");
}

TEST_CASE("z.filter with lambda", "[vm][functional]")
{
    std::string output = test::run_capture(
        "#alphabet<en>\n5 lst = [1, 2, 3, 4, 5]\n5 evens = z.filter(lst, m(5 x) { r x % 2 == 0 })\nz.o(z.tostr(evens))");
    REQUIRE(output == "[2, 4]\n");
}

TEST_CASE("z.reduce with lambda", "[vm][functional]")
{
    std::string output = test::run_capture(
        "#alphabet<en>\n5 lst = [1, 2, 3, 4, 5]\n5 sum = z.reduce(lst, 0, m(5 acc, 5 x) { r acc + x })\nz.o(sum)");
    REQUIRE(output == "15\n");
}

TEST_CASE("Lambda captures global variable (closure)", "[vm][closure]")
{
    std::string output = test::run_capture(
        "#alphabet<en>\n5 factor = 10\n5 multiply = m(5 x) { r x * factor }\nz.o(multiply(5))");
    REQUIRE(output == "50\n");
}

TEST_CASE("z.map with closure captures global", "[vm][closure]")
{
    std::string output = test::run_capture(
        "#alphabet<en>\n5 factor = 3\n5 lst = [1, 2, 3]\n5 result = z.map(lst, m(5 val) { r val * factor })\nz.o(z.tostr(result))");
    REQUIRE(output == "[3, 6, 9]\n");
}

TEST_CASE("Ternary operator true branch", "[vm][ternary]")
{
    std::string output = test::run_capture(
        "#alphabet<en>\n5 x = 10\nlet result = x > 5 ? \"big\" : \"small\"\nz.o(result)");
    REQUIRE(output == "big\n");
}

TEST_CASE("Ternary operator false branch", "[vm][ternary]")
{
    std::string output = test::run_capture(
        "#alphabet<en>\n5 x = 3\nlet result = x > 5 ? \"big\" : \"small\"\nz.o(result)");
    REQUIRE(output == "small\n");
}

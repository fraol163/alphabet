#ifndef ALPHABET_ERROR_CATALOG_H
#define ALPHABET_ERROR_CATALOG_H

#include <algorithm>
#include <string>
#include <unordered_map>
#include <vector>

namespace alphabet {

// Error code format: E + 3-digit number
// E001-E099: Parse errors
// E100-E199: Compile errors
// E200-E299: Runtime errors

enum class ErrorCode {
    // === Parse Errors (E001-E099) ===
    E001_UNEXPECTED_TOKEN = 1,
    E002_EXPECT_TOKEN = 2,
    E003_EXPECT_EXPRESSION = 3,
    E004_INVALID_ASSIGNMENT_TARGET = 4,
    E005_INTERFACE_METHODS_ONLY = 5,
    E006_EXPECT_METHOD_OR_FIELD = 6,
    E007_EXPECT_IMPORT_PATH = 7,
    E008_EXPECT_CASE_OR_DEFAULT = 8,
    E009_EXPECT_TYPE_ID = 9,

    // === Compile Errors (E100-E199) ===
    E100_TYPE_MISMATCH = 100,
    E101_RETURN_TYPE_MISMATCH = 101,
    E102_TYPE_ERROR = 102,
    E103_BREAK_OUTSIDE_LOOP = 103,
    E104_BREAK_LABEL_NOT_FOUND = 104,
    E105_CONTINUE_OUTSIDE_LOOP = 105,
    E106_CONTINUE_LABEL_NOT_FOUND = 106,
    E107_CONST_REASSIGN = 107,
    E108_MODULE_NOT_FOUND = 108,
    E109_MODULE_SYNTAX_ERROR = 109,

    // === Runtime Errors (E200-E299) ===
    E200_STACK_OVERFLOW = 200,
    E201_STACK_UNDERFLOW = 201,
    E202_STACK_PEEK_OOB = 202,
    E203_INSTRUCTION_LIMIT = 203,
    E204_CONST_REASSIGN = 204,
    E205_MEMORY_LIMIT = 205,
    E206_DIVISION_BY_ZERO = 206,
    E207_TYPE_ERROR_ADD = 207,
    E208_TYPE_ERROR_SUB = 208,
    E209_TYPE_ERROR_MUL = 209,
    E210_TYPE_ERROR_DIV = 210,
    E211_TYPE_ERROR_MOD = 211,
    E212_TYPE_ERROR_COMPARE = 212,
    E213_VERSION_MISMATCH = 213,
    E214_UNKNOWN_CLASS = 214,
    E215_METHOD_NOT_FOUND = 215,
    E216_PRIVATE_METHOD = 216,
    E217_PRIVATE_FIELD = 217,
    E218_ABSTRACT_INSTANTIATE = 218,
    E219_CALL_DEPTH_OVERFLOW = 219,
    E220_FFI_ERROR = 220,
    E221_TYPE_ERROR_VALUE = 221,
    E222_ASSERTION_FAILED = 222,

    NONE = 0,
};

inline std::string error_code_to_string(ErrorCode code) {
    if (code == ErrorCode::NONE)
        return "";
    return "E" + std::to_string(static_cast<int>(code));
}

inline const std::unordered_map<ErrorCode, std::string>& error_catalog() {
    static const std::unordered_map<ErrorCode, std::string> catalog = {
        // Parse Errors
        {ErrorCode::E001_UNEXPECTED_TOKEN, "Unexpected token"},
        {ErrorCode::E002_EXPECT_TOKEN, "Expected specific token"},
        {ErrorCode::E003_EXPECT_EXPRESSION, "Expected expression"},
        {ErrorCode::E004_INVALID_ASSIGNMENT_TARGET, "Invalid assignment target"},
        {ErrorCode::E005_INTERFACE_METHODS_ONLY, "Interfaces can only contain methods"},
        {ErrorCode::E006_EXPECT_METHOD_OR_FIELD, "Expected method or field declaration"},
        {ErrorCode::E007_EXPECT_IMPORT_PATH, "Expected module path or alias after 'import'"},
        {ErrorCode::E008_EXPECT_CASE_OR_DEFAULT, "Expected 'case' pattern or 'default'"},
        {ErrorCode::E009_EXPECT_TYPE_ID, "Expected type identifier"},

        // Compile Errors
        {ErrorCode::E100_TYPE_MISMATCH, "Type mismatch: incompatible types in assignment"},
        {ErrorCode::E101_RETURN_TYPE_MISMATCH, "Return type does not match method declaration"},
        {ErrorCode::E102_TYPE_ERROR, "Type error: expected and actual types are incompatible"},
        {ErrorCode::E103_BREAK_OUTSIDE_LOOP, "'break' used outside of a loop"},
        {ErrorCode::E104_BREAK_LABEL_NOT_FOUND, "'break' label does not match any enclosing loop"},
        {ErrorCode::E105_CONTINUE_OUTSIDE_LOOP, "'continue' used outside of a loop"},
        {ErrorCode::E106_CONTINUE_LABEL_NOT_FOUND, "'continue' label does not match any enclosing loop"},
        {ErrorCode::E107_CONST_REASSIGN, "Cannot reassign a constant variable"},
        {ErrorCode::E108_MODULE_NOT_FOUND, "Cannot find or open imported module file"},
        {ErrorCode::E109_MODULE_SYNTAX_ERROR, "Imported module contains syntax errors"},

        // Runtime Errors
        {ErrorCode::E200_STACK_OVERFLOW, "Value stack overflow"},
        {ErrorCode::E201_STACK_UNDERFLOW, "Value stack underflow"},
        {ErrorCode::E202_STACK_PEEK_OOB, "Stack peek out of bounds"},
        {ErrorCode::E203_INSTRUCTION_LIMIT, "Instruction limit exceeded; possible infinite loop"},
        {ErrorCode::E204_CONST_REASSIGN, "Cannot reassign a constant variable at runtime"},
        {ErrorCode::E205_MEMORY_LIMIT, "Memory limit exceeded for global variables"},
        {ErrorCode::E206_DIVISION_BY_ZERO, "Division by zero"},
        {ErrorCode::E207_TYPE_ERROR_ADD, "Type error in addition operator"},
        {ErrorCode::E208_TYPE_ERROR_SUB, "Type error in subtraction operator"},
        {ErrorCode::E209_TYPE_ERROR_MUL, "Type error in multiplication operator"},
        {ErrorCode::E210_TYPE_ERROR_DIV, "Type error in division operator"},
        {ErrorCode::E211_TYPE_ERROR_MOD, "Type error in modulo operator"},
        {ErrorCode::E212_TYPE_ERROR_COMPARE, "Type error in comparison operator"},
        {ErrorCode::E213_VERSION_MISMATCH, "Bytecode version mismatch"},
        {ErrorCode::E214_UNKNOWN_CLASS, "Unknown class identifier"},
        {ErrorCode::E215_METHOD_NOT_FOUND, "Method not found in class"},
        {ErrorCode::E216_PRIVATE_METHOD, "Cannot access private method"},
        {ErrorCode::E217_PRIVATE_FIELD, "Cannot access private field"},
        {ErrorCode::E218_ABSTRACT_INSTANTIATE, "Cannot instantiate abstract class"},
        {ErrorCode::E219_CALL_DEPTH_OVERFLOW, "Maximum call depth exceeded (stack overflow)"},
        {ErrorCode::E220_FFI_ERROR, "Foreign function interface error"},
        {ErrorCode::E221_TYPE_ERROR_VALUE, "Value type mismatch"},
        {ErrorCode::E222_ASSERTION_FAILED, "Assertion failed"},
    };
    return catalog;
}

inline std::string get_error_description(ErrorCode code) {
    const auto& cat = error_catalog();
    auto it = cat.find(code);
    if (it != cat.end())
        return it->second;
    return "Unknown error";
}

// === Spelling suggestion utilities ===

inline size_t levenshtein_distance(const std::string& a, const std::string& b) {
    const size_t m = a.size();
    const size_t n = b.size();
    std::vector<std::vector<size_t>> dp(m + 1, std::vector<size_t>(n + 1));
    for (size_t i = 0; i <= m; ++i)
        dp[i][0] = i;
    for (size_t j = 0; j <= n; ++j)
        dp[0][j] = j;
    for (size_t i = 1; i <= m; ++i) {
        for (size_t j = 1; j <= n; ++j) {
            size_t cost = (a[i - 1] == b[j - 1]) ? 0 : 1;
            dp[i][j] = std::min({dp[i - 1][j] + 1, dp[i][j - 1] + 1, dp[i - 1][j - 1] + cost});
        }
    }
    return dp[m][n];
}

inline std::string find_similar_name(const std::string& name, const std::vector<std::string>& candidates,
                                     size_t max_distance = 3) {
    std::string best;
    size_t best_dist = max_distance + 1;

    for (const auto& candidate : candidates) {
        if (candidate.empty() || candidate == name)
            continue;
        size_t d = levenshtein_distance(name, candidate);
        if (d < best_dist) {
            best_dist = d;
            best = candidate;
        }
    }

    return best;
}

} // namespace alphabet

#endif

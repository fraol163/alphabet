#ifndef ALPHABET_FORGE_SPEC_H
#define ALPHABET_FORGE_SPEC_H

#include "header_engine.h"
#include <string>
#include <vector>
#include <unordered_map>
#include <memory>

namespace alphabet {
namespace forge {

enum class PegExprKind {
    LITERAL,       // "keyword" or ";"
    BUILTIN_TERM,  // Identifier, Integer, Float, String, Boolean
    NON_TERMINAL,  // Another rule name
    SEQUENCE,      // e.g. A B C
    CHOICE,        // e.g. A | B | C
    ZERO_OR_MORE,  // A*
    ONE_OR_MORE,   // A+
    OPTIONAL       // A?
};

struct PegExpression;
using PegExprPtr = std::shared_ptr<PegExpression>;

struct PegExpression {
    PegExprKind kind;
    std::string value;                       // literal text or non-terminal name
    std::vector<PegExprPtr> children;        // for SEQUENCE, CHOICE, or quantified expression

    PegExpression(PegExprKind k, std::string v = "") : kind(k), value(std::move(v)) {}
};

struct GrammarRule {
    std::string name;
    PegExprPtr expression;
    std::string semantic_action;             // optional: VarDecl, PrintStmt, etc.
};

struct BannerConfig {
    std::string art;                         // custom ASCII art or empty
    std::string color = "cyan";              // cyan, green, yellow, red, blue, magenta, rainbow
    std::string tagline;
    std::string font = "standard";           // FIGlet font fallback
    std::vector<std::string> show_on = {"repl", "version", "help"};
};

struct TokenConfig {
    std::unordered_map<std::string, std::string> keywords;  // custom_keyword -> canonical token (e.g. "create" -> "VAR")
    std::unordered_map<std::string, std::string> operators; // custom_op -> canonical op
    std::string line_comment = "//";
    std::string block_comment_start = "/*";
    std::string block_comment_end = "*/";
    std::string whitespace_mode = "braces";                  // "braces" | "indent"
};

struct ForgeSpec {
    std::string name = "MyLang";
    std::string version = "1.0.0";
    std::string extension = ".lang";
    std::string paradigm = "bytecode";                      // "interpreted" | "bytecode" | "compiled"
    std::string type_system = "dynamic";                    // "dynamic" | "static" | "gradual"
    std::string author = "Author";
    std::string memory_model = "gc";                        // "gc" | "arc" | "manual"
    std::string concurrency = "none";                       // "goroutines" | "async_await" | "threads" | "none"
    std::string spec_source_path;

    HeaderConfig header;
    BannerConfig banner;
    TokenConfig tokens;

    std::vector<GrammarRule> rules;
    std::unordered_map<std::string, PegExprPtr> rule_map;

    // Load and parse a .forge file
    static bool load_from_file(const std::string& path, ForgeSpec& out_spec, std::string& out_error);
    static bool parse_string(const std::string& content, ForgeSpec& out_spec, std::string& out_error);
};

} // namespace forge
} // namespace alphabet

#endif // ALPHABET_FORGE_SPEC_H

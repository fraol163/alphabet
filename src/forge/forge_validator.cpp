#include "forge_validator.h"
#include <iostream>
#include <iomanip>
#include <unordered_set>
#include <queue>
#include <regex>

#ifdef ERROR
#undef ERROR
#endif

namespace alphabet {
namespace forge {

namespace {

void collect_non_terminals(const PegExprPtr& expr, std::vector<std::string>& refs) {
    if (!expr) return;
    if (expr->kind == PegExprKind::NON_TERMINAL) {
        refs.push_back(expr->value);
    }
    for (const auto& child : expr->children) {
        collect_non_terminals(child, refs);
    }
}

// Determines leading non-terminals that can be expanded immediately without consuming tokens
void collect_leading_non_terminals(const PegExprPtr& expr, std::vector<std::string>& leading, bool& can_match_empty) {
    if (!expr) {
        can_match_empty = true;
        return;
    }
    switch (expr->kind) {
        case PegExprKind::LITERAL:
        case PegExprKind::BUILTIN_TERM:
            can_match_empty = false;
            break;
        case PegExprKind::NON_TERMINAL:
            leading.push_back(expr->value);
            can_match_empty = false; // assumes non-terminals normally consume tokens
            break;
        case PegExprKind::OPTIONAL:
        case PegExprKind::ZERO_OR_MORE:
            if (!expr->children.empty()) {
                bool dummy = false;
                collect_leading_non_terminals(expr->children[0], leading, dummy);
            }
            can_match_empty = true;
            break;
        case PegExprKind::ONE_OR_MORE:
            if (!expr->children.empty()) {
                collect_leading_non_terminals(expr->children[0], leading, can_match_empty);
            }
            break;
        case PegExprKind::CHOICE: {
            bool any_can_be_empty = false;
            for (const auto& child : expr->children) {
                bool branch_empty = false;
                collect_leading_non_terminals(child, leading, branch_empty);
                if (branch_empty) any_can_be_empty = true;
            }
            can_match_empty = any_can_be_empty;
            break;
        }
        case PegExprKind::SEQUENCE: {
            can_match_empty = true;
            for (const auto& child : expr->children) {
                bool child_empty = false;
                collect_leading_non_terminals(child, leading, child_empty);
                if (!child_empty) {
                    can_match_empty = false;
                    break;
                }
            }
            break;
        }
    }
}

} // namespace

ValidationResult ForgeValidator::validate(const ForgeSpec& spec) {
    ValidationResult result;

    // 1. Metadata Validation
    if (spec.name.empty()) {
        result.add_issue(IssueSeverity::ERR, "metadata", "name",
                         "Language name is empty.", "Specify 'language <Name> { ... }'");
    } else {
        bool valid_ident = true;
        if (!std::isalpha(static_cast<unsigned char>(spec.name[0])) && spec.name[0] != '_') valid_ident = false;
        for (char c : spec.name) {
            if (!std::isalnum(static_cast<unsigned char>(c)) && c != '_') {
                valid_ident = false;
                break;
            }
        }
        if (!valid_ident) {
            result.add_issue(IssueSeverity::ERR, "metadata", "name",
                             "Language name '" + spec.name + "' contains invalid characters.",
                             "Use alphanumeric characters and underscores only.");
        }
    }

    if (spec.version.empty()) {
        result.add_issue(IssueSeverity::WARNING, "metadata", "version",
                         "Language version is empty.", "Specify version: \"1.0.0\"");
    } else {
        std::regex ver_regex(R"(^\d+(\.\d+)*$)");
        if (!std::regex_match(spec.version, ver_regex)) {
            result.add_issue(IssueSeverity::WARNING, "metadata", "version",
                             "Version string '" + spec.version + "' does not follow semantic versioning.",
                             "Use format like '1.0' or '1.0.0'.");
        }
    }

    if (spec.extension.empty()) {
        result.add_issue(IssueSeverity::ERR, "metadata", "extension",
                         "File extension is empty.", "Specify extension: \".lang\"");
    } else if (spec.extension[0] != '.') {
        result.add_issue(IssueSeverity::ERR, "metadata", "extension",
                         "File extension '" + spec.extension + "' must start with a leading dot ('.').",
                         "Change extension to '." + spec.extension + "'");
    }

    if (spec.paradigm != "bytecode" && spec.paradigm != "interpreted" && spec.paradigm != "compiled") {
        result.add_issue(IssueSeverity::WARNING, "metadata", "paradigm",
                         "Unknown execution paradigm '" + spec.paradigm + "'.",
                         "Supported paradigms: 'bytecode', 'interpreted', 'compiled'.");
    }

    if (spec.type_system != "dynamic" && spec.type_system != "static" && spec.type_system != "gradual") {
        result.add_issue(IssueSeverity::WARNING, "metadata", "types",
                         "Unknown type system '" + spec.type_system + "'.",
                         "Supported type systems: 'dynamic', 'static', 'gradual'.");
    }

    // 2. Header Configuration Validation
    if (spec.header.style == HeaderStyle::PREFIX) {
        if (spec.header.prefix.empty()) {
            result.add_issue(IssueSeverity::ERR, "header", "prefix",
                             "Header prefix is empty for PREFIX style.",
                             "Specify prefix like '#" + spec.name + "'");
        } else if (spec.header.prefix[0] != '#') {
            result.add_issue(IssueSeverity::WARNING, "header", "prefix",
                             "Prefix header '" + spec.header.prefix + "' typically starts with '#'.",
                             "Consider using '#" + spec.header.prefix + "'");
        }
    } else if (spec.header.style == HeaderStyle::PRAGMA) {
        if (spec.header.prefix.empty()) {
            result.add_issue(IssueSeverity::ERR, "header", "prefix",
                             "Header prefix is empty for PRAGMA style.",
                             "Specify prefix like '@" + spec.name + "'");
        } else if (spec.header.prefix[0] != '@') {
            result.add_issue(IssueSeverity::WARNING, "header", "prefix",
                             "Pragma header prefix '" + spec.header.prefix + "' typically starts with '@'.",
                             "Consider using '@" + spec.header.prefix + "'");
        }
    } else if (spec.header.style == HeaderStyle::SHEBANG) {
        if (spec.header.prefix.rfind("#!", 0) != 0) {
            result.add_issue(IssueSeverity::WARNING, "header", "prefix",
                             "Shebang prefix '" + spec.header.prefix + "' should start with '#!'.",
                             "Specify prefix: '#!/usr/bin/env " + spec.name + "'");
        }
    }

    // 3. Token Configuration Validation
    static const std::unordered_set<std::string> known_canonical = {
        "VAR", "PRINT", "IF", "ELSE", "WHILE", "FOR", "FUNCTION", "FN", "DEF",
        "RETURN", "BREAK", "CONTINUE", "TRUE", "FALSE", "NIL", "NULL"
    };

    std::unordered_map<std::string, std::string> seen_keywords;
    for (const auto& [kw, target] : spec.tokens.keywords) {
        if (kw.empty()) {
            result.add_issue(IssueSeverity::ERR, "tokens", "keyword",
                             "Empty keyword definition found in token config.");
            continue;
        }
        if (seen_keywords.count(kw)) {
            result.add_issue(IssueSeverity::WARNING, "tokens", kw,
                             "Keyword '" + kw + "' defined multiple times; mapped to '" + target + "'.",
                             "Remove duplicate keyword declaration.");
        }
        seen_keywords[kw] = target;

        if (!known_canonical.count(target)) {
            result.add_issue(IssueSeverity::WARNING, "tokens", kw,
                             "Custom keyword '" + kw + "' maps to unrecognized canonical token '" + target + "'.",
                             "Recognized tokens: VAR, PRINT, IF, ELSE, WHILE, FOR, FUNCTION, RETURN, TRUE, FALSE, NIL.");
        }
    }

    if (spec.tokens.line_comment.empty()) {
        result.add_issue(IssueSeverity::INFO, "tokens", "line_comment",
                         "No line_comment prefix defined.",
                         "Consider setting line_comment to '//' or '#'.");
    }

    if (!spec.tokens.block_comment_start.empty() && spec.tokens.block_comment_end.empty()) {
        result.add_issue(IssueSeverity::ERR, "tokens", "block_comment",
                         "block_comment_start defined without matching block_comment_end.");
    } else if (spec.tokens.block_comment_start.empty() && !spec.tokens.block_comment_end.empty()) {
        result.add_issue(IssueSeverity::ERR, "tokens", "block_comment",
                         "block_comment_end defined without matching block_comment_start.");
    }

    // 4. Grammar Rule Validation
    if (spec.rules.empty()) {
        result.add_issue(IssueSeverity::INFO, "grammar", "rules",
                         "No grammar rules specified; using default statement grammar.",
                         "Add 'grammar { rule Program = ... }' to customize syntax.");
        return result;
    }

    std::unordered_set<std::string> defined_rules;
    std::unordered_map<std::string, std::vector<std::string>> graph;

    for (const auto& rule : spec.rules) {
        if (defined_rules.count(rule.name)) {
            result.add_issue(IssueSeverity::WARNING, "grammar", rule.name,
                             "Rule '" + rule.name + "' is defined multiple times.",
                             "Combine alternatives using choice '|'.");
        }
        defined_rules.insert(rule.name);
    }

    static const std::unordered_set<std::string> builtin_non_terminals = {
        "Expr", "Expression", "IfStmt", "ExprStmt", "WhileStmt", "ForStmt",
        "ReturnStmt", "Block", "BlockStmt", "Statement", "VarDecl", "PrintStmt"
    };

    // Collect reference graph & check for undefined non-terminals
    for (const auto& rule : spec.rules) {
        std::vector<std::string> refs;
        collect_non_terminals(rule.expression, refs);
        graph[rule.name] = refs;

        for (const auto& ref : refs) {
            if (!defined_rules.count(ref) && !builtin_non_terminals.count(ref)) {
                result.add_issue(IssueSeverity::ERR, "grammar", rule.name,
                                 "Rule '" + rule.name + "' references undefined non-terminal '" + ref + "'.",
                                 "Define 'rule " + ref + " = ...' or correct rule name spelling.");
            }
        }
    }

    // Direct Left-Recursion Check
    for (const auto& rule : spec.rules) {
        std::vector<std::string> leading;
        bool dummy = false;
        collect_leading_non_terminals(rule.expression, leading, dummy);
        for (const auto& l : leading) {
            if (l == rule.name) {
                result.add_issue(IssueSeverity::WARNING, "grammar", rule.name,
                                 "Direct left-recursion detected in rule '" + rule.name + "'.",
                                 "PEG parsers may loop infinitely on left recursion. Rewrite using repetition, e.g. '" +
                                 rule.name + " = Atom (Op Atom)*'.");
                break;
            }
        }
    }

    // Reachability Analysis
    std::string start_rule = defined_rules.count("Program") ? "Program" : spec.rules[0].name;
    std::unordered_set<std::string> reachable;
    std::queue<std::string> queue;
    queue.push(start_rule);
    reachable.insert(start_rule);

    while (!queue.empty()) {
        std::string current = queue.front();
        queue.pop();
        for (const auto& next : graph[current]) {
            if (defined_rules.count(next) && !reachable.count(next)) {
                reachable.insert(next);
                queue.push(next);
            }
        }
    }

    for (const auto& rule : spec.rules) {
        if (!reachable.count(rule.name)) {
            result.add_issue(IssueSeverity::INFO, "grammar", rule.name,
                             "Rule '" + rule.name + "' is defined but unreachable from entry rule '" + start_rule + "'.",
                             "Reference '" + rule.name + "' in '" + start_rule + "' or another reachable rule.");
        }
    }

    return result;
}

ValidationResult ForgeValidator::validate_file(const std::string& path, ForgeSpec& out_spec) {
    std::string err;
    if (!ForgeSpec::load_from_file(path, out_spec, err)) {
        ValidationResult res;
        res.add_issue(IssueSeverity::ERR, "syntax", path,
                      "Failed to parse specification file: " + err,
                      "Check .forge syntax, braces, quotes, and rule declarations.");
        return res;
    }
    return validate(out_spec);
}

void ForgeValidator::print_report(const ValidationResult& result, const ForgeSpec& spec, std::ostream& out) {
    out << "\n=======================================================\n";
    out << " Alphabet Forge: Grammar & Specification Validator\n";
    out << " Target: " << spec.name << " v" << spec.version << " (" << spec.extension << ")\n";
    out << "=======================================================\n\n";

    if (result.issues.empty()) {
        out << "  \033[1;32m[PASS]\033[0m No issues found!\n";
        out << "  Specification is 100% valid and ready for forging.\n\n";
        return;
    }

    for (const auto& issue : result.issues) {
        switch (issue.severity) {
            case IssueSeverity::ERR:
                out << "  \033[1;31m[ERROR]\033[0m ";
                break;
            case IssueSeverity::WARNING:
                out << "  \033[1;33m[WARN]\033[0m  ";
                break;
            case IssueSeverity::INFO:
                out << "  \033[1;36m[INFO]\033[0m  ";
                break;
        }
        out << "[" << issue.category << "] ";
        if (!issue.item_name.empty()) out << issue.item_name << ": ";
        out << issue.message << "\n";
        if (!issue.suggestion.empty()) {
            out << "          -> Suggestion: " << issue.suggestion << "\n";
        }
    }

    out << "\n-------------------------------------------------------\n";
    out << " Summary: ";
    if (result.error_count == 0) {
        out << "\033[1;32mVALID\033[0m";
    } else {
        out << "\033[1;31mINVALID (" << result.error_count << " error" << (result.error_count > 1 ? "s" : "") << ")\033[0m";
    }
    out << " | " << result.warning_count << " warning" << (result.warning_count != 1 ? "s" : "");
    out << " | " << result.info_count << " info\n";
    out << "-------------------------------------------------------\n\n";
}

} // namespace forge
} // namespace alphabet

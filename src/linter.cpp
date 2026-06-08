#include "linter.h"
#include <algorithm>
#include <sstream>

namespace alphabet {

// ---------------------------------------------------------------------------
// LintWarning::format
// ---------------------------------------------------------------------------
std::string LintWarning::format() const {
    std::ostringstream oss;
    oss << file << ":" << line << ": ";

    switch (kind) {
    case LintWarningKind::UnusedVariable:
        oss << "warning [W76]: ";
        break;
    case LintWarningKind::UndefinedVariable:
        oss << "info [I71]: ";
        break;
    case LintWarningKind::UnreachableCode:
        oss << "warning [W77]: ";
        break;
    case LintWarningKind::EmptyBlock:
        oss << "warning [W78]: ";
        break;
    case LintWarningKind::DuplicateFunction:
        oss << "warning [W79]: ";
        break;
    }

    oss << message;
    return oss.str();
}

// ---------------------------------------------------------------------------
// LintVisitor ctor
// ---------------------------------------------------------------------------
LintVisitor::LintVisitor(const std::string& filename) : filename_(filename) {}

// ---------------------------------------------------------------------------
// Scope management
// ---------------------------------------------------------------------------
void LintVisitor::push_scope() {
    scopes_.emplace_back();
}

void LintVisitor::pop_scope() {
    if (scopes_.empty())
        return;

    // Report unused variables in this scope
    const auto& scope = scopes_.back();
    for (const auto& [name, line] : scope.defined) {
        if (scope.used.find(name) == scope.used.end()) {
            // Don't warn about 'this' or params starting with _
            if (name == "this" || (!name.empty() && name[0] == '_'))
                continue;
            warn(LintWarningKind::UnusedVariable, "unused variable '" + name + "'", line);
        }
    }

    scopes_.pop_back();
}

void LintVisitor::define_var(const std::string& name, size_t line) {
    if (!scopes_.empty()) {
        scopes_.back().defined[name] = line;
        scopes_.back().defined_lines[name] = line;
    }
}

void LintVisitor::use_var(const std::string& name, size_t line) {
    // Mark as used in the innermost scope where it is defined
    for (auto it = scopes_.rbegin(); it != scopes_.rend(); ++it) {
        if (it->defined.find(name) != it->defined.end()) {
            it->used.insert(name);
            return;
        }
    }

    // Check if it's a known function name (functions are global)
    if (function_names_.find(name) != function_names_.end())
        return;

    // Not found in any scope - might be a built-in or undefined
    // Only warn if we have at least one scope (i.e., inside a function/program)
    // and the name is not a common built-in
    static const std::set<std::string> builtins = {
        "z",     "print", "len",    "str",    "int",         "float",    "type",    "range",     "input",     "abs",
        "max",   "min",   "append", "remove", "insert",      "keys",     "values",  "sort",      "reverse",   "join",
        "split", "upper", "lower",  "trim",   "starts_with", "contains", "replace", "to_string", "to_number", "is_null",
    };

    if (!scopes_.empty() && builtins.find(name) == builtins.end()) {
        warn(LintWarningKind::UndefinedVariable, "variable '" + name + "' may be undefined", line);
    }
}

bool LintVisitor::is_defined(const std::string& name) const {
    for (auto it = scopes_.rbegin(); it != scopes_.rend(); ++it) {
        if (it->defined.find(name) != it->defined.end())
            return true;
    }
    return false;
}

// ---------------------------------------------------------------------------
// Helper: warn
// ---------------------------------------------------------------------------
void LintVisitor::warn(LintWarningKind kind, const std::string& msg, size_t line) {
    warnings_.push_back({kind, msg, filename_, line});
}

// ---------------------------------------------------------------------------
// Public entry point
// ---------------------------------------------------------------------------
std::vector<LintWarning> LintVisitor::lint(const std::vector<StmtPtr>& statements) {
    warnings_.clear();
    function_names_.clear();
    function_name_lines_.clear();
    scopes_.clear();

    // First pass: collect top-level function names for duplicate detection
    for (const auto& stmt : statements) {
        if (auto* fn = dynamic_cast<const FunctionStmt*>(stmt.get())) {
            std::string name(fn->name.lexeme);
            auto it = function_names_.find(name);
            if (it != function_names_.end()) {
                warn(LintWarningKind::DuplicateFunction, "duplicate function '" + name + "'", fn->name.line);
            }
            function_names_.insert(name);
            function_name_lines_[name] = fn->name.line;
        }
    }

    // Second pass: walk the AST
    push_scope(); // global scope

    for (const auto& stmt : statements) {
        visit_stmt(stmt);
    }

    pop_scope(); // global scope - reports unused global variables

    return warnings_;
}

// ---------------------------------------------------------------------------
// Statement visitors
// ---------------------------------------------------------------------------
void LintVisitor::visit_stmt(const StmtPtr& stmt) {
    if (!stmt)
        return;

    if (auto* s = dynamic_cast<const ExpressionStmt*>(stmt.get())) {
        visit_expression_stmt(*s);
    } else if (auto* s = dynamic_cast<const VarStmt*>(stmt.get())) {
        visit_var_stmt(*s);
    } else if (auto* s = dynamic_cast<const Block*>(stmt.get())) {
        visit_block(*s);
    } else if (auto* s = dynamic_cast<const IfStmt*>(stmt.get())) {
        visit_if_stmt(*s);
    } else if (auto* s = dynamic_cast<const LoopStmt*>(stmt.get())) {
        visit_loop_stmt(*s);
    } else if (auto* s = dynamic_cast<const ForStmt*>(stmt.get())) {
        visit_for_stmt(*s);
    } else if (auto* s = dynamic_cast<const TryStmt*>(stmt.get())) {
        visit_try_stmt(*s);
    } else if (auto* s = dynamic_cast<const ReturnStmt*>(stmt.get())) {
        visit_return_stmt(*s);
    } else if (auto* s = dynamic_cast<const BreakStmt*>(stmt.get())) {
        visit_break_stmt(*s);
    } else if (auto* s = dynamic_cast<const ContinueStmt*>(stmt.get())) {
        visit_continue_stmt(*s);
    } else if (auto* s = dynamic_cast<const FunctionStmt*>(stmt.get())) {
        visit_function_stmt(*s);
    } else if (auto* s = dynamic_cast<const ClassStmt*>(stmt.get())) {
        visit_class_stmt(*s);
    } else if (auto* s = dynamic_cast<const ImportStmt*>(stmt.get())) {
        visit_import_stmt(*s);
    } else if (auto* s = dynamic_cast<const ExportStmt*>(stmt.get())) {
        visit_export_stmt(*s);
    } else if (auto* s = dynamic_cast<const MatchStmt*>(stmt.get())) {
        visit_match_stmt(*s);
    }
}

void LintVisitor::visit_expression_stmt(const ExpressionStmt& stmt) {
    visit_expr(stmt.expression);
}

void LintVisitor::visit_var_stmt(const VarStmt& stmt) {
    if (stmt.initializer) {
        visit_expr(stmt.initializer);
    }
    define_var(std::string(stmt.name.lexeme), stmt.name.line);
}

void LintVisitor::visit_block(const Block& block) {
    if (block.statements.empty()) {
        // We need a line number; use 0 as a sentinel
        // (empty blocks at the top level are unusual)
        warn(LintWarningKind::EmptyBlock, "empty block", 0);
    }

    push_scope();
    for (const auto& stmt : block.statements) {
        visit_stmt(stmt);
    }
    pop_scope();
}

void LintVisitor::visit_if_stmt(const IfStmt& stmt) {
    visit_expr(stmt.condition);

    if (auto* block = dynamic_cast<const Block*>(stmt.then_branch.get())) {
        if (block->statements.empty()) {
            warn(LintWarningKind::EmptyBlock, "empty then-block in if statement", 0);
        }
    }
    visit_stmt(stmt.then_branch);

    if (stmt.else_branch) {
        if (auto* block = dynamic_cast<const Block*>(stmt.else_branch.get())) {
            if (block->statements.empty()) {
                warn(LintWarningKind::EmptyBlock, "empty else-block in if statement", 0);
            }
        }
        visit_stmt(stmt.else_branch);
    }
}

void LintVisitor::visit_loop_stmt(const LoopStmt& stmt) {
    if (stmt.condition)
        visit_expr(stmt.condition);

    if (auto* block = dynamic_cast<const Block*>(stmt.body.get())) {
        if (block->statements.empty()) {
            warn(LintWarningKind::EmptyBlock, "empty loop body", 0);
        }
    }
    visit_stmt(stmt.body);
}

void LintVisitor::visit_for_stmt(const ForStmt& stmt) {
    push_scope();

    if (stmt.initializer)
        visit_stmt(stmt.initializer);
    if (stmt.condition)
        visit_expr(stmt.condition);
    if (stmt.increment)
        visit_expr(stmt.increment);

    if (auto* block = dynamic_cast<const Block*>(stmt.body.get())) {
        if (block->statements.empty()) {
            warn(LintWarningKind::EmptyBlock, "empty for-loop body", 0);
        }
    }
    visit_stmt(stmt.body);

    pop_scope();
}

void LintVisitor::visit_try_stmt(const TryStmt& stmt) {
    // Visit try block
    if (stmt.try_block.statements.empty()) {
        warn(LintWarningKind::EmptyBlock, "empty try block", 0);
    }
    push_scope();
    for (const auto& s : stmt.try_block.statements) {
        visit_stmt(s);
    }
    pop_scope();

    // Visit handle block with exception variable
    if (stmt.handle_block.statements.empty()) {
        warn(LintWarningKind::EmptyBlock, "empty handle block", 0);
    }
    push_scope();
    define_var(std::string(stmt.exception_var.lexeme), stmt.exception_var.line);
    for (const auto& s : stmt.handle_block.statements) {
        visit_stmt(s);
    }
    pop_scope();
}

void LintVisitor::visit_return_stmt(const ReturnStmt& stmt) {
    if (stmt.value)
        visit_expr(stmt.value);
}

void LintVisitor::visit_break_stmt(const BreakStmt& /*stmt*/) {}
void LintVisitor::visit_continue_stmt(const ContinueStmt& /*stmt*/) {}

void LintVisitor::visit_function_stmt(const FunctionStmt& stmt) {
    // New scope for function body
    push_scope();

    // Parameters are "defined" variables
    for (const auto& param : stmt.params) {
        define_var(std::string(param.name.lexeme), param.name.line);
    }

    // Walk body and check for unreachable code
    check_unreachable(stmt.body);

    for (const auto& s : stmt.body) {
        visit_stmt(s);
    }

    pop_scope();
}

void LintVisitor::visit_class_stmt(const ClassStmt& stmt) {
    // Visit methods
    for (auto& method : stmt.methods) {
        // Class methods: push class context
        push_scope();
        define_var("this", stmt.name.line);
        for (const auto& param : method.params) {
            define_var(std::string(param.name.lexeme), param.name.line);
        }
        check_unreachable(method.body);
        for (const auto& s : method.body) {
            visit_stmt(s);
        }
        pop_scope();
    }
}

void LintVisitor::visit_import_stmt(const ImportStmt& /*stmt*/) {}

void LintVisitor::visit_export_stmt(const ExportStmt& /*stmt*/) {}

void LintVisitor::visit_match_stmt(const MatchStmt& stmt) {
    visit_expr(stmt.expression);

    for (const auto& c : stmt.cases) {
        if (c.pattern)
            visit_expr(c.pattern);
        if (c.body)
            visit_stmt(c.body);
    }

    if (stmt.default_case)
        visit_stmt(stmt.default_case);
}

// ---------------------------------------------------------------------------
// Unreachable code check
// ---------------------------------------------------------------------------
void LintVisitor::check_unreachable(const std::vector<StmtPtr>& stmts) {
    for (size_t i = 0; i + 1 < stmts.size(); ++i) {
        // Check if statement i is a return/break/continue
        bool is_terminal = false;
        if (dynamic_cast<const ReturnStmt*>(stmts[i].get()))
            is_terminal = true;
        if (dynamic_cast<const BreakStmt*>(stmts[i].get()))
            is_terminal = true;
        if (dynamic_cast<const ContinueStmt*>(stmts[i].get()))
            is_terminal = true;

        if (is_terminal) {
            // Everything after this in the same block is unreachable
            // Find the line of the next statement
            size_t next_line = 0;
            // Try to extract line from the next statement
            if (auto* es = dynamic_cast<const ExpressionStmt*>(stmts[i + 1].get())) {
                // We don't have line info on most expressions, report at return line
                next_line = 0;
            }
            // We use the return stmt line as fallback
            if (next_line == 0) {
                if (auto* ret = dynamic_cast<const ReturnStmt*>(stmts[i].get()))
                    next_line = ret->keyword.line;
                else if (auto* brk = dynamic_cast<const BreakStmt*>(stmts[i].get()))
                    next_line = brk->keyword.line;
                else if (auto* cont = dynamic_cast<const ContinueStmt*>(stmts[i].get()))
                    next_line = cont->keyword.line;
            }

            if (next_line > 0) {
                warn(LintWarningKind::UnreachableCode, "unreachable code after return/break/continue", next_line);
            }
            break; // Only report once per block
        }
    }
}

// ---------------------------------------------------------------------------
// Expression visitors
// ---------------------------------------------------------------------------
void LintVisitor::visit_expr(const ExprPtr& expr) {
    if (!expr)
        return;

    if (auto* e = dynamic_cast<const Binary*>(expr.get())) {
        visit_binary(*e);
    } else if (auto* e = dynamic_cast<const Unary*>(expr.get())) {
        visit_unary(*e);
    } else if (auto* e = dynamic_cast<const Literal*>(expr.get())) {
        visit_literal(*e);
    } else if (auto* e = dynamic_cast<const Grouping*>(expr.get())) {
        visit_grouping(*e);
    } else if (auto* e = dynamic_cast<const Variable*>(expr.get())) {
        visit_variable(*e);
    } else if (auto* e = dynamic_cast<const Assign*>(expr.get())) {
        visit_assign(*e);
    } else if (auto* e = dynamic_cast<const Logical*>(expr.get())) {
        visit_logical(*e);
    } else if (auto* e = dynamic_cast<const Call*>(expr.get())) {
        visit_call(*e);
    } else if (auto* e = dynamic_cast<const Get*>(expr.get())) {
        visit_get(*e);
    } else if (auto* e = dynamic_cast<const Set*>(expr.get())) {
        visit_set(*e);
    } else if (auto* e = dynamic_cast<const New*>(expr.get())) {
        visit_new_expr(*e);
    } else if (auto* e = dynamic_cast<const ListLiteral*>(expr.get())) {
        visit_list_literal(*e);
    } else if (auto* e = dynamic_cast<const MapLiteral*>(expr.get())) {
        visit_map_literal(*e);
    } else if (auto* e = dynamic_cast<const IndexExpr*>(expr.get())) {
        visit_index_expr(*e);
    } else if (auto* e = dynamic_cast<const IndexAssign*>(expr.get())) {
        visit_index_assign(*e);
    } else if (auto* e = dynamic_cast<const LambdaExpr*>(expr.get())) {
        visit_lambda_expr(*e);
    } else if (auto* e = dynamic_cast<const TernaryExpr*>(expr.get())) {
        visit_ternary_expr(*e);
    } else if (auto* e = dynamic_cast<const FString*>(expr.get())) {
        visit_fstring(*e);
    } else if (auto* e = dynamic_cast<const NullSafeGet*>(expr.get())) {
        visit_expr(e->obj);
    }
}

void LintVisitor::visit_binary(const Binary& expr) {
    visit_expr(expr.left);
    visit_expr(expr.right);
}

void LintVisitor::visit_unary(const Unary& expr) {
    visit_expr(expr.right);
}

void LintVisitor::visit_literal(const Literal& /*expr*/) {}

void LintVisitor::visit_grouping(const Grouping& expr) {
    visit_expr(expr.expression);
}

void LintVisitor::visit_variable(const Variable& expr) {
    use_var(std::string(expr.name.lexeme), expr.name.line);
}

void LintVisitor::visit_assign(const Assign& expr) {
    // The target is being assigned to, define if not already
    if (!is_defined(std::string(expr.name.lexeme))) {
        // It's an assignment to an undeclared name (could be global set)
        // Don't warn - might be intentional
    }
    visit_expr(expr.value);
}

void LintVisitor::visit_logical(const Logical& expr) {
    visit_expr(expr.left);
    visit_expr(expr.right);
}

void LintVisitor::visit_call(const Call& expr) {
    visit_expr(expr.callee);
    for (const auto& arg : expr.arguments) {
        visit_expr(arg);
    }
}

void LintVisitor::visit_get(const Get& expr) {
    visit_expr(expr.obj);
}

void LintVisitor::visit_set(const Set& expr) {
    visit_expr(expr.obj);
    visit_expr(expr.value);
}

void LintVisitor::visit_new_expr(const New& expr) {
    for (const auto& arg : expr.arguments) {
        visit_expr(arg);
    }
}

void LintVisitor::visit_list_literal(const ListLiteral& expr) {
    for (const auto& elem : expr.elements) {
        visit_expr(elem);
    }
}

void LintVisitor::visit_map_literal(const MapLiteral& expr) {
    for (size_t i = 0; i < expr.keys.size(); ++i) {
        visit_expr(expr.keys[i]);
        visit_expr(expr.values[i]);
    }
}

void LintVisitor::visit_index_expr(const IndexExpr& expr) {
    visit_expr(expr.obj);
    visit_expr(expr.index);
}

void LintVisitor::visit_index_assign(const IndexAssign& expr) {
    visit_expr(expr.obj);
    visit_expr(expr.index);
    visit_expr(expr.value);
}

void LintVisitor::visit_lambda_expr(const LambdaExpr& expr) {
    push_scope();
    for (const auto& param : expr.params) {
        define_var(std::string(param.name.lexeme), param.name.line);
    }
    for (const auto& s : expr.body) {
        visit_stmt(s);
    }
    pop_scope();
}

void LintVisitor::visit_ternary_expr(const TernaryExpr& expr) {
    visit_expr(expr.condition);
    visit_expr(expr.true_expr);
    visit_expr(expr.false_expr);
}

void LintVisitor::visit_fstring(const FString& expr) {
    for (const auto& part : expr.parts) {
        if (!part.is_literal && part.expr) {
            visit_expr(part.expr);
        }
    }
}

} // namespace alphabet

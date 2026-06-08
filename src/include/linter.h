#ifndef ALPHABET_LINTER_H
#define ALPHABET_LINTER_H

#include "alphabet_ast.h"
#include <map>
#include <set>
#include <string>
#include <vector>

namespace alphabet {

enum class LintWarningKind {
    UnusedVariable,    // W76
    UndefinedVariable, // I71
    UnreachableCode,   // W77
    EmptyBlock,        // W78
    DuplicateFunction, // W79
};

struct LintWarning {
    LintWarningKind kind;
    std::string message;
    std::string file;
    size_t line;

    std::string format() const;
};

// Scope for tracking variable definitions and usages
struct LintScope {
    std::map<std::string, size_t> defined;       // name -> line defined
    std::set<std::string> used;                  // names that were referenced
    std::map<std::string, size_t> defined_lines; // name -> line (for reporting)
};

class LintVisitor {
  public:
    explicit LintVisitor(const std::string& filename = "<stdin>");

    std::vector<LintWarning> lint(const std::vector<StmtPtr>& statements);

  private:
    std::string filename_;
    std::vector<LintWarning> warnings_;
    std::vector<LintScope> scopes_;        // scope stack
    std::set<std::string> function_names_; // track duplicate function names
    std::map<std::string, size_t> function_name_lines_;

    // Scope management
    void push_scope();
    void pop_scope();
    void define_var(const std::string& name, size_t line);
    void use_var(const std::string& name, size_t line);
    bool is_defined(const std::string& name) const;

    // AST traversal
    void visit_stmt(const StmtPtr& stmt);
    void visit_expr(const ExprPtr& expr);

    void visit_block(const Block& block);
    void visit_var_stmt(const VarStmt& stmt);
    void visit_expression_stmt(const ExpressionStmt& stmt);
    void visit_if_stmt(const IfStmt& stmt);
    void visit_loop_stmt(const LoopStmt& stmt);
    void visit_for_stmt(const ForStmt& stmt);
    void visit_try_stmt(const TryStmt& stmt);
    void visit_return_stmt(const ReturnStmt& stmt);
    void visit_break_stmt(const BreakStmt& stmt);
    void visit_continue_stmt(const ContinueStmt& stmt);
    void visit_function_stmt(const FunctionStmt& stmt);
    void visit_class_stmt(const ClassStmt& stmt);
    void visit_import_stmt(const ImportStmt& stmt);
    void visit_export_stmt(const ExportStmt& stmt);
    void visit_match_stmt(const MatchStmt& stmt);

    // Expression traversal
    void visit_binary(const Binary& expr);
    void visit_unary(const Unary& expr);
    void visit_literal(const Literal& expr);
    void visit_grouping(const Grouping& expr);
    void visit_variable(const Variable& expr);
    void visit_assign(const Assign& expr);
    void visit_logical(const Logical& expr);
    void visit_call(const Call& expr);
    void visit_get(const Get& expr);
    void visit_set(const Set& expr);
    void visit_new_expr(const New& expr);
    void visit_list_literal(const ListLiteral& expr);
    void visit_map_literal(const MapLiteral& expr);
    void visit_index_expr(const IndexExpr& expr);
    void visit_index_assign(const IndexAssign& expr);
    void visit_lambda_expr(const LambdaExpr& expr);
    void visit_ternary_expr(const TernaryExpr& expr);
    void visit_fstring(const FString& expr);

    // Helpers
    void check_unreachable(const std::vector<StmtPtr>& stmts);
    void warn(LintWarningKind kind, const std::string& msg, size_t line);
};

} // namespace alphabet

#endif

#ifndef ALPHABET_AST_H
#define ALPHABET_AST_H

#include "lexer.h"
#include <memory>
#include <optional>
#include <string>
#include <variant>
#include <vector>

namespace alphabet {

struct Expr;
struct Stmt;
struct VarStmt;

using ExprPtr = std::shared_ptr<Expr>;
using StmtPtr = std::shared_ptr<Stmt>;

struct Expr {
    virtual ~Expr() = default;
};

struct Binary : Expr {
    ExprPtr left;
    Token op;
    ExprPtr right;

    Binary(ExprPtr l, Token o, ExprPtr r) : left(std::move(l)), op(o), right(std::move(r)) {}
};

struct Unary : Expr {
    Token op;
    ExprPtr right;

    Unary(Token o, ExprPtr r) : op(o), right(std::move(r)) {}
};

struct Literal : Expr {
    std::variant<std::monostate, int64_t, double, std::string> value;

    Literal() : value(std::monostate{}) {}
    explicit Literal(int64_t v) : value(v) {}
    explicit Literal(double v) : value(v) {}
    explicit Literal(std::string v) : value(std::move(v)) {}
    explicit Literal(std::nullptr_t) : value(std::monostate{}) {}
};

struct TernaryExpr : Expr {
    ExprPtr condition;
    ExprPtr true_expr;
    ExprPtr false_expr;

    TernaryExpr(ExprPtr cond, ExprPtr t, ExprPtr f)
        : condition(std::move(cond)), true_expr(std::move(t)), false_expr(std::move(f)) {}
};

struct FString : Expr {
    struct Part {
        bool is_literal;
        std::string literal;
        ExprPtr expr;
    };

    std::vector<Part> parts;

    explicit FString(std::vector<Part> p) : parts(std::move(p)) {}
};

struct Grouping : Expr {
    ExprPtr expression;

    explicit Grouping(ExprPtr expr) : expression(std::move(expr)) {}
};

struct Variable : Expr {
    Token name;

    explicit Variable(Token n) : name(n) {}
};

struct Assign : Expr {
    Token name;
    ExprPtr value;

    Assign(Token n, ExprPtr v) : name(n), value(std::move(v)) {}
};

struct Logical : Expr {
    ExprPtr left;
    Token op;
    ExprPtr right;

    Logical(ExprPtr l, Token o, ExprPtr r) : left(std::move(l)), op(o), right(std::move(r)) {}
};

struct Call : Expr {
    ExprPtr callee;
    std::vector<ExprPtr> arguments;

    Call(ExprPtr c, std::vector<ExprPtr> args) : callee(std::move(c)), arguments(std::move(args)) {}
};

struct Get : Expr {
    ExprPtr obj;
    Token name;

    Get(ExprPtr o, Token n) : obj(std::move(o)), name(n) {}
};

struct NullSafeGet : Expr {
    ExprPtr obj;
    Token name;

    NullSafeGet(ExprPtr o, Token n) : obj(std::move(o)), name(n) {}
};

struct Set : Expr {
    ExprPtr obj;
    Token name;
    ExprPtr value;

    Set(ExprPtr o, Token n, ExprPtr v) : obj(std::move(o)), name(n), value(std::move(v)) {}
};

struct New : Expr {
    Token name;
    std::vector<ExprPtr> arguments;

    New(Token n, std::vector<ExprPtr> args) : name(n), arguments(std::move(args)) {}
};

struct ListLiteral : Expr {
    std::vector<ExprPtr> elements;

    explicit ListLiteral(std::vector<ExprPtr> elems) : elements(std::move(elems)) {}
};

struct MapLiteral : Expr {
    std::vector<ExprPtr> keys;
    std::vector<ExprPtr> values;

    MapLiteral(std::vector<ExprPtr> k, std::vector<ExprPtr> v) : keys(std::move(k)), values(std::move(v)) {}
};

struct IndexExpr : Expr {
    ExprPtr obj;
    ExprPtr index;

    IndexExpr(ExprPtr o, ExprPtr i) : obj(std::move(o)), index(std::move(i)) {}
};

struct IndexAssign : Expr {
    ExprPtr obj;
    ExprPtr index;
    ExprPtr value;

    IndexAssign(ExprPtr o, ExprPtr i, ExprPtr v) : obj(std::move(o)), index(std::move(i)), value(std::move(v)) {}
};

struct LambdaExpr : Expr {
    std::vector<VarStmt> params;
    std::vector<StmtPtr> body;

    LambdaExpr(std::vector<VarStmt> p, std::vector<StmtPtr> b) : params(std::move(p)), body(std::move(b)) {}
};

struct Stmt {
    virtual ~Stmt() = default;
};

struct ExpressionStmt : Stmt {
    ExprPtr expression;

    explicit ExpressionStmt(ExprPtr expr) : expression(std::move(expr)) {}
};

struct VarStmt : Stmt {
    Token type_id;
    Token name;
    ExprPtr initializer;
    std::optional<Token> visibility;
    bool is_static = false;
    bool is_const = false;

    VarStmt() = default;
    VarStmt(Token tid, Token n, ExprPtr init, std::optional<Token> vis, bool stat = false, bool con = false)
        : type_id(tid), name(n), initializer(std::move(init)), visibility(std::move(vis)), is_static(stat),
          is_const(con) {}
};

struct Block : Stmt {
    std::vector<StmtPtr> statements;

    explicit Block(std::vector<StmtPtr> stmts) : statements(std::move(stmts)) {}
};

struct IfStmt : Stmt {
    ExprPtr condition;
    StmtPtr then_branch;
    StmtPtr else_branch;

    IfStmt(ExprPtr cond, StmtPtr then_b, StmtPtr else_b = nullptr)
        : condition(std::move(cond)), then_branch(std::move(then_b)), else_branch(std::move(else_b)) {}
};

struct LoopStmt : Stmt {
    ExprPtr condition;
    StmtPtr body;
    std::string label;
    bool is_do_while = false;

    LoopStmt(ExprPtr cond, StmtPtr b, std::string l = "", bool dow = false)
        : condition(std::move(cond)), body(std::move(b)), label(std::move(l)), is_do_while(dow) {}
};

struct ForStmt : Stmt {
    StmtPtr initializer;
    ExprPtr condition;
    ExprPtr increment;
    StmtPtr body;
    std::string label;

    ForStmt(StmtPtr init, ExprPtr cond, ExprPtr incr, StmtPtr b, std::string l = "")
        : initializer(std::move(init)), condition(std::move(cond)), increment(std::move(incr)), body(std::move(b)),
          label(std::move(l)) {}
};

struct TryStmt : Stmt {
    Block try_block;
    Token exception_type;
    Token exception_var;
    Block handle_block;

    TryStmt(Block try_b, Token exc_type, Token exc_var, Block handle_b)
        : try_block(std::move(try_b)), exception_type(exc_type), exception_var(exc_var),
          handle_block(std::move(handle_b)) {}
};

struct ReturnStmt : Stmt {
    Token keyword;
    ExprPtr value;

    ReturnStmt(Token kw, ExprPtr v) : keyword(kw), value(std::move(v)) {}
};

struct BreakStmt : Stmt {
    Token keyword;
    std::string label;
    explicit BreakStmt(Token kw, std::string l = "") : keyword(kw), label(std::move(l)) {}
};

struct ContinueStmt : Stmt {
    Token keyword;
    std::string label;
    explicit ContinueStmt(Token kw, std::string l = "") : keyword(kw), label(std::move(l)) {}
};

struct FunctionStmt : Stmt {
    Token name;
    std::vector<VarStmt> params;
    std::vector<StmtPtr> body;
    Token return_type;
    std::optional<Token> visibility;
    bool is_static = false;
    bool is_abstract = false;

    FunctionStmt(Token n, std::vector<VarStmt> p, std::vector<StmtPtr> b, Token ret, std::optional<Token> vis,
                 bool stat = false, bool abstr = false)
        : name(n), params(std::move(p)), body(std::move(b)), return_type(ret), visibility(std::move(vis)),
          is_static(stat), is_abstract(abstr) {}
};

struct ClassStmt : Stmt {
    Token name;
    std::shared_ptr<Variable> superclass;
    std::vector<FunctionStmt> methods;
    std::vector<VarStmt> fields;
    std::vector<Variable> interfaces;
    bool is_interface = false;
    bool is_abstract = false;

    ClassStmt(Token n, std::shared_ptr<Variable> sup, std::vector<FunctionStmt> meth, std::vector<VarStmt> fld,
              std::vector<Variable> intf, bool is_intf = false, bool is_abstr = false)
        : name(n), superclass(std::move(sup)), methods(std::move(meth)), fields(std::move(fld)),
          interfaces(std::move(intf)), is_interface(is_intf), is_abstract(is_abstr) {}
};

struct ImportStmt : Stmt {
    std::string module_path;
    std::optional<std::string> alias;

    ImportStmt(std::string path, std::optional<std::string> a = std::nullopt)
        : module_path(std::move(path)), alias(std::move(a)) {}
};

struct ExportStmt : Stmt {
    std::vector<Token> names;

    explicit ExportStmt(std::vector<Token> n) : names(std::move(n)) {}
};

struct Case {
    ExprPtr pattern;
    StmtPtr body;

    Case(ExprPtr p, StmtPtr b) : pattern(std::move(p)), body(std::move(b)) {}
};

struct MatchStmt : Stmt {
    ExprPtr expression;
    std::vector<Case> cases;
    StmtPtr default_case;

    MatchStmt(ExprPtr expr, std::vector<Case> c, StmtPtr def = nullptr)
        : expression(std::move(expr)), cases(std::move(c)), default_case(std::move(def)) {}
};

// W6: Visitor pattern
struct Assign;
struct Binary;
struct Block;
struct BreakStmt;
struct Call;
struct ClassStmt;
struct ContinueStmt;
struct ExportStmt;
struct ExpressionStmt;
struct ForStmt;
struct FString;
struct FunctionStmt;
struct Get;
struct Grouping;
struct IfStmt;
struct ImportStmt;
struct IndexAssign;
struct IndexExpr;
struct LambdaExpr;
struct ListLiteral;
struct Literal;
struct Logical;
struct LoopStmt;
struct MapLiteral;
struct MatchStmt;
struct New;
struct NullSafeGet;
struct ReturnStmt;
struct Set;
struct TernaryExpr;
struct TryStmt;
struct Unary;
struct Variable;
struct VarStmt;

class ASTVisitor {
  public:
    virtual ~ASTVisitor() = default;
    virtual void visit_assign(const Assign& node) = 0;
    virtual void visit_binary(const Binary& node) = 0;
    virtual void visit_block(const Block& node) = 0;
    virtual void visit_break(const BreakStmt& node) = 0;
    virtual void visit_call(const Call& node) = 0;
    virtual void visit_class(const ClassStmt& node) = 0;
    virtual void visit_continue(const ContinueStmt& node) = 0;
    virtual void visit_export(const ExportStmt& node) = 0;
    virtual void visit_expression_stmt(const ExpressionStmt& node) = 0;
    virtual void visit_for(const ForStmt& node) = 0;
    virtual void visit_fstring(const FString& node) = 0;
    virtual void visit_function(const FunctionStmt& node) = 0;
    virtual void visit_get(const Get& node) = 0;
    virtual void visit_grouping(const Grouping& node) = 0;
    virtual void visit_if(const IfStmt& node) = 0;
    virtual void visit_import(const ImportStmt& node) = 0;
    virtual void visit_index_assign(const IndexAssign& node) = 0;
    virtual void visit_index(const IndexExpr& node) = 0;
    virtual void visit_lambda(const LambdaExpr& node) = 0;
    virtual void visit_list(const ListLiteral& node) = 0;
    virtual void visit_literal(const Literal& node) = 0;
    virtual void visit_logical(const Logical& node) = 0;
    virtual void visit_loop(const LoopStmt& node) = 0;
    virtual void visit_map(const MapLiteral& node) = 0;
    virtual void visit_match(const MatchStmt& node) = 0;
    virtual void visit_new(const New& node) = 0;
    virtual void visit_null_safe_get(const NullSafeGet& node) = 0;
    virtual void visit_return(const ReturnStmt& node) = 0;
    virtual void visit_set(const Set& node) = 0;
    virtual void visit_ternary(const TernaryExpr& node) = 0;
    virtual void visit_try(const TryStmt& node) = 0;
    virtual void visit_unary(const Unary& node) = 0;
    virtual void visit_variable(const Variable& node) = 0;
    virtual void visit_var(const VarStmt& node) = 0;
};

} // namespace alphabet

#endif

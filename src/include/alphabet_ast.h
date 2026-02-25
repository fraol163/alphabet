#ifndef ALPHABET_AST_H
#define ALPHABET_AST_H

#include <memory>
#include <vector>
#include <string>
#include <variant>
#include <optional>
#include "lexer.h"

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

    Binary(ExprPtr l, Token o, ExprPtr r)
        : left(std::move(l)), op(o), right(std::move(r)) {}
};

struct Unary : Expr {
    Token op;
    ExprPtr right;

    Unary(Token o, ExprPtr r) : op(o), right(std::move(r)) {}
};

struct Literal : Expr {
    std::variant<std::monostate, double, std::string> value;

    Literal() : value(std::monostate{}) {}
    explicit Literal(double v) : value(v) {}
    explicit Literal(std::string v) : value(std::move(v)) {}
    explicit Literal(std::nullptr_t) : value(std::monostate{}) {}
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

    Logical(ExprPtr l, Token o, ExprPtr r)
        : left(std::move(l)), op(o), right(std::move(r)) {}
};

struct Call : Expr {
    ExprPtr callee;
    std::vector<ExprPtr> arguments;

    Call(ExprPtr c, std::vector<ExprPtr> args)
        : callee(std::move(c)), arguments(std::move(args)) {}
};

struct Get : Expr {
    ExprPtr obj;
    Token name;

    Get(ExprPtr o, Token n) : obj(std::move(o)), name(n) {}
};

struct Set : Expr {
    ExprPtr obj;
    Token name;
    ExprPtr value;

    Set(ExprPtr o, Token n, ExprPtr v)
        : obj(std::move(o)), name(n), value(std::move(v)) {}
};

struct New : Expr {
    Token name;
    std::vector<ExprPtr> arguments;

    New(Token n, std::vector<ExprPtr> args)
        : name(n), arguments(std::move(args)) {}
};

struct ListLiteral : Expr {
    std::vector<ExprPtr> elements;

    explicit ListLiteral(std::vector<ExprPtr> elems)
        : elements(std::move(elems)) {}
};

struct MapLiteral : Expr {
    std::vector<ExprPtr> keys;
    std::vector<ExprPtr> values;

    MapLiteral(std::vector<ExprPtr> k, std::vector<ExprPtr> v)
        : keys(std::move(k)), values(std::move(v)) {}
};

struct IndexExpr : Expr {
    ExprPtr obj;
    ExprPtr index;

    IndexExpr(ExprPtr o, ExprPtr i)
        : obj(std::move(o)), index(std::move(i)) {}
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

    VarStmt() = default;
    VarStmt(Token tid, Token n, ExprPtr init, std::optional<Token> vis, bool stat = false)
        : type_id(tid), name(n), initializer(std::move(init)),
          visibility(std::move(vis)), is_static(stat) {}
};

struct Block : Stmt {
    std::vector<StmtPtr> statements;

    explicit Block(std::vector<StmtPtr> stmts)
        : statements(std::move(stmts)) {}
};

struct IfStmt : Stmt {
    ExprPtr condition;
    StmtPtr then_branch;
    StmtPtr else_branch;

    IfStmt(ExprPtr cond, StmtPtr then_b, StmtPtr else_b = nullptr)
        : condition(std::move(cond)), then_branch(std::move(then_b)),
          else_branch(std::move(else_b)) {}
};

struct LoopStmt : Stmt {
    ExprPtr condition;
    StmtPtr body;

    LoopStmt(ExprPtr cond, StmtPtr b)
        : condition(std::move(cond)), body(std::move(b)) {}
};

struct TryStmt : Stmt {
    Block try_block;
    Token exception_type;
    Token exception_var;
    Block handle_block;

    TryStmt(Block try_b, Token exc_type, Token exc_var, Block handle_b)
        : try_block(std::move(try_b)), exception_type(exc_type),
          exception_var(exc_var), handle_block(std::move(handle_b)) {}
};

struct ReturnStmt : Stmt {
    Token keyword;
    ExprPtr value;

    ReturnStmt(Token kw, ExprPtr v) : keyword(kw), value(std::move(v)) {}
};

struct FunctionStmt : Stmt {
    Token name;
    std::vector<VarStmt> params;
    std::vector<StmtPtr> body;
    Token return_type;
    std::optional<Token> visibility;
    bool is_static = false;

    FunctionStmt(Token n, std::vector<VarStmt> p, std::vector<StmtPtr> b,
                 Token ret, std::optional<Token> vis, bool stat = false)
        : name(n), params(std::move(p)), body(std::move(b)),
          return_type(ret), visibility(std::move(vis)), is_static(stat) {}
};

struct ClassStmt : Stmt {
    Token name;
    std::shared_ptr<Variable> superclass;
    std::vector<FunctionStmt> methods;
    std::vector<VarStmt> fields;
    std::vector<Variable> interfaces;
    bool is_interface = false;

    ClassStmt(Token n, std::shared_ptr<Variable> sup,
              std::vector<FunctionStmt> meth, std::vector<VarStmt> fld,
              std::vector<Variable> intf, bool is_intf = false)
        : name(n), superclass(std::move(sup)), methods(std::move(meth)),
          fields(std::move(fld)), interfaces(std::move(intf)),
          is_interface(is_intf) {}
};

}

#endif

#ifndef ALPHABET_PARSER_H
#define ALPHABET_PARSER_H

#include <vector>
#include <stdexcept>
#include <memory>
#include "lexer.h"
#include "alphabet_ast.h"

namespace alphabet {

class ParseError : public std::runtime_error {
public:
    explicit ParseError(const std::string& msg) : std::runtime_error(msg) {}
};

class Parser {
public:
    explicit Parser(std::vector<Token> tokens);
    
    std::vector<StmtPtr> parse();

private:
    std::vector<Token> tokens_;
    size_t current_ = 0;

    [[nodiscard]] bool is_at_end() const;
    [[nodiscard]] const Token& peek() const;
    [[nodiscard]] const Token& previous() const;
    Token advance();
    bool match(std::initializer_list<TokenType> types);
    [[nodiscard]] bool check(TokenType type) const;
    Token consume(TokenType type, const std::string& message);
    [[nodiscard]] ParseError error(const Token& token, const std::string& message) const;
    void synchronize();
    [[nodiscard]] bool is_identifier() const;
    Token consume_identifier(const std::string& message);
    [[nodiscard]] bool check_next_is_identifier() const;

    std::optional<StmtPtr> declaration();
    StmtPtr interface_declaration();
    StmtPtr class_declaration();
    FunctionStmt method(std::optional<Token> visibility, bool is_static);
    VarStmt var_declaration(std::optional<Token> visibility = std::nullopt, bool is_static = false);
    StmtPtr var_statement(std::optional<Token> visibility = std::nullopt, bool is_static = false);

    StmtPtr statement();
    StmtPtr if_statement();
    StmtPtr loop_statement();
    StmtPtr try_statement();
    StmtPtr return_statement();
    std::vector<StmtPtr> block();
    StmtPtr expression_statement();

    ExprPtr expression();
    ExprPtr assignment();
    ExprPtr or_expr();
    ExprPtr and_expr();
    ExprPtr equality();
    ExprPtr comparison();
    ExprPtr term();
    ExprPtr factor();
    ExprPtr unary();
    ExprPtr call();
    ExprPtr primary();

    ExprPtr finish_call(ExprPtr callee);
};

}

#endif

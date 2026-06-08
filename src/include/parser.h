#ifndef ALPHABET_PARSER_H
#define ALPHABET_PARSER_H

#include "alphabet_ast.h"
#include "error_catalog.h"
#include "keywords.h"
#include "lexer.h"
#include <deque>
#include <memory>
#include <stdexcept>
#include <unordered_map>
#include <vector>

namespace alphabet {

class ParseError : public std::runtime_error {
  public:
    ErrorCode code = ErrorCode::NONE;
    explicit ParseError(const std::string& msg) : std::runtime_error(msg) {}
    explicit ParseError(const std::string& msg, ErrorCode ec) : std::runtime_error(msg), code(ec) {}
};

class Parser {
  public:
    explicit Parser(std::vector<Token> tokens, std::string_view source = "");

    std::vector<StmtPtr> parse();

    [[nodiscard]] bool had_errors() const { return had_errors_; }
    [[nodiscard]] const std::string& first_error() const { return first_error_; }
    [[nodiscard]] const std::vector<std::string>& errors() const { return errors_; }

  private:
    std::vector<Token> tokens_;
    std::string_view source_;
    std::vector<std::unique_ptr<Lexer>> sub_lexers_;
    std::deque<std::string> sub_sources_;
    std::deque<std::string> type_keyword_pool_;
    std::deque<std::string> fe_name_pool_;
    size_t current_ = 0;
    int for_each_counter_ = 0;
    bool had_errors_ = false;
    std::string first_error_;
    std::vector<std::string> errors_;

    [[nodiscard]] bool is_at_end() const;
    [[nodiscard]] const Token& peek() const;
    [[nodiscard]] const Token& previous() const;
    Token advance();
    bool match(std::initializer_list<TokenType> types);
    [[nodiscard]] bool check(TokenType type) const;
    Token consume(TokenType type, const std::string& message);
    [[nodiscard]] ParseError error(const Token& token, const std::string& message) const;
    [[nodiscard]] ParseError error(const Token& token, const std::string& message, ErrorCode ec) const;
    void synchronize();
    [[nodiscard]] bool is_identifier() const;
    Token consume_identifier(const std::string& message);
    [[nodiscard]] bool check_next_is_identifier() const;
    [[nodiscard]] int get_type_keyword_id() const;
    Token consume_type_id(const std::string& message);

    std::optional<StmtPtr> declaration();
    StmtPtr interface_declaration();
    StmtPtr class_declaration(bool is_abstract = false);
    FunctionStmt method(std::optional<Token> visibility, bool is_static, bool is_abstract = false);
    VarStmt var_declaration(std::optional<Token> visibility = std::nullopt, bool is_static = false);
    StmtPtr var_statement(std::optional<Token> visibility = std::nullopt, bool is_static = false);
    StmtPtr const_statement();

    StmtPtr statement();
    StmtPtr if_statement();
    StmtPtr loop_statement(const std::string& label);
    StmtPtr do_while_statement(const std::string& label);
    StmtPtr try_statement();
    StmtPtr return_statement();
    StmtPtr import_statement();
    StmtPtr match_statement();
    StmtPtr top_level_function();
    std::vector<StmtPtr> block();
    StmtPtr expression_statement();

    ExprPtr fstring_expression();
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
    ExprPtr lambda_expression();
};

} // namespace alphabet

#endif

#include "peg_engine.h"
#include "lexer.h"
#include <sstream>
#include <iostream>
#include <algorithm>

namespace alphabet {
namespace forge {

std::string DiagnosticError::format(const std::string& filename) const {
    std::ostringstream oss;
    std::string fname = filename.empty() ? "source" : filename;
    oss << "\033[1;31merror\033[0m: " << message << "\n";
    oss << "  \033[1;34m-->\033[0m " << fname << ":" << line << ":" << column << "\n";
    oss << "   \033[1;34m|\033[0m\n";
    oss << " " << line << " \033[1;34m|\033[0m " << line_content << "\n";
    oss << "   \033[1;34m|\033[0m ";
    for (int i = 1; i < column; ++i) oss << " ";
    oss << "\033[1;31m^\033[0m";
    if (!hint.empty()) {
        oss << " \033[1;36mhelp: " << hint << "\033[0m";
    }
    oss << "\n";
    return oss.str();
}

namespace {

enum class FTokenType {
    KW_VAR,
    KW_PRINT,
    KW_IF,
    KW_ELSE,
    KW_WHILE,
    KW_FUNCTION,
    KW_RETURN,
    KW_FOR,
    KW_BREAK,
    KW_CONTINUE,
    KW_TRUE,
    KW_FALSE,
    KW_NIL,
    IDENTIFIER,
    INTEGER,
    FLOAT,
    STRING,
    PLUS,
    MINUS,
    STAR,
    SLASH,
    PERCENT,
    EQ,
    EQ_EQ,
    BANG_EQ,
    LT,
    LT_EQ,
    GT,
    GT_EQ,
    AND,
    OR,
    NOT,
    LPAREN,
    RPAREN,
    LBRACE,
    RBRACE,
    LBRACKET,
    RBRACKET,
    COMMA,
    SEMICOLON,
    COLON,
    DOT,
    ARROW,
    COLON_EQ,
    END_OF_FILE
};

struct FToken {
    FTokenType type;
    std::string text;
    int line = 1;
    int column = 1;
};

class CustomTokenizer {
public:
    CustomTokenizer(const std::string& src, const TokenConfig& cfg, int start_line = 1)
        : source(src), config(cfg), pos(0), line(start_line), line_start(0) {}

    std::vector<FToken> tokenize() {
        std::vector<FToken> tokens;
        while (pos < source.length()) {
            skip_whitespace_and_comments();
            if (pos >= source.length()) break;

            int col = static_cast<int>(pos - line_start + 1);
            char c = source[pos];

            if (c == '(') { tokens.push_back({FTokenType::LPAREN, "(", line, col}); pos++; }
            else if (c == ')') { tokens.push_back({FTokenType::RPAREN, ")", line, col}); pos++; }
            else if (c == '{') { tokens.push_back({FTokenType::LBRACE, "{", line, col}); pos++; }
            else if (c == '}') { tokens.push_back({FTokenType::RBRACE, "}", line, col}); pos++; }
            else if (c == '[') { tokens.push_back({FTokenType::LBRACKET, "[", line, col}); pos++; }
            else if (c == ']') { tokens.push_back({FTokenType::RBRACKET, "]", line, col}); pos++; }
            else if (c == ',') { tokens.push_back({FTokenType::COMMA, ",", line, col}); pos++; }
            else if (c == ';') { tokens.push_back({FTokenType::SEMICOLON, ";", line, col}); pos++; }
            else if (c == '.') { tokens.push_back({FTokenType::DOT, ".", line, col}); pos++; }
            else if (c == ':') {
                if (pos + 1 < source.length() && source[pos + 1] == '=') {
                    tokens.push_back({FTokenType::COLON_EQ, ":=", line, col});
                    pos += 2;
                } else {
                    tokens.push_back({FTokenType::COLON, ":", line, col});
                    pos++;
                }
            } else if (c == '=') {
                if (pos + 1 < source.length() && source[pos + 1] == '=') {
                    tokens.push_back({FTokenType::EQ_EQ, "==", line, col});
                    pos += 2;
                } else if (pos + 1 < source.length() && source[pos + 1] == '>') {
                    tokens.push_back({FTokenType::ARROW, "=>", line, col});
                    pos += 2;
                } else {
                    tokens.push_back({FTokenType::EQ, "=", line, col});
                    pos++;
                }
            } else if (c == '!') {
                if (pos + 1 < source.length() && source[pos + 1] == '=') {
                    tokens.push_back({FTokenType::BANG_EQ, "!=", line, col});
                    pos += 2;
                } else {
                    tokens.push_back({FTokenType::NOT, "!", line, col});
                    pos++;
                }
            } else if (c == '<') {
                if (pos + 1 < source.length() && source[pos + 1] == '=') {
                    tokens.push_back({FTokenType::LT_EQ, "<=", line, col});
                    pos += 2;
                } else {
                    tokens.push_back({FTokenType::LT, "<", line, col});
                    pos++;
                }
            } else if (c == '>') {
                if (pos + 1 < source.length() && source[pos + 1] == '=') {
                    tokens.push_back({FTokenType::GT_EQ, ">=", line, col});
                    pos += 2;
                } else {
                    tokens.push_back({FTokenType::GT, ">", line, col});
                    pos++;
                }
            } else if (c == '+') { tokens.push_back({FTokenType::PLUS, "+", line, col}); pos++; }
            else if (c == '-') {
                if (pos + 1 < source.length() && source[pos + 1] == '>') {
                    tokens.push_back({FTokenType::ARROW, "->", line, col});
                    pos += 2;
                } else {
                    tokens.push_back({FTokenType::MINUS, "-", line, col});
                    pos++;
                }
            } else if (c == '*') { tokens.push_back({FTokenType::STAR, "*", line, col}); pos++; }
            else if (c == '/') { tokens.push_back({FTokenType::SLASH, "/", line, col}); pos++; }
            else if (c == '%') { tokens.push_back({FTokenType::PERCENT, "%", line, col}); pos++; }
            else if (c == '&' && pos + 1 < source.length() && source[pos + 1] == '&') {
                tokens.push_back({FTokenType::AND, "&&", line, col});
                pos += 2;
            } else if (c == '|' && pos + 1 < source.length() && source[pos + 1] == '|') {
                tokens.push_back({FTokenType::OR, "||", line, col});
                pos += 2;
            } else if (c == '"') {
                tokens.push_back(lex_string(col));
            } else if (std::isdigit(static_cast<unsigned char>(c))) {
                tokens.push_back(lex_number(col));
            } else if (std::isalpha(static_cast<unsigned char>(c)) || c == '_' || static_cast<unsigned char>(c) >= 128) {
                tokens.push_back(lex_identifier(col));
            } else {
                pos++;
            }
        }
        tokens.push_back({FTokenType::END_OF_FILE, "", line, static_cast<int>(pos - line_start + 1)});
        return tokens;
    }

private:
    std::string source;
    TokenConfig config;
    size_t pos;
    int line;
    size_t line_start;

    void skip_whitespace_and_comments() {
        while (pos < source.length()) {
            char c = source[pos];
            if (c == '\n') {
                line++;
                pos++;
                line_start = pos;
            } else if (std::isspace(static_cast<unsigned char>(c))) {
                pos++;
            } else if (!config.line_comment.empty() && 
                       source.compare(pos, config.line_comment.length(), config.line_comment) == 0) {
                pos += config.line_comment.length();
                while (pos < source.length() && source[pos] != '\n') pos++;
            } else if (!config.block_comment_start.empty() && 
                       source.compare(pos, config.block_comment_start.length(), config.block_comment_start) == 0) {
                pos += config.block_comment_start.length();
                while (pos + config.block_comment_end.length() <= source.length()) {
                    if (source.compare(pos, config.block_comment_end.length(), config.block_comment_end) == 0) {
                        pos += config.block_comment_end.length();
                        break;
                    }
                    if (source[pos] == '\n') {
                        line++;
                        line_start = pos + 1;
                    }
                    pos++;
                }
            } else {
                break;
            }
        }
    }

    FToken lex_string(int col) {
        pos++; // skip open quote
        std::string val;
        while (pos < source.length() && source[pos] != '"') {
            if (source[pos] == '\\' && pos + 1 < source.length()) {
                pos++;
                char esc = source[pos++];
                if (esc == 'n') val += '\n';
                else if (esc == 't') val += '\t';
                else if (esc == 'r') val += '\r';
                else if (esc == '\\') val += '\\';
                else if (esc == '"') val += '"';
                else val += esc;
            } else {
                if (source[pos] == '\n') {
                    line++;
                    line_start = pos + 1;
                }
                val += source[pos++];
            }
        }
        if (pos < source.length() && source[pos] == '"') pos++;
        return {FTokenType::STRING, val, line, col};
    }

    FToken lex_number(int col) {
        size_t start = pos;
        bool is_float = false;
        while (pos < source.length() && std::isdigit(static_cast<unsigned char>(source[pos]))) {
            pos++;
        }
        if (pos + 1 < source.length() && source[pos] == '.' && std::isdigit(static_cast<unsigned char>(source[pos + 1]))) {
            is_float = true;
            pos++; // skip '.'
            while (pos < source.length() && std::isdigit(static_cast<unsigned char>(source[pos]))) {
                pos++;
            }
        }
        std::string text = source.substr(start, pos - start);
        return {is_float ? FTokenType::FLOAT : FTokenType::INTEGER, text, line, col};
    }

    FToken lex_identifier(int col) {
        size_t start = pos;
        while (pos < source.length()) {
            char c = source[pos];
            if (std::isalnum(static_cast<unsigned char>(c)) || c == '_' || static_cast<unsigned char>(c) >= 128) {
                pos++;
            } else {
                break;
            }
        }
        std::string text = source.substr(start, pos - start);

        // Check custom keyword mappings
        auto it = config.keywords.find(text);
        if (it != config.keywords.end()) {
            const std::string& target = it->second;
            if (target == "VAR") return {FTokenType::KW_VAR, text, line, col};
            if (target == "PRINT") return {FTokenType::KW_PRINT, text, line, col};
            if (target == "IF") return {FTokenType::KW_IF, text, line, col};
            if (target == "ELSE") return {FTokenType::KW_ELSE, text, line, col};
            if (target == "WHILE") return {FTokenType::KW_WHILE, text, line, col};
            if (target == "FUNCTION" || target == "FN" || target == "DEF") return {FTokenType::KW_FUNCTION, text, line, col};
            if (target == "RETURN") return {FTokenType::KW_RETURN, text, line, col};
            if (target == "FOR") return {FTokenType::KW_FOR, text, line, col};
            if (target == "BREAK") return {FTokenType::KW_BREAK, text, line, col};
            if (target == "CONTINUE") return {FTokenType::KW_CONTINUE, text, line, col};
            if (target == "TRUE") return {FTokenType::KW_TRUE, text, line, col};
            if (target == "FALSE") return {FTokenType::KW_FALSE, text, line, col};
            if (target == "NIL") return {FTokenType::KW_NIL, text, line, col};
        }

        // Standard fallbacks
        if (text == "true") return {FTokenType::KW_TRUE, text, line, col};
        if (text == "false") return {FTokenType::KW_FALSE, text, line, col};
        if (text == "nil" || text == "null") return {FTokenType::KW_NIL, text, line, col};

        return {FTokenType::IDENTIFIER, text, line, col};
    }
};

class PackratParser {
public:
    PackratParser(std::vector<FToken> tks, std::string src, const ForgeSpec& sp, int line_offset = 0)
        : tokens(std::move(tks)), source(std::move(src)), spec(sp), cur(0), line_offset_(line_offset) {}

    ParseResult parse(const std::string& filename) {
        ParseResult res;
        try {
            while (!is_at_end()) {
                auto stmt = parse_statement();
                if (stmt) {
                    res.statements.push_back(stmt);
                } else {
                    break;
                }
            }
            res.success = errors.empty();
            res.errors = errors;
        } catch (const std::exception& e) {
            res.success = false;
            DiagnosticError err;
            err.line = peek().line;
            err.column = peek().column;
            err.message = e.what();
            err.line_content = get_line_content(err.line);
            res.errors.push_back(err);
        }
        (void)filename;
        return res;
    }

private:
    std::vector<FToken> tokens;
    std::string source;
    ForgeSpec spec;
    size_t cur;
    int line_offset_ = 0;
    std::vector<DiagnosticError> errors;

    FToken peek() const {
        if (cur < tokens.size()) return tokens[cur];
        return {FTokenType::END_OF_FILE, "", 0, 0};
    }

    FToken previous() const {
        if (cur > 0) return tokens[cur - 1];
        return {FTokenType::END_OF_FILE, "", 0, 0};
    }

    bool is_at_end() const {
        return peek().type == FTokenType::END_OF_FILE;
    }

    FToken advance() {
        if (!is_at_end()) cur++;
        return previous();
    }

    bool check(FTokenType t) const {
        if (is_at_end()) return false;
        return peek().type == t;
    }

    bool match(FTokenType t) {
        if (check(t)) {
            advance();
            return true;
        }
        return false;
    }

    FToken consume(FTokenType t, const std::string& msg, const std::string& hint = "") {
        if (check(t)) return advance();
        report_error(peek(), msg, hint);
        throw std::runtime_error(msg);
    }

    void report_error(const FToken& tok, const std::string& msg, const std::string& hint = "") {
        DiagnosticError err;
        err.line = tok.line;
        err.column = tok.column;
        err.message = msg;
        err.line_content = get_line_content(tok.line);
        err.hint = hint;
        errors.push_back(err);
    }

    std::string get_line_content(int line_num) const {
        int target = line_num - line_offset_;
        std::istringstream stream(source);
        std::string l;
        int cur_l = 1;
        while (std::getline(stream, l)) {
            if (cur_l == target) {
                // remove trailing \r
                if (!l.empty() && l.back() == '\r') l.pop_back();
                return l;
            }
            cur_l++;
        }
        return "";
    }

    static std::string_view intern(const std::string& str) {
        static std::unordered_set<std::string> global_pool;
        auto it = global_pool.insert(str).first;
        return std::string_view(*it);
    }

    // AST helper: creates a synthetic native Token for Alphabet AST with permanently interned lexeme
    alphabet::Token make_token(alphabet::TokenType type, const std::string& lexeme, int line, int col) {
        return alphabet::Token(type, intern(lexeme), 0.0, static_cast<size_t>(line), static_cast<size_t>(col));
    }

    alphabet::StmtPtr parse_statement() {
        if (match(FTokenType::KW_VAR)) {
            return parse_var_decl();
        }
        if (match(FTokenType::KW_PRINT)) {
            return parse_print_stmt();
        }
        if (match(FTokenType::KW_IF)) {
            return parse_if_stmt();
        }
        if (match(FTokenType::KW_WHILE)) {
            return parse_while_stmt();
        }
        if (match(FTokenType::KW_FUNCTION)) {
            return parse_function_decl();
        }
        if (match(FTokenType::KW_RETURN)) {
            return parse_return_stmt();
        }
        if (match(FTokenType::LBRACE)) {
            return parse_block();
        }
        return parse_expression_stmt();
    }

    alphabet::StmtPtr parse_var_decl() {
        FToken kw = previous();
        FToken name = consume(FTokenType::IDENTIFIER, "expected variable name after declaration keyword", "provide a valid identifier name");
        alphabet::ExprPtr init = nullptr;
        if (match(FTokenType::EQ) || match(FTokenType::COLON_EQ)) {
            init = parse_expression();
        }
        match(FTokenType::SEMICOLON); // optional semicolon

        auto type_tok = make_token(alphabet::TokenType::IDENTIFIER, "any", kw.line, kw.column);
        auto name_tok = make_token(alphabet::TokenType::IDENTIFIER, name.text, name.line, name.column);
        return std::make_shared<alphabet::VarStmt>(type_tok, name_tok, init, std::nullopt);
    }

    alphabet::StmtPtr parse_print_stmt() {
        FToken kw = previous();
        bool has_paren = match(FTokenType::LPAREN);
        auto expr = parse_expression();
        if (has_paren) {
            consume(FTokenType::RPAREN, "expected ')' after print arguments", "insert ')' to close arguments");
        }
        match(FTokenType::SEMICOLON);

        // In Alphabet, OpCode::PRINT is generated by calling z.o(expr)
        auto z_var = std::make_shared<alphabet::Variable>(make_token(alphabet::TokenType::IDENTIFIER, "z", kw.line, kw.column));
        auto o_tok = make_token(alphabet::TokenType::IDENTIFIER, "o", kw.line, kw.column);
        auto get_o = std::make_shared<alphabet::Get>(z_var, o_tok);
        auto call_print = std::make_shared<alphabet::Call>(get_o, std::vector<alphabet::ExprPtr>{expr});
        return std::make_shared<alphabet::ExpressionStmt>(call_print);
    }

    alphabet::StmtPtr parse_if_stmt() {
        bool has_paren = match(FTokenType::LPAREN);
        auto condition = parse_expression();
        if (has_paren) {
            consume(FTokenType::RPAREN, "expected ')' after if condition", "insert ')' here");
        }
        auto then_branch = parse_statement();
        alphabet::StmtPtr else_branch = nullptr;
        if (match(FTokenType::KW_ELSE)) {
            else_branch = parse_statement();
        }
        return std::make_shared<alphabet::IfStmt>(condition, then_branch, else_branch);
    }

    alphabet::StmtPtr parse_while_stmt() {
        bool has_paren = match(FTokenType::LPAREN);
        auto condition = parse_expression();
        if (has_paren) {
            consume(FTokenType::RPAREN, "expected ')' after while condition", "insert ')' here");
        }
        auto body = parse_statement();
        return std::make_shared<alphabet::LoopStmt>(condition, body);
    }

    alphabet::StmtPtr parse_function_decl() {
        FToken name = consume(FTokenType::IDENTIFIER, "expected function name", "provide function identifier");
        consume(FTokenType::LPAREN, "expected '(' after function name", "insert '('");
        std::vector<alphabet::VarStmt> params;
        if (!check(FTokenType::RPAREN)) {
            do {
                FToken p = consume(FTokenType::IDENTIFIER, "expected parameter name");
                auto t_tok = make_token(alphabet::TokenType::IDENTIFIER, "any", p.line, p.column);
                auto n_tok = make_token(alphabet::TokenType::IDENTIFIER, p.text, p.line, p.column);
                params.push_back(alphabet::VarStmt(t_tok, n_tok, nullptr, std::nullopt));
            } while (match(FTokenType::COMMA));
        }
        consume(FTokenType::RPAREN, "expected ')' after parameters");
        consume(FTokenType::LBRACE, "expected '{' before function body");
        auto blk = std::dynamic_pointer_cast<alphabet::Block>(parse_block());
        std::vector<alphabet::StmtPtr> body = blk ? blk->statements : std::vector<alphabet::StmtPtr>{};

        auto name_tok = make_token(alphabet::TokenType::IDENTIFIER, name.text, name.line, name.column);
        auto ret_tok = make_token(alphabet::TokenType::IDENTIFIER, "any", name.line, name.column);
        return std::make_shared<alphabet::FunctionStmt>(name_tok, params, body, ret_tok, std::nullopt);
    }

    alphabet::StmtPtr parse_return_stmt() {
        FToken kw = previous();
        alphabet::ExprPtr value = nullptr;
        if (!check(FTokenType::SEMICOLON) && !check(FTokenType::RBRACE) && !is_at_end()) {
            value = parse_expression();
        }
        match(FTokenType::SEMICOLON);
        auto kw_tok = make_token(alphabet::TokenType::RETURN, kw.text, kw.line, kw.column);
        return std::make_shared<alphabet::ReturnStmt>(kw_tok, value);
    }

    alphabet::StmtPtr parse_block() {
        std::vector<alphabet::StmtPtr> stmts;
        while (!check(FTokenType::RBRACE) && !is_at_end()) {
            stmts.push_back(parse_statement());
        }
        consume(FTokenType::RBRACE, "expected '}' to close block", "insert '}'");
        return std::make_shared<alphabet::Block>(std::move(stmts));
    }

    alphabet::StmtPtr parse_expression_stmt() {
        auto expr = parse_expression();
        match(FTokenType::SEMICOLON);
        return std::make_shared<alphabet::ExpressionStmt>(expr);
    }

    alphabet::ExprPtr parse_expression() {
        return parse_assignment();
    }

    alphabet::ExprPtr parse_assignment() {
        auto expr = parse_logical_or();
        if (match(FTokenType::EQ) || match(FTokenType::COLON_EQ)) {
            FToken eq = previous();
            auto value = parse_assignment();
            if (auto* var = dynamic_cast<alphabet::Variable*>(expr.get())) {
                return std::make_shared<alphabet::Assign>(var->name, value);
            }
            report_error(eq, "invalid assignment target", "left-hand side must be a variable");
        }
        return expr;
    }

    alphabet::ExprPtr parse_logical_or() {
        auto expr = parse_logical_and();
        while (match(FTokenType::OR)) {
            FToken op = previous();
            auto right = parse_logical_and();
            auto op_tok = make_token(alphabet::TokenType::OR, op.text, op.line, op.column);
            expr = std::make_shared<alphabet::Logical>(expr, op_tok, right);
        }
        return expr;
    }

    alphabet::ExprPtr parse_logical_and() {
        auto expr = parse_equality();
        while (match(FTokenType::AND)) {
            FToken op = previous();
            auto right = parse_equality();
            auto op_tok = make_token(alphabet::TokenType::AND, op.text, op.line, op.column);
            expr = std::make_shared<alphabet::Logical>(expr, op_tok, right);
        }
        return expr;
    }

    alphabet::ExprPtr parse_equality() {
        auto expr = parse_comparison();
        while (match(FTokenType::EQ_EQ) || match(FTokenType::BANG_EQ)) {
            FToken op = previous();
            auto right = parse_comparison();
            auto type = (op.type == FTokenType::EQ_EQ) ? alphabet::TokenType::DOUBLE_EQUALS : alphabet::TokenType::NOT_EQUALS;
            auto op_tok = make_token(type, op.text, op.line, op.column);
            expr = std::make_shared<alphabet::Binary>(expr, op_tok, right);
        }
        return expr;
    }

    alphabet::ExprPtr parse_comparison() {
        auto expr = parse_term();
        while (match(FTokenType::LT) || match(FTokenType::LT_EQ) || 
               match(FTokenType::GT) || match(FTokenType::GT_EQ)) {
            FToken op = previous();
            auto right = parse_term();
            alphabet::TokenType type = alphabet::TokenType::LESS;
            if (op.type == FTokenType::LT_EQ) type = alphabet::TokenType::LESS_EQUALS;
            else if (op.type == FTokenType::GT) type = alphabet::TokenType::GREATER;
            else if (op.type == FTokenType::GT_EQ) type = alphabet::TokenType::GREATER_EQUALS;
            auto op_tok = make_token(type, op.text, op.line, op.column);
            expr = std::make_shared<alphabet::Binary>(expr, op_tok, right);
        }
        return expr;
    }

    alphabet::ExprPtr parse_term() {
        auto expr = parse_factor();
        while (match(FTokenType::PLUS) || match(FTokenType::MINUS)) {
            FToken op = previous();
            auto right = parse_factor();
            auto type = (op.type == FTokenType::PLUS) ? alphabet::TokenType::PLUS : alphabet::TokenType::MINUS;
            auto op_tok = make_token(type, op.text, op.line, op.column);
            expr = std::make_shared<alphabet::Binary>(expr, op_tok, right);
        }
        return expr;
    }

    alphabet::ExprPtr parse_factor() {
        auto expr = parse_unary();
        while (match(FTokenType::STAR) || match(FTokenType::SLASH) || match(FTokenType::PERCENT)) {
            FToken op = previous();
            auto right = parse_unary();
            alphabet::TokenType type = alphabet::TokenType::STAR;
            if (op.type == FTokenType::SLASH) type = alphabet::TokenType::SLASH;
            else if (op.type == FTokenType::PERCENT) type = alphabet::TokenType::PERCENT;
            auto op_tok = make_token(type, op.text, op.line, op.column);
            expr = std::make_shared<alphabet::Binary>(expr, op_tok, right);
        }
        return expr;
    }

    alphabet::ExprPtr parse_unary() {
        if (match(FTokenType::NOT) || match(FTokenType::MINUS)) {
            FToken op = previous();
            auto right = parse_unary();
            auto type = (op.type == FTokenType::NOT) ? alphabet::TokenType::NOT : alphabet::TokenType::MINUS;
            auto op_tok = make_token(type, op.text, op.line, op.column);
            return std::make_shared<alphabet::Unary>(op_tok, right);
        }
        return parse_call();
    }

    alphabet::ExprPtr parse_call() {
        auto expr = parse_primary();
        while (true) {
            if (match(FTokenType::LPAREN)) {
                std::vector<alphabet::ExprPtr> args;
                if (!check(FTokenType::RPAREN)) {
                    do {
                        args.push_back(parse_expression());
                    } while (match(FTokenType::COMMA));
                }
                consume(FTokenType::RPAREN, "expected ')' after arguments");
                expr = std::make_shared<alphabet::Call>(expr, std::move(args));
            } else if (match(FTokenType::DOT)) {
                FToken name = consume(FTokenType::IDENTIFIER, "expected member name after '.'");
                auto name_tok = make_token(alphabet::TokenType::IDENTIFIER, name.text, name.line, name.column);
                expr = std::make_shared<alphabet::Get>(expr, name_tok);
            } else {
                break;
            }
        }
        return expr;
    }

    alphabet::ExprPtr parse_primary() {
        if (match(FTokenType::KW_TRUE)) {
            return std::make_shared<alphabet::Literal>(static_cast<int64_t>(1));
        }
        if (match(FTokenType::KW_FALSE)) {
            return std::make_shared<alphabet::Literal>(static_cast<int64_t>(0));
        }
        if (match(FTokenType::KW_NIL)) {
            return std::make_shared<alphabet::Literal>(nullptr);
        }
        if (match(FTokenType::INTEGER)) {
            try {
                int64_t v = std::stoll(previous().text);
                return std::make_shared<alphabet::Literal>(v);
            } catch (...) {
                return std::make_shared<alphabet::Literal>(static_cast<int64_t>(0));
            }
        }
        if (match(FTokenType::FLOAT)) {
            try {
                double v = std::stod(previous().text);
                return std::make_shared<alphabet::Literal>(v);
            } catch (...) {
                return std::make_shared<alphabet::Literal>(0.0);
            }
        }
        if (match(FTokenType::STRING)) {
            return std::make_shared<alphabet::Literal>(previous().text);
        }
        if (match(FTokenType::IDENTIFIER)) {
            FToken id = previous();
            auto tok = make_token(alphabet::TokenType::IDENTIFIER, id.text, id.line, id.column);
            return std::make_shared<alphabet::Variable>(tok);
        }
        if (match(FTokenType::LPAREN)) {
            auto inner = parse_expression();
            consume(FTokenType::RPAREN, "expected ')' after expression");
            return std::make_shared<alphabet::Grouping>(inner);
        }

        report_error(peek(), "unexpected token '" + peek().text + "'", "expected expression");
        throw std::runtime_error("Unexpected token in expression");
    }
};

} // namespace

PegEngine::PegEngine(const ForgeSpec& spec) : spec_(spec) {}

ParseResult PegEngine::parse(const std::string& source, const std::string& filename, int line_offset) {
    CustomTokenizer tokenizer(source, spec_.tokens, 1 + line_offset);
    std::vector<FToken> tokens = tokenizer.tokenize();
    PackratParser parser(std::move(tokens), source, spec_, line_offset);
    return parser.parse(filename);
}

} // namespace forge
} // namespace alphabet

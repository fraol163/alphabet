#include "parser.h"
#include <sstream>

namespace alphabet {

Parser::Parser(std::vector<Token> tokens, std::string_view source)
    : tokens_(std::move(tokens)), source_(source) {}

std::vector<StmtPtr> Parser::parse() {
    std::vector<StmtPtr> statements;
    while (!is_at_end()) {
        auto stmt = declaration();
        if (stmt) {
            statements.push_back(*stmt);
        }
    }
    return statements;
}

bool Parser::is_at_end() const {
    return peek().type == TokenType::EOF_TOKEN;
}

const Token& Parser::peek() const {
    return tokens_[current_];
}

const Token& Parser::previous() const {
    return tokens_[current_ - 1];
}

Token Parser::advance() {
    if (!is_at_end()) ++current_;
    return previous();
}

bool Parser::match(std::initializer_list<TokenType> types) {
    for (TokenType type : types) {
        if (check(type)) {
            advance();
            return true;
        }
    }
    return false;
}

bool Parser::check(TokenType type) const {
    if (is_at_end()) return false;
    return peek().type == type;
}

Token Parser::consume(TokenType type, const std::string& message) {
    if (check(type)) return advance();
    throw error(peek(), message);
}

ParseError Parser::error(const Token& token, const std::string& message) const {
    std::ostringstream oss;
    oss << "Error at line " << token.line << ", column " << token.column << ": " << message;
    
    // Show source context if available
    if (!source_.empty()) {
        size_t line_start = 0;
        size_t current_line = 1;
        for (size_t i = 0; i < source_.size() && current_line < token.line; ++i) {
            if (source_[i] == '\n') {
                ++current_line;
                line_start = i + 1;
            }
        }
        size_t line_end = source_.find('\n', line_start);
        if (line_end == std::string_view::npos) line_end = source_.size();
        std::string_view error_line = source_.substr(line_start, line_end - line_start);
        
        oss << "\n  " << error_line;
        oss << "\n  " << std::string(token.column > 0 ? token.column - 1 : 0, ' ') << "^";
    }
    
    return ParseError(oss.str());
}

void Parser::synchronize() {
    advance();
    while (!is_at_end()) {
        switch (previous().type) {
            case TokenType::CLASS:
            case TokenType::METHOD:
            case TokenType::IF:
            case TokenType::LOOP:
            case TokenType::RETURN:
                return;
            default:
                break;
        }
        advance();
    }
}

bool Parser::is_identifier() const {
    if (is_at_end()) return false;
    const Token& t = peek();
    if (t.type == TokenType::IDENTIFIER) return true;
    // Check for single ASCII alpha or UTF-8 identifier
    if (t.lexeme.size() == 1 && std::isalpha(static_cast<unsigned char>(t.lexeme[0]))) return true;
    // Check for UTF-8 (multi-byte characters)
    if (t.lexeme.size() > 1) {
        // If first byte indicates multi-byte UTF-8 sequence
        unsigned char first = static_cast<unsigned char>(t.lexeme[0]);
        if (first >= 0xC0) return true;
    }
    return false;
}

Token Parser::consume_identifier(const std::string& message) {
    if (is_identifier()) return advance();
    throw error(peek(), message);
}

bool Parser::check_next_is_identifier() const {
    if (current_ + 1 >= tokens_.size()) return false;
    const Token& t = tokens_[current_ + 1];
    if (t.type == TokenType::IDENTIFIER) return true;
    if (t.type == TokenType::METHOD) return true;
    return t.lexeme.size() == 1 && std::isalpha(static_cast<unsigned char>(t.lexeme[0]));
}

std::optional<StmtPtr> Parser::declaration() {
    try {
        if (check(TokenType::INTERFACE)) {
            advance();
            return interface_declaration();
        }
        if (check(TokenType::CLASS) && check_next_is_identifier()) {
            advance();
            return class_declaration();
        }
        if (check(TokenType::IMPORT)) {
            advance();
            return import_statement();
        }
        if (check(TokenType::MATCH)) {
            advance();
            return match_statement();
        }
        return statement();
    } catch (const ParseError& e) {
        had_errors_ = true;
        if (first_error_.empty()) first_error_ = e.what();
        synchronize();
        return std::nullopt;
    }
}

StmtPtr Parser::interface_declaration() {
    Token name = consume_identifier("Expect interface name.");
    consume(TokenType::LBRACE, "Expect '{' before interface body.");

    std::vector<FunctionStmt> methods;
    while (!check(TokenType::RBRACE) && !is_at_end()) {
        if (match({TokenType::METHOD})) {
            Token return_type = consume(TokenType::NUMBER, "Expect return type ID.");
            Token method_name = consume_identifier("Expect method name.");
            consume(TokenType::LPAREN, "Expect '(' after method name.");
            
            std::vector<VarStmt> parameters;
            if (!check(TokenType::RPAREN)) {
                while (true) {
                    Token type_id = consume(TokenType::NUMBER, "Expect parameter type ID.");
                    Token param_name = consume_identifier("Expect parameter name.");
                    parameters.emplace_back(type_id, param_name, nullptr, std::nullopt);
                    if (!match({TokenType::COMMA})) break;
                }
            }
            consume(TokenType::RPAREN, "Expect ')' after parameters.");
            methods.emplace_back(method_name, std::move(parameters), 
                                std::vector<StmtPtr>{}, return_type, std::nullopt);
        } else {
            throw error(peek(), "Interfaces can only contain methods.");
        }
    }
    consume(TokenType::RBRACE, "Expect '}' after interface body.");
    
    return std::make_shared<ClassStmt>(name, nullptr, std::move(methods),
                                       std::vector<VarStmt>{},
                                       std::vector<Variable>{}, true);
}

StmtPtr Parser::class_declaration() {
    Token name = consume_identifier("Expect class name.");

    std::shared_ptr<Variable> superclass;
    std::vector<Variable> interfaces;

    if (match({TokenType::EXTENDS})) {
        Token super_name = consume_identifier("Expect superclass or interface name.");
        superclass = std::make_shared<Variable>(super_name);

        while (match({TokenType::COMMA})) {
            Token if_name = consume_identifier("Expect interface name.");
            interfaces.emplace_back(if_name);
        }
    }

    consume(TokenType::LBRACE, "Expect '{' before class body.");

    std::vector<FunctionStmt> methods;
    std::vector<VarStmt> fields;

    while (!check(TokenType::RBRACE) && !is_at_end()) {
        std::optional<Token> visibility;
        bool is_static = false;

        while (true) {
            if (check(TokenType::PUBLIC) || check(TokenType::PRIVATE)) {
                if (visibility) break;
                visibility = advance();
            } else if (match({TokenType::STATIC})) {
                if (is_static) break;
                is_static = true;
            } else {
                break;
            }
        }

        if (match({TokenType::METHOD})) {
            // m name(params) { body } - no return type
            methods.push_back(method(visibility, is_static));
        } else if (check(TokenType::NUMBER)) {
            // Could be: return_type m name() { body }  (method)
            // Or: type_id field_name [= value]         (field)
            size_t saved = current_;
            advance(); // consume NUMBER
            if (check(TokenType::METHOD)) {
                // It's a method with return type
                // Don't consume METHOD - method() will handle it
                current_ = saved;
                methods.push_back(method(visibility, is_static));
            } else {
                // It's a field
                current_ = saved;
                fields.push_back(var_declaration(visibility, is_static));
            }
        } else {
            throw error(peek(), "Expect method or field declaration.");
        }
    }

    consume(TokenType::RBRACE, "Expect '}' after class body.");

    return std::make_shared<ClassStmt>(name, std::move(superclass), std::move(methods),
                                       std::move(fields), std::move(interfaces));
}

FunctionStmt Parser::method(std::optional<Token> visibility, bool is_static) {
    // METHOD keyword may or may not be consumed by caller
    // If METHOD is current token, consume it
    if (check(TokenType::METHOD)) {
        advance();
    }
    
    // Now we may have:
    //   name(params)          - no return type
    //   return_type name(params) - with return type
    
    Token return_type;
    Token name;
    
    if (check(TokenType::NUMBER)) {
        // Has return type: NUMBER name
        return_type = advance(); // consume return type NUMBER
        name = consume_identifier("Expect method name.");
    } else {
        // No return type: name - default to INT (5)
        name = consume_identifier("Expect method name.");
        return_type = Token(TokenType::NUMBER, std::string_view("5"), 5, name.line);
    }
    
    consume(TokenType::LPAREN, "Expect '(' after method name.");
    
    std::vector<VarStmt> parameters;
    if (!check(TokenType::RPAREN)) {
        while (true) {
            Token type_id = consume(TokenType::NUMBER, "Expect parameter type ID.");
            Token param_name = consume_identifier("Expect parameter name.");
            parameters.emplace_back(type_id, param_name, nullptr, std::nullopt);
            if (!match({TokenType::COMMA})) break;
        }
    }
    consume(TokenType::RPAREN, "Expect ')' after parameters.");
    consume(TokenType::LBRACE, "Expect '{' before method body.");

    std::vector<StmtPtr> body = block();

    return FunctionStmt(name, std::move(parameters), std::move(body),
                       return_type, std::move(visibility), is_static);
}

StmtPtr Parser::top_level_function() {
    // m [return_type] name(params) { body }
    // The METHOD token has already been consumed
    std::optional<Token> return_type;
    
    // Check if next token is a number (return type) or identifier (name)
    if (check(TokenType::NUMBER)) {
        return_type = advance();
    }
    
    Token name = consume_identifier("Expect function name after 'm'.");
    consume(TokenType::LPAREN, "Expect '(' after function name.");
    
    std::vector<VarStmt> parameters;
    if (!check(TokenType::RPAREN)) {
        while (true) {
            Token type_id = consume(TokenType::NUMBER, "Expect parameter type ID.");
            Token param_name = consume_identifier("Expect parameter name.");
            parameters.emplace_back(type_id, param_name, nullptr, std::nullopt);
            if (!match({TokenType::COMMA})) break;
        }
    }
    consume(TokenType::RPAREN, "Expect ')' after parameters.");
    consume(TokenType::LBRACE, "Expect '{' before function body.");
    
    std::vector<StmtPtr> body = block();
    
    static const std::string_view void_lexeme = "0";
    Token void_type(TokenType::NUMBER, void_lexeme, 0, name.line);
    return std::make_shared<FunctionStmt>(name, std::move(parameters), std::move(body),
                                          return_type.value_or(void_type),
                                          std::nullopt, false);
}

VarStmt Parser::var_declaration(std::optional<Token> visibility, bool is_static) {
    Token type_id = consume(TokenType::NUMBER, "Expect type ID.");
    Token name = consume_identifier("Expect variable name.");

    ExprPtr initializer;
    if (match({TokenType::EQUALS})) {
        initializer = expression();
    }

    return VarStmt(type_id, name, std::move(initializer),
                   std::move(visibility), is_static);
}

StmtPtr Parser::var_statement(std::optional<Token> visibility, bool is_static) {
    return std::make_shared<VarStmt>(var_declaration(visibility, is_static));
}

StmtPtr Parser::statement() {
    if (match({TokenType::IF})) return if_statement();
    if (match({TokenType::RETURN})) return return_statement();
    if (match({TokenType::LOOP})) return loop_statement();
    if (match({TokenType::TRY})) return try_statement();
    if (match({TokenType::BREAK})) return std::make_shared<BreakStmt>(previous());
    if (match({TokenType::CONTINUE})) return std::make_shared<ContinueStmt>(previous());
    if (match({TokenType::LBRACE})) return std::make_shared<Block>(block());
    if (check(TokenType::NUMBER)) return var_statement();
    if (match({TokenType::METHOD})) return top_level_function();
    return expression_statement();
}

StmtPtr Parser::if_statement() {
    consume(TokenType::LPAREN, "Expect '(' after 'i'.");
    ExprPtr condition = expression();
    consume(TokenType::RPAREN, "Expect ')' after if condition.");
    
    StmtPtr then_branch = statement();
    StmtPtr else_branch;
    
    if (match({TokenType::ELSE})) {
        else_branch = statement();
    }
    
    return std::make_shared<IfStmt>(std::move(condition), std::move(then_branch),
                                    std::move(else_branch));
}

StmtPtr Parser::loop_statement() {
    consume(TokenType::LPAREN, "Expect '(' after 'l'.");
    
    // Check if this is a for-loop by looking for ';' before ')'
    // For-loop: l (init; cond; incr) { body }
    // While-loop: l (cond) { body }
    bool is_for = false;
    size_t lookahead = current_;
    int paren_depth = 1;
    while (lookahead < tokens_.size() && paren_depth > 0) {
        if (tokens_[lookahead].type == TokenType::LPAREN) ++paren_depth;
        if (tokens_[lookahead].type == TokenType::RPAREN) --paren_depth;
        if (tokens_[lookahead].type == TokenType::COLON && paren_depth == 1) {
            is_for = true;
            break;
        }
        ++lookahead;
    }
    
    if (!is_for) {
        // While-loop: l (cond) { body }
        ExprPtr condition = expression();
        consume(TokenType::RPAREN, "Expect ')' after loop condition.");
        StmtPtr body = statement();
        return std::make_shared<LoopStmt>(std::move(condition), std::move(body));
    }
    
    // For-loop: l (init; cond; incr) { body }
    // Parse initializer (variable declaration or expression)
    StmtPtr init;
    if (check(TokenType::NUMBER)) {
        init = std::make_shared<VarStmt>(var_declaration());
    } else {
        init = expression_statement();
    }
    consume(TokenType::COLON, "Expect ':' after for-loop initializer.");
    
    // Parse condition
    ExprPtr condition = expression();
    consume(TokenType::COLON, "Expect ':' after for-loop condition.");
    
    // Parse increment
    ExprPtr increment = expression();
    consume(TokenType::RPAREN, "Expect ')' after for-loop increment.");
    
    StmtPtr body = statement();
    
    return std::make_shared<ForStmt>(std::move(init), std::move(condition),
                                     std::move(increment), std::move(body));
}

StmtPtr Parser::try_statement() {
    consume(TokenType::LBRACE, "Expect '{' before try block.");
    Block try_block(block());
    
    consume(TokenType::HANDLE, "Expect 'h' after try block.");
    consume(TokenType::LPAREN, "Expect '(' after 'h'.");
    Token exception_type = consume(TokenType::NUMBER, "Expect exception type ID.");
    Token exception_var = consume_identifier("Expect exception variable name.");
    consume(TokenType::RPAREN, "Expect ')' after exception catch details.");
    consume(TokenType::LBRACE, "Expect '{' before handle block.");
    Block handle_block(block());
    
    return std::make_shared<TryStmt>(std::move(try_block), exception_type,
                                     exception_var, std::move(handle_block));
}

StmtPtr Parser::return_statement() {
    Token keyword = previous();
    ExprPtr value;

    if (!check(TokenType::RBRACE) && !is_at_end()) {
        try {
            value = expression();
        } catch (...) {
            value = nullptr;
        }
    }

    return std::make_shared<ReturnStmt>(keyword, std::move(value));
}

StmtPtr Parser::import_statement() {
    // x "module/path" [as alias]
    // x io "std/io"
    (void)previous();  // Consume IMPORT token

    std::optional<std::string> alias;
    std::string module_path;
    
    // Check if there's an alias first: x io "std/io"
    if (check(TokenType::IDENTIFIER) || 
        (peek().type == TokenType::NUMBER && peek().lexeme.size() == 1 && std::isalpha(peek().lexeme[0]))) {
        // Could be a single-letter identifier alias
        Token alias_token = advance();
        module_path = std::string(alias_token.lexeme);
        
        // Now expect the string path
        consume(TokenType::STRING, "Expect module path string after alias.");
        // Remove quotes from string
        std::string_view path_sv = previous().lexeme;
        module_path = std::string(path_sv);
    } else if (check(TokenType::STRING)) {
        // Just the path: x "std/io"
        advance();
        std::string_view path_sv = previous().lexeme;
        module_path = std::string(path_sv);
    } else {
        throw error(peek(), "Expect module path or alias after 'import'.");
    }
    
    return std::make_shared<ImportStmt>(module_path, alias);
}

StmtPtr Parser::match_statement() {
    // q expression { case pattern: body ... default: body }
    (void)previous();  // Consume MATCH token

    consume(TokenType::LPAREN, "Expect '(' after 'match'.");
    ExprPtr match_expr = expression();
    consume(TokenType::RPAREN, "Expect ')' after expression.");
    consume(TokenType::LBRACE, "Expect '{' before match cases.");
    
    std::vector<Case> cases;
    StmtPtr default_case;
    
    while (!check(TokenType::RBRACE) && !is_at_end()) {
        if (match({TokenType::ELSE})) {
            // default case
            consume(TokenType::COLON, "Expect ':' after 'default'.");
            default_case = statement();
        } else if (check(TokenType::NUMBER) || check(TokenType::STRING) || 
                   check(TokenType::IDENTIFIER)) {
            // case pattern: body
            ExprPtr pattern = expression();
            consume(TokenType::COLON, "Expect ':' after case pattern.");
            StmtPtr body = statement();
            cases.emplace_back(pattern, body);
        } else {
            throw error(peek(), "Expect 'case' pattern or 'default'.");
        }
    }
    
    consume(TokenType::RBRACE, "Expect '}' after match cases.");
    
    return std::make_shared<MatchStmt>(match_expr, std::move(cases), default_case);
}

std::vector<StmtPtr> Parser::block() {
    std::vector<StmtPtr> statements;
    while (!check(TokenType::RBRACE) && !is_at_end()) {
        auto stmt = declaration();
        if (stmt) {
            statements.push_back(*stmt);
        }
    }
    consume(TokenType::RBRACE, "Expect '}' after block.");
    return statements;
}

StmtPtr Parser::expression_statement() {
    ExprPtr expr = expression();
    return std::make_shared<ExpressionStmt>(std::move(expr));
}

ExprPtr Parser::expression() {
    return assignment();
}

ExprPtr Parser::assignment() {
    ExprPtr expr = or_expr();
    
    if (match({TokenType::EQUALS})) {
        Token equals = previous();
        ExprPtr value = assignment();
        
        if (auto* var = dynamic_cast<Variable*>(expr.get())) {
            return std::make_shared<Assign>(var->name, std::move(value));
        } else if (auto* get = dynamic_cast<Get*>(expr.get())) {
            return std::make_shared<Set>(get->obj, get->name, std::move(value));
        }
        
        throw error(equals, "Invalid assignment target.");
    }
    
    return expr;
}

ExprPtr Parser::or_expr() {
    ExprPtr expr = and_expr();
    
    while (match({TokenType::OR})) {
        Token op = previous();
        ExprPtr right = and_expr();
        expr = std::make_shared<Logical>(std::move(expr), op, std::move(right));
    }
    
    return expr;
}

ExprPtr Parser::and_expr() {
    ExprPtr expr = equality();
    
    while (match({TokenType::AND})) {
        Token op = previous();
        ExprPtr right = equality();
        expr = std::make_shared<Logical>(std::move(expr), op, std::move(right));
    }
    
    return expr;
}

ExprPtr Parser::equality() {
    ExprPtr expr = comparison();
    
    while (match({TokenType::DOUBLE_EQUALS, TokenType::NOT_EQUALS})) {
        Token op = previous();
        ExprPtr right = comparison();
        expr = std::make_shared<Binary>(std::move(expr), op, std::move(right));
    }
    
    return expr;
}

ExprPtr Parser::comparison() {
    ExprPtr expr = term();
    
    while (match({TokenType::GREATER, TokenType::GREATER_EQUALS,
                  TokenType::LESS, TokenType::LESS_EQUALS})) {
        Token op = previous();
        ExprPtr right = term();
        expr = std::make_shared<Binary>(std::move(expr), op, std::move(right));
    }
    
    return expr;
}

ExprPtr Parser::term() {
    ExprPtr expr = factor();
    
    while (match({TokenType::MINUS, TokenType::PLUS})) {
        Token op = previous();
        ExprPtr right = factor();
        expr = std::make_shared<Binary>(std::move(expr), op, std::move(right));
    }
    
    return expr;
}

ExprPtr Parser::factor() {
    ExprPtr expr = unary();
    
    while (match({TokenType::SLASH, TokenType::STAR, TokenType::PERCENT})) {
        Token op = previous();
        ExprPtr right = unary();
        expr = std::make_shared<Binary>(std::move(expr), op, std::move(right));
    }
    
    return expr;
}

ExprPtr Parser::unary() {
    if (match({TokenType::NOT, TokenType::MINUS, TokenType::AT})) {
        Token op = previous();
        ExprPtr right = unary();
        return std::make_shared<Unary>(op, std::move(right));
    }
    return call();
}

ExprPtr Parser::call() {
    ExprPtr expr = primary();
    
    while (true) {
        if (match({TokenType::LPAREN})) {
            expr = finish_call(expr);
        } else if (match({TokenType::DOT})) {
            Token name = consume_identifier("Expect property name after '.'.");
            expr = std::make_shared<Get>(std::move(expr), name);
        } else if (match({TokenType::LBRACKET})) {
            ExprPtr index = expression();
            consume(TokenType::RBRACKET, "Expect ']' after index.");
            expr = std::make_shared<IndexExpr>(std::move(expr), std::move(index));
        } else {
            break;
        }
    }
    
    return expr;
}

ExprPtr Parser::finish_call(ExprPtr callee) {
    std::vector<ExprPtr> arguments;
    
    if (!check(TokenType::RPAREN)) {
        while (true) {
            arguments.push_back(expression());
            if (!match({TokenType::COMMA})) break;
        }
    }
    
    consume(TokenType::RPAREN, "Expect ')' after arguments.");
    return std::make_shared<Call>(std::move(callee), std::move(arguments));
}

ExprPtr Parser::primary() {
    if (match({TokenType::NUMBER, TokenType::STRING})) {
        Token tok = previous();
        if (tok.type == TokenType::NUMBER) {
            return std::make_shared<Literal>(tok.literal);
        } else {
            return std::make_shared<Literal>(std::string(tok.lexeme));
        }
    }
    
    if (match({TokenType::SYSTEM})) {
        return std::make_shared<Variable>(previous());
    }
    
    if (match({TokenType::NEW})) {
        Token name = consume_identifier("Expect class name after 'n'.");
        std::vector<ExprPtr> arguments;
        
        if (match({TokenType::LPAREN})) {
            if (!check(TokenType::RPAREN)) {
                while (true) {
                    arguments.push_back(expression());
                    if (!match({TokenType::COMMA})) break;
                }
            }
            consume(TokenType::RPAREN, "Expect ')' after arguments.");
        }
        
        return std::make_shared<New>(name, std::move(arguments));
    }
    
    if (is_identifier()) {
        return std::make_shared<Variable>(advance());
    }
    
    if (match({TokenType::LBRACKET})) {
        std::vector<ExprPtr> elements;
        if (!check(TokenType::RBRACKET)) {
            while (true) {
                elements.push_back(expression());
                if (!match({TokenType::COMMA})) break;
            }
        }
        consume(TokenType::RBRACKET, "Expect ']' after list elements.");
        return std::make_shared<ListLiteral>(std::move(elements));
    }
    
    if (match({TokenType::LBRACE})) {
        std::vector<ExprPtr> keys;
        std::vector<ExprPtr> values;
        
        if (!check(TokenType::RBRACE)) {
            while (true) {
                keys.push_back(expression());
                consume(TokenType::COLON, "Expect ':' after map key.");
                values.push_back(expression());
                if (!match({TokenType::COMMA})) break;
            }
        }
        consume(TokenType::RBRACE, "Expect '}' after map elements.");
        return std::make_shared<MapLiteral>(std::move(keys), std::move(values));
    }
    
    if (match({TokenType::LPAREN})) {
        ExprPtr expr = expression();
        consume(TokenType::RPAREN, "Expect ')' after expression.");
        return std::make_shared<Grouping>(std::move(expr));
    }
    
    throw error(peek(), "Expect expression.");
}

}

#include "lexer.h"
#include <cctype>
#include <stdexcept>

namespace alphabet {

const char *token_type_to_string(TokenType type)
{
    switch (type) {
    case TokenType::IF:
        return "IF";
    case TokenType::ELSE:
        return "ELSE";
    case TokenType::LOOP:
        return "LOOP";
    case TokenType::BREAK:
        return "BREAK";
    case TokenType::CONTINUE:
        return "CONTINUE";
    case TokenType::RETURN:
        return "RETURN";
    case TokenType::CLASS:
        return "CLASS";
    case TokenType::ABSTRACT:
        return "ABSTRACT";
    case TokenType::INTERFACE:
        return "INTERFACE";
    case TokenType::NEW:
        return "NEW";
    case TokenType::EXTENDS:
        return "EXTENDS";
    case TokenType::PUBLIC:
        return "PUBLIC";
    case TokenType::PRIVATE:
        return "PRIVATE";
    case TokenType::STATIC:
        return "STATIC";
    case TokenType::METHOD:
        return "METHOD";
    case TokenType::TRY:
        return "TRY";
    case TokenType::HANDLE:
        return "HANDLE";
    case TokenType::SYSTEM:
        return "SYSTEM";
    case TokenType::TOK_CONST:
        return "CONST";
    case TokenType::IDENTIFIER:
        return "IDENTIFIER";
    case TokenType::NUMBER:
        return "NUMBER";
    case TokenType::STRING:
        return "STRING";
    case TokenType::PLUS:
        return "PLUS";
    case TokenType::MINUS:
        return "MINUS";
    case TokenType::STAR:
        return "STAR";
    case TokenType::SLASH:
        return "SLASH";
    case TokenType::PERCENT:
        return "PERCENT";
    case TokenType::EQUALS:
        return "EQUALS";
    case TokenType::DOUBLE_EQUALS:
        return "DOUBLE_EQUALS";
    case TokenType::NOT_EQUALS:
        return "NOT_EQUALS";
    case TokenType::GREATER:
        return "GREATER";
    case TokenType::LESS:
        return "LESS";
    case TokenType::GREATER_EQUALS:
        return "GREATER_EQUALS";
    case TokenType::LESS_EQUALS:
        return "LESS_EQUALS";
    case TokenType::AND:
        return "AND";
    case TokenType::OR:
        return "OR";
    case TokenType::NOT:
        return "NOT";
    case TokenType::DOT:
        return "DOT";
    case TokenType::AT:
        return "AT";
    case TokenType::LBRACE:
        return "LBRACE";
    case TokenType::RBRACE:
        return "RBRACE";
    case TokenType::LPAREN:
        return "LPAREN";
    case TokenType::RPAREN:
        return "RPAREN";
    case TokenType::LBRACKET:
        return "LBRACKET";
    case TokenType::RBRACKET:
        return "RBRACKET";
    case TokenType::COMMA:
        return "COMMA";
    case TokenType::COLON:
        return "COLON";
    case TokenType::EOF_TOKEN:
        return "EOF";
    default:
        return "UNKNOWN";
    }
}

Lexer::Lexer(std::string_view source) : source_(source) {}

std::vector<Token> Lexer::scan_tokens()
{
    if (source_.size() >= 2 && source_[0] == '#' && source_[1] == '!') {
        while (peek() != '\n' && !is_at_end()) {
            advance();
        }
        if (peek() == '\n') {
            advance();
        }
    }

    validate_header();

    // Pre-count escape sequences to reserve string_pool_ and prevent
    // reallocation from invalidating string_view tokens
    size_t escape_count = 0;
    for (size_t i = current_; i < source_.size(); ++i) {
        if (source_[i] == '\\')
            escape_count++;
    }
    string_pool_.reserve(escape_count);

    while (!is_at_end()) {
        start_ = current_;
        start_column_ = column_;
        scan_token();
    }

    tokens_.emplace_back(TokenType::EOF_TOKEN, std::string_view(), 0, line_, column_);
    return tokens_;
}

void Lexer::validate_header()
{
    std::string_view header_source = source_.substr(current_);

    if (header_source.size() < 12) {
        throw MissingLanguageHeader();
    }

    const std::string_view prefix = "#alphabet<";
    if (header_source.substr(0, prefix.size()) != prefix) {
        throw MissingLanguageHeader();
    }

    size_t close_pos = header_source.find('>', prefix.size());
    if (close_pos == std::string_view::npos) {
        throw MissingLanguageHeader();
    }

    // Extract language code
    size_t lang_start = prefix.size();
    size_t lang_length = close_pos - lang_start;
    language_ = std::string(header_source.substr(lang_start, lang_length));

    size_t newline_pos = header_source.find('\n', close_pos);
    if (newline_pos != std::string_view::npos) {
        current_ = current_ + newline_pos + 1;
        start_ = current_;
        line_ = 2;
        column_ = 0;
    }
}

bool Lexer::is_at_end() const
{
    return current_ >= source_.size();
}

void Lexer::scan_token()
{
    char c = advance();

    switch (c) {
    case '(':
        add_token(TokenType::LPAREN);
        break;
    case ')':
        add_token(TokenType::RPAREN);
        break;
    case '{':
        add_token(TokenType::LBRACE);
        break;
    case '}':
        add_token(TokenType::RBRACE);
        break;
    case '[':
        add_token(TokenType::LBRACKET);
        break;
    case ']':
        add_token(TokenType::RBRACKET);
        break;
    case ',':
        add_token(TokenType::COMMA);
        break;
    case ':':
        add_token(TokenType::COLON);
        break;
    case '.':
        add_token(TokenType::DOT);
        break;
    case '-':
        add_token(TokenType::MINUS);
        break;
    case '+':
        add_token(TokenType::PLUS);
        break;
    case '*':
        add_token(TokenType::STAR);
        break;
    case '%':
        add_token(TokenType::PERCENT);
        break;
    case '^':
        add_token(TokenType::EXTENDS);
        break;
    case '@':
        add_token(TokenType::AT);
        break;

    case '!':
        add_token(match('=') ? TokenType::NOT_EQUALS : TokenType::NOT);
        break;

    case '=':
        add_token(match('=') ? TokenType::DOUBLE_EQUALS : TokenType::EQUALS);
        break;

    case '<':
        add_token(match('=') ? TokenType::LESS_EQUALS : TokenType::LESS);
        break;

    case '>':
        add_token(match('=') ? TokenType::GREATER_EQUALS : TokenType::GREATER);
        break;

    case '/':
        if (match('/')) {
            while (peek() != '\n' && !is_at_end()) {
                advance();
            }
        }
        else if (match('*')) {
            // Multi-line comment: consume until */
            while (!is_at_end()) {
                if (peek() == '*' && peek_next() == '/') {
                    advance(); // consume *
                    advance(); // consume /
                    break;
                }
                advance();
            }
        }
        else {
            add_token(TokenType::SLASH);
        }
        break;

    case '&':
        if (match('&')) {
            add_token(TokenType::AND);
        }
        break;

    case '|':
        if (match('|')) {
            add_token(TokenType::OR);
        }
        break;

    case ' ':
    case '\r':
    case '\t':
        break;

    case '\n':
        break;

    case '\"':
        if (peek() == '"' && peek_next() == '"') {
            multi_line_string();
        }
        else {
            string();
        }
        break;

    default:
        if (std::isdigit(static_cast<unsigned char>(c))) {
            number();
        }
        else if (std::isalpha(static_cast<unsigned char>(c)) ||
                 static_cast<unsigned char>(c) > 127) {
            // ASCII letters or UTF-8 characters
            identifier();
        }
        break;
    }
}

char Lexer::advance()
{
    char c = source_[current_++];
    if (c == '\n') {
        ++line_;
        column_ = 0;
    }
    else {
        ++column_;
    }
    return c;
}

char Lexer::peek() const
{
    if (is_at_end())
        return '\0';
    return source_[current_];
}

char Lexer::peek_next() const
{
    if (current_ + 1 >= source_.size())
        return '\0';
    return source_[current_ + 1];
}

bool Lexer::match(char expected)
{
    if (is_at_end())
        return false;
    if (source_[current_] != expected)
        return false;
    ++current_;
    return true;
}

void Lexer::add_token(TokenType type, double literal)
{
    std::string_view lexeme = source_.substr(start_, current_ - start_);
    tokens_.emplace_back(type, lexeme, literal, line_, start_column_);
}

void Lexer::string()
{
    std::string processed;
    bool has_escapes = false;

    while (peek() != '"' && !is_at_end()) {
        if (peek() == '\\') {
            has_escapes = true;
            advance(); // consume backslash
            if (is_at_end())
                return;

            char escaped = advance();
            switch (escaped) {
            case 'n':
                processed += '\n';
                break;
            case 't':
                processed += '\t';
                break;
            case '\\':
                processed += '\\';
                break;
            case '"':
                processed += '"';
                break;
            case '0':
                processed += '\0';
                break;
            default:
                // Unknown escape -- keep as-is
                processed += '\\';
                processed += escaped;
                break;
            }
        }
        else {
            processed += advance();
        }
    }

    if (is_at_end())
        return;

    advance(); // consume closing "

    if (has_escapes) {
        string_pool_.push_back(std::move(processed));
        tokens_.emplace_back(TokenType::STRING, string_pool_.back(), 0, line_, start_column_);
    }
    else {
        std::string_view value = source_.substr(start_ + 1, current_ - start_ - 2);
        tokens_.emplace_back(TokenType::STRING, value, 0, line_, start_column_);
    }
}

void Lexer::multi_line_string()
{
    advance(); // consume second "
    advance(); // consume third "

    std::string processed;
    bool has_escapes = false;

    while (!is_at_end()) {
        if (peek() == '"' && current_ + 2 < source_.size() && source_[current_ + 1] == '"' &&
            source_[current_ + 2] == '"') {
            advance(); // consume first "
            advance(); // consume second "
            advance(); // consume third "
            break;
        }
        if (peek() == '\\') {
            has_escapes = true;
            advance();
            if (is_at_end())
                break;
            char escaped = advance();
            switch (escaped) {
            case 'n':
                processed += '\n';
                break;
            case 't':
                processed += '\t';
                break;
            case '\\':
                processed += '\\';
                break;
            case '"':
                processed += '"';
                break;
            case '0':
                processed += '\0';
                break;
            default:
                processed += '\\';
                processed += escaped;
                break;
            }
        }
        else {
            char c = advance();
            if (c == '\n')
                ++line_;
            processed += c;
        }
    }

    string_pool_.push_back(std::move(processed));
    tokens_.emplace_back(TokenType::STRING, string_pool_.back(), 0, line_, start_column_);
}

void Lexer::number()
{
    while (std::isdigit(static_cast<unsigned char>(peek()))) {
        advance();
    }

    if (peek() == '.' && std::isdigit(static_cast<unsigned char>(peek_next()))) {
        advance(); // consume the '.'
        while (std::isdigit(static_cast<unsigned char>(peek()))) {
            advance();
        }
    }

    std::string_view num_str = source_.substr(start_, current_ - start_);
    double value = std::stod(std::string(num_str));
    tokens_.emplace_back(TokenType::NUMBER, num_str, value, line_);
}

void Lexer::identifier()
{
    while (std::isalnum(static_cast<unsigned char>(peek())) || peek() == '_' ||
           static_cast<unsigned char>(peek()) > 127) {
        advance();
    }

    std::string_view text = source_.substr(start_, current_ - start_);
    std::string text_str(text);

    // Check for true/false literals
    if (text == "true" || text == "false") {
        add_token(TokenType::NUMBER, (text == "true") ? 1.0 : 0.0);
        return;
    }

    // Check for const keyword (single-word, not a keyword char)
    if (text == "const") {
        add_token(TokenType::TOK_CONST);
        return;
    }

    // Check if this is a UTF-8 keyword
    if (is_utf8_keyword(text_str)) {
        std::string translated = translate_keyword(text_str, language_);
        // If translation is different from original, it's a keyword
        if (translated != text_str) {
            // It's a translated keyword - process the translation
            if (translated == "z") {
                // Handle system functions - emit 'z' as SYSTEM token
                tokens_.emplace_back(TokenType::SYSTEM, std::string_view("z"), 0, line_);
                return;
            }
            if (translated == "z.i") {
                // Handle system.input - emit 'z' as SYSTEM token
                tokens_.emplace_back(TokenType::SYSTEM, std::string_view("z"), 0, line_);
                return;
            }
            if (translated == "\x80") {
                add_token(TokenType::TOK_CONST);
                return;
            }
            // For other translated keywords, check first char
            if (translated.size() == 1 && is_keyword_char(translated[0])) {
                add_token(keyword_type(translated[0]));
                return;
            }
        }
        // UTF-8 identifier (not a keyword)
        add_token(TokenType::IDENTIFIER);
        return;
    }

    // ASCII keyword handling - check for aliases first
    std::string translated = translate_keyword(text_str, language_);
    if (translated != text_str) {
        // It's a translated ASCII keyword
        if (translated == "z") {
            tokens_.emplace_back(TokenType::SYSTEM, std::string_view("z"), 0, line_);
            return;
        }
        if (translated == "z.i") {
            tokens_.emplace_back(TokenType::SYSTEM, std::string_view("z"), 0, line_);
            return;
        }
        if (translated == "\x80") {
            add_token(TokenType::TOK_CONST);
            return;
        }
        // Handle multi-char translated keywords only (class->c, method->m, new->n, etc.)
        // Single-char source keywords (like "n" as variable) are NOT translated to avoid ambiguity
        if (translated.size() == 1 && text_str.size() > 1) {
            char c = translated[0];
            if (is_keyword_char(c)) {
                add_token(keyword_type(c));
                return;
            }
        }
    }

    // Standard single-char keywords
    if (text.size() == 1 && is_keyword_char(text[0])) {
        add_token(keyword_type(text[0]));
    }
    else {
        add_token(TokenType::IDENTIFIER);
    }
}

bool Lexer::is_keyword_char(char c) const
{
    switch (c) {
    case 'i':
    case 'e':
    case 'l':
    case 'b':
    case 'k':
    case 'r':
    case 'c':
    case 'a':
    case 'j':
    case 'n':
    case 'v':
    case 'p':
    case 's':
    case 'm':
    case 't':
    case 'h':
    case 'z':
    case 'x':
    case 'q':
        return true;
    default:
        return false;
    }
}

TokenType Lexer::keyword_type(char c) const
{
    switch (c) {
    case 'i':
        return TokenType::IF;
    case 'e':
        return TokenType::ELSE;
    case 'l':
        return TokenType::LOOP;
    case 'b':
        return TokenType::BREAK;
    case 'k':
        return TokenType::CONTINUE;
    case 'r':
        return TokenType::RETURN;
    case 'c':
        return TokenType::CLASS;
    case 'a':
        return TokenType::ABSTRACT;
    case 'j':
        return TokenType::INTERFACE;
    case 'n':
        return TokenType::NEW;
    case 'v':
        return TokenType::PUBLIC;
    case 'p':
        return TokenType::PRIVATE;
    case 's':
        return TokenType::STATIC;
    case 'm':
        return TokenType::METHOD;
    case 't':
        return TokenType::TRY;
    case 'h':
        return TokenType::HANDLE;
    case 'z':
        return TokenType::SYSTEM;
    case 'x':
        return TokenType::IMPORT;
    case 'q':
        return TokenType::MATCH;
    default:
        return TokenType::IDENTIFIER;
    }
}

TokenType Lexer::previous_token_type() const
{
    if (tokens_.empty())
        return TokenType::EOF_TOKEN;
    return tokens_.back().type;
}

} // namespace alphabet

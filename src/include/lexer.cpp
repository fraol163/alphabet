#include "lexer.h"
#include <cctype>
#include <stdexcept>

namespace alphabet {

const char* token_type_to_string(TokenType type) {
    switch (type) {
        case TokenType::IF: return "IF";
        case TokenType::ELSE: return "ELSE";
        case TokenType::LOOP: return "LOOP";
        case TokenType::BREAK: return "BREAK";
        case TokenType::CONTINUE: return "CONTINUE";
        case TokenType::RETURN: return "RETURN";
        case TokenType::CLASS: return "CLASS";
        case TokenType::ABSTRACT: return "ABSTRACT";
        case TokenType::INTERFACE: return "INTERFACE";
        case TokenType::NEW: return "NEW";
        case TokenType::EXTENDS: return "EXTENDS";
        case TokenType::PUBLIC: return "PUBLIC";
        case TokenType::PRIVATE: return "PRIVATE";
        case TokenType::STATIC: return "STATIC";
        case TokenType::METHOD: return "METHOD";
        case TokenType::TRY: return "TRY";
        case TokenType::HANDLE: return "HANDLE";
        case TokenType::SYSTEM: return "SYSTEM";
        case TokenType::IDENTIFIER: return "IDENTIFIER";
        case TokenType::NUMBER: return "NUMBER";
        case TokenType::STRING: return "STRING";
        case TokenType::PLUS: return "PLUS";
        case TokenType::MINUS: return "MINUS";
        case TokenType::STAR: return "STAR";
        case TokenType::SLASH: return "SLASH";
        case TokenType::PERCENT: return "PERCENT";
        case TokenType::EQUALS: return "EQUALS";
        case TokenType::DOUBLE_EQUALS: return "DOUBLE_EQUALS";
        case TokenType::NOT_EQUALS: return "NOT_EQUALS";
        case TokenType::GREATER: return "GREATER";
        case TokenType::LESS: return "LESS";
        case TokenType::GREATER_EQUALS: return "GREATER_EQUALS";
        case TokenType::LESS_EQUALS: return "LESS_EQUALS";
        case TokenType::AND: return "AND";
        case TokenType::OR: return "OR";
        case TokenType::NOT: return "NOT";
        case TokenType::DOT: return "DOT";
        case TokenType::AT: return "AT";
        case TokenType::LBRACE: return "LBRACE";
        case TokenType::RBRACE: return "RBRACE";
        case TokenType::LPAREN: return "LPAREN";
        case TokenType::RPAREN: return "RPAREN";
        case TokenType::LBRACKET: return "LBRACKET";
        case TokenType::RBRACKET: return "RBRACKET";
        case TokenType::COMMA: return "COMMA";
        case TokenType::COLON: return "COLON";
        case TokenType::EOF_TOKEN: return "EOF";
        default: return "UNKNOWN";
    }
}

Lexer::Lexer(std::string_view source) : source_(source) {}

std::vector<Token> Lexer::scan_tokens() {
    if (source_.size() >= 2 && source_[0] == '#' && source_[1] == '!') {
        while (peek() != '\n' && !is_at_end()) {
            advance();
        }
        if (peek() == '\n') {
            advance();
        }
    }

    validate_header();

    while (!is_at_end()) {
        start_ = current_;
        scan_token();
    }

    tokens_.emplace_back(TokenType::EOF_TOKEN, std::string_view(), 0, line_);
    return tokens_;
}

void Lexer::validate_header() {
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

    size_t newline_pos = header_source.find('\n', close_pos);
    if (newline_pos != std::string_view::npos) {
        current_ = current_ + newline_pos + 1;
        start_ = current_;
        line_ = 2;
    }
}

bool Lexer::is_at_end() const {
    return current_ >= source_.size();
}

void Lexer::scan_token() {
    char c = advance();

    switch (c) {
        case '(': add_token(TokenType::LPAREN); break;
        case ')': add_token(TokenType::RPAREN); break;
        case '{': add_token(TokenType::LBRACE); break;
        case '}': add_token(TokenType::RBRACE); break;
        case '[': add_token(TokenType::LBRACKET); break;
        case ']': add_token(TokenType::RBRACKET); break;
        case ',': add_token(TokenType::COMMA); break;
        case ':': add_token(TokenType::COLON); break;
        case '.': add_token(TokenType::DOT); break;
        case '-': add_token(TokenType::MINUS); break;
        case '+': add_token(TokenType::PLUS); break;
        case '*': add_token(TokenType::STAR); break;
        case '%': add_token(TokenType::PERCENT); break;
        case '^': add_token(TokenType::EXTENDS); break;
        case '@': add_token(TokenType::AT); break;
        
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
            } else {
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
            ++line_;
            break;
            
        case '"':
            string();
            break;
            
        default:
            if (std::isdigit(static_cast<unsigned char>(c))) {
                number();
            } else if (std::isalpha(static_cast<unsigned char>(c))) {
                identifier();
            }
            break;
    }
}

char Lexer::advance() {
    return source_[current_++];
}

char Lexer::peek() const {
    if (is_at_end()) return '\0';
    return source_[current_];
}

char Lexer::peek_next() const {
    if (current_ + 1 >= source_.size()) return '\0';
    return source_[current_ + 1];
}

bool Lexer::match(char expected) {
    if (is_at_end()) return false;
    if (source_[current_] != expected) return false;
    ++current_;
    return true;
}

void Lexer::add_token(TokenType type, double literal) {
    std::string_view lexeme = source_.substr(start_, current_ - start_);
    tokens_.emplace_back(type, lexeme, literal, line_);
}

void Lexer::string() {
    while (peek() != '"' && !is_at_end()) {
        if (peek() == '\n') ++line_;
        advance();
    }
    
    if (is_at_end()) return;

    advance();

    std::string_view value = source_.substr(start_ + 1, current_ - start_ - 2);
    tokens_.emplace_back(TokenType::STRING, value, 0, line_);
}

void Lexer::number() {
    while (std::isdigit(static_cast<unsigned char>(peek()))) {
        advance();
    }

    if (peek() == '.' && std::isdigit(static_cast<unsigned char>(peek_next()))) {
        while (std::isdigit(static_cast<unsigned char>(peek()))) {
            advance();
        }
    }
    
    std::string_view num_str = source_.substr(start_, current_ - start_);
    double value = std::stod(std::string(num_str));
    tokens_.emplace_back(TokenType::NUMBER, num_str, value, line_);
}

void Lexer::identifier() {
    while (std::isalnum(static_cast<unsigned char>(peek())) || static_cast<unsigned char>(peek()) > 127) {
        advance();
    }

    std::string_view text = source_.substr(start_, current_ - start_);

    if (text.size() == 1 && is_keyword_char(text[0])) {
        add_token(keyword_type(text[0]));
    } else {
        add_token(TokenType::IDENTIFIER);
    }
}

bool Lexer::is_keyword_char(char c) const {
    switch (c) {
        case 'i': case 'e': case 'l': case 'b': case 'k':
        case 'r': case 'c': case 'a': case 'j': case 'n':
        case 'v': case 'p': case 's': case 'm': case 't':
        case 'h': case 'z':
            return true;
        default:
            return false;
    }
}

TokenType Lexer::keyword_type(char c) const {
    switch (c) {
        case 'i': return TokenType::IF;
        case 'e': return TokenType::ELSE;
        case 'l': return TokenType::LOOP;
        case 'b': return TokenType::BREAK;
        case 'k': return TokenType::CONTINUE;
        case 'r': return TokenType::RETURN;
        case 'c': return TokenType::CLASS;
        case 'a': return TokenType::ABSTRACT;
        case 'j': return TokenType::INTERFACE;
        case 'n': return TokenType::NEW;
        case 'v': return TokenType::PUBLIC;
        case 'p': return TokenType::PRIVATE;
        case 's': return TokenType::STATIC;
        case 'm': return TokenType::METHOD;
        case 't': return TokenType::TRY;
        case 'h': return TokenType::HANDLE;
        case 'z': return TokenType::SYSTEM;
        default: return TokenType::IDENTIFIER;
    }
}

}

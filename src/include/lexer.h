#ifndef ALPHABET_LEXER_H
#define ALPHABET_LEXER_H

#include <string>
#include <string_view>
#include <vector>
#include <cstdint>
#include <stdexcept>
#include <unordered_map>
#include "keywords.h"

namespace alphabet {

enum class TokenType : int {
    // Keywords (single letter)
    IF = 100,
    ELSE = 101,
    LOOP = 102,
    BREAK = 103,
    CONTINUE = 104,
    RETURN = 105,
    CLASS = 106,
    ABSTRACT = 107,
    INTERFACE = 108,
    NEW = 109,
    PUBLIC = 110,
    PRIVATE = 111,
    STATIC = 112,
    METHOD = 113,
    TRY = 114,
    HANDLE = 115,
    SYSTEM = 116,
    IMPORT = 117,
    MATCH = 118,
    TOK_CONST = 119,
    
    // Operators and special chars
    EXTENDS = '^',
    EXPORT = '@',

    IDENTIFIER = 200,
    NUMBER = 201,
    STRING = 202,

    PLUS = '+', MINUS = '-', STAR = '*', SLASH = '/', PERCENT = '%',
    EQUALS = '=', DOUBLE_EQUALS = 300, NOT_EQUALS = 301,
    GREATER = '>', LESS = '<', GREATER_EQUALS = 302, LESS_EQUALS = 303,
    AND = 304, OR = 305, NOT = 306,

    DOT = '.', AT = '@',
    LBRACE = '{', RBRACE = '}',
    LPAREN = '(', RPAREN = ')',
    LBRACKET = '[', RBRACKET = ']',
    COMMA = ',', COLON = ':',

    EOF_TOKEN = 0
};

struct Token {
    TokenType type;
    std::string_view lexeme;
    double literal;
    size_t line;
    size_t column;
    
    Token() : type(TokenType::EOF_TOKEN), literal(0), line(1), column(0) {}
    Token(TokenType t, std::string_view lex, double lit, size_t ln, size_t col = 0)
        : type(t), lexeme(lex), literal(lit), line(ln), column(col) {}
};

class MissingLanguageHeader : public std::runtime_error {
public:
    MissingLanguageHeader() : std::runtime_error("Missing magic header '#alphabet<lang>' on line 1") {}
};

class Lexer {
public:
    explicit Lexer(std::string_view source);

    std::vector<Token> scan_tokens();

private:
    std::string_view source_;
    std::vector<Token> tokens_;
    std::vector<std::string> string_pool_;  // Owns processed strings with escapes
    size_t start_ = 0;
    size_t current_ = 0;
    size_t line_ = 1;
    size_t column_ = 0;  // Current column (0-based)
    size_t start_column_ = 0;  // Column at start of current token
    std::string language_ = "en";  // Default language

    [[nodiscard]] bool is_at_end() const;
    void scan_token();
    char advance();
    [[nodiscard]] char peek() const;
    [[nodiscard]] char peek_next() const;
    bool match(char expected);
    void add_token(TokenType type, double literal = 0);
    void string();
    void multi_line_string();
    void number();
    void identifier();
    [[nodiscard]] bool is_keyword_char(char c) const;
    [[nodiscard]] TokenType keyword_type(char c) const;
    void validate_header();
    [[nodiscard]] TokenType previous_token_type() const;
    std::string extract_identifier_string() const;
};

const char* token_type_to_string(TokenType type);

}

#endif

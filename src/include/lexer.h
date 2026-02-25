#ifndef ALPHABET_LEXER_H
#define ALPHABET_LEXER_H

#include <string>
#include <string_view>
#include <vector>
#include <cstdint>
#include <stdexcept>

namespace alphabet {

enum class TokenType {
    IF = 'i',
    ELSE = 'e',
    LOOP = 'l',
    BREAK = 'b',
    CONTINUE = 'k',
    RETURN = 'r',
    CLASS = 'c',
    ABSTRACT = 'a',
    INTERFACE = 'j',
    NEW = 'n',
    EXTENDS = '^',
    PUBLIC = 'v',
    PRIVATE = 'p',
    STATIC = 's',
    METHOD = 'm',
    TRY = 't',
    HANDLE = 'h',
    SYSTEM = 'z',
    
    IDENTIFIER,
    NUMBER,
    STRING,

    PLUS, MINUS, STAR, SLASH, PERCENT,
    EQUALS, DOUBLE_EQUALS, NOT_EQUALS,
    GREATER, LESS, GREATER_EQUALS, LESS_EQUALS,
    AND, OR, NOT,
    
    DOT, AT,
    LBRACE, RBRACE,
    LPAREN, RPAREN,
    LBRACKET, RBRACKET,
    COMMA, COLON,

    EOF_TOKEN
};

struct Token {
    TokenType type;
    std::string_view lexeme;  // Zero-copy reference into source
    double literal;           // For NUMBER tokens
    size_t line;
    
    Token() : type(TokenType::EOF_TOKEN), literal(0), line(1) {}
    Token(TokenType t, std::string_view lex, double lit, size_t ln)
        : type(t), lexeme(lex), literal(lit), line(ln) {}
};

class MissingLanguageHeader : public std::runtime_error {
public:
    MissingLanguageHeader() : std::runtime_error("Missing magic header '#alphabet<lang>' on line 1") {}
};

class Lexer {
public:
    explicit Lexer(std::string_view source);
    
    // Scan all tokens from source
    std::vector<Token> scan_tokens();
    
private:
    std::string_view source_;
    std::vector<Token> tokens_;
    size_t start_ = 0;
    size_t current_ = 0;
    size_t line_ = 1;

    [[nodiscard]] bool is_at_end() const;
    void scan_token();
    char advance();
    [[nodiscard]] char peek() const;
    [[nodiscard]] char peek_next() const;
    bool match(char expected);
    void add_token(TokenType type, double literal = 0);
    void string();
    void number();
    void identifier();
    [[nodiscard]] bool is_keyword_char(char c) const;
    [[nodiscard]] TokenType keyword_type(char c) const;
    void validate_header();
};

const char* token_type_to_string(TokenType type);

} // namespace alphabet

#endif // ALPHABET_LEXER_H

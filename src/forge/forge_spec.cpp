#include "forge_spec.h"
#include <fstream>
#include <sstream>
#include <cctype>
#include <stdexcept>
#include <iostream>
#include <filesystem>

namespace alphabet {
namespace forge {

namespace {

enum class TokenType {
    IDENTIFIER,
    STRING,
    NUMBER,
    LBRACE,
    RBRACE,
    LPAREN,
    RPAREN,
    LBRACKET,
    RBRACKET,
    COLON,
    SEMICOLON,
    EQUALS,
    ARROW,      // =>
    PIPE,       // |
    STAR,       // *
    PLUS,       // +
    QUESTION,   // ?
    END_OF_FILE
};

struct Token {
    TokenType type;
    std::string text;
    int line = 1;
};

class ForgeLexer {
public:
    explicit ForgeLexer(const std::string& src) : source(src), pos(0), line(1) {}

    std::vector<Token> tokenize() {
        std::vector<Token> tokens;
        while (pos < source.length()) {
            skip_whitespace_and_comments();
            if (pos >= source.length()) break;

            char c = source[pos];
            if (c == '{') { tokens.push_back({TokenType::LBRACE, "{", line}); pos++; }
            else if (c == '}') { tokens.push_back({TokenType::RBRACE, "}", line}); pos++; }
            else if (c == '(') { tokens.push_back({TokenType::LPAREN, "(", line}); pos++; }
            else if (c == ')') { tokens.push_back({TokenType::RPAREN, ")", line}); pos++; }
            else if (c == '[') { tokens.push_back({TokenType::LBRACKET, "[", line}); pos++; }
            else if (c == ']') { tokens.push_back({TokenType::RBRACKET, "]", line}); pos++; }
            else if (c == ':') { tokens.push_back({TokenType::COLON, ":", line}); pos++; }
            else if (c == ';') { tokens.push_back({TokenType::SEMICOLON, ";", line}); pos++; }
            else if (c == '|') { tokens.push_back({TokenType::PIPE, "|", line}); pos++; }
            else if (c == '*') { tokens.push_back({TokenType::STAR, "*", line}); pos++; }
            else if (c == '+') { tokens.push_back({TokenType::PLUS, "+", line}); pos++; }
            else if (c == '?') { tokens.push_back({TokenType::QUESTION, "?", line}); pos++; }
            else if (c == '=') {
                if (pos + 1 < source.length() && source[pos + 1] == '>') {
                    tokens.push_back({TokenType::ARROW, "=>", line});
                    pos += 2;
                } else {
                    tokens.push_back({TokenType::EQUALS, "=", line});
                    pos++;
                }
            } else if (c == '"') {
                tokens.push_back(lex_string());
            } else if (std::isalpha(static_cast<unsigned char>(c)) || c == '_') {
                tokens.push_back(lex_identifier());
            } else if (std::isdigit(static_cast<unsigned char>(c))) {
                tokens.push_back(lex_number());
            } else {
                pos++;
            }
        }
        tokens.push_back({TokenType::END_OF_FILE, "", line});
        return tokens;
    }

private:
    std::string source;
    size_t pos;
    int line;

    void skip_whitespace_and_comments() {
        while (pos < source.length()) {
            char c = source[pos];
            if (c == '\n') {
                line++;
                pos++;
            } else if (std::isspace(static_cast<unsigned char>(c))) {
                pos++;
            } else if (c == '/' && pos + 1 < source.length() && source[pos + 1] == '/') {
                pos += 2;
                while (pos < source.length() && source[pos] != '\n') pos++;
            } else if (c == '/' && pos + 1 < source.length() && source[pos + 1] == '*') {
                pos += 2;
                while (pos + 1 < source.length() && !(source[pos] == '*' && source[pos + 1] == '/')) {
                    if (source[pos] == '\n') line++;
                    pos++;
                }
                if (pos + 1 < source.length()) pos += 2;
            } else {
                break;
            }
        }
    }

    Token lex_string() {
        int start_line = line;
        // Check for triple quotes """
        if (pos + 2 < source.length() && source[pos + 1] == '"' && source[pos + 2] == '"') {
            pos += 3;
            std::string text;
            while (pos + 2 < source.length()) {
                if (source[pos] == '"' && source[pos + 1] == '"' && source[pos + 2] == '"') {
                    pos += 3;
                    return {TokenType::STRING, text, start_line};
                }
                if (source[pos] == '\n') line++;
                text += source[pos++];
            }
            return {TokenType::STRING, text, start_line};
        }

        // Single quote "..."
        pos++;
        std::string text;
        while (pos < source.length() && source[pos] != '"') {
            if (source[pos] == '\\' && pos + 1 < source.length()) {
                pos++;
                char esc = source[pos++];
                if (esc == 'n') text += '\n';
                else if (esc == 't') text += '\t';
                else if (esc == 'r') text += '\r';
                else if (esc == '\\') text += '\\';
                else if (esc == '"') text += '"';
                else text += esc;
            } else {
                if (source[pos] == '\n') line++;
                text += source[pos++];
            }
        }
        if (pos < source.length() && source[pos] == '"') pos++;
        return {TokenType::STRING, text, start_line};
    }

    Token lex_identifier() {
        int start_line = line;
        size_t start = pos;
        while (pos < source.length() && (std::isalnum(static_cast<unsigned char>(source[pos])) || source[pos] == '_' || source[pos] == '.' || source[pos] == '-')) {
            pos++;
        }
        return {TokenType::IDENTIFIER, source.substr(start, pos - start), start_line};
    }

    Token lex_number() {
        int start_line = line;
        size_t start = pos;
        while (pos < source.length() && (std::isdigit(static_cast<unsigned char>(source[pos])) || source[pos] == '.')) {
            pos++;
        }
        return {TokenType::NUMBER, source.substr(start, pos - start), start_line};
    }
};

class ForgeParser {
public:
    explicit ForgeParser(std::vector<Token> tks) : tokens(std::move(tks)), idx(0) {}

    bool parse(ForgeSpec& spec, std::string& err) {
        try {
            if (match_keyword("language")) {
                spec.name = expect(TokenType::IDENTIFIER, "Expected language name").text;
                expect(TokenType::LBRACE, "Expected '{' after language name");
                parse_language_body(spec);
                expect(TokenType::RBRACE, "Expected '}' at end of language definition");
                return true;
            } else {
                err = "Expected 'language <Name> { ... }' at top-level";
                return false;
            }
        } catch (const std::exception& e) {
            err = e.what();
            return false;
        }
    }

private:
    std::vector<Token> tokens;
    size_t idx;

    Token peek() const {
        if (idx < tokens.size()) return tokens[idx];
        return {TokenType::END_OF_FILE, "", 0};
    }

    Token advance() {
        if (idx < tokens.size()) return tokens[idx++];
        return {TokenType::END_OF_FILE, "", 0};
    }

    bool check(TokenType type) const {
        return peek().type == type;
    }

    bool match(TokenType type) {
        if (check(type)) {
            advance();
            return true;
        }
        return false;
    }

    bool match_keyword(const std::string& kw) {
        if (check(TokenType::IDENTIFIER) && peek().text == kw) {
            advance();
            return true;
        }
        return false;
    }

    Token expect(TokenType type, const std::string& msg) {
        if (check(type)) return advance();
        throw std::runtime_error("Line " + std::to_string(peek().line) + ": " + msg + " (got '" + peek().text + "')");
    }

    void parse_language_body(ForgeSpec& spec) {
        while (!check(TokenType::RBRACE) && !check(TokenType::END_OF_FILE)) {
            if (match_keyword("version")) {
                expect_colon();
                spec.version = expect_string();
            } else if (match_keyword("extension")) {
                expect_colon();
                spec.extension = expect_string();
            } else if (match_keyword("paradigm")) {
                expect_colon();
                spec.paradigm = expect_string();
            } else if (match_keyword("types") || match_keyword("type_system")) {
                expect_colon();
                spec.type_system = expect_string();
            } else if (match_keyword("author")) {
                expect_colon();
                spec.author = expect_string();
            } else if (match_keyword("memory")) {
                expect_colon();
                spec.memory_model = expect_string();
            } else if (match_keyword("concurrency")) {
                expect_colon();
                spec.concurrency = expect_string();
            } else if (match_keyword("header")) {
                parse_header_block(spec.header, spec.name);
            } else if (match_keyword("banner")) {
                parse_banner_block(spec.banner);
            } else if (match_keyword("tokens")) {
                parse_tokens_block(spec.tokens);
            } else if (match_keyword("grammar")) {
                parse_grammar_block(spec);
            } else {
                // Unknown field or skip
                advance();
            }
        }
    }

    void expect_colon() {
        if (check(TokenType::COLON)) advance();
    }

    std::string expect_string() {
        if (check(TokenType::STRING) || check(TokenType::IDENTIFIER) || check(TokenType::NUMBER)) {
            return advance().text;
        }
        throw std::runtime_error("Line " + std::to_string(peek().line) + ": Expected string or identifier");
    }

    void parse_header_block(HeaderConfig& header, const std::string& lang_name) {
        expect(TokenType::LBRACE, "Expected '{' after header");
        while (!check(TokenType::RBRACE) && !check(TokenType::END_OF_FILE)) {
            if (match_keyword("style")) {
                expect_colon();
                header.style = HeaderEngine::parse_style(expect_string());
            } else if (match_keyword("prefix")) {
                expect_colon();
                header.prefix = expect_string();
            } else if (match_keyword("required")) {
                expect_colon();
                std::string val = expect_string();
                header.required = (val == "true" || val == "1");
            } else if (match_keyword("syntax")) {
                expect_colon();
                header.syntax_pattern = expect_string();
            } else if (match_keyword("default")) {
                expect_colon();
                header.default_version = expect_string();
            } else {
                advance();
            }
        }
        expect(TokenType::RBRACE, "Expected '}' after header block");
        if (header.prefix.empty()) {
            header.prefix = "#" + lang_name;
        }
    }

    void parse_banner_block(BannerConfig& banner) {
        expect(TokenType::LBRACE, "Expected '{' after banner");
        while (!check(TokenType::RBRACE) && !check(TokenType::END_OF_FILE)) {
            if (match_keyword("art")) {
                expect_colon();
                banner.art = expect_string();
            } else if (match_keyword("color")) {
                expect_colon();
                banner.color = expect_string();
            } else if (match_keyword("tagline")) {
                expect_colon();
                banner.tagline = expect_string();
            } else if (match_keyword("font")) {
                expect_colon();
                banner.font = expect_string();
            } else if (match_keyword("show_on")) {
                expect_colon();
                banner.show_on.clear();
                if (match(TokenType::LBRACKET)) {
                    while (!check(TokenType::RBRACKET) && !check(TokenType::END_OF_FILE)) {
                        banner.show_on.push_back(expect_string());
                        if (check(TokenType::IDENTIFIER) && peek().text == ",") advance();
                    }
                    expect(TokenType::RBRACKET, "Expected ']' after show_on list");
                }
            } else {
                advance();
            }
        }
        expect(TokenType::RBRACE, "Expected '}' after banner block");
    }

    void parse_tokens_block(TokenConfig& tokens) {
        expect(TokenType::LBRACE, "Expected '{' after tokens");
        while (!check(TokenType::RBRACE) && !check(TokenType::END_OF_FILE)) {
            if (match_keyword("keyword")) {
                std::string kw = expect_string();
                expect(TokenType::ARROW, "Expected '=>' after keyword string");
                std::string target = expect(TokenType::IDENTIFIER, "Expected canonical token name").text;
                tokens.keywords[kw] = target;
            } else if (match_keyword("operator")) {
                std::string op = expect_string();
                expect(TokenType::ARROW, "Expected '=>' after operator string");
                std::string target = expect(TokenType::IDENTIFIER, "Expected canonical operator name").text;
                tokens.operators[op] = target;
            } else if (match_keyword("comment")) {
                expect_colon();
                tokens.line_comment = expect_string();
            } else if (match_keyword("whitespace")) {
                expect_colon();
                tokens.whitespace_mode = expect_string();
            } else {
                advance();
            }
        }
        expect(TokenType::RBRACE, "Expected '}' after tokens block");
    }

    void parse_grammar_block(ForgeSpec& spec) {
        expect(TokenType::LBRACE, "Expected '{' after grammar");
        while (!check(TokenType::RBRACE) && !check(TokenType::END_OF_FILE)) {
            if (match_keyword("rule")) {
                std::string rule_name = expect(TokenType::IDENTIFIER, "Expected rule name").text;
                expect(TokenType::EQUALS, "Expected '=' after rule name");
                PegExprPtr expr = parse_choice_expression();
                GrammarRule rule;
                rule.name = rule_name;
                rule.expression = expr;
                spec.rules.push_back(rule);
                spec.rule_map[rule_name] = expr;
            } else {
                advance();
            }
        }
        expect(TokenType::RBRACE, "Expected '}' after grammar block");
    }

    PegExprPtr parse_choice_expression() {
        std::vector<PegExprPtr> choices;
        choices.push_back(parse_sequence_expression());
        while (match(TokenType::PIPE)) {
            choices.push_back(parse_sequence_expression());
        }
        if (choices.size() == 1) return choices[0];
        auto choice_node = std::make_shared<PegExpression>(PegExprKind::CHOICE);
        choice_node->children = std::move(choices);
        return choice_node;
    }

    PegExprPtr parse_sequence_expression() {
        std::vector<PegExprPtr> seq;
        while (!check(TokenType::PIPE) && !check(TokenType::RBRACE) && 
               !check(TokenType::RPAREN) && !check(TokenType::END_OF_FILE) && 
               !(check(TokenType::IDENTIFIER) && peek().text == "rule")) {
            PegExprPtr item = parse_quantified_expression();
            if (!item) break;
            seq.push_back(item);
        }
        if (seq.empty()) return nullptr;
        if (seq.size() == 1) return seq[0];
        auto seq_node = std::make_shared<PegExpression>(PegExprKind::SEQUENCE);
        seq_node->children = std::move(seq);
        return seq_node;
    }

    PegExprPtr parse_quantified_expression() {
        PegExprPtr atom = parse_atom_expression();
        if (!atom) return nullptr;

        if (match(TokenType::STAR)) {
            auto rep = std::make_shared<PegExpression>(PegExprKind::ZERO_OR_MORE);
            rep->children.push_back(atom);
            return rep;
        } else if (match(TokenType::PLUS)) {
            auto rep = std::make_shared<PegExpression>(PegExprKind::ONE_OR_MORE);
            rep->children.push_back(atom);
            return rep;
        } else if (match(TokenType::QUESTION)) {
            auto opt = std::make_shared<PegExpression>(PegExprKind::OPTIONAL);
            opt->children.push_back(atom);
            return opt;
        }
        return atom;
    }

    PegExprPtr parse_atom_expression() {
        if (check(TokenType::STRING)) {
            std::string lit = advance().text;
            return std::make_shared<PegExpression>(PegExprKind::LITERAL, lit);
        }
        if (check(TokenType::IDENTIFIER)) {
            std::string name = advance().text;
            if (name == "Identifier" || name == "Integer" || name == "Float" || 
                name == "String" || name == "Boolean") {
                return std::make_shared<PegExpression>(PegExprKind::BUILTIN_TERM, name);
            }
            return std::make_shared<PegExpression>(PegExprKind::NON_TERMINAL, name);
        }
        if (match(TokenType::LPAREN)) {
            PegExprPtr inner = parse_choice_expression();
            expect(TokenType::RPAREN, "Expected ')' to close grouped expression");
            return inner;
        }
        return nullptr;
    }
};

} // namespace

bool ForgeSpec::parse_string(const std::string& content, ForgeSpec& out_spec, std::string& out_error) {
    ForgeLexer lexer(content);
    std::vector<Token> tokens = lexer.tokenize();
    ForgeParser parser(std::move(tokens));
    return parser.parse(out_spec, out_error);
}

bool ForgeSpec::load_from_file(const std::string& path, ForgeSpec& out_spec, std::string& out_error) {
    std::ifstream file(path);
    if (!file.is_open()) {
        out_error = "Could not open file: " + path;
        return false;
    }
    std::stringstream buffer;
    buffer << file.rdbuf();
    bool ok = parse_string(buffer.str(), out_spec, out_error);
    if (ok) {
        try {
            out_spec.spec_source_path = std::filesystem::absolute(path).string();
        } catch (...) {
            out_spec.spec_source_path = path;
        }
    }
    return ok;
}

} // namespace forge
} // namespace alphabet

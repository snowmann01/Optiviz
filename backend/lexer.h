#pragma once
#include <string>
#include <vector>
#include <stdexcept>

// ── Token Types ───────────────────────────────────────────────────────────────
enum class TokenType {
    // Literals
    NUMBER, IDENTIFIER,

    // Keywords
    KW_INT, KW_RETURN, KW_IF, KW_ELSE, KW_WHILE,

    // Operators
    PLUS, MINUS, STAR, SLASH,
    ASSIGN,           // =
    EQ, NEQ,          // == !=
    LT, GT, LTE, GTE, // < > <= >=

    // Punctuation
    LPAREN, RPAREN,   // ( )
    LBRACE, RBRACE,   // { }
    SEMICOLON,        // ;

    // End
    END_OF_FILE
};

// ── Token ─────────────────────────────────────────────────────────────────────
struct Token {
    TokenType   type;
    std::string value;
    int         line;

    std::string typeName() const {
        switch (type) {
            case TokenType::NUMBER:     return "NUMBER";
            case TokenType::IDENTIFIER: return "IDENTIFIER";
            case TokenType::KW_INT:     return "int";
            case TokenType::KW_RETURN:  return "return";
            case TokenType::KW_IF:      return "if";
            case TokenType::KW_ELSE:    return "else";
            case TokenType::KW_WHILE:   return "while";
            case TokenType::PLUS:       return "+";
            case TokenType::MINUS:      return "-";
            case TokenType::STAR:       return "*";
            case TokenType::SLASH:      return "/";
            case TokenType::ASSIGN:     return "=";
            case TokenType::EQ:         return "==";
            case TokenType::NEQ:        return "!=";
            case TokenType::LT:         return "<";
            case TokenType::GT:         return ">";
            case TokenType::LTE:        return "<=";
            case TokenType::GTE:        return ">=";
            case TokenType::LPAREN:     return "(";
            case TokenType::RPAREN:     return ")";
            case TokenType::LBRACE:     return "{";
            case TokenType::RBRACE:     return "}";
            case TokenType::SEMICOLON:  return ";";
            case TokenType::END_OF_FILE:return "EOF";
        }
        return "?";
    }
};

// ── Lexer ─────────────────────────────────────────────────────────────────────
class Lexer {
public:
    explicit Lexer(const std::string& source)
        : src(source), pos(0), line(1) {}

    // Tokenize the entire source and return all tokens
    std::vector<Token> tokenize() {
        std::vector<Token> tokens;
        while (true) {
            Token tok = nextToken();
            tokens.push_back(tok);
            if (tok.type == TokenType::END_OF_FILE) break;
        }
        return tokens;
    }

private:
    std::string src;
    size_t      pos;
    int         line;

    char peek(int offset = 0) {
        size_t i = pos + offset;
        if (i >= src.size()) return '\0';
        return src[i];
    }

    char advance() {
        char c = src[pos++];
        if (c == '\n') line++;
        return c;
    }

    void skipWhitespaceAndComments() {
        while (pos < src.size()) {
            // whitespace
            if (std::isspace(peek())) { advance(); continue; }

            // single-line comment //
            if (peek() == '/' && peek(1) == '/') {
                while (pos < src.size() && peek() != '\n') advance();
                continue;
            }

            // multi-line comment /* */
            if (peek() == '/' && peek(1) == '*') {
                advance(); advance(); // skip /*
                while (pos < src.size()) {
                    if (peek() == '*' && peek(1) == '/') {
                        advance(); advance(); break;
                    }
                    advance();
                }
                continue;
            }

            break;
        }
    }

    Token makeToken(TokenType type, const std::string& val) {
        return Token{type, val, line};
    }

    Token nextToken() {
        skipWhitespaceAndComments();

        if (pos >= src.size())
            return makeToken(TokenType::END_OF_FILE, "");

        char c = peek();

        // ── Numbers ────────────────────────────────────────────────────────
        if (std::isdigit(c)) {
            std::string num;
            while (pos < src.size() && std::isdigit(peek()))
                num += advance();
            return makeToken(TokenType::NUMBER, num);
        }

        // ── Identifiers & Keywords ─────────────────────────────────────────
        if (std::isalpha(c) || c == '_') {
            std::string word;
            while (pos < src.size() && (std::isalnum(peek()) || peek() == '_'))
                word += advance();

            if (word == "int")    return makeToken(TokenType::KW_INT,    word);
            if (word == "return") return makeToken(TokenType::KW_RETURN, word);
            if (word == "if")     return makeToken(TokenType::KW_IF,     word);
            if (word == "else")   return makeToken(TokenType::KW_ELSE,   word);
            if (word == "while")  return makeToken(TokenType::KW_WHILE,  word);
            return makeToken(TokenType::IDENTIFIER, word);
        }

        // ── Two-character operators ────────────────────────────────────────
        advance(); // consume first char
        if (c == '=' && peek() == '=') { advance(); return makeToken(TokenType::EQ,  "=="); }
        if (c == '!' && peek() == '=') { advance(); return makeToken(TokenType::NEQ, "!="); }
        if (c == '<' && peek() == '=') { advance(); return makeToken(TokenType::LTE, "<="); }
        if (c == '>' && peek() == '=') { advance(); return makeToken(TokenType::GTE, ">="); }

        // ── Single-character tokens ────────────────────────────────────────
        switch (c) {
            case '+': return makeToken(TokenType::PLUS,      "+");
            case '-': return makeToken(TokenType::MINUS,     "-");
            case '*': return makeToken(TokenType::STAR,      "*");
            case '/': return makeToken(TokenType::SLASH,     "/");
            case '=': return makeToken(TokenType::ASSIGN,    "=");
            case '<': return makeToken(TokenType::LT,        "<");
            case '>': return makeToken(TokenType::GT,        ">");
            case '(': return makeToken(TokenType::LPAREN,    "(");
            case ')': return makeToken(TokenType::RPAREN,    ")");
            case '{': return makeToken(TokenType::LBRACE,    "{");
            case '}': return makeToken(TokenType::RBRACE,    "}");
            case ';': return makeToken(TokenType::SEMICOLON, ";");
            default:
                throw std::runtime_error(
                    "Line " + std::to_string(line) +
                    ": Unknown character '" + c + "'");
        }
    }
};
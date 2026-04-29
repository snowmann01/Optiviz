#pragma once
#include "lexer.h"
#include "ast.h"
#include <vector>
#include <stdexcept>
#include <string>

// ── Parser ────────────────────────────────────────────────────────────────────
// Recursive Descent Parser for subset of C.
//
// Grammar supported:
//   program     → statement*
//   statement   → varDecl | assignStmt | returnStmt | ifStmt | whileStmt | block
//   varDecl     → 'int' IDENTIFIER ('=' expr)? ';'
//   assignStmt  → IDENTIFIER '=' expr ';'
//   returnStmt  → 'return' expr ';'
//   ifStmt      → 'if' '(' expr ')' block ('else' block)?
//   whileStmt   → 'while' '(' expr ')' block
//   block       → '{' statement* '}'
//   expr        → comparison
//   comparison  → addition (('==' | '!=' | '<' | '>' | '<=' | '>=') addition)*
//   addition    → multiplication (('+' | '-') multiplication)*
//   multiplication → primary (('*' | '/') primary)*
//   primary     → NUMBER | IDENTIFIER | '(' expr ')'

class Parser {
public:
    explicit Parser(const std::vector<Token>& tokens)
        : tokens(tokens), pos(0) {}

    // Parse the entire token stream and return the root AST node
    ASTNodePtr parse() {
        auto program = ASTNode::make(NodeType::PROGRAM);
        while (!isAtEnd()) {
            program->addChild(parseStatement());
        }
        return program;
    }

private:
    std::vector<Token> tokens;
    size_t             pos;

    // ── Token helpers ────────────────────────────────────────────────────────

    Token& current() { return tokens[pos]; }

    Token& peek(int offset = 0) {
        size_t i = pos + offset;
        if (i >= tokens.size()) return tokens.back(); // EOF
        return tokens[i];
    }

    bool isAtEnd() {
        return current().type == TokenType::END_OF_FILE;
    }

    bool check(TokenType t) {
        return current().type == t;
    }

    Token consume() {
        return tokens[pos++];
    }

    Token expect(TokenType t) {
        if (!check(t)) {
            throw std::runtime_error(
                "Line " + std::to_string(current().line) +
                ": Expected '" + Token{t, "", 0}.typeName() +
                "' but got '" + current().value + "'");
        }
        return consume();
    }

    bool match(TokenType t) {
        if (check(t)) { consume(); return true; }
        return false;
    }

    // ── Statement Parsers ────────────────────────────────────────────────────

    ASTNodePtr parseStatement() {
        // int x = expr;
        if (check(TokenType::KW_INT))
            return parseVarDecl();

        // return expr;
        if (check(TokenType::KW_RETURN))
            return parseReturnStmt();

        // if (cond) { }
        if (check(TokenType::KW_IF))
            return parseIfStmt();

        // while (cond) { }
        if (check(TokenType::KW_WHILE))
            return parseWhileStmt();

        // { ... }
        if (check(TokenType::LBRACE))
            return parseBlock();

        // x = expr;  — must be IDENTIFIER followed by '='
        if (check(TokenType::IDENTIFIER) &&
            peek(1).type == TokenType::ASSIGN)
            return parseAssignStmt();

        throw std::runtime_error(
            "Line " + std::to_string(current().line) +
            ": Unexpected token '" + current().value + "'");
    }

    // int x;  or  int x = expr;
    ASTNodePtr parseVarDecl() {
        expect(TokenType::KW_INT);
        Token name = expect(TokenType::IDENTIFIER);

        auto node = ASTNode::make(NodeType::VAR_DECL, name.value);

        if (match(TokenType::ASSIGN)) {
            node->addChild(parseExpr());
        }

        expect(TokenType::SEMICOLON);
        return node;
    }

    // x = expr;
    ASTNodePtr parseAssignStmt() {
        Token name = expect(TokenType::IDENTIFIER);
        expect(TokenType::ASSIGN);

        auto node = ASTNode::make(NodeType::ASSIGN_STMT, name.value);
        node->addChild(parseExpr());

        expect(TokenType::SEMICOLON);
        return node;
    }

    // return expr;
    ASTNodePtr parseReturnStmt() {
        expect(TokenType::KW_RETURN);
        auto node = ASTNode::make(NodeType::RETURN_STMT);
        node->addChild(parseExpr());
        expect(TokenType::SEMICOLON);
        return node;
    }

    // if (cond) block [else block]
    ASTNodePtr parseIfStmt() {
        expect(TokenType::KW_IF);
        expect(TokenType::LPAREN);
        auto cond = parseExpr();
        expect(TokenType::RPAREN);
        auto thenBlock = parseBlock();

        auto node = ASTNode::make(NodeType::IF_STMT);
        node->addChild(std::move(cond));
        node->addChild(std::move(thenBlock));

        if (match(TokenType::KW_ELSE)) {
            node->addChild(parseBlock());
        }

        return node;
    }

    // while (cond) block
    ASTNodePtr parseWhileStmt() {
        expect(TokenType::KW_WHILE);
        expect(TokenType::LPAREN);
        auto cond = parseExpr();
        expect(TokenType::RPAREN);
        auto body = parseBlock();

        auto node = ASTNode::make(NodeType::WHILE_STMT);
        node->addChild(std::move(cond));
        node->addChild(std::move(body));
        return node;
    }

    // { stmt* }
    ASTNodePtr parseBlock() {
        expect(TokenType::LBRACE);
        auto block = ASTNode::make(NodeType::BLOCK);
        while (!check(TokenType::RBRACE) && !isAtEnd()) {
            block->addChild(parseStatement());
        }
        expect(TokenType::RBRACE);
        return block;
    }

    // ── Expression Parsers (lowest → highest precedence) ─────────────────────

    // expr → comparison
    ASTNodePtr parseExpr() {
        return parseComparison();
    }

    // comparison → addition (op addition)*
    ASTNodePtr parseComparison() {
        auto left = parseAddition();

        while (check(TokenType::EQ)  || check(TokenType::NEQ) ||
               check(TokenType::LT)  || check(TokenType::GT)  ||
               check(TokenType::LTE) || check(TokenType::GTE)) {
            Token op = consume();
            auto right = parseAddition();
            auto node = ASTNode::make(NodeType::BINARY_OP, op.value);
            node->addChild(std::move(left));
            node->addChild(std::move(right));
            left = std::move(node);
        }

        return left;
    }

    // addition → multiplication (('+' | '-') multiplication)*
    ASTNodePtr parseAddition() {
        auto left = parseMultiplication();

        while (check(TokenType::PLUS) || check(TokenType::MINUS)) {
            Token op = consume();
            auto right = parseMultiplication();
            auto node = ASTNode::make(NodeType::BINARY_OP, op.value);
            node->addChild(std::move(left));
            node->addChild(std::move(right));
            left = std::move(node);
        }

        return left;
    }

    // multiplication → primary (('*' | '/') primary)*
    ASTNodePtr parseMultiplication() {
        auto left = parsePrimary();

        while (check(TokenType::STAR) || check(TokenType::SLASH)) {
            Token op = consume();
            auto right = parsePrimary();
            auto node = ASTNode::make(NodeType::BINARY_OP, op.value);
            node->addChild(std::move(left));
            node->addChild(std::move(right));
            left = std::move(node);
        }

        return left;
    }

    // primary → NUMBER | IDENTIFIER | '(' expr ')'
    ASTNodePtr parsePrimary() {
        if (check(TokenType::NUMBER)) {
            Token t = consume();
            return ASTNode::make(NodeType::NUMBER_LITERAL, t.value);
        }

        if (check(TokenType::IDENTIFIER)) {
            Token t = consume();
            return ASTNode::make(NodeType::IDENTIFIER, t.value);
        }

        if (check(TokenType::LPAREN)) {
            consume(); // skip '('
            auto expr = parseExpr();
            expect(TokenType::RPAREN);
            return expr;
        }

        throw std::runtime_error(
            "Line " + std::to_string(current().line) +
            ": Expected expression but got '" + current().value + "'");
    }
};
#pragma once
#include <string>
#include <vector>
#include <memory>

// ── AST Node Types ────────────────────────────────────────────────────────────
// Every node in the tree is one of these types.
// The tree represents the structure of the C program.

// Forward declarations
struct ASTNode;
using ASTNodePtr = std::unique_ptr<ASTNode>;

enum class NodeType {
    // Statements
    PROGRAM,        // root: list of statements
    VAR_DECL,       // int x = expr;
    ASSIGN_STMT,    // x = expr;
    RETURN_STMT,    // return expr;
    IF_STMT,        // if (cond) { } else { }
    WHILE_STMT,     // while (cond) { }
    BLOCK,          // { stmt; stmt; }

    // Expressions
    NUMBER_LITERAL, // 42
    IDENTIFIER,     // x
    BINARY_OP,      // expr + expr
};

// ── AST Node ──────────────────────────────────────────────────────────────────
struct ASTNode {
    NodeType                    type;
    std::string                 value;      // for NUMBER_LITERAL, IDENTIFIER, BINARY_OP operator
    std::vector<ASTNodePtr>     children;   // child nodes

    // Factory helpers
    static ASTNodePtr make(NodeType t, const std::string& val = "") {
        auto node = std::make_unique<ASTNode>();
        node->type  = t;
        node->value = val;
        return node;
    }

    void addChild(ASTNodePtr child) {
        children.push_back(std::move(child));
    }

    // Pretty-print the tree for debugging
    void print(int indent = 0) const {
        std::string pad(indent * 2, ' ');
        std::string name;
        switch (type) {
            case NodeType::PROGRAM:        name = "PROGRAM";        break;
            case NodeType::VAR_DECL:       name = "VAR_DECL";       break;
            case NodeType::ASSIGN_STMT:    name = "ASSIGN_STMT";    break;
            case NodeType::RETURN_STMT:    name = "RETURN_STMT";    break;
            case NodeType::IF_STMT:        name = "IF_STMT";        break;
            case NodeType::WHILE_STMT:     name = "WHILE_STMT";     break;
            case NodeType::BLOCK:          name = "BLOCK";          break;
            case NodeType::NUMBER_LITERAL: name = "NUMBER";         break;
            case NodeType::IDENTIFIER:     name = "IDENTIFIER";     break;
            case NodeType::BINARY_OP:      name = "BINARY_OP";      break;
        }
        printf("%s[%s]%s\n",
            pad.c_str(), name.c_str(),
            value.empty() ? "" : (" (" + value + ")").c_str());
        for (auto& child : children)
            child->print(indent + 1);
    }
};
#pragma once
#include "ast.h"
#include "tac.h"
#include <string>
#include <vector>
#include <unordered_map>

// ── TAC Generator ─────────────────────────────────────────────────────────────
// Walks the AST and emits TAC instructions.
// This is the bridge between the parser and the optimizer.
//
// Temporaries:  t0, t1, t2, ...
// Labels:       L0, L1, L2, ...

class TACGenerator {
public:
    TACProgram generate(const ASTNode* root) {
        program.clear();
        tempCount = 0;
        labelCount = 0;
        genNode(root);
        return program;
    }

private:
    TACProgram program;
    int tempCount  = 0;
    int labelCount = 0;

    // Create a new unique temporary variable: t0, t1, t2, ...
    std::string newTemp() {
        return "t" + std::to_string(tempCount++);
    }

    // Create a new unique label: L0, L1, L2, ...
    std::string newLabel() {
        return "L" + std::to_string(labelCount++);
    }

    void emit(TACInstruction instr) {
        program.push_back(std::move(instr));
    }

    // ── Node dispatcher ──────────────────────────────────────────────────────

    // Returns the variable/temp that holds the result of this node
    std::string genNode(const ASTNode* node) {
        switch (node->type) {

            case NodeType::PROGRAM:
                for (auto& child : node->children)
                    genNode(child.get());
                return "";

            case NodeType::BLOCK:
                for (auto& child : node->children)
                    genNode(child.get());
                return "";

            case NodeType::VAR_DECL:
                return genVarDecl(node);

            case NodeType::ASSIGN_STMT:
                return genAssignStmt(node);

            case NodeType::RETURN_STMT:
                return genReturnStmt(node);

            case NodeType::IF_STMT:
                return genIfStmt(node);

            case NodeType::WHILE_STMT:
                return genWhileStmt(node);

            case NodeType::NUMBER_LITERAL:
                return node->value; // just return the number as a string

            case NodeType::IDENTIFIER:
                return node->value; // just return the variable name

            case NodeType::BINARY_OP:
                return genBinaryOp(node);

            default:
                return "";
        }
    }

    // ── Statement Generators ─────────────────────────────────────────────────

    // int x = expr;   →   x = <expr result>
    std::string genVarDecl(const ASTNode* node) {
        std::string varName = node->value;
        if (!node->children.empty()) {
            std::string rhs = genNode(node->children[0].get());
            emit(TACInstruction::makeAssign(varName, rhs));
        }
        return varName;
    }

    // x = expr;   →   x = <expr result>
    std::string genAssignStmt(const ASTNode* node) {
        std::string varName = node->value;
        std::string rhs = genNode(node->children[0].get());
        emit(TACInstruction::makeAssign(varName, rhs));
        return varName;
    }

    // return expr;   →   return <expr result>
    std::string genReturnStmt(const ASTNode* node) {
        std::string val = genNode(node->children[0].get());
        emit(TACInstruction::makeReturn(val));
        return "";
    }

    // if (cond) { thenBlock } [else { elseBlock }]
    //
    // Emits:
    //   if cond goto L_then
    //   goto L_else
    //   L_then:
    //     <then block>
    //   goto L_end
    //   L_else:
    //     <else block>   (if exists)
    //   L_end:
    std::string genIfStmt(const ASTNode* node) {
        // node->children[0] = condition
        // node->children[1] = then block
        // node->children[2] = else block (optional)

        std::string lThen = newLabel();
        std::string lElse = newLabel();
        std::string lEnd  = newLabel();

        // Evaluate condition
        std::string cond = genCondition(node->children[0].get(), lThen, lElse);

        // Then block
        emit(TACInstruction::makeLabel(lThen));
        genNode(node->children[1].get());
        emit(TACInstruction::makeGoto(lEnd));

        // Else block
        emit(TACInstruction::makeLabel(lElse));
        if (node->children.size() >= 3) {
            genNode(node->children[2].get());
        }

        emit(TACInstruction::makeLabel(lEnd));
        return "";
    }

    // while (cond) { body }
    //
    // Emits:
    //   L_start:
    //   if cond goto L_body
    //   goto L_end
    //   L_body:
    //     <body>
    //   goto L_start
    //   L_end:
    std::string genWhileStmt(const ASTNode* node) {
        std::string lStart = newLabel();
        std::string lBody  = newLabel();
        std::string lEnd   = newLabel();

        emit(TACInstruction::makeLabel(lStart));
        genCondition(node->children[0].get(), lBody, lEnd);

        emit(TACInstruction::makeLabel(lBody));
        genNode(node->children[1].get());
        emit(TACInstruction::makeGoto(lStart));

        emit(TACInstruction::makeLabel(lEnd));
        return "";
    }

    // ── Expression Generator ─────────────────────────────────────────────────

    // Handles +, -, *, /, ==, !=, <, >, <=, >=
    std::string genBinaryOp(const ASTNode* node) {
        std::string left  = genNode(node->children[0].get());
        std::string right = genNode(node->children[1].get());
        std::string temp  = newTemp();
        std::string op    = node->value;

        TACOp tacOp;
        if      (op == "+") tacOp = TACOp::ADD;
        else if (op == "-") tacOp = TACOp::SUB;
        else if (op == "*") tacOp = TACOp::MUL;
        else if (op == "/") tacOp = TACOp::DIV;
        else {
            // Comparison operators — store result as a temp
            // We emit: temp = left op right  (as an ASSIGN with a special encoding)
            // For use in if/while conditions, genCondition handles them directly.
            // Here we just store it as a plain assign for expression contexts.
            emit(TACInstruction::makeAssign(temp, left + " " + op + " " + right));
            return temp;
        }

        emit(TACInstruction::makeBinOp(tacOp, temp, left, right));
        return temp;
    }

    // Generates condition check for if/while:
    // Emits:  if left op right goto lTrue
    //         goto lFalse
    std::string genCondition(const ASTNode* cond,
                             const std::string& lTrue,
                             const std::string& lFalse) {
        if (cond->type == NodeType::BINARY_OP) {
            std::string op = cond->value;
            bool isCmp = (op == "==" || op == "!=" ||
                          op == "<"  || op == ">"  ||
                          op == "<=" || op == ">=");
            if (isCmp) {
                std::string left  = genNode(cond->children[0].get());
                std::string right = genNode(cond->children[1].get());
                emit(TACInstruction::makeIfGoto(left, op, right, lTrue));
                emit(TACInstruction::makeGoto(lFalse));
                return "";
            }
        }

        // Non-comparison condition: evaluate to a temp, check != 0
        std::string val = genNode(cond);
        emit(TACInstruction::makeIfGoto(val, "!=", "0", lTrue));
        emit(TACInstruction::makeGoto(lFalse));
        return "";
    }
};
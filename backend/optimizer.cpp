#include "optimizer.h"
#include <unordered_map>
#include <unordered_set>
#include <cctype>
#include <stdexcept>


bool Optimizer::isConstant(const std::string& s) {
    if (s.empty()) return false;
    size_t i = (s[0] == '-') ? 1 : 0;
    for (; i < s.size(); i++)
        if (!std::isdigit(s[i])) return false;
    return true;
}

int Optimizer::evalOp(int a, char op, int b) {
    switch (op) {
        case '+': return a + b;
        case '-': return a - b;
        case '*': return a * b;
        case '/': if (b == 0) throw std::runtime_error("Division by zero");
                  return a / b;
    }
    throw std::runtime_error("Unknown op");
}

void Optimizer::logChange(const std::string& pass, int idx,
                          const std::string& before, const std::string& after) {
    logs.push_back({pass, idx, before, after});
}

//Pass 1: Constant Folding
// x = 2 + 3  →  x = 5

TACProgram Optimizer::constantFolding(const TACProgram& input) {
    TACProgram output = input;
    for (int i = 0; i < (int)output.size(); i++) {
        auto& instr = output[i];
        bool isBinOp = (instr.op == TACOp::ADD || instr.op == TACOp::SUB ||
                        instr.op == TACOp::MUL || instr.op == TACOp::DIV);
        if (!isBinOp) continue;
        if (!isConstant(instr.arg1) || !isConstant(instr.arg2)) continue;

        int a = std::stoi(instr.arg1);
        int b = std::stoi(instr.arg2);
        char op = (instr.op == TACOp::ADD) ? '+' :
                  (instr.op == TACOp::SUB) ? '-' :
                  (instr.op == TACOp::MUL) ? '*' : '/';

        try {
            int result = evalOp(a, op, b);
            std::string before = instr.toString();
            instr = TACInstruction::makeAssign(instr.result, std::to_string(result));
            logChange("Constant Folding", i, before, instr.toString());
        } catch (...) {}
    }
    return output;
}

// Pass 2: Constant Propagation
// x = 5; y = x + 1  →  y = 5 + 1

TACProgram Optimizer::constantPropagation(const TACProgram& input) {
    TACProgram output = input;
    std::unordered_map<std::string, std::string> constMap;

    for (int i = 0; i < (int)output.size(); i++) {
        auto& instr = output[i];

        // Replace known constants in args
        auto replace = [&](std::string& arg) {
            if (!arg.empty() && constMap.count(arg)) {
                arg = constMap[arg];
            }
        };

        std::string before = instr.toString();
        bool changed = false;

        if (instr.op == TACOp::ASSIGN) {
            replace(instr.arg1);
            changed = (instr.toString() != before);
            // Track this assignment if RHS is now a constant
            if (isConstant(instr.arg1))
                constMap[instr.result] = instr.arg1;
            else
                constMap.erase(instr.result); // no longer constant
        } else if (instr.op == TACOp::ADD || instr.op == TACOp::SUB ||
                   instr.op == TACOp::MUL || instr.op == TACOp::DIV) {
            replace(instr.arg1);
            replace(instr.arg2);
            changed = (instr.toString() != before);
            constMap.erase(instr.result); // result is not necessarily constant yet
        } else if (instr.op == TACOp::IF_GOTO) {
            replace(instr.arg1);
            replace(instr.arg2);
            changed = (instr.toString() != before);
        } else if (instr.op == TACOp::RETURN) {
            replace(instr.arg1);
            changed = (instr.toString() != before);
        } else if (instr.op == TACOp::LABEL || instr.op == TACOp::GOTO) {
            // Labels and gotos reset our knowledge (conservative)
            constMap.clear();
        }

        if (changed)
            logChange("Constant Propagation", i, before, instr.toString());
    }
    return output;
}

// Pass 3: Dead Code Elimination 
// Removes assignments whose result is never used

TACProgram Optimizer::deadCodeElimination(const TACProgram& input) {
    // Count uses of each variable
    std::unordered_map<std::string, int> useCount;

    auto countUse = [&](const std::string& s) {
        if (!s.empty() && !isConstant(s)) useCount[s]++;
    };

    for (auto& instr : input) {
        countUse(instr.arg1);
        countUse(instr.arg2);
        if (instr.op == TACOp::RETURN)  countUse(instr.arg1);
        if (instr.op == TACOp::IF_GOTO) { countUse(instr.arg1); countUse(instr.arg2); }
    }

    TACProgram output;
    for (int i = 0; i < (int)input.size(); i++) {
        const auto& instr = input[i];
        bool isAssignment = (instr.op == TACOp::ASSIGN ||
                             instr.op == TACOp::ADD    ||
                             instr.op == TACOp::SUB    ||
                             instr.op == TACOp::MUL    ||
                             instr.op == TACOp::DIV);

        if (isAssignment && !instr.result.empty() && useCount[instr.result] == 0) {
            logChange("Dead Code Elimination", i, instr.toString(), "REMOVED");
            output.push_back(TACInstruction::makeNOP()); // mark as NOP instead of deleting
        } else {
            output.push_back(instr);
        }
    }
    return output;
}

// Pass 4: Algebraic Simplification 
// x * 1 → x,  x + 0 → x,  x * 0 → 0,  x - 0 → x

TACProgram Optimizer::algebraicSimplification(const TACProgram& input) {
    TACProgram output = input;
    for (int i = 0; i < (int)output.size(); i++) {
        auto& instr = output[i];
        std::string before = instr.toString();
        std::string newVal = "";

        if (instr.op == TACOp::MUL) {
            if (instr.arg1 == "1")       newVal = instr.arg2;  // 1 * x = x
            else if (instr.arg2 == "1")  newVal = instr.arg1;  // x * 1 = x
            else if (instr.arg1 == "0" || instr.arg2 == "0") newVal = "0"; // x * 0 = 0
        } else if (instr.op == TACOp::ADD) {
            if (instr.arg1 == "0")       newVal = instr.arg2;  // 0 + x = x
            else if (instr.arg2 == "0")  newVal = instr.arg1;  // x + 0 = x
        } else if (instr.op == TACOp::SUB) {
            if (instr.arg2 == "0")       newVal = instr.arg1;  // x - 0 = x
        } else if (instr.op == TACOp::DIV) {
            if (instr.arg2 == "1")       newVal = instr.arg1;  // x / 1 = x
        }

        if (!newVal.empty()) {
            instr = TACInstruction::makeAssign(instr.result, newVal);
            logChange("Algebraic Simplification", i, before, instr.toString());
        }
    }
    return output;
}



TACProgram Optimizer::runAll(const TACProgram& input) {
    logs.clear();
    TACProgram prog = input;
    prog = constantFolding(prog);
    prog = constantPropagation(prog);
    prog = algebraicSimplification(prog);
    prog = deadCodeElimination(prog);
    return prog;
}
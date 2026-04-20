#pragma once
#include <string>
#include <vector>

// Types of TAC instructions
enum class TACOp {
    ASSIGN,      // x = y
    ADD,         // x = y + z
    SUB,         // x = y - z
    MUL,         // x = y * z
    DIV,         // x = y / z
    GOTO,        // goto L
    IF_GOTO,     // if x > y goto L
    LABEL,       // L:
    RETURN,      // return x
    NOP          // no operation (used after dead code elimination)
};

struct TACInstruction {
    TACOp op;
    std::string result;   // destination
    std::string arg1;     // first operand
    std::string arg2;     // second operand (optional)
    std::string label;    // for GOTO / IF_GOTO / LABEL
    std::string cmp_op;   // for IF_GOTO: >, <, ==, !=, >=, <=

    // Constructors for convenience
    static TACInstruction makeAssign(const std::string& res, const std::string& val);
    static TACInstruction makeBinOp(TACOp op, const std::string& res, const std::string& a, const std::string& b);
    static TACInstruction makeGoto(const std::string& lbl);
    static TACInstruction makeIfGoto(const std::string& a, const std::string& cmpOp, const std::string& b, const std::string& lbl);
    static TACInstruction makeLabel(const std::string& lbl);
    static TACInstruction makeReturn(const std::string& val);
    static TACInstruction makeNOP();

    std::string toString() const;
};

using TACProgram = std::vector<TACInstruction>;
#include "tac.h"
#include <sstream>

TACInstruction TACInstruction::makeAssign(const std::string& res, const std::string& val) {
    TACInstruction i;
    i.op = TACOp::ASSIGN;
    i.result = res;
    i.arg1 = val;
    return i;
}

TACInstruction TACInstruction::makeBinOp(TACOp op, const std::string& res, const std::string& a, const std::string& b) {
    TACInstruction i;
    i.op = op;
    i.result = res;
    i.arg1 = a;
    i.arg2 = b;
    return i;
}

TACInstruction TACInstruction::makeGoto(const std::string& lbl) {
    TACInstruction i;
    i.op = TACOp::GOTO;
    i.label = lbl;
    return i;
}

TACInstruction TACInstruction::makeIfGoto(const std::string& a, const std::string& cmpOp, const std::string& b, const std::string& lbl) {
    TACInstruction i;
    i.op = TACOp::IF_GOTO;
    i.arg1 = a;
    i.cmp_op = cmpOp;
    i.arg2 = b;
    i.label = lbl;
    return i;
}

TACInstruction TACInstruction::makeLabel(const std::string& lbl) {
    TACInstruction i;
    i.op = TACOp::LABEL;
    i.label = lbl;
    return i;
}

TACInstruction TACInstruction::makeReturn(const std::string& val) {
    TACInstruction i;
    i.op = TACOp::RETURN;
    i.arg1 = val;
    return i;
}

TACInstruction TACInstruction::makeNOP() {
    TACInstruction i;
    i.op = TACOp::NOP;
    return i;
}

std::string TACInstruction::toString() const {
    std::ostringstream ss;
    switch (op) {
        case TACOp::ASSIGN:
            ss << result << " = " << arg1;
            break;
        case TACOp::ADD:
            ss << result << " = " << arg1 << " + " << arg2;
            break;
        case TACOp::SUB:
            ss << result << " = " << arg1 << " - " << arg2;
            break;
        case TACOp::MUL:
            ss << result << " = " << arg1 << " * " << arg2;
            break;
        case TACOp::DIV:
            ss << result << " = " << arg1 << " / " << arg2;
            break;
        case TACOp::GOTO:
            ss << "goto " << label;
            break;
        case TACOp::IF_GOTO:
            ss << "if " << arg1 << " " << cmp_op << " " << arg2 << " goto " << label;
            break;
        case TACOp::LABEL:
            ss << label << ":";
            break;
        case TACOp::RETURN:
            ss << "return " << arg1;
            break;
        case TACOp::NOP:
            ss << "NOP";
            break;
    }
    return ss.str();
}
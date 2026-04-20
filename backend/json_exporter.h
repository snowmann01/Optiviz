#pragma once
#include "tac.h"
#include "optimizer.h"
#include <string>
#include <fstream>
#include <sstream>

// Escape special characters for JSON strings
static std::string jsonEscape(const std::string& s) {
    std::string out;
    for (char c : s) {
        if (c == '"')  out += "\\\"";
        else if (c == '\\') out += "\\\\";
        else out += c;
    }
    return out;
}

static std::string opToString(TACOp op) {
    switch(op) {
        case TACOp::ASSIGN:   return "ASSIGN";
        case TACOp::ADD:      return "ADD";
        case TACOp::SUB:      return "SUB";
        case TACOp::MUL:      return "MUL";
        case TACOp::DIV:      return "DIV";
        case TACOp::GOTO:     return "GOTO";
        case TACOp::IF_GOTO:  return "IF_GOTO";
        case TACOp::LABEL:    return "LABEL";
        case TACOp::RETURN:   return "RETURN";
        case TACOp::NOP:      return "NOP";
    }
    return "UNKNOWN";
}

static std::string instrToJSON(const TACInstruction& instr, int index) {
    std::ostringstream ss;
    ss << "    {\n";
    ss << "      \"index\": " << index << ",\n";
    ss << "      \"op\": \"" << opToString(instr.op) << "\",\n";
    ss << "      \"result\": \"" << jsonEscape(instr.result) << "\",\n";
    ss << "      \"arg1\": \"" << jsonEscape(instr.arg1) << "\",\n";
    ss << "      \"arg2\": \"" << jsonEscape(instr.arg2) << "\",\n";
    ss << "      \"label\": \"" << jsonEscape(instr.label) << "\",\n";
    ss << "      \"cmp_op\": \"" << jsonEscape(instr.cmp_op) << "\",\n";
    ss << "      \"text\": \"" << jsonEscape(instr.toString()) << "\"\n";
    ss << "    }";
    return ss.str();
}

static std::string logToJSON(const OptimizationLog& log) {
    std::ostringstream ss;
    ss << "    {\n";
    ss << "      \"pass\": \"" << jsonEscape(log.pass) << "\",\n";
    ss << "      \"lineIndex\": " << log.lineIndex << ",\n";
    ss << "      \"before\": \"" << jsonEscape(log.before) << "\",\n";
    ss << "      \"after\": \"" << jsonEscape(log.after) << "\"\n";
    ss << "    }";
    return ss.str();
}

static void exportToJSON(
    const TACProgram& original,
    const TACProgram& optimized,
    const std::vector<OptimizationLog>& logs,
    const std::string& outputFile)
{
    std::ofstream f(outputFile);
    if (!f.is_open())
        throw std::runtime_error("Cannot open output file: " + outputFile);

    f << "{\n";

    // Original IR
    f << "  \"original\": [\n";
    for (int i = 0; i < (int)original.size(); i++) {
        f << instrToJSON(original[i], i);
        if (i < (int)original.size() - 1) f << ",";
        f << "\n";
    }
    f << "  ],\n";

    // Optimized IR
    f << "  \"optimized\": [\n";
    for (int i = 0; i < (int)optimized.size(); i++) {
        f << instrToJSON(optimized[i], i);
        if (i < (int)optimized.size() - 1) f << ",";
        f << "\n";
    }
    f << "  ],\n";

    // Optimization logs
    f << "  \"logs\": [\n";
    for (int i = 0; i < (int)logs.size(); i++) {
        f << logToJSON(logs[i]);
        if (i < (int)logs.size() - 1) f << ",";
        f << "\n";
    }
    f << "  ]\n";

    f << "}\n";
    f.close();
}
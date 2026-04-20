#pragma once
#include "tac.h"
#include <string>
#include <fstream>
#include <sstream>
#include <stdexcept>
#include <algorithm>
#include <cctype>

// Trims whitespace from both ends
static std::string trim(const std::string& s) {
    size_t start = s.find_first_not_of(" \t\r\n");
    if (start == std::string::npos) return "";
    size_t end = s.find_last_not_of(" \t\r\n");
    return s.substr(start, end - start + 1);
}

// Checks if a string is a number
static bool isNumber(const std::string& s) {
    if (s.empty()) return false;
    size_t i = (s[0] == '-') ? 1 : 0;
    for (; i < s.size(); i++)
        if (!std::isdigit(s[i])) return false;
    return true;
}

// Parses a single line into a TACInstruction
// Supported syntax:
//   label:
//   goto LABEL
//   if A op B goto LABEL
//   return X
//   X = A
//   X = A + B  (or -, *, /)
static TACInstruction parseLine(const std::string& rawLine) {
    std::string line = trim(rawLine);

    // Skip empty lines and comments
    if (line.empty() || line[0] == '#') {
        return TACInstruction::makeNOP();
    }

    // Label: ends with ':'
    if (line.back() == ':') {
        std::string lbl = trim(line.substr(0, line.size() - 1));
        return TACInstruction::makeLabel(lbl);
    }

    // goto LABEL
    if (line.substr(0, 4) == "goto") {
        std::string lbl = trim(line.substr(4));
        return TACInstruction::makeGoto(lbl);
    }

    // return X
    if (line.substr(0, 6) == "return") {
        std::string val = trim(line.substr(6));
        return TACInstruction::makeReturn(val);
    }

    // if A op B goto LABEL
    if (line.substr(0, 2) == "if") {
        std::istringstream ss(line.substr(2));
        std::string a, cmpOp, b, gotoKw, lbl;
        ss >> a >> cmpOp >> b >> gotoKw >> lbl;
        if (gotoKw != "goto")
            throw std::runtime_error("Expected 'goto' in if statement: " + line);
        return TACInstruction::makeIfGoto(a, cmpOp, b, lbl);
    }

    // X = ...
    size_t eqPos = line.find('=');
    if (eqPos == std::string::npos)
        throw std::runtime_error("Unrecognized instruction: " + line);

    std::string result = trim(line.substr(0, eqPos));
    std::string rhs    = trim(line.substr(eqPos + 1));

    // Try to split RHS by operator
    // Scan for +, -, *, / (skip leading sign in numbers)
    int opIdx = -1;
    char foundOp = 0;
    for (int i = 1; i < (int)rhs.size(); i++) {
        char c = rhs[i];
        if (c == '+' || c == '-' || c == '*' || c == '/') {
            // Make sure it's not a negative number sign
            if (c == '-' && i > 0 && rhs[i-1] == ' ') {
                opIdx = i;
                foundOp = c;
                break;
            } else if (c != '-') {
                opIdx = i;
                foundOp = c;
                break;
            }
        }
    }

    if (opIdx == -1) {
        // Simple assign: X = Y
        return TACInstruction::makeAssign(result, rhs);
    }

    std::string a = trim(rhs.substr(0, opIdx));
    std::string b = trim(rhs.substr(opIdx + 1));

    TACOp op;
    switch (foundOp) {
        case '+': op = TACOp::ADD; break;
        case '-': op = TACOp::SUB; break;
        case '*': op = TACOp::MUL; break;
        case '/': op = TACOp::DIV; break;
        default:  throw std::runtime_error("Unknown operator");
    }

    return TACInstruction::makeBinOp(op, result, a, b);
}

// Parses an entire .tac file
static TACProgram parseTACFile(const std::string& filename) {
    std::ifstream file(filename);
    if (!file.is_open())
        throw std::runtime_error("Cannot open file: " + filename);

    TACProgram program;
    std::string line;
    int lineNum = 0;
    while (std::getline(file, line)) {
        lineNum++;
        try {
            TACInstruction instr = parseLine(line);
            program.push_back(instr);
        } catch (const std::exception& e) {
            throw std::runtime_error("Line " + std::to_string(lineNum) + ": " + e.what());
        }
    }
    return program;
}
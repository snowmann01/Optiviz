#pragma once
#include "tac.h"
#include <string>
#include <vector>

struct OptimizationLog {
    std::string pass;       // name of the pass
    int         lineIndex;  // which instruction was changed
    std::string before;     // instruction before
    std::string after;      // instruction after (or "REMOVED")
};

class Optimizer {
public:
    std::vector<OptimizationLog> logs;

    // Run all passes in sequence
    TACProgram runAll(const TACProgram& input);

    // Individual passes
    TACProgram constantFolding(const TACProgram& input);
    TACProgram constantPropagation(const TACProgram& input);
    TACProgram deadCodeElimination(const TACProgram& input);
    TACProgram algebraicSimplification(const TACProgram& input);

private:
    bool isConstant(const std::string& s);
    int  evalOp(int a, char op, int b);
    void logChange(const std::string& pass, int idx,
                   const std::string& before, const std::string& after);
};
#include <iostream>
#include <stdexcept>
#include "tac.h"
#include "tac_parser.h"
#include "optimizer.h"
#include "json_exporter.h"

int main(int argc, char* argv[]) {
    if (argc < 3) {
        std::cerr << "Usage: compiler <input.tac> <output.json>\n";
        return 1;
    }

    std::string inputFile  = argv[1];
    std::string outputFile = argv[2];

    try {
        std::cout << "[1/3] Parsing TAC file: " << inputFile << "\n";
        TACProgram original = parseTACFile(inputFile);

        std::cout << "      Found " << original.size() << " instructions.\n";
        for (auto& instr : original)
            std::cout << "      " << instr.toString() << "\n";

        std::cout << "\n[2/3] Running optimization passes...\n";
        Optimizer opt;
        TACProgram optimized = opt.runAll(original);

        std::cout << "      Applied " << opt.logs.size() << " optimizations:\n";
        for (auto& log : opt.logs)
            std::cout << "      [" << log.pass << "] Line " << log.lineIndex
                      << ": " << log.before << "  -->  " << log.after << "\n";

        std::cout << "\n[3/3] Exporting to JSON: " << outputFile << "\n";
        exportToJSON(original, optimized, opt.logs, outputFile);

        std::cout << "\nDone! Open frontend/index.html in your browser.\n";
        return 0;

    } catch (const std::exception& e) {
        std::cerr << "\n[ERROR] " << e.what() << "\n";
        return 1;
    }
}
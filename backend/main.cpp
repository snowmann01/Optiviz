#include <iostream>
#include <fstream>
#include <sstream>
#include <stdexcept>
#include <string>

#include "tac.h"
#include "tac_parser.h"
#include "lexer.h"
#include "ast.h"
#include "parser.h"
#include "tac_gen.h"
#include "optimizer.h"
#include "json_exporter.h"

static std::string readFile(const std::string& path) {
    std::ifstream f(path);
    if (!f.is_open())
        throw std::runtime_error("Cannot open file: " + path);
    std::ostringstream ss;
    ss << f.rdbuf();
    return ss.str();
}

static std::string getExtension(const std::string& path) {
    size_t dot = path.rfind('.');
    if (dot == std::string::npos) return "";
    return path.substr(dot);
}

int main(int argc, char* argv[]) {
    if (argc < 3) {
        std::cerr << "Usage: compiler <input.c or input.tac> <output.json>\n";
        std::cerr << "  C source:  compiler program.c   output.json\n";
        std::cerr << "  TAC file:  compiler program.tac output.json\n";
        return 1;
    }

    std::string inputFile  = argv[1];
    std::string outputFile = argv[2];
    std::string ext        = getExtension(inputFile);

    try {
        TACProgram original;

        if (ext == ".c") {
            std::cout << "[1/4] Reading C source: " << inputFile << "\n";
            std::string source = readFile(inputFile);

            std::cout << "[2/4] Lexing and Parsing...\n";
            Lexer lexer(source);
            std::vector<Token> tokens = lexer.tokenize();
            std::cout << "      Tokens produced: " << tokens.size() << "\n";

            Parser parser(tokens);
            ASTNodePtr ast = parser.parse();

            std::cout << "\n      AST Structure:\n";
            ast->print(3);

            std::cout << "\n[3/4] Generating Three Address Code...\n";
            TACGenerator tacGen;
            original = tacGen.generate(ast.get());

            std::cout << "      Generated " << original.size() << " TAC instructions:\n";
            for (auto& instr : original)
                std::cout << "        " << instr.toString() << "\n";

        } else {
            std::cout << "[1/4] Parsing TAC file: " << inputFile << "\n";
            original = parseTACFile(inputFile);
            std::cout << "      Found " << original.size() << " instructions.\n";
            for (auto& instr : original)
                std::cout << "        " << instr.toString() << "\n";
            std::cout << "[skipping lexer/parser - TAC input mode]\n";
        }

        int step = (ext == ".c") ? 4 : 2;
        std::cout << "\n[" << step << "/4] Running optimization passes...\n";
        Optimizer opt;
        TACProgram optimized = opt.runAll(original);

        std::cout << "      Applied " << opt.logs.size() << " optimizations:\n";
        for (auto& log : opt.logs)
            std::cout << "      [" << log.pass << "] Line " << log.lineIndex
                      << ": " << log.before << "  -->  " << log.after << "\n";

        std::cout << "\n[4/4] Exporting JSON: " << outputFile << "\n";
        exportToJSON(original, optimized, opt.logs, outputFile);

        std::cout << "\nDone! Open frontend/index.html and load " << outputFile << "\n";
        return 0;

    } catch (const std::exception& e) {
        std::cerr << "\n[ERROR] " << e.what() << "\n";
        return 1;
    }
}
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>

#include "lexer/lexer.h"
#include "parser/parser.h"
#include "parser/ast.h"

namespace {
std::string readFile(const std::string& path) {
    std::ifstream file(path, std::ios::binary);
    if (!file.is_open()) {
        throw std::runtime_error("Cannot open input file: " + path);
    }
    return std::string((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
}

void writeText(const std::string& path, const std::string& text) {
    if (path.empty()) {
        std::cout << text;
        return;
    }
    std::ofstream file(path);
    if (!file.is_open()) {
        throw std::runtime_error("Cannot open output file: " + path);
    }
    file << text;
}

void printUsage() {
    std::cerr << "Usage:\n"
              << "  compiler lex --input <file.src> --output <tokens.txt>\n"
              << "  compiler parse --input <file.src> [--ast-format text|dot] [--output-file <file>] [--verbose]\n";
}

std::string getArg(int argc, char* argv[], const std::string& name, const std::string& defaultValue = "") {
    for (int i = 1; i + 1 < argc; ++i) {
        if (argv[i] == name) return argv[i + 1];
    }
    return defaultValue;
}

bool hasArg(int argc, char* argv[], const std::string& name) {
    for (int i = 1; i < argc; ++i) {
        if (argv[i] == name) return true;
    }
    return false;
}
}

int main(int argc, char* argv[]) {
    try {
        if (argc < 2) {
            printUsage();
            return 1;
        }

        std::string command = argv[1];
        std::string inputPath = getArg(argc, argv, "--input");
        if (inputPath.empty()) {
            printUsage();
            return 1;
        }

        std::string source = readFile(inputPath);
        Lexer lexer(source);

        const auto& lexErrors = lexer.get_errors();
        if (!lexErrors.empty()) {
            std::cerr << "Lexical errors:\n";
            for (const auto& err : lexErrors) std::cerr << err << "\n";
            return 1;
        }

        if (command == "lex") {
            std::string outputPath = getArg(argc, argv, "--output");
            if (outputPath.empty()) {
                printUsage();
                return 1;
            }

            std::ostringstream out;
            for (const auto& tok : lexer.all_tokens()) out << tok.to_string() << "\n";
            writeText(outputPath, out.str());
            std::cout << "Tokens written to " << outputPath << "\n";
            return 0;
        }

        if (command == "parse") {
            Parser parser(lexer.all_tokens());
            ProgramNode program = parser.parse();

            const auto& parseErrors = parser.get_errors();
            if (!parseErrors.empty()) {
                std::cerr << "Syntax errors:\n";
                for (const auto& err : parseErrors) std::cerr << err << "\n";
                return 1;
            }

            std::string format = getArg(argc, argv, "--ast-format", "text");
            if (format == "") format = getArg(argc, argv, "--format", "text");
            std::string outputPath = getArg(argc, argv, "--output-file");
            if (outputPath.empty()) outputPath = getArg(argc, argv, "--output");

            std::ostringstream out;
            if (format == "text") {
                printAstText(program, out);
            } else if (format == "dot") {
                printAstDot(program, out);
            } else {
                std::cerr << "Unsupported AST format: " << format << "\n";
                return 1;
            }

            writeText(outputPath, out.str());
            if (!outputPath.empty()) std::cout << "AST written to " << outputPath << "\n";
            if (hasArg(argc, argv, "--verbose")) {
                std::cout << "Tokens: " << lexer.all_tokens().size() << "\n";
                std::cout << "Declarations: " << program.declarations.size() << "\n";
            }
            return 0;
        }

        printUsage();
        return 1;
    } catch (const std::exception& ex) {
        std::cerr << "Error: " << ex.what() << "\n";
        return 1;
    }
}

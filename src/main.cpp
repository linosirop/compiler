#include <fstream>
#include <iostream>
#include <sstream>
#include <stdexcept>
#include <string>

#include "lexer/lexer.h"
#include "parser/parser.h"
#include "parser/ast.h"
#include "semantic/analyzer.h"
#include "ir/ir_generator.h"
#include "ir/control_flow.h"
#include "ir/optimizer.h"
#include "codegen/x86_generator.h"

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
              << "  compiler parse --input <file.src> [--ast-format text|dot] [--output-file <file>] [--verbose]\n"
              << "  compiler check --input <file.src> [--output-file <file>] [--show-types] [--verbose]\n"
              << "  compiler symbols --input <file.src> [--output-file <file>]\n"
              << "  compiler ir --input <file.src> [--format text|dot] [--output-file <file>] [--stats] [--validate] [--optimize] [--optimization-report]\n"
              << "  compiler compile --input <file.src> --output <file.asm> [--target x86_64] [--emit-stack-map] [--optimize] [--optimization-report]\n";
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

bool lexAndParse(const std::string& source, ProgramNode& program) {
    Lexer lexer(source);
    const auto& lexErrors = lexer.get_errors();
    if (!lexErrors.empty()) {
        std::cerr << "Lexical errors:\n";
        for (const auto& err : lexErrors) std::cerr << err << "\n";
        return false;
    }

    Parser parser(lexer.all_tokens());
    program = parser.parse();
    const auto& parseErrors = parser.get_errors();
    if (!parseErrors.empty()) {
        std::cerr << "Syntax errors:\n";
        for (const auto& err : parseErrors) std::cerr << err << "\n";
        return false;
    }
    return true;
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
            if (outputPath.empty()) outputPath = getArg(argc, argv, "--output-file");
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

            std::string format = getArg(argc, argv, "--ast-format", "");
            if (format.empty()) format = getArg(argc, argv, "--format", "text");
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

        if (command == "check" || command == "symbols" || command == "ir" || command == "compile") {
            ProgramNode program;
            if (!lexAndParse(source, program)) return 1;

            semantic::SemanticAnalyzer analyzer;
            analyzer.analyze(program);

            if (command == "check" || command == "symbols") {
                std::string outputPath = getArg(argc, argv, "--output-file");
                if (outputPath.empty()) outputPath = getArg(argc, argv, "--output");

                std::ostringstream out;
                if (command == "symbols") {
                    out << analyzer.get_symbol_table().dump();
                } else {
                    bool showTypes = hasArg(argc, argv, "--show-types") || hasArg(argc, argv, "--verbose");
                    out << analyzer.report(inputPath, showTypes);
                }

                writeText(outputPath, out.str());
                if (!outputPath.empty()) std::cout << "Semantic report written to " << outputPath << "\n";
                if (analyzer.has_errors()) return 1;
                return 0;
            }

            if (analyzer.has_errors()) {
                std::cerr << analyzer.report(inputPath, true);
                return 1;
            }

            ir::IRGenerator generator(&analyzer.get_symbol_table());
            ir::IRProgram irProgram = generator.generate(program);
            ir::OptimizationReport optimizationReport;
            if (hasArg(argc, argv, "--optimize")) {
                optimizationReport = ir::optimizeProgram(irProgram);
            }

            if (command == "compile") {
                std::string target = getArg(argc, argv, "--target", "x86_64");
                if (target != "x86_64") {
                    std::cerr << "Unsupported target: " << target << "\n";
                    return 1;
                }

                std::string outputPath = getArg(argc, argv, "--output-file");
                if (outputPath.empty()) outputPath = getArg(argc, argv, "--output");
                if (outputPath.empty()) {
                    printUsage();
                    return 1;
                }

                codegen::X86GeneratorOptions options;
                options.emitStackMap = hasArg(argc, argv, "--emit-stack-map") || hasArg(argc, argv, "--verbose");
                codegen::X86Generator x86(options);
                writeText(outputPath, x86.generate(irProgram));
                std::cout << "Assembly written to " << outputPath << "\n";
                if (hasArg(argc, argv, "--optimization-report")) {
                    std::cout << optimizationReport.toString();
                }
                return 0;
            }

            std::string format = getArg(argc, argv, "--format", "text");
            std::string outputPath = getArg(argc, argv, "--output-file");
            if (outputPath.empty()) outputPath = getArg(argc, argv, "--output");

            std::ostringstream out;
            if (format == "text") {
                out << irProgram.toText();
            } else if (format == "dot") {
                out << irProgram.toDot();
            } else {
                std::cerr << "Unsupported IR format: " << format << "\n";
                return 1;
            }

            if (hasArg(argc, argv, "--stats")) out << "\n" << irProgram.statistics();
            if (hasArg(argc, argv, "--optimization-report")) out << "\n" << optimizationReport.toString();
            if (hasArg(argc, argv, "--validate")) out << "\n" << ir::validateControlFlow(irProgram);

            writeText(outputPath, out.str());
            if (!outputPath.empty()) std::cout << "IR written to " << outputPath << "\n";
            return 0;
        }

        printUsage();
        return 1;
    } catch (const std::exception& ex) {
        std::cerr << "Error: " << ex.what() << "\n";
        return 1;
    }
}

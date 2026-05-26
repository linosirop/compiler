#include <cstdlib>
#include <fstream>
#include <iostream>
#include <sstream>
#include <stdexcept>
#include <string>
#include <vector>

#include "lexer/lexer.h"
#include "parser/parser.h"
#include "parser/ast.h"
#include "semantic/analyzer.h"
#include "ir/ir_generator.h"
#include "ir/control_flow.h"
#include "ir/optimizer.h"
#include "codegen/x86_generator.h"
#include "utils/diagnostics.h"

namespace {
constexpr const char* kVersion = "mycc 1.0.0 (MiniCompiler final sprint)";

std::string readFile(const std::string& path) {
    std::ifstream file(path, std::ios::binary);
    if (!file.is_open()) {
        throw std::runtime_error("Cannot open input file: " + path);
    }
    return std::string((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
}

void writeText(const std::string& path, const std::string& text) {
    if (path.empty() || path == "-") {
        std::cout << text;
        return;
    }
    std::ofstream file(path);
    if (!file.is_open()) {
        throw std::runtime_error("Cannot open output file: " + path);
    }
    file << text;
}

std::string basenameNoExt(const std::string& path) {
    std::string name = path;
    const auto slash = name.find_last_of("/\\");
    if (slash != std::string::npos) name = name.substr(slash + 1);
    const auto dot = name.find_last_of('.');
    if (dot != std::string::npos) name = name.substr(0, dot);
    return name.empty() ? "a" : name;
}

std::string shellQuote(const std::string& value) {
    std::string out = "'";
    for (char ch : value) {
        if (ch == '\'') out += "'\\''";
        else out += ch;
    }
    out += "'";
    return out;
}

void printLegacyUsage() {
    std::cerr << "Legacy usage:\n"
              << "  compiler lex --input <file.src> --output <tokens.txt>\n"
              << "  compiler parse --input <file.src> [--ast-format text|dot] [--output-file <file>] [--verbose]\n"
              << "  compiler check --input <file.src> [--output-file <file>] [--show-types] [--verbose]\n"
              << "  compiler symbols --input <file.src> [--output-file <file>]\n"
              << "  compiler ir --input <file.src> [--format text|dot] [--output-file <file>] [--stats] [--validate] [--optimize] [--optimization-report]\n"
              << "  compiler compile --input <file.src> --output <file.asm> [--target x86_64] [--emit-stack-map] [--optimize] [--optimization-report]\n"
              << "\nModern mycc-style usage:\n"
              << "  mycc [options] <source.src>\n"
              << "  mycc -S <source.src> -o <file.asm>\n"
              << "  mycc -c <source.src> -o <file.o>\n"
              << "  mycc --ast <source.src>\n"
              << "  mycc --ir <source.src> [--optimize]\n";
}

void printHelp() {
    std::cout << "MiniCompiler final CLI (mycc)\n"
              << "Usage:\n"
              << "  mycc [options] <source.src>\n\n"
              << "Compilation modes:\n"
              << "  mycc program.src -o program       Full compilation: source -> asm -> object -> executable\n"
              << "  mycc -S program.src -o program.asm  Generate assembly only\n"
              << "  mycc -c program.src -o program.o    Compile to object file only\n"
              << "  mycc --ast program.src              Print AST\n"
              << "  mycc --ir program.src               Print IR\n"
              << "  mycc -E program.src                 Preprocess-only placeholder: prints source\n\n"
              << "Options:\n"
              << "  -o <file>              Output file\n"
              << "  -S                     Generate assembly only\n"
              << "  -c                     Compile to object file only\n"
              << "  -E                     Preprocess only\n"
              << "  --ast                  Output AST in text format\n"
              << "  --ir                   Output intermediate representation\n"
              << "  --optimize, -O, -O0..3 Enable IR optimizations\n"
              << "  --optimization-report  Print optimization statistics\n"
              << "  --target <arch>        Target architecture, default: x86_64\n"
              << "  -l<lib>                Link with library, for example -lm\n"
              << "  -v, --verbose          Print detailed compilation steps\n"
              << "  -h, --help             Show this help\n"
              << "  --version              Show version\n\n"
              << "Examples:\n"
              << "  mycc demo/final_showcase.src -o final_showcase\n"
              << "  mycc -S tests/control_flow/valid/logical_ops/short_circuit_and.src -o sc_and.asm\n"
              << "  mycc --ir tests/optimization/constant_folding.src --optimize --optimization-report\n";
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

int runCommand(const std::string& command, bool verbose) {
    if (verbose) std::cerr << "+ " << command << "\n";
    return std::system(command.c_str());
}

bool isLegacyCommand(const std::string& command) {
    return command == "lex" || command == "parse" || command == "check" ||
           command == "symbols" || command == "ir" || command == "compile";
}

int runLegacy(int argc, char* argv[]) {
    if (argc < 2) {
        printLegacyUsage();
        return 1;
    }

    std::string command = argv[1];
    std::string inputPath = getArg(argc, argv, "--input");
    if (inputPath.empty()) {
        printLegacyUsage();
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
        if (outputPath.empty()) outputPath = "-";

        std::ostringstream out;
        for (const auto& tok : lexer.all_tokens()) out << tok.to_string() << "\n";
        writeText(outputPath, out.str());
        if (outputPath != "-") std::cout << "Tokens written to " << outputPath << "\n";
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
                printLegacyUsage();
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

    printLegacyUsage();
    return 1;
}

struct ModernOptions {
    bool help{false};
    bool version{false};
    bool assemblyOnly{false};
    bool objectOnly{false};
    bool preprocessOnly{false};
    bool ast{false};
    bool ir{false};
    bool optimize{false};
    bool optimizationReport{false};
    bool verbose{false};
    bool emitStackMap{false};
    std::string output;
    std::string target{"x86_64"};
    std::vector<std::string> libraries;
    std::vector<std::string> inputs;
};

ModernOptions parseModernOptions(int argc, char* argv[]) {
    ModernOptions opts;
    for (int i = 1; i < argc; ++i) {
        const std::string arg = argv[i];
        if (arg == "-h" || arg == "--help") opts.help = true;
        else if (arg == "--version") opts.version = true;
        else if (arg == "-S") opts.assemblyOnly = true;
        else if (arg == "-c") opts.objectOnly = true;
        else if (arg == "-E") opts.preprocessOnly = true;
        else if (arg == "--ast") opts.ast = true;
        else if (arg == "--ir") opts.ir = true;
        else if (arg == "--optimize" || arg == "-O" || arg.rfind("-O", 0) == 0) opts.optimize = true;
        else if (arg == "--optimization-report") opts.optimizationReport = true;
        else if (arg == "-v" || arg == "--verbose") opts.verbose = true;
        else if (arg == "--emit-stack-map") opts.emitStackMap = true;
        else if (arg == "--target" && i + 1 < argc) opts.target = argv[++i];
        else if (arg == "-o" && i + 1 < argc) opts.output = argv[++i];
        else if (arg.rfind("-l", 0) == 0 && arg.size() > 2) opts.libraries.push_back(arg);
        else if (!arg.empty() && arg[0] == '-') {
            // Unknown options are ignored with a warning to keep the final CLI user-friendly.
            std::cerr << "warning W001: unsupported option ignored: " << arg << "\n";
        } else {
            opts.inputs.push_back(arg);
        }
    }
    return opts;
}

int runModern(int argc, char* argv[]) {
    ModernOptions opts = parseModernOptions(argc, argv);
    if (opts.help || argc == 1) {
        printHelp();
        return 0;
    }
    if (opts.version) {
        std::cout << kVersion << "\nTarget: x86_64-linux-gnu\n";
        return 0;
    }
    if (opts.inputs.empty()) {
        diagnostics::DiagnosticBag bag;
        bag.error("E001", "no input files", "<command line>", 1, 1, "", "pass a .src file, for example: mycc demo/final_showcase.src -o final_showcase");
        std::cerr << bag.toString(false);
        return 1;
    }
    if (opts.target != "x86_64") {
        diagnostics::DiagnosticBag bag;
        bag.error("E002", "unsupported target '" + opts.target + "'", "<command line>", 1, 1, "", "the final backend currently supports x86_64 only");
        std::cerr << bag.toString(false);
        return 1;
    }
    if (opts.inputs.size() > 1) {
        std::cerr << "warning W101: multiple input files are accepted by the CLI, but this educational compiler compiles the first file only in this build\n";
    }

    const std::string input = opts.inputs.front();
    if (opts.preprocessOnly) {
        writeText(opts.output, readFile(input));
        return 0;
    }

    const std::string source = readFile(input);
    ProgramNode program;
    if (!lexAndParse(source, program)) return 1;

    if (opts.ast) {
        std::ostringstream out;
        printAstText(program, out);
        writeText(opts.output, out.str());
        return 0;
    }

    semantic::SemanticAnalyzer analyzer;
    analyzer.analyze(program);
    if (analyzer.has_errors()) {
        std::cerr << analyzer.report(input, true);
        return 1;
    }

    ir::IRGenerator generator(&analyzer.get_symbol_table());
    ir::IRProgram irProgram = generator.generate(program);
    ir::OptimizationReport optimizationReport;
    if (opts.optimize) {
        optimizationReport = ir::optimizeProgram(irProgram);
    }

    if (opts.ir) {
        std::ostringstream out;
        out << irProgram.toText();
        if (opts.optimizationReport) out << "\n" << optimizationReport.toString();
        writeText(opts.output, out.str());
        return 0;
    }

    codegen::X86GeneratorOptions x86Options;
    x86Options.emitStackMap = opts.emitStackMap || opts.verbose;
    codegen::X86Generator x86(x86Options);
    const std::string assembly = x86.generate(irProgram);

    if (opts.assemblyOnly) {
        const std::string asmOut = opts.output.empty() ? basenameNoExt(input) + ".asm" : opts.output;
        writeText(asmOut, assembly);
        if (opts.verbose) std::cerr << "assembly written to " << asmOut << "\n";
        if (opts.optimizationReport) std::cout << optimizationReport.toString();
        return 0;
    }

    const std::string base = opts.output.empty() ? basenameNoExt(input) : opts.output;
    const std::string asmPath = opts.objectOnly ? base + ".tmp.asm" : base + ".mycc.asm";
    const std::string objPath = opts.objectOnly ? (opts.output.empty() ? basenameNoExt(input) + ".o" : opts.output)
                                                : base + ".mycc.o";
    writeText(asmPath, assembly);

    const std::string nasmCommand = "nasm -f elf64 -o " + shellQuote(objPath) + " " + shellQuote(asmPath);
    if (runCommand(nasmCommand, opts.verbose) != 0) return 1;

    if (opts.objectOnly) {
        if (opts.verbose) std::cerr << "object written to " << objPath << "\n";
        if (opts.optimizationReport) std::cout << optimizationReport.toString();
        return 0;
    }

    const std::string exePath = opts.output.empty() ? "a.out" : opts.output;
    std::string linkCommand = "gcc -no-pie -o " + shellQuote(exePath) + " " + shellQuote(objPath);
    for (const auto& lib : opts.libraries) linkCommand += " " + shellQuote(lib);
    if (runCommand(linkCommand, opts.verbose) != 0) return 1;

    if (opts.verbose) std::cerr << "executable written to " << exePath << "\n";
    if (opts.optimizationReport) std::cout << optimizationReport.toString();
    return 0;
}
}

int main(int argc, char* argv[]) {
    try {
        if (argc >= 2 && isLegacyCommand(argv[1])) {
            return runLegacy(argc, argv);
        }
        return runModern(argc, argv);
    } catch (const std::exception& ex) {
        diagnostics::DiagnosticBag bag;
        bag.error("E999", ex.what(), "<compiler>", 1, 1);
        std::cerr << bag.toString(false);
        return 1;
    }
}

#include "lexer/lexer.h"
#include "parser/parser.h"
#include "semantic/analyzer.h"
#include "ir/ir_generator.h"
#include "ir/control_flow.h"

#include <iostream>
#include <sstream>
#include <string>
#include <vector>

namespace {
int passed = 0;
int failed = 0;

void check(bool condition, const std::string& name, const std::string& details = "") {
    if (condition) {
        ++passed;
        std::cout << "[PASS] " << name << "\n";
    } else {
        ++failed;
        std::cout << "[FAIL] " << name;
        if (!details.empty()) std::cout << " -- " << details;
        std::cout << "\n";
    }
}

bool contains(const std::string& text, const std::string& needle) {
    return text.find(needle) != std::string::npos;
}

int countOccurrences(const std::string& text, const std::string& needle) {
    int count = 0;
    size_t pos = 0;
    while ((pos = text.find(needle, pos)) != std::string::npos) {
        ++count;
        pos += needle.size();
    }
    return count;
}

bool compileToIR(const std::string& source, ir::IRProgram& outProgram, std::string& errorText) {
    Lexer lexer(source);
    if (!lexer.get_errors().empty()) {
        std::ostringstream out;
        for (const auto& error : lexer.get_errors()) out << error << "\n";
        errorText = out.str();
        return false;
    }

    Parser parser(lexer.all_tokens());
    ProgramNode ast = parser.parse();
    if (!parser.get_errors().empty()) {
        std::ostringstream out;
        for (const auto& error : parser.get_errors()) out << error << "\n";
        errorText = out.str();
        return false;
    }

    semantic::SemanticAnalyzer analyzer;
    analyzer.analyze(ast);
    if (analyzer.has_errors()) {
        errorText = analyzer.report("test.src", true);
        return false;
    }

    ir::IRGenerator generator(&analyzer.get_symbol_table());
    outProgram = generator.generate(ast);
    return true;
}

std::string irText(const std::string& source) {
    ir::IRProgram program;
    std::string errors;
    if (!compileToIR(source, program, errors)) return errors;
    return program.toText();
}
}

int main() {
    std::cout << "MiniCompiler IR tests\n";
    std::cout << "=====================\n";

    {
        std::string source =
            "fn main() -> void {\n"
            "    int x = 2 * 3 + 4;\n"
            "}\n";
        ir::IRProgram program;
        std::string errors;
        bool ok = compileToIR(source, program, errors);
        std::string text = ok ? program.toText() : errors;
        check(ok, "ir: arithmetic sample compiles", errors);
        check(contains(text, "MUL 2, 3"), "ir: multiplication is emitted before addition", text);
        check(contains(text, "ADD"), "ir: addition instruction is emitted", text);
        check(contains(text, "STORE [x_0]"), "ir: variable initializer is stored", text);
        check(contains(text, "x -> [x_0]"), "ir: variable mapping is dumped", text);
        check(program.getFunction("main") != nullptr, "ir: main function exists");
        check(program.getFunction("main") && program.getFunction("main")->tempCount >= 2, "ir: arithmetic uses temporaries");
    }

    {
        std::string source =
            "fn add(int a, int b) -> int {\n"
            "    return a + b;\n"
            "}\n"
            "fn main() -> void {\n"
            "    int x = add(1, 2);\n"
            "}\n";
        std::string text = irText(source);
        check(contains(text, "function add: int"), "ir: function signature for add is emitted", text);
        check(contains(text, "LOAD [a]"), "ir: parameter a can be loaded", text);
        check(contains(text, "LOAD [b]"), "ir: parameter b can be loaded", text);
        check(contains(text, "RETURN"), "ir: return instruction is emitted", text);
        check(contains(text, "PARAM 0, 1"), "ir: first call argument is emitted as PARAM", text);
        check(contains(text, "PARAM 1, 2"), "ir: second call argument is emitted as PARAM", text);
        check(contains(text, "CALL add"), "ir: call instruction is emitted", text);
    }

    {
        std::string source =
            "fn main() -> void {\n"
            "    int x = 0;\n"
            "    if (x < 10) {\n"
            "        x = x + 1;\n"
            "    } else {\n"
            "        x = x - 1;\n"
            "    }\n"
            "}\n";
        ir::IRProgram program;
        std::string errors;
        bool ok = compileToIR(source, program, errors);
        std::string text = ok ? program.toText() : errors;
        check(ok, "ir: if/else sample compiles", errors);
        check(contains(text, "CMP_LT"), "ir: if condition comparison is emitted", text);
        check(contains(text, "JUMP_IF"), "ir: conditional jump is emitted", text);
        check(contains(text, "L_then"), "ir: then block label is emitted", text);
        check(contains(text, "L_else"), "ir: else block label is emitted", text);
        check(contains(text, "L_endif"), "ir: join block label is emitted", text);
        std::string validation;
        check(ir::allJumpTargetsExist(program, &validation), "ir: if/else jump targets exist", validation);
    }

    {
        std::string source =
            "fn main() -> void {\n"
            "    int i = 0;\n"
            "    while (i < 3) {\n"
            "        i = i + 1;\n"
            "    }\n"
            "}\n";
        ir::IRProgram program;
        std::string errors;
        bool ok = compileToIR(source, program, errors);
        std::string text = ok ? program.toText() : errors;
        check(ok, "ir: while sample compiles", errors);
        check(contains(text, "L_while_header"), "ir: while header block is emitted", text);
        check(contains(text, "L_while_body"), "ir: while body block is emitted", text);
        check(contains(text, "L_while_end"), "ir: while end block is emitted", text);
        check(countOccurrences(text, "JUMP") >= 3, "ir: while emits multiple jumps", text);
        std::string validation;
        check(ir::allJumpTargetsExist(program, &validation), "ir: while jump targets exist", validation);
    }

    {
        std::string source =
            "fn main() -> void {\n"
            "    int sum = 0;\n"
            "    for (int i = 0; i < 3; i = i + 1) {\n"
            "        sum = sum + i;\n"
            "    }\n"
            "}\n";
        std::string text = irText(source);
        check(contains(text, "L_for_header"), "ir: for header block is emitted", text);
        check(contains(text, "L_for_body"), "ir: for body block is emitted", text);
        check(contains(text, "L_for_end"), "ir: for end block is emitted", text);
        check(contains(text, "STORE [sum_0]"), "ir: for loop body stores updated sum", text);
        check(contains(text, "STORE [i_1]") || contains(text, "STORE [i_0]"), "ir: for update stores loop variable", text);
    }

    {
        std::string source =
            "fn main() -> void {\n"
            "    bool ok = !(true && false) || true;\n"
            "}\n";
        std::string text = irText(source);
        check(contains(text, "AND true, false"), "ir: logical AND is emitted", text);
        check(contains(text, "NOT"), "ir: logical NOT is emitted", text);
        check(contains(text, "OR"), "ir: logical OR is emitted", text);
    }

    {
        std::string source =
            "fn main() -> void {\n"
            "    int x = 1;\n"
            "    x += 2;\n"
            "    x *= 3;\n"
            "}\n";
        std::string text = irText(source);
        check(contains(text, "ADD"), "ir: += emits ADD", text);
        check(contains(text, "MUL"), "ir: *= emits MUL", text);
        check(countOccurrences(text, "STORE [x_0]") >= 3, "ir: compound assignments store results", text);
    }

    {
        std::string source =
            "fn main() -> void {\n"
            "    int x = 1;\n"
            "    if (x > 0) { x = 2; }\n"
            "}\n";
        ir::IRProgram program;
        std::string errors;
        bool ok = compileToIR(source, program, errors);
        std::string dot = ok ? program.toDot() : errors;
        check(ok, "ir: DOT sample compiles", errors);
        check(contains(dot, "digraph CFG"), "ir: DOT output contains graph heading", dot);
        check(contains(dot, "function main"), "ir: DOT output contains function cluster", dot);
        check(contains(dot, "->"), "ir: DOT output contains CFG edges", dot);
        check(contains(program.statistics(), "Basic blocks:"), "ir: statistics report basic blocks", program.statistics());
        check(contains(ir::validateControlFlow(program), "jump targets: ok"), "ir: validation report checks jump targets", ir::validateControlFlow(program));
    }

    std::cout << "\nPassed: " << passed << "\n";
    std::cout << "Failed: " << failed << "\n";
    return failed == 0 ? 0 : 1;
}

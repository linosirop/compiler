#include "lexer/lexer.h"
#include "parser/parser.h"
#include "semantic/analyzer.h"
#include "ir/ir_generator.h"
#include "codegen/x86_generator.h"

#include <iostream>
#include <sstream>
#include <string>

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

bool compileToAssembly(const std::string& source, std::string& assembly, std::string& errors) {
    Lexer lexer(source);
    if (!lexer.get_errors().empty()) {
        std::ostringstream out;
        for (const auto& error : lexer.get_errors()) out << error << "\n";
        errors = out.str();
        return false;
    }

    Parser parser(lexer.all_tokens());
    ProgramNode ast = parser.parse();
    if (!parser.get_errors().empty()) {
        std::ostringstream out;
        for (const auto& error : parser.get_errors()) out << error << "\n";
        errors = out.str();
        return false;
    }

    semantic::SemanticAnalyzer analyzer;
    analyzer.analyze(ast);
    if (analyzer.has_errors()) {
        errors = analyzer.report("codegen_test.src", true);
        return false;
    }

    ir::IRGenerator irGenerator(&analyzer.get_symbol_table());
    codegen::X86Generator x86;
    assembly = x86.generate(irGenerator.generate(ast));
    return true;
}
}

int main() {
    std::cout << "MiniCompiler codegen tests\n";
    std::cout << "==========================\n";

    {
        std::string source =
            "fn main() -> int {\n"
            "    int x = 2 * 3 + 4;\n"
            "    return x;\n"
            "}\n";
        std::string assembly;
        std::string errors;
        bool ok = compileToAssembly(source, assembly, errors);
        check(ok, "codegen: arithmetic source compiles", errors);
        check(contains(assembly, "global main"), "codegen: exports main", assembly);
        check(contains(assembly, "push rbp"), "codegen: emits function prologue", assembly);
        check(contains(assembly, "imul rax, r10"), "codegen: maps MUL to imul", assembly);
        check(contains(assembly, "add rax, r10"), "codegen: maps ADD to add", assembly);
        check(contains(assembly, "ret"), "codegen: emits epilogue ret", assembly);
    }

    {
        std::string source =
            "fn add(int a, int b) -> int { return a + b; }\n"
            "fn main() -> int { return add(2, 3); }\n";
        std::string assembly;
        std::string errors;
        bool ok = compileToAssembly(source, assembly, errors);
        check(ok, "codegen: function call source compiles", errors);
        check(contains(assembly, "mov rdi, rax"), "codegen: passes first integer arg in rdi", assembly);
        check(contains(assembly, "mov rsi, rax"), "codegen: passes second integer arg in rsi", assembly);
        check(contains(assembly, "call add"), "codegen: emits call instruction", assembly);
    }

    {
        std::string source =
            "fn main() -> int {\n"
            "    int x = 1;\n"
            "    if (x < 2) { return 7; } else { return 9; }\n"
            "}\n";
        std::string assembly;
        std::string errors;
        bool ok = compileToAssembly(source, assembly, errors);
        check(ok, "codegen: control flow source compiles", errors);
        check(contains(assembly, "cmp rax, r10"), "codegen: emits cmp", assembly);
        check(contains(assembly, "setl al"), "codegen: maps CMP_LT to setl", assembly);
        check(contains(assembly, "jne .main_L_then"), "codegen: maps JUMP_IF to jne", assembly);
        check(contains(assembly, "jmp .main_L_else") || contains(assembly, "jmp .main_L_endif"), "codegen: emits unconditional jumps", assembly);
    }

    std::cout << "\nPassed: " << passed << "\n";
    std::cout << "Failed: " << failed << "\n";
    return failed == 0 ? 0 : 1;
}

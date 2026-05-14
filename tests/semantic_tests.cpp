#include "lexer/lexer.h"
#include "parser/parser.h"
#include "semantic/analyzer.h"
#include "semantic/symbol_table.h"
#include "semantic/type_system.h"

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

std::string joinErrors(const std::vector<semantic::SemanticError>& errors) {
    std::ostringstream out;
    for (const auto& error : errors) out << error.toString() << "\n";
    return out.str();
}

bool parseProgram(const std::string& source, ProgramNode& program, std::string& errorText) {
    Lexer lexer(source);
    if (!lexer.get_errors().empty()) {
        for (const auto& error : lexer.get_errors()) errorText += error + "\n";
        return false;
    }
    Parser parser(lexer.all_tokens());
    program = parser.parse();
    if (!parser.get_errors().empty()) {
        for (const auto& error : parser.get_errors()) errorText += error + "\n";
        return false;
    }
    return true;
}

semantic::SemanticAnalyzer analyzeSource(const std::string& source, bool& parsed, std::string& parseErrors) {
    ProgramNode program;
    parsed = parseProgram(source, program, parseErrors);
    semantic::SemanticAnalyzer analyzer;
    if (parsed) analyzer.analyze(program);
    return analyzer;
}

bool hasErrorContaining(const semantic::SemanticAnalyzer& analyzer, const std::string& text) {
    for (const auto& error : analyzer.get_errors()) {
        if (error.toString().find(text) != std::string::npos) return true;
    }
    return false;
}

void testSymbolTable() {
    semantic::SymbolTable table;
    semantic::SymbolInfo global;
    global.name = "x";
    global.kind = semantic::SymbolKind::Variable;
    global.type = semantic::Type::intType();
    global.line = 1;
    global.column = 1;

    check(table.insert("x", global), "symbol table inserts into global scope");
    check(!table.insert("x", global), "symbol table rejects duplicate in same scope");
    check(table.lookup("x") != nullptr, "symbol table lookup finds global symbol");

    table.enter_scope("inner");
    semantic::SymbolInfo local = global;
    local.name = "y";
    local.type = semantic::Type::floatType();
    check(table.insert("y", local), "symbol table inserts into nested scope");
    check(table.lookup("x") != nullptr, "nested scope can see outer symbol");
    check(table.lookup_local("x") == nullptr, "lookup_local ignores outer symbol");
    check(table.lookup("y") != nullptr, "nested scope can see local symbol");
    table.exit_scope();
    check(table.lookup("y") == nullptr, "symbol disappears after scope exit");
}

void testTypeSystem() {
    using semantic::Type;
    check(semantic::canAssign(Type::intType(), Type::intType()), "type system allows int to int assignment");
    check(semantic::canAssign(Type::floatType(), Type::intType()), "type system allows int to float widening");
    check(!semantic::canAssign(Type::intType(), Type::floatType()), "type system rejects float to int narrowing");
    check(semantic::arithmeticResult(Type::intType(), Type::floatType(), "+").kind == semantic::TypeKind::Float,
          "type system infers int + float as float");
    check(semantic::logicalResult(Type::boolType(), Type::boolType(), "&&").kind == semantic::TypeKind::Bool,
          "type system infers bool && bool as bool");
    check(semantic::unaryResult(Type::boolType(), "!").kind == semantic::TypeKind::Bool,
          "type system infers !bool as bool");
}

void testValidPrograms() {
    const std::string program =
        "struct Point { int x; int y; }\n"
        "fn add(int a, float b) -> float {\n"
        "  float c = a + b;\n"
        "  return c;\n"
        "}\n"
        "fn main() {\n"
        "  int x = 1;\n"
        "  float y = add(x, 2.5);\n"
        "  bool ok = y > 0.0;\n"
        "  if (ok) { y = y + 1.0; }\n"
        "  return;\n"
        "}\n";
    bool parsed = false;
    std::string parseErrors;
    auto analyzer = analyzeSource(program, parsed, parseErrors);
    check(parsed, "valid semantic program parses", parseErrors);
    check(!analyzer.has_errors(), "valid semantic program has no semantic errors", joinErrors(analyzer.get_errors()));
    check(analyzer.get_symbol_table().dump().find("add: function") != std::string::npos,
          "symbol dump contains function declaration");
    check(!analyzer.get_type_annotations().empty(), "semantic analyzer records expression type annotations");

    const std::string forward =
        "fn main() { int x = inc(1); }\n"
        "fn inc(int v) -> int { return v + 1; }\n";
    parseErrors.clear();
    analyzer = analyzeSource(forward, parsed, parseErrors);
    check(parsed, "forward-reference program parses", parseErrors);
    check(!analyzer.has_errors(), "function forward reference is allowed", joinErrors(analyzer.get_errors()));
}

void testInvalidPrograms() {
    struct Case { std::string name; std::string source; std::string expected; };
    const std::vector<Case> cases = {
        {"duplicate variable", "fn main() { int x = 1; int x = 2; }", "duplicate declaration"},
        {"undeclared variable", "fn main() { x = 1; }", "undeclared identifier"},
        {"assignment type mismatch", "fn main() { int x = 0; x = 3.14; }", "type mismatch"},
        {"return type mismatch", "fn f() -> int { return true; }", "invalid return type"},
        {"argument count mismatch", "fn f(int a, int b) -> int { return a + b; } fn main() { int x = f(1); }", "argument count mismatch"},
        {"argument type mismatch", "fn f(bool b) -> int { return 1; } fn main() { int x = f(123); }", "argument type mismatch"},
        {"invalid if condition", "fn main() { if (1) { return; } }", "invalid condition type"},
        {"invalid while condition", "fn main() { while (2.5) { return; } }", "invalid condition type"},
        {"scope error", "fn main() { { int x = 1; } int y = x; }", "undeclared identifier"},
        {"use before initialization", "fn main() { int x; int y = x + 1; }", "use before declaration"},
        {"unknown type", "fn main() { Unknown value; }", "unknown type"},
        {"duplicate function", "fn f() { } fn f() { }", "duplicate declaration"}
    };

    for (const auto& testCase : cases) {
        bool parsed = false;
        std::string parseErrors;
        auto analyzer = analyzeSource(testCase.source, parsed, parseErrors);
        check(parsed, testCase.name + " parses before semantic check", parseErrors);
        check(hasErrorContaining(analyzer, testCase.expected), testCase.name + " reports " + testCase.expected,
              joinErrors(analyzer.get_errors()));
    }
}

void testReportOutput() {
    bool parsed = false;
    std::string parseErrors;
    auto analyzer = analyzeSource("fn main() { int x = true; }", parsed, parseErrors);
    check(parsed, "report sample parses", parseErrors);
    std::string report = analyzer.report("sample.src", true);
    check(report.find("Semantic Analysis Report") != std::string::npos, "semantic report has heading");
    check(report.find("sample.src") != std::string::npos, "semantic report includes file name");
    check(report.find("Symbol Table") != std::string::npos, "semantic report includes symbol table");
    check(report.find("Type Annotations") != std::string::npos, "semantic report includes type annotations");
}
}

int main() {
    testSymbolTable();
    testTypeSystem();
    testValidPrograms();
    testInvalidPrograms();
    testReportOutput();

    std::cout << "\nPassed: " << passed << "\n";
    std::cout << "Failed: " << failed << "\n";
    return failed == 0 ? 0 : 1;
}

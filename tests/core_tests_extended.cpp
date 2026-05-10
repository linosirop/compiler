#include "lexer/lexer.h"
#include "parser/parser.h"
#include "parser/ast.h"

#include <algorithm>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>

namespace {
int passed = 0;
int failed = 0;

void check(bool condition, const std::string& name, const std::string& details = "") {
    if (condition) {
        std::cout << "[PASS] " << name << "\n";
        ++passed;
    } else {
        std::cout << "[FAIL] " << name;
        if (!details.empty()) std::cout << " -- " << details;
        std::cout << "\n";
        ++failed;
    }
}

std::string joinErrors(const std::vector<std::string>& errors) {
    std::ostringstream out;
    for (const auto& e : errors) out << e << "\n";
    return out.str();
}

std::string astText(const ProgramNode& program) {
    std::ostringstream out;
    printAstText(program, out);
    return out.str();
}

std::string astDot(const ProgramNode& program) {
    std::ostringstream out;
    printAstDot(program, out);
    return out.str();
}

bool contains(const std::string& haystack, const std::string& needle) {
    return haystack.find(needle) != std::string::npos;
}

int countType(const std::vector<Token>& tokens, TokenType type) {
    return static_cast<int>(std::count_if(tokens.begin(), tokens.end(), [type](const Token& t) {
        return t.type == type;
    }));
}

struct ParseResult {
    Lexer lexer;
    Parser parser;
    ProgramNode program;

    explicit ParseResult(const std::string& source)
        : lexer(source), parser(lexer.all_tokens()), program(parser.parse()) {}
};

void lexerTests() {
    {
        Lexer lx("fn main() -> void { return; }");
        check(lx.get_errors().empty(), "lexer: valid function has no lexical errors", joinErrors(lx.get_errors()));
        const auto& t = lx.all_tokens();
        check(!t.empty() && t.front().type == TokenType::KW_FN, "lexer: first token is KW_FN");
        check(!t.empty() && t.back().type == TokenType::END_OF_FILE, "lexer: final token is END_OF_FILE");
        check(countType(t, TokenType::ARROW) == 1, "lexer: recognizes function return arrow");
    }

    {
        Lexer lx("if else while for int float bool return true false void struct fn");
        check(lx.get_errors().empty(), "lexer: keywords and booleans have no lexical errors", joinErrors(lx.get_errors()));
        check(countType(lx.all_tokens(), TokenType::KW_IF) == 1, "lexer: recognizes if keyword");
        check(countType(lx.all_tokens(), TokenType::KW_ELSE) == 1, "lexer: recognizes else keyword");
        check(countType(lx.all_tokens(), TokenType::BOOL_LITERAL) == 2, "lexer: true and false are bool literals");
        check(countType(lx.all_tokens(), TokenType::KW_STRUCT) == 1, "lexer: recognizes struct keyword");
        check(countType(lx.all_tokens(), TokenType::KW_FN) == 1, "lexer: recognizes fn keyword");
    }

    {
        Lexer lx("abc a1 a_b A_B9 z_123");
        check(lx.get_errors().empty(), "lexer: valid identifiers have no lexical errors", joinErrors(lx.get_errors()));
        check(countType(lx.all_tokens(), TokenType::IDENTIFIER) == 5, "lexer: recognizes identifiers with digits and underscores after first character");
    }

    {
        Lexer lx("_bad");
        check(!lx.get_errors().empty(), "lexer: rejects identifier starting with underscore");
    }

    {
        Lexer lx(std::string(255, 'a'));
        check(lx.get_errors().empty(), "lexer: accepts 255-character identifier", joinErrors(lx.get_errors()));
        check(countType(lx.all_tokens(), TokenType::IDENTIFIER) == 1, "lexer: emits identifier token for 255-character identifier");
    }

    {
        Lexer lx(std::string(256, 'a'));
        check(!lx.get_errors().empty(), "lexer: rejects identifier longer than 255 characters");
    }

    {
        Lexer lx("0 42 2147483647 0.5 123.456");
        check(lx.get_errors().empty(), "lexer: valid integer and float literals have no lexical errors", joinErrors(lx.get_errors()));
        check(countType(lx.all_tokens(), TokenType::INT_LITERAL) == 3, "lexer: recognizes integer literals");
        check(countType(lx.all_tokens(), TokenType::FLOAT_LITERAL) == 2, "lexer: recognizes float literals");
    }

    {
        Lexer lx("2147483648");
        check(!lx.get_errors().empty(), "lexer: rejects integer literal above int32 max");
    }

    {
        Lexer lx("1.");
        check(!lx.get_errors().empty(), "lexer: rejects float literal without digits after dot");
    }

    {
        Lexer lx("\"hello world\"");
        check(lx.get_errors().empty(), "lexer: valid string literal has no lexical errors", joinErrors(lx.get_errors()));
        check(countType(lx.all_tokens(), TokenType::STRING_LITERAL) == 1, "lexer: recognizes string literal");
    }

    {
        Lexer lx("\"unterminated");
        check(!lx.get_errors().empty(), "lexer: rejects unterminated string literal");
    }

    {
        Lexer lx("// comment\nfn main() {} /* closed at eof */");
        check(lx.get_errors().empty(), "lexer: skips line and block comments", joinErrors(lx.get_errors()));
        check(countType(lx.all_tokens(), TokenType::KW_FN) == 1, "lexer: tokens after comments are preserved");
    }

    {
        Lexer lx("/* unterminated");
        check(!lx.get_errors().empty(), "lexer: rejects unterminated block comment");
    }

    {
        Lexer lx("int x = @42;");
        check(!lx.get_errors().empty(), "lexer: reports invalid character");
        std::string err = joinErrors(lx.get_errors());
        check(contains(err, "1:9"), "lexer: reports invalid character at starting column", err);
    }

    {
        Lexer lx("fn main() {\r\n  int x = 1;\r\n}");
        check(lx.get_errors().empty(), "lexer: accepts CRLF line endings", joinErrors(lx.get_errors()));
        bool foundIntLine2 = false;
        for (const auto& t : lx.all_tokens()) {
            if (t.type == TokenType::KW_INT && t.line == 2 && t.column == 3) foundIntLine2 = true;
        }
        check(foundIntLine2, "lexer: tracks line and column after CRLF");
    }

    {
        Lexer lx("+ - * / % == != < <= > >= && || ! = += -= *= /= ( ) { } [ ] ; , : ->");
        check(lx.get_errors().empty(), "lexer: accepts all required operators and delimiters", joinErrors(lx.get_errors()));
        check(countType(lx.all_tokens(), TokenType::PLUS_EQUAL) == 1, "lexer: recognizes +=");
        check(countType(lx.all_tokens(), TokenType::MINUS_EQUAL) == 1, "lexer: recognizes -=");
        check(countType(lx.all_tokens(), TokenType::STAR_EQUAL) == 1, "lexer: recognizes *=");
        check(countType(lx.all_tokens(), TokenType::SLASH_EQUAL) == 1, "lexer: recognizes /=");
        check(countType(lx.all_tokens(), TokenType::ARROW) == 1, "lexer: recognizes ->");
    }
}

void parserTests() {
    {
        ParseResult r("fn main() -> void { int x = 1 + 2 * 3; return; }");
        check(r.lexer.get_errors().empty(), "parser: precedence sample has no lexical errors", joinErrors(r.lexer.get_errors()));
        check(r.parser.get_errors().empty(), "parser: precedence sample has no parse errors", joinErrors(r.parser.get_errors()));
        std::string text = astText(r.program);
        check(contains(text, "Operator: +") && contains(text, "Operator: *"), "parser: AST contains additive and multiplicative operators");
    }

    {
        ParseResult r("fn add(int a, int b) -> int { return a + b; }");
        check(r.lexer.get_errors().empty(), "parser: function with parameters has no lexical errors", joinErrors(r.lexer.get_errors()));
        check(r.parser.get_errors().empty(), "parser: function with parameters has no parse errors", joinErrors(r.parser.get_errors()));
        std::string text = astText(r.program);
        check(contains(text, "Name: add") && contains(text, "ReturnType: int"), "parser: AST contains function name and return type");
        check(contains(text, "Name: a") && contains(text, "Name: b"), "parser: AST contains parameters");
    }

    {
        ParseResult r("struct Point { int x; int y; } fn main() -> void { return; }");
        check(r.lexer.get_errors().empty(), "parser: struct sample has no lexical errors", joinErrors(r.lexer.get_errors()));
        check(r.parser.get_errors().empty(), "parser: struct sample has no parse errors", joinErrors(r.parser.get_errors()));
        std::string text = astText(r.program);
        check(contains(text, "StructDecl") && contains(text, "Name: Point"), "parser: AST contains struct declaration");
        check(contains(text, "Name: x") && contains(text, "Name: y"), "parser: AST contains struct fields");
    }

    {
        ParseResult r("int global = 10; fn main() -> void { return; }");
        check(r.lexer.get_errors().empty(), "parser: global variable sample has no lexical errors", joinErrors(r.lexer.get_errors()));
        check(r.parser.get_errors().empty(), "parser: global variable sample has no parse errors", joinErrors(r.parser.get_errors()));
        check(contains(astText(r.program), "GlobalVarDecl"), "parser: AST contains global variable declaration");
    }

    {
        ParseResult r("fn main() -> void { int x = 0; while (x < 3) { x += 1; } for (int i = 0; i < 2; i += 1) { if (i == 1) return; else x = x + i; } }");
        check(r.lexer.get_errors().empty(), "parser: control-flow sample has no lexical errors", joinErrors(r.lexer.get_errors()));
        check(r.parser.get_errors().empty(), "parser: control-flow sample has no parse errors", joinErrors(r.parser.get_errors()));
        std::string text = astText(r.program);
        check(contains(text, "WhileStmt") && contains(text, "ForStmt") && contains(text, "IfStmt"), "parser: AST contains while, for, and if nodes");
        check(contains(text, "ReturnStmt"), "parser: AST contains return node");
    }

    {
        ParseResult r("fn main() -> void { foo(); foo(1, 2, 3); ; }");
        check(r.lexer.get_errors().empty(), "parser: function call sample has no lexical errors", joinErrors(r.lexer.get_errors()));
        check(r.parser.get_errors().empty(), "parser: function call sample has no parse errors", joinErrors(r.parser.get_errors()));
        std::string text = astText(r.program);
        check(contains(text, "CallExpr") && contains(text, "Callee: foo"), "parser: AST contains function calls");
        check(contains(text, "EmptyStmt"), "parser: AST contains empty statement");
    }

    {
        ParseResult r("fn main() -> void { int x = 0; int y = 0; x = y = 1; }");
        check(r.lexer.get_errors().empty(), "parser: right-associative assignment sample has no lexical errors", joinErrors(r.lexer.get_errors()));
        check(r.parser.get_errors().empty(), "parser: right-associative assignment sample has no parse errors", joinErrors(r.parser.get_errors()));
        check(countType(r.lexer.all_tokens(), TokenType::ASSIGN) == 4, "parser: assignment sample tokenizes all assignment operators");
    }

    {
        ParseResult r("fn main() -> void { bool x = !false || true && false; }");
        check(r.lexer.get_errors().empty(), "parser: logical expression sample has no lexical errors", joinErrors(r.lexer.get_errors()));
        check(r.parser.get_errors().empty(), "parser: logical expression sample has no parse errors", joinErrors(r.parser.get_errors()));
        std::string text = astText(r.program);
        check(contains(text, "Operator: ||") && contains(text, "Operator: &&") && contains(text, "UnaryExpr"), "parser: AST contains logical and unary operators");
    }

    {
        ParseResult r("fn main() -> void { if (true) if (false) return; else return; }");
        check(r.lexer.get_errors().empty(), "parser: dangling else sample has no lexical errors", joinErrors(r.lexer.get_errors()));
        check(r.parser.get_errors().empty(), "parser: dangling else sample has no parse errors", joinErrors(r.parser.get_errors()));
        std::string text = astText(r.program);
        check(contains(text, "IfStmt") && contains(text, "Else:"), "parser: AST represents if/else nesting");
    }

    {
        ParseResult r("fn main() -> void { int x = 1 }");
        check(r.lexer.get_errors().empty(), "parser: missing semicolon sample has no lexical errors", joinErrors(r.lexer.get_errors()));
        check(!r.parser.get_errors().empty(), "parser: rejects missing semicolon");
    }

    {
        ParseResult r("fn main( -> void { return; }");
        check(r.lexer.get_errors().empty(), "parser: malformed parameter list sample has no lexical errors", joinErrors(r.lexer.get_errors()));
        check(!r.parser.get_errors().empty(), "parser: rejects malformed parameter list");
    }

    {
        ParseResult r("fn main() -> void { bool x = 1 < 2 < 3; }");
        check(r.lexer.get_errors().empty(), "parser: chained comparison sample has no lexical errors", joinErrors(r.lexer.get_errors()));
        check(!r.parser.get_errors().empty(), "parser: rejects chained comparison operators");
    }

    {
        ParseResult r("fn main() -> void { bool x = 1 == 1 == 1; }");
        check(r.lexer.get_errors().empty(), "parser: chained equality sample has no lexical errors", joinErrors(r.lexer.get_errors()));
        check(!r.parser.get_errors().empty(), "parser: rejects chained equality operators");
    }

    {
        ParseResult r("fn main() -> void { 1 = x; }");
        check(r.lexer.get_errors().empty(), "parser: invalid assignment target sample has no lexical errors", joinErrors(r.lexer.get_errors()));
        check(!r.parser.get_errors().empty(), "parser: rejects literal assignment target");
    }

    {
        ParseResult r("fn main() -> void { foo() = 2; }");
        check(r.lexer.get_errors().empty(), "parser: call assignment target sample has no lexical errors", joinErrors(r.lexer.get_errors()));
        check(!r.parser.get_errors().empty(), "parser: rejects call assignment target");
    }

    {
        ParseResult r("fn main() -> void { int x = 1; return x; }");
        check(r.lexer.get_errors().empty(), "ast: DOT sample has no lexical errors", joinErrors(r.lexer.get_errors()));
        check(r.parser.get_errors().empty(), "ast: DOT sample has no parse errors", joinErrors(r.parser.get_errors()));
        std::string dot = astDot(r.program);
        check(contains(dot, "digraph AST") && contains(dot, "FunctionDecl"), "ast: DOT output contains graph and function nodes");
    }
}
}

int main() {
    std::cout << "MiniCompiler extended core tests\n";
    std::cout << "================================\n";
    lexerTests();
    parserTests();
    std::cout << "\nPassed: " << passed << "\n";
    std::cout << "Failed: " << failed << "\n";
    return failed == 0 ? 0 : 1;
}

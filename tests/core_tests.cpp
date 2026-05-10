#include <cstdlib>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>

#include "lexer/lexer.h"
#include "parser/parser.h"

namespace {
int passed = 0;
int failed = 0;

void expect(bool condition, const std::string& name, const std::string& details = "") {
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

std::string joinErrors(const std::vector<std::string>& errors) {
    std::ostringstream out;
    for (const auto& e : errors) out << e << "\n";
    return out.str();
}

std::string parseToText(const std::string& source, std::vector<std::string>* lexErrors, std::vector<std::string>* parseErrors) {
    Lexer lexer(source);
    if (lexErrors) *lexErrors = lexer.get_errors();
    Parser parser(lexer.all_tokens());
    ProgramNode program = parser.parse();
    if (parseErrors) *parseErrors = parser.get_errors();
    std::ostringstream out;
    printAstText(program, out);
    return out.str();
}

void testLexerValidBasics() {
    Lexer lexer("fn main() -> int { int x = 42; bool ok = true; return x; }");
    expect(lexer.get_errors().empty(), "lexer accepts a small valid function", joinErrors(lexer.get_errors()));

    const auto& tokens = lexer.all_tokens();
    expect(tokens.size() >= 20, "lexer emits a non-empty token stream");
    expect(tokens.front().type == TokenType::KW_FN, "first token is KW_FN");
    expect(tokens.back().type == TokenType::END_OF_FILE, "last token is END_OF_FILE");
}

void testLexerOperators() {
    Lexer lexer("+ - * / % == != < <= > >= && || ! = += -= *= /= -> ( ) { } [ ] ; , :");
    expect(lexer.get_errors().empty(), "lexer accepts all required operators and delimiters", joinErrors(lexer.get_errors()));
    expect(lexer.all_tokens().size() == 30, "lexer emits expected operator/delimiter count including EOF");
}

void testLexerClosedCommentAtEof() {
    Lexer lexer("/* closed at eof */");
    expect(lexer.get_errors().empty(), "lexer accepts a closed multiline comment at EOF", joinErrors(lexer.get_errors()));
}

void testLexerIdentifierStartRule() {
    Lexer lexer("_bad");
    expect(!lexer.get_errors().empty(), "lexer rejects identifiers that start with underscore");
}

void testLexerMalformedFloat() {
    Lexer lexer("float x = 1.;");
    std::string errors = joinErrors(lexer.get_errors());
    expect(!lexer.get_errors().empty() && contains(errors, "Malformed"), "lexer rejects float literal without digits after dot", errors);
}

void testLexerInvalidCharacterColumn() {
    Lexer lexer("int x = @42;");
    std::string errors = joinErrors(lexer.get_errors());
    expect(contains(errors, "Unexpected character '@'"), "lexer reports invalid character");
    expect(contains(errors, "1:9"), "lexer reports invalid character at the starting column", errors);
}

void testParserValidPrecedence() {
    std::vector<std::string> lexErrors;
    std::vector<std::string> parseErrors;
    std::string ast = parseToText("fn main() { int x = 1 + 2 * 3; return x; }", &lexErrors, &parseErrors);
    expect(lexErrors.empty(), "parser precedence sample has no lex errors", joinErrors(lexErrors));
    expect(parseErrors.empty(), "parser precedence sample has no parse errors", joinErrors(parseErrors));
    expect(contains(ast, "Operator: +") && contains(ast, "Operator: *"), "AST contains additive and multiplicative operators");
}

void testParserValidControlFlow() {
    const std::string source =
        "fn main() {"
        "  for (int i = 0; i < 3; i = i + 1) {"
        "    if (i == 2) return;"
        "  }"
        "}";
    std::vector<std::string> lexErrors;
    std::vector<std::string> parseErrors;
    std::string ast = parseToText(source, &lexErrors, &parseErrors);
    expect(lexErrors.empty(), "parser control-flow sample has no lex errors", joinErrors(lexErrors));
    expect(parseErrors.empty(), "parser control-flow sample has no parse errors", joinErrors(parseErrors));
    expect(contains(ast, "ForStmt") && contains(ast, "IfStmt") && contains(ast, "ReturnStmt"), "AST contains for/if/return nodes");
}

void testParserRejectsChainedComparison() {
    std::vector<std::string> lexErrors;
    std::vector<std::string> parseErrors;
    parseToText("fn main() { bool x = 1 < 2 < 3; }", &lexErrors, &parseErrors);
    expect(!parseErrors.empty(), "parser rejects chained comparison operators");
}

void testParserRejectsInvalidAssignmentTarget() {
    std::vector<std::string> lexErrors;
    std::vector<std::string> parseErrors;
    parseToText("fn main() { 1 = x; foo() = 2; }", &lexErrors, &parseErrors);
    std::string errors = joinErrors(parseErrors);
    expect(!parseErrors.empty() && contains(errors, "assignment"), "parser rejects non-identifier assignment targets", errors);
}
}

int main() {
    testLexerValidBasics();
    testLexerOperators();
    testLexerClosedCommentAtEof();
    testLexerIdentifierStartRule();
    testLexerMalformedFloat();
    testLexerInvalidCharacterColumn();
    testParserValidPrecedence();
    testParserValidControlFlow();
    testParserRejectsChainedComparison();
    testParserRejectsInvalidAssignmentTarget();

    std::cout << "\nPassed: " << passed << "\nFailed: " << failed << "\n";
    return failed == 0 ? EXIT_SUCCESS : EXIT_FAILURE;
}

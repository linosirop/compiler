#pragma once

#include "ast.h"
#include "lexer/token.h"

#include <string>
#include <vector>

class Parser {
public:
    explicit Parser(const std::vector<Token>& tokens);

    ProgramNode parse();
    const std::vector<std::string>& get_errors() const { return errors_; }

private:
    std::vector<Token> tokens_;
    size_t current_ = 0;
    std::vector<std::string> errors_;

    bool isAtEnd() const;
    const Token& peek() const;
    const Token& previous() const;
    const Token& advance();
    bool check(TokenType type) const;
    bool match(TokenType type);
    bool matchAny(std::initializer_list<TokenType> types);
    const Token& consume(TokenType type, const std::string& message);
    void errorAtCurrent(const std::string& message);
    void synchronize();

    DeclPtr parseDeclaration();
    DeclPtr parseFunctionDeclaration();
    DeclPtr parseStructDeclaration();
    std::shared_ptr<VarDeclStmtNode> parseVariableDeclarationAfterType(const std::string& type, const Token& typeToken);

    StmtPtr parseStatement();
    std::shared_ptr<BlockStmtNode> parseBlock();
    StmtPtr parseIfStatement();
    StmtPtr parseWhileStatement();
    StmtPtr parseForStatement();
    StmtPtr parseReturnStatement();
    StmtPtr parseExpressionStatement();

    ExprPtr parseExpression();
    ExprPtr parseAssignment();
    ExprPtr parseLogicalOr();
    ExprPtr parseLogicalAnd();
    ExprPtr parseEquality();
    ExprPtr parseComparison();
    ExprPtr parseTerm();
    ExprPtr parseFactor();
    ExprPtr parseUnary();
    ExprPtr parsePrimary();

    bool isTypeToken(TokenType type) const;
    std::string parseTypeName();
    ParamNode parseParameter();
};

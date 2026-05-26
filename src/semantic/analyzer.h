#pragma once

#include "errors.h"
#include "symbol_table.h"
#include "parser/ast.h"

#include <ostream>
#include <string>
#include <unordered_map>
#include <vector>

namespace semantic {

class SemanticAnalyzer {
public:
    SemanticAnalyzer() = default;

    void analyze(const ProgramNode& ast);
    const std::vector<SemanticError>& get_errors() const { return errors_; }
    const SymbolTable& get_symbol_table() const { return symbols_; }
    const std::unordered_map<const ExpressionNode*, Type>& get_type_annotations() const { return expressionTypes_; }

    bool has_errors() const { return !errors_.empty(); }
    std::string report(const std::string& fileName = "program.src", bool showTypes = true) const;

private:
    SymbolTable symbols_;
    std::vector<SemanticError> errors_;
    std::unordered_map<const ExpressionNode*, Type> expressionTypes_;
    Type currentReturnType_ = Type::voidType();
    std::string currentFunction_;

    void predeclareStruct(const StructDeclNode& node);
    void predeclareFunction(const FunctionDeclNode& node);
    void analyzeDeclaration(const DeclPtr& decl);
    void analyzeFunction(const FunctionDeclNode& node);
    void analyzeStruct(const StructDeclNode& node);
    void analyzeGlobalVariable(const GlobalVarDeclNode& node);

    void analyzeStatement(const StmtPtr& stmt);
    void analyzeBlock(const BlockStmtNode& node, bool createScope);
    void analyzeVariableDeclaration(const VarDeclStmtNode& node, SymbolKind kind = SymbolKind::Variable);
    void analyzeIf(const IfStmtNode& node);
    void analyzeWhile(const WhileStmtNode& node);
    void analyzeFor(const ForStmtNode& node);
    void analyzeReturn(const ReturnStmtNode& node);

    Type analyzeExpression(const ExprPtr& expr);
    Type analyzeLiteral(const LiteralExprNode& node);
    Type analyzeIdentifier(const IdentifierExprNode& node);
    Type analyzeBinary(const BinaryExprNode& node);
    Type analyzeUnary(const UnaryExprNode& node);
    Type analyzeAssignment(const AssignmentExprNode& node);
    Type analyzeArrayAccess(const ArrayAccessExprNode& node);
    Type analyzeCall(const CallExprNode& node);

    Type resolveTypeName(const std::string& name, int line, int column);
    void addError(const std::string& category, const std::string& message, int line, int column);
    void annotate(const ExpressionNode& node, const Type& type);
};

void printSemanticReport(const SemanticAnalyzer& analyzer, std::ostream& out, const std::string& fileName = "program.src", bool showTypes = true);

} // namespace semantic

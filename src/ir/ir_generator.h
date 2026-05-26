#pragma once

#include "basic_block.h"
#include "parser/ast.h"
#include "semantic/symbol_table.h"

#include <string>
#include <unordered_map>

namespace ir {

class IRGenerator {
public:
    IRGenerator() = default;
    explicit IRGenerator(const semantic::SymbolTable* symbolTable);

    IRProgram generate(const ProgramNode& ast);
    const IRProgram& get_all_ir() const { return program_; }
    const IRFunction* get_function_ir(const std::string& name) const { return program_.getFunction(name); }

private:
    const semantic::SymbolTable* symbolTable_ = nullptr;
    IRProgram program_;
    IRFunction* currentFunction_ = nullptr;

    void generateDeclaration(const DeclPtr& decl);
    void generateFunction(const FunctionDeclNode& node);
    void generateGlobalVariable(const GlobalVarDeclNode& node);

    void generateStatement(const StmtPtr& stmt);
    void generateBlock(const BlockStmtNode& node);
    void generateVariableDeclaration(const VarDeclStmtNode& node);
    void generateIf(const IfStmtNode& node);
    void generateWhile(const WhileStmtNode& node);
    void generateFor(const ForStmtNode& node);
    void generateReturn(const ReturnStmtNode& node);

    std::string generateExpression(const ExprPtr& expr);
    std::string generateLiteral(const LiteralExprNode& node);
    std::string generateIdentifier(const IdentifierExprNode& node);
    std::string generateArrayAccess(const ArrayAccessExprNode& node);
    std::string generateArrayElementLocation(const ArrayAccessExprNode& node);
    std::string generateBinary(const BinaryExprNode& node);
    std::string generateShortCircuitLogical(const BinaryExprNode& node);
    std::string generateUnary(const UnaryExprNode& node);
    std::string generateAssignment(const AssignmentExprNode& node);
    std::string generateCall(const CallExprNode& node);

    std::string locationForVariable(const std::string& name);
    void emit(Opcode opcode, const std::string& dest, const std::vector<std::string>& operands = {}, const std::string& comment = "");
    void emitJump(const std::string& target);
    void ensureCurrentBlockTerminates();
    semantic::Type parseType(const std::string& name) const;
    Opcode opcodeForBinary(const std::string& op) const;
};

std::string generateIRText(const ProgramNode& ast, const semantic::SymbolTable* symbolTable = nullptr);
std::string generateIRDot(const ProgramNode& ast, const semantic::SymbolTable* symbolTable = nullptr);

} // namespace ir

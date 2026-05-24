#include "ir_generator.h"

#include <memory>
#include <sstream>
#include <stdexcept>

namespace ir {

IRGenerator::IRGenerator(const semantic::SymbolTable* symbolTable) : symbolTable_(symbolTable) {}

IRProgram IRGenerator::generate(const ProgramNode& ast) {
    program_ = IRProgram{};
    currentFunction_ = nullptr;
    for (const auto& decl : ast.declarations) generateDeclaration(decl);
    return program_;
}

void IRGenerator::generateDeclaration(const DeclPtr& decl) {
    if (auto fn = std::dynamic_pointer_cast<FunctionDeclNode>(decl)) {
        generateFunction(*fn);
    } else if (auto global = std::dynamic_pointer_cast<GlobalVarDeclNode>(decl)) {
        generateGlobalVariable(*global);
    }
}

void IRGenerator::generateGlobalVariable(const GlobalVarDeclNode& node) {
    if (!node.declaration) return;
    if (!program_.getFunction("__global")) {
        IRFunction global("__global");
        global.returnType = semantic::Type::voidType();
        program_.functions.push_back(global);
    }
    IRFunction* previous = currentFunction_;
    currentFunction_ = program_.getFunction("__global");
    generateVariableDeclaration(*node.declaration);
    ensureCurrentBlockTerminates();
    currentFunction_ = previous;
}

semantic::Type IRGenerator::parseType(const std::string& name) const {
    if (name == "int") return semantic::Type::intType();
    if (name == "float") return semantic::Type::floatType();
    if (name == "bool") return semantic::Type::boolType();
    if (name == "void") return semantic::Type::voidType();
    if (name == "string") return semantic::Type::stringType();
    return semantic::Type::structType(name);
}

void IRGenerator::generateFunction(const FunctionDeclNode& node) {
    IRFunction function(node.name);
    function.returnType = parseType(node.returnType);
    for (const auto& param : node.parameters) {
        auto type = parseType(param.type);
        function.parameters.push_back({param.name, type});
        function.variableLocations[param.name] = param.name;
    }

    program_.functions.push_back(function);
    currentFunction_ = &program_.functions.back();

    for (const auto& param : node.parameters) {
        emit(Opcode::MOVE, locationForVariable(param.name), {param.name}, "parameter " + param.name);
    }

    if (node.body) generateBlock(*node.body);
    ensureCurrentBlockTerminates();
    currentFunction_ = nullptr;
}

void IRGenerator::generateStatement(const StmtPtr& stmt) {
    if (!stmt || !currentFunction_) return;
    if (auto block = std::dynamic_pointer_cast<BlockStmtNode>(stmt)) generateBlock(*block);
    else if (auto var = std::dynamic_pointer_cast<VarDeclStmtNode>(stmt)) generateVariableDeclaration(*var);
    else if (auto expr = std::dynamic_pointer_cast<ExprStmtNode>(stmt)) generateExpression(expr->expression);
    else if (std::dynamic_pointer_cast<EmptyStmtNode>(stmt)) emit(Opcode::NOP, "", {}, "empty statement");
    else if (auto ifs = std::dynamic_pointer_cast<IfStmtNode>(stmt)) generateIf(*ifs);
    else if (auto wh = std::dynamic_pointer_cast<WhileStmtNode>(stmt)) generateWhile(*wh);
    else if (auto fs = std::dynamic_pointer_cast<ForStmtNode>(stmt)) generateFor(*fs);
    else if (auto ret = std::dynamic_pointer_cast<ReturnStmtNode>(stmt)) generateReturn(*ret);
}

void IRGenerator::generateBlock(const BlockStmtNode& node) {
    for (const auto& stmt : node.statements) {
        if (currentFunction_->currentBlock().endsWithTerminator()) break;
        generateStatement(stmt);
    }
}

std::string IRGenerator::locationForVariable(const std::string& name) {
    return "[" + currentFunction_->localName(name) + "]";
}

void IRGenerator::generateVariableDeclaration(const VarDeclStmtNode& node) {
    std::string location = locationForVariable(node.name);
    emit(Opcode::ALLOCA, location.substr(1, location.size() - 2), {node.type}, "declare " + node.type + " " + node.name);
    if (node.initializer) {
        std::string value = generateExpression(node.initializer);
        emit(Opcode::STORE, location, {value}, node.name + " = initializer");
    }
}

void IRGenerator::generateIf(const IfStmtNode& node) {
    std::string condition = generateExpression(node.condition);
    std::string thenLabel = currentFunction_->newLabel("L_then");
    std::string elseLabel = currentFunction_->newLabel("L_else");
    std::string endLabel = currentFunction_->newLabel("L_endif");

    emit(Opcode::JUMP_IF, "", {condition, thenLabel});
    currentFunction_->currentBlock().addSuccessor(thenLabel);
    emit(Opcode::JUMP, "", {node.elseBranch ? elseLabel : endLabel});
    currentFunction_->currentBlock().addSuccessor(node.elseBranch ? elseLabel : endLabel);

    currentFunction_->createBlock(thenLabel);
    generateStatement(node.thenBranch);
    if (!currentFunction_->currentBlock().endsWithTerminator()) {
        emitJump(endLabel);
    }

    if (node.elseBranch) {
        currentFunction_->createBlock(elseLabel);
        generateStatement(node.elseBranch);
        if (!currentFunction_->currentBlock().endsWithTerminator()) {
            emitJump(endLabel);
        }
    }

    currentFunction_->createBlock(endLabel);
}

void IRGenerator::generateWhile(const WhileStmtNode& node) {
    std::string headerLabel = currentFunction_->newLabel("L_while_header");
    std::string bodyLabel = currentFunction_->newLabel("L_while_body");
    std::string endLabel = currentFunction_->newLabel("L_while_end");

    emitJump(headerLabel);
    currentFunction_->createBlock(headerLabel);
    std::string condition = generateExpression(node.condition);
    emit(Opcode::JUMP_IF, "", {condition, bodyLabel});
    currentFunction_->currentBlock().addSuccessor(bodyLabel);
    emit(Opcode::JUMP, "", {endLabel});
    currentFunction_->currentBlock().addSuccessor(endLabel);

    currentFunction_->createBlock(bodyLabel);
    generateStatement(node.body);
    if (!currentFunction_->currentBlock().endsWithTerminator()) emitJump(headerLabel);

    currentFunction_->createBlock(endLabel);
}

void IRGenerator::generateFor(const ForStmtNode& node) {
    if (node.init) generateStatement(node.init);

    std::string headerLabel = currentFunction_->newLabel("L_for_header");
    std::string bodyLabel = currentFunction_->newLabel("L_for_body");
    std::string endLabel = currentFunction_->newLabel("L_for_end");

    emitJump(headerLabel);
    currentFunction_->createBlock(headerLabel);
    if (node.condition) {
        std::string condition = generateExpression(node.condition);
        emit(Opcode::JUMP_IF, "", {condition, bodyLabel});
        currentFunction_->currentBlock().addSuccessor(bodyLabel);
        emit(Opcode::JUMP, "", {endLabel});
        currentFunction_->currentBlock().addSuccessor(endLabel);
    } else {
        emitJump(bodyLabel);
    }

    currentFunction_->createBlock(bodyLabel);
    generateStatement(node.body);
    if (node.update && !currentFunction_->currentBlock().endsWithTerminator()) generateExpression(node.update);
    if (!currentFunction_->currentBlock().endsWithTerminator()) emitJump(headerLabel);

    currentFunction_->createBlock(endLabel);
}

void IRGenerator::generateReturn(const ReturnStmtNode& node) {
    if (node.value) {
        std::string value = generateExpression(node.value);
        emit(Opcode::RETURN, "", {value});
    } else {
        emit(Opcode::RETURN, "", {});
    }
}

std::string IRGenerator::generateExpression(const ExprPtr& expr) {
    if (!expr) return "";
    if (auto literal = std::dynamic_pointer_cast<LiteralExprNode>(expr)) return generateLiteral(*literal);
    if (auto id = std::dynamic_pointer_cast<IdentifierExprNode>(expr)) return generateIdentifier(*id);
    if (auto binary = std::dynamic_pointer_cast<BinaryExprNode>(expr)) return generateBinary(*binary);
    if (auto unary = std::dynamic_pointer_cast<UnaryExprNode>(expr)) return generateUnary(*unary);
    if (auto assignment = std::dynamic_pointer_cast<AssignmentExprNode>(expr)) return generateAssignment(*assignment);
    if (auto call = std::dynamic_pointer_cast<CallExprNode>(expr)) return generateCall(*call);
    return "<expr>";
}

std::string IRGenerator::generateLiteral(const LiteralExprNode& node) { return node.value; }

std::string IRGenerator::generateIdentifier(const IdentifierExprNode& node) {
    std::string temp = currentFunction_->newTemp();
    emit(Opcode::LOAD, temp, {locationForVariable(node.name)}, "load " + node.name);
    return temp;
}

Opcode IRGenerator::opcodeForBinary(const std::string& op) const {
    if (op == "+") return Opcode::ADD;
    if (op == "-") return Opcode::SUB;
    if (op == "*") return Opcode::MUL;
    if (op == "/") return Opcode::DIV;
    if (op == "%") return Opcode::MOD;
    if (op == "&&") return Opcode::AND;
    if (op == "||") return Opcode::OR;
    if (op == "==") return Opcode::CMP_EQ;
    if (op == "!=") return Opcode::CMP_NE;
    if (op == "<") return Opcode::CMP_LT;
    if (op == "<=") return Opcode::CMP_LE;
    if (op == ">") return Opcode::CMP_GT;
    if (op == ">=") return Opcode::CMP_GE;
    return Opcode::NOP;
}

std::string IRGenerator::generateBinary(const BinaryExprNode& node) {
    if (node.op == "&&" || node.op == "||") {
        return generateShortCircuitLogical(node);
    }

    std::string left = generateExpression(node.left);
    std::string right = generateExpression(node.right);
    std::string temp = currentFunction_->newTemp();
    emit(opcodeForBinary(node.op), temp, {left, right}, left + " " + node.op + " " + right);
    return temp;
}

std::string IRGenerator::generateShortCircuitLogical(const BinaryExprNode& node) {
    const bool isAnd = node.op == "&&";
    const std::string result = currentFunction_->newTemp();
    const std::string rightLabel = currentFunction_->newLabel(isAnd ? "L_and_rhs" : "L_or_rhs");
    const std::string trueLabel = currentFunction_->newLabel("L_logic_true");
    const std::string falseLabel = currentFunction_->newLabel("L_logic_false");
    const std::string endLabel = currentFunction_->newLabel("L_logic_end");

    std::string left = generateExpression(node.left);
    if (isAnd) {
        emit(Opcode::JUMP_IF_NOT, "", {left, falseLabel}, "short-circuit &&: left operand is false");
        currentFunction_->currentBlock().addSuccessor(falseLabel);
        emitJump(rightLabel);

        currentFunction_->createBlock(rightLabel);
        std::string right = generateExpression(node.right);
        emit(Opcode::JUMP_IF, "", {right, trueLabel}, "&& right operand is true");
        currentFunction_->currentBlock().addSuccessor(trueLabel);
        emitJump(falseLabel);
    } else {
        emit(Opcode::JUMP_IF, "", {left, trueLabel}, "short-circuit ||: left operand is true");
        currentFunction_->currentBlock().addSuccessor(trueLabel);
        emitJump(rightLabel);

        currentFunction_->createBlock(rightLabel);
        std::string right = generateExpression(node.right);
        emit(Opcode::JUMP_IF, "", {right, trueLabel}, "|| right operand is true");
        currentFunction_->currentBlock().addSuccessor(trueLabel);
        emitJump(falseLabel);
    }

    currentFunction_->createBlock(trueLabel);
    emit(Opcode::MOVE, result, {"true"}, node.op + " result = true");
    emitJump(endLabel);

    currentFunction_->createBlock(falseLabel);
    emit(Opcode::MOVE, result, {"false"}, node.op + " result = false");
    emitJump(endLabel);

    currentFunction_->createBlock(endLabel);
    return result;
}

std::string IRGenerator::generateUnary(const UnaryExprNode& node) {
    std::string operand = generateExpression(node.operand);
    std::string temp = currentFunction_->newTemp();
    emit(node.op == "!" ? Opcode::NOT : Opcode::NEG, temp, {operand}, node.op + operand);
    return temp;
}

std::string IRGenerator::generateAssignment(const AssignmentExprNode& node) {
    auto id = std::dynamic_pointer_cast<IdentifierExprNode>(node.target);
    if (!id) return generateExpression(node.value);

    std::string location = locationForVariable(id->name);
    if (node.op == "=") {
        std::string value = generateExpression(node.value);
        emit(Opcode::STORE, location, {value}, id->name + " = value");
        return value;
    }

    std::string oldValue = generateIdentifier(*id);
    std::string right = generateExpression(node.value);
    std::string result = currentFunction_->newTemp();
    std::string binaryOp = node.op.substr(0, 1);
    emit(opcodeForBinary(binaryOp), result, {oldValue, right}, id->name + " " + node.op + " value");
    emit(Opcode::STORE, location, {result}, id->name + " = result");
    return result;
}

std::string IRGenerator::generateCall(const CallExprNode& node) {
    std::vector<std::string> args;
    for (size_t i = 0; i < node.arguments.size(); ++i) {
        std::string value = generateExpression(node.arguments[i]);
        args.push_back(value);
        emit(Opcode::PARAM, std::to_string(i), {value}, "argument " + std::to_string(i));
    }
    std::string temp = currentFunction_->newTemp();
    std::vector<std::string> operands;
    operands.push_back(node.callee);
    for (const auto& arg : args) operands.push_back(arg);
    emit(Opcode::CALL, temp, operands, "call " + node.callee);
    return temp;
}

void IRGenerator::emit(Opcode opcode, const std::string& dest, const std::vector<std::string>& operands, const std::string& comment) {
    if (!currentFunction_) return;
    IRInstruction instruction(opcode, dest, operands, comment);
    currentFunction_->currentBlock().addInstruction(instruction);
}

void IRGenerator::emitJump(const std::string& target) {
    emit(Opcode::JUMP, "", {target});
    currentFunction_->currentBlock().addSuccessor(target);
}

void IRGenerator::ensureCurrentBlockTerminates() {
    if (!currentFunction_) return;
    if (currentFunction_->currentBlock().endsWithTerminator()) return;
    if (currentFunction_->returnType.isVoid()) {
        emit(Opcode::RETURN, "", {});
    } else {
        emit(Opcode::RETURN, "", {"0"});
    }
}

std::string generateIRText(const ProgramNode& ast, const semantic::SymbolTable* symbolTable) {
    IRGenerator generator(symbolTable);
    return generator.generate(ast).toText();
}

std::string generateIRDot(const ProgramNode& ast, const semantic::SymbolTable* symbolTable) {
    IRGenerator generator(symbolTable);
    return generator.generate(ast).toDot();
}

} // namespace ir

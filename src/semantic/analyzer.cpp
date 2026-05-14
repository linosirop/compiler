#include "analyzer.h"

#include <cctype>
#include <sstream>

namespace semantic {
namespace {

bool isIntegerLiteral(const std::string& text) {
    if (text.empty()) return false;
    for (char c : text) {
        if (!std::isdigit(static_cast<unsigned char>(c))) return false;
    }
    return true;
}

bool isFloatLiteral(const std::string& text) {
    bool dot = false;
    bool before = false;
    bool after = false;
    for (char c : text) {
        if (c == '.') {
            if (dot) return false;
            dot = true;
        } else if (std::isdigit(static_cast<unsigned char>(c))) {
            if (dot) after = true;
            else before = true;
        } else {
            return false;
        }
    }
    return dot && before && after;
}

std::string locationOf(const ASTNode& node) {
    return std::to_string(node.line) + ":" + std::to_string(node.column);
}

} // namespace

void SemanticAnalyzer::analyze(const ProgramNode& ast) {
    errors_.clear();
    expressionTypes_.clear();
    symbols_ = SymbolTable();

    for (const auto& decl : ast.declarations) {
        if (auto st = std::dynamic_pointer_cast<StructDeclNode>(decl)) {
            predeclareStruct(*st);
        }
    }
    for (const auto& decl : ast.declarations) {
        if (auto fn = std::dynamic_pointer_cast<FunctionDeclNode>(decl)) {
            predeclareFunction(*fn);
        }
    }
    for (const auto& decl : ast.declarations) {
        analyzeDeclaration(decl);
    }
}

void SemanticAnalyzer::predeclareStruct(const StructDeclNode& node) {
    SymbolInfo symbol;
    symbol.name = node.name;
    symbol.kind = SymbolKind::Struct;
    symbol.type = Type::structType(node.name);
    symbol.line = node.line;
    symbol.column = node.column;

    for (const auto& field : node.fields) {
        if (!field) continue;
        Type fieldType = resolveTypeName(field->type, field->line, field->column);
        if (symbol.fields.find(field->name) != symbol.fields.end()) {
            addError("duplicate declaration", "duplicate field '" + field->name + "' in struct '" + node.name + "'", field->line, field->column);
        } else {
            symbol.fields[field->name] = fieldType;
        }
    }

    if (!symbols_.insert(node.name, symbol)) {
        addError("duplicate declaration", "symbol '" + node.name + "' is already declared in this scope", node.line, node.column);
    }
}

void SemanticAnalyzer::predeclareFunction(const FunctionDeclNode& node) {
    SymbolInfo symbol;
    symbol.name = node.name;
    symbol.kind = SymbolKind::Function;
    symbol.line = node.line;
    symbol.column = node.column;
    symbol.returnType = resolveTypeName(node.returnType, node.line, node.column);

    for (const auto& param : node.parameters) {
        symbol.parameters.push_back(resolveTypeName(param.type, param.line, param.column));
    }
    symbol.type = Type::functionType(symbol.parameters, symbol.returnType);

    if (!symbols_.insert(node.name, symbol)) {
        addError("duplicate declaration", "symbol '" + node.name + "' is already declared in this scope", node.line, node.column);
    }
}

void SemanticAnalyzer::analyzeDeclaration(const DeclPtr& decl) {
    if (auto fn = std::dynamic_pointer_cast<FunctionDeclNode>(decl)) {
        analyzeFunction(*fn);
    } else if (auto st = std::dynamic_pointer_cast<StructDeclNode>(decl)) {
        analyzeStruct(*st);
    } else if (auto gv = std::dynamic_pointer_cast<GlobalVarDeclNode>(decl)) {
        analyzeGlobalVariable(*gv);
    }
}

void SemanticAnalyzer::analyzeFunction(const FunctionDeclNode& node) {
    currentFunction_ = node.name;
    currentReturnType_ = resolveTypeName(node.returnType, node.line, node.column);
    symbols_.enter_scope("function " + node.name);

    for (const auto& param : node.parameters) {
        Type paramType = resolveTypeName(param.type, param.line, param.column);
        SymbolInfo symbol;
        symbol.name = param.name;
        symbol.kind = SymbolKind::Parameter;
        symbol.type = paramType;
        symbol.line = param.line;
        symbol.column = param.column;
        symbol.initialized = true;
        if (!symbols_.insert(param.name, symbol)) {
            addError("duplicate declaration", "duplicate parameter '" + param.name + "'", param.line, param.column);
        }
    }

    if (node.body) analyzeBlock(*node.body, true);
    symbols_.exit_scope();
    currentFunction_.clear();
    currentReturnType_ = Type::voidType();
}

void SemanticAnalyzer::analyzeStruct(const StructDeclNode& node) {
    symbols_.enter_scope("struct " + node.name);
    for (const auto& field : node.fields) {
        if (!field) continue;
        analyzeVariableDeclaration(*field, SymbolKind::Field);
    }
    symbols_.exit_scope();
}

void SemanticAnalyzer::analyzeGlobalVariable(const GlobalVarDeclNode& node) {
    if (node.declaration) analyzeVariableDeclaration(*node.declaration, SymbolKind::Variable);
}

void SemanticAnalyzer::analyzeStatement(const StmtPtr& stmt) {
    if (!stmt) return;
    if (auto block = std::dynamic_pointer_cast<BlockStmtNode>(stmt)) analyzeBlock(*block, true);
    else if (auto var = std::dynamic_pointer_cast<VarDeclStmtNode>(stmt)) analyzeVariableDeclaration(*var);
    else if (auto expr = std::dynamic_pointer_cast<ExprStmtNode>(stmt)) analyzeExpression(expr->expression);
    else if (std::dynamic_pointer_cast<EmptyStmtNode>(stmt)) return;
    else if (auto ifs = std::dynamic_pointer_cast<IfStmtNode>(stmt)) analyzeIf(*ifs);
    else if (auto wh = std::dynamic_pointer_cast<WhileStmtNode>(stmt)) analyzeWhile(*wh);
    else if (auto fs = std::dynamic_pointer_cast<ForStmtNode>(stmt)) analyzeFor(*fs);
    else if (auto ret = std::dynamic_pointer_cast<ReturnStmtNode>(stmt)) analyzeReturn(*ret);
}

void SemanticAnalyzer::analyzeBlock(const BlockStmtNode& node, bool createScope) {
    if (createScope) symbols_.enter_scope("block");
    for (const auto& stmt : node.statements) analyzeStatement(stmt);
    if (createScope) symbols_.exit_scope();
}

void SemanticAnalyzer::analyzeVariableDeclaration(const VarDeclStmtNode& node, SymbolKind kind) {
    Type declaredType = resolveTypeName(node.type, node.line, node.column);
    if (declaredType.kind == TypeKind::Void) {
        addError("type mismatch", "variable '" + node.name + "' cannot have type void", node.line, node.column);
    }

    if (symbols_.lookup_local(node.name)) {
        addError("duplicate declaration", "symbol '" + node.name + "' is already declared in this scope", node.line, node.column);
        return;
    }

    SymbolInfo symbol;
    symbol.name = node.name;
    symbol.kind = kind;
    symbol.type = declaredType;
    symbol.line = node.line;
    symbol.column = node.column;
    symbol.initialized = false;
    symbols_.insert(node.name, symbol);

    if (node.initializer) {
        Type initType = analyzeExpression(node.initializer);
        if (!canAssign(declaredType, initType)) {
            addError("type mismatch", "cannot initialize '" + node.name + "' of type " + declaredType.toString() + " with " + initType.toString(), node.initializer->line, node.initializer->column);
        }
        if (SymbolInfo* inserted = symbols_.lookup(node.name)) inserted->initialized = true;
    }
}

void SemanticAnalyzer::analyzeIf(const IfStmtNode& node) {
    Type cond = analyzeExpression(node.condition);
    if (!cond.isErrorLike() && cond.kind != TypeKind::Bool) {
        addError("invalid condition type", "if condition must be bool, found " + cond.toString(), node.condition ? node.condition->line : node.line, node.condition ? node.condition->column : node.column);
    }
    analyzeStatement(node.thenBranch);
    analyzeStatement(node.elseBranch);
}

void SemanticAnalyzer::analyzeWhile(const WhileStmtNode& node) {
    Type cond = analyzeExpression(node.condition);
    if (!cond.isErrorLike() && cond.kind != TypeKind::Bool) {
        addError("invalid condition type", "while condition must be bool, found " + cond.toString(), node.condition ? node.condition->line : node.line, node.condition ? node.condition->column : node.column);
    }
    analyzeStatement(node.body);
}

void SemanticAnalyzer::analyzeFor(const ForStmtNode& node) {
    symbols_.enter_scope("for");
    analyzeStatement(node.init);
    if (node.condition) {
        Type cond = analyzeExpression(node.condition);
        if (!cond.isErrorLike() && cond.kind != TypeKind::Bool) {
            addError("invalid condition type", "for condition must be bool, found " + cond.toString(), node.condition->line, node.condition->column);
        }
    }
    analyzeExpression(node.update);
    analyzeStatement(node.body);
    symbols_.exit_scope();
}

void SemanticAnalyzer::analyzeReturn(const ReturnStmtNode& node) {
    Type actual = node.value ? analyzeExpression(node.value) : Type::voidType();
    if (!canAssign(currentReturnType_, actual)) {
        addError("invalid return type", "function '" + currentFunction_ + "' must return " + currentReturnType_.toString() + ", found " + actual.toString(), node.line, node.column);
    }
}

Type SemanticAnalyzer::analyzeExpression(const ExprPtr& expr) {
    if (!expr) return Type::voidType();
    if (auto literal = std::dynamic_pointer_cast<LiteralExprNode>(expr)) return analyzeLiteral(*literal);
    if (auto id = std::dynamic_pointer_cast<IdentifierExprNode>(expr)) return analyzeIdentifier(*id);
    if (auto binary = std::dynamic_pointer_cast<BinaryExprNode>(expr)) return analyzeBinary(*binary);
    if (auto unary = std::dynamic_pointer_cast<UnaryExprNode>(expr)) return analyzeUnary(*unary);
    if (auto assign = std::dynamic_pointer_cast<AssignmentExprNode>(expr)) return analyzeAssignment(*assign);
    if (auto call = std::dynamic_pointer_cast<CallExprNode>(expr)) return analyzeCall(*call);
    return Type::unknown();
}

Type SemanticAnalyzer::analyzeLiteral(const LiteralExprNode& node) {
    Type type = Type::unknown();
    if (node.value == "true" || node.value == "false") type = Type::boolType();
    else if (node.value.size() >= 2 && node.value.front() == '"' && node.value.back() == '"') type = Type::stringType();
    else if (isFloatLiteral(node.value)) type = Type::floatType();
    else if (isIntegerLiteral(node.value)) type = Type::intType();
    else type = Type::error();
    annotate(node, type);
    return type;
}

Type SemanticAnalyzer::analyzeIdentifier(const IdentifierExprNode& node) {
    const SymbolInfo* symbol = symbols_.lookup(node.name);
    if (!symbol) {
        addError("undeclared identifier", "identifier '" + node.name + "' is not declared", node.line, node.column);
        Type type = Type::error();
        annotate(node, type);
        return type;
    }
    if ((symbol->kind == SymbolKind::Variable || symbol->kind == SymbolKind::Parameter) && !symbol->initialized) {
        addError("use before declaration", "variable '" + node.name + "' may be used before initialization", node.line, node.column);
    }
    annotate(node, symbol->type);
    return symbol->type;
}

Type SemanticAnalyzer::analyzeBinary(const BinaryExprNode& node) {
    Type left = analyzeExpression(node.left);
    Type right = analyzeExpression(node.right);
    Type result = Type::error();

    if (node.op == "+" || node.op == "-" || node.op == "*" || node.op == "/" || node.op == "%") {
        result = arithmeticResult(left, right, node.op);
        if (result.kind == TypeKind::Error && !left.isErrorLike() && !right.isErrorLike()) {
            addError("type mismatch", "operator '" + node.op + "' cannot be applied to " + left.toString() + " and " + right.toString(), node.line, node.column);
        }
    } else if (node.op == "<" || node.op == "<=" || node.op == ">" || node.op == ">=" || node.op == "==" || node.op == "!=") {
        result = comparisonResult(left, right, node.op);
        if (result.kind == TypeKind::Error && !left.isErrorLike() && !right.isErrorLike()) {
            addError("type mismatch", "comparison '" + node.op + "' cannot compare " + left.toString() + " and " + right.toString(), node.line, node.column);
        }
    } else if (node.op == "&&" || node.op == "||") {
        result = logicalResult(left, right, node.op);
        if (result.kind == TypeKind::Error && !left.isErrorLike() && !right.isErrorLike()) {
            addError("type mismatch", "logical operator '" + node.op + "' requires bool operands", node.line, node.column);
        }
    }

    annotate(node, result);
    return result;
}

Type SemanticAnalyzer::analyzeUnary(const UnaryExprNode& node) {
    Type operand = analyzeExpression(node.operand);
    Type result = unaryResult(operand, node.op);
    if (result.kind == TypeKind::Error && !operand.isErrorLike()) {
        addError("type mismatch", "unary operator '" + node.op + "' cannot be applied to " + operand.toString(), node.line, node.column);
    }
    annotate(node, result);
    return result;
}

Type SemanticAnalyzer::analyzeAssignment(const AssignmentExprNode& node) {
    auto id = std::dynamic_pointer_cast<IdentifierExprNode>(node.target);
    if (!id) {
        addError("invalid assignment target", "left side of assignment must be an identifier", node.line, node.column);
        Type type = Type::error();
        annotate(node, type);
        return type;
    }

    SymbolInfo* symbol = symbols_.lookup(id->name);
    if (!symbol) {
        addError("undeclared identifier", "identifier '" + id->name + "' is not declared", id->line, id->column);
        Type type = Type::error();
        annotate(node, type);
        return type;
    }

    Type valueType = analyzeExpression(node.value);
    if (node.op == "=" && !canAssign(symbol->type, valueType)) {
        addError("type mismatch", "cannot assign " + valueType.toString() + " to '" + id->name + "' of type " + symbol->type.toString(), node.line, node.column);
    }
    if (node.op != "=") {
        Type result = arithmeticResult(symbol->type, valueType, node.op.substr(0, 1));
        if (result.kind == TypeKind::Error || !canAssign(symbol->type, result)) {
            addError("type mismatch", "compound assignment '" + node.op + "' is incompatible with " + symbol->type.toString() + " and " + valueType.toString(), node.line, node.column);
        }
    }

    symbol->initialized = true;
    annotate(node, symbol->type);
    return symbol->type;
}

Type SemanticAnalyzer::analyzeCall(const CallExprNode& node) {
    const SymbolInfo* symbol = symbols_.lookup(node.callee);
    if (!symbol) {
        addError("undeclared identifier", "function '" + node.callee + "' is not declared", node.line, node.column);
        Type type = Type::error();
        annotate(node, type);
        return type;
    }
    if (symbol->kind != SymbolKind::Function) {
        addError("type mismatch", "symbol '" + node.callee + "' is not callable", node.line, node.column);
        Type type = Type::error();
        annotate(node, type);
        return type;
    }

    if (node.arguments.size() != symbol->parameters.size()) {
        addError("argument count mismatch", "function '" + node.callee + "' expects " + std::to_string(symbol->parameters.size()) + " argument(s), found " + std::to_string(node.arguments.size()), node.line, node.column);
    }

    const size_t count = node.arguments.size() < symbol->parameters.size() ? node.arguments.size() : symbol->parameters.size();
    for (size_t i = 0; i < node.arguments.size(); ++i) {
        Type actual = analyzeExpression(node.arguments[i]);
        if (i < count && !canAssign(symbol->parameters[i], actual)) {
            addError("argument type mismatch", "argument " + std::to_string(i + 1) + " of '" + node.callee + "' expects " + symbol->parameters[i].toString() + ", found " + actual.toString(), node.arguments[i]->line, node.arguments[i]->column);
        }
    }

    annotate(node, symbol->returnType);
    return symbol->returnType;
}

Type SemanticAnalyzer::resolveTypeName(const std::string& name, int line, int column) {
    if (name == "int") return Type::intType();
    if (name == "float") return Type::floatType();
    if (name == "bool") return Type::boolType();
    if (name == "void") return Type::voidType();
    if (name == "string") return Type::stringType();

    const SymbolInfo* symbol = symbols_.lookup(name);
    if (symbol && symbol->kind == SymbolKind::Struct) return Type::structType(name);

    addError("undeclared identifier", "unknown type '" + name + "'", line, column);
    return Type::error();
}

void SemanticAnalyzer::addError(const std::string& category, const std::string& message, int line, int column) {
    SemanticError error;
    error.category = category;
    error.message = message;
    error.line = line;
    error.column = column;
    if (!currentFunction_.empty()) error.context = "in function '" + currentFunction_ + "'";
    errors_.push_back(error);
}

void SemanticAnalyzer::annotate(const ExpressionNode& node, const Type& type) {
    expressionTypes_[&node] = type;
}

std::string SemanticAnalyzer::report(const std::string& fileName, bool showTypes) const {
    std::ostringstream out;
    out << "Semantic Analysis Report\n";
    out << "Errors: " << errors_.size() << "\n\n";

    if (!errors_.empty()) {
        out << "Errors:\n";
        for (const auto& error : errors_) {
            out << error.toString(fileName) << "\n";
        }
        out << "\n";
    }

    out << symbols_.dump() << "\n";

    if (showTypes) {
        out << "Type Annotations:\n";
        if (expressionTypes_.empty()) {
            out << "  []\n";
        } else {
            for (const auto& entry : expressionTypes_) {
                out << "  - expression at " << locationOf(*entry.first) << ": " << entry.second.toString() << "\n";
            }
        }
    }

    return out.str();
}

void printSemanticReport(const SemanticAnalyzer& analyzer, std::ostream& out, const std::string& fileName, bool showTypes) {
    out << analyzer.report(fileName, showTypes);
}

} // namespace semantic

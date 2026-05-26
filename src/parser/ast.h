#pragma once

#include <memory>
#include <ostream>
#include <string>
#include <vector>

struct ASTNode {
    int line = 1;
    int column = 1;
    virtual ~ASTNode() = default;
};

struct ExpressionNode : ASTNode { virtual ~ExpressionNode() = default; };
struct StatementNode : ASTNode { virtual ~StatementNode() = default; };
struct DeclarationNode : ASTNode { virtual ~DeclarationNode() = default; };

using ExprPtr = std::shared_ptr<ExpressionNode>;
using StmtPtr = std::shared_ptr<StatementNode>;
using DeclPtr = std::shared_ptr<DeclarationNode>;

struct ParamNode : ASTNode {
    std::string type;
    std::string name;
};

struct ProgramNode : ASTNode {
    std::vector<DeclPtr> declarations;
};

struct LiteralExprNode : ExpressionNode {
    std::string value;
    explicit LiteralExprNode(const std::string& value) : value(value) {}
};

struct IdentifierExprNode : ExpressionNode {
    std::string name;
    explicit IdentifierExprNode(const std::string& name) : name(name) {}
};

struct BinaryExprNode : ExpressionNode {
    ExprPtr left;
    std::string op;
    ExprPtr right;
    BinaryExprNode(ExprPtr left, const std::string& op, ExprPtr right)
        : left(std::move(left)), op(op), right(std::move(right)) {}
};

struct UnaryExprNode : ExpressionNode {
    std::string op;
    ExprPtr operand;
    UnaryExprNode(const std::string& op, ExprPtr operand)
        : op(op), operand(std::move(operand)) {}
};

struct AssignmentExprNode : ExpressionNode {
    ExprPtr target;
    std::string op;
    ExprPtr value;
    AssignmentExprNode(ExprPtr target, const std::string& op, ExprPtr value)
        : target(std::move(target)), op(op), value(std::move(value)) {}
};

struct ArrayAccessExprNode : ExpressionNode {
    ExprPtr array;
    ExprPtr index;
    ArrayAccessExprNode(ExprPtr array, ExprPtr index)
        : array(std::move(array)), index(std::move(index)) {}
};

struct CallExprNode : ExpressionNode {
    std::string callee;
    std::vector<ExprPtr> arguments;
    CallExprNode(const std::string& callee, std::vector<ExprPtr> arguments)
        : callee(callee), arguments(std::move(arguments)) {}
};

struct BlockStmtNode : StatementNode {
    std::vector<StmtPtr> statements;
};

struct ExprStmtNode : StatementNode {
    ExprPtr expression;
    explicit ExprStmtNode(ExprPtr expression) : expression(std::move(expression)) {}
};

struct EmptyStmtNode : StatementNode {};

struct VarDeclStmtNode : StatementNode {
    std::string type;
    std::string name;
    ExprPtr initializer;
    std::vector<int> arrayDimensions;
    std::vector<ExprPtr> arrayInitializers;

    bool isArray() const { return !arrayDimensions.empty(); }

    VarDeclStmtNode(const std::string& type, const std::string& name, ExprPtr initializer)
        : type(type), name(name), initializer(std::move(initializer)) {}
};

struct IfStmtNode : StatementNode {
    ExprPtr condition;
    StmtPtr thenBranch;
    StmtPtr elseBranch;
};

struct WhileStmtNode : StatementNode {
    ExprPtr condition;
    StmtPtr body;
};

struct ForStmtNode : StatementNode {
    StmtPtr init;
    ExprPtr condition;
    ExprPtr update;
    StmtPtr body;
};

struct ReturnStmtNode : StatementNode {
    ExprPtr value;
};

struct FunctionDeclNode : DeclarationNode {
    std::string returnType = "void";
    std::string name;
    std::vector<ParamNode> parameters;
    std::shared_ptr<BlockStmtNode> body;
    bool isExtern = false;
    bool isVariadic = false;
};

struct StructDeclNode : DeclarationNode {
    std::string name;
    std::vector<std::shared_ptr<VarDeclStmtNode>> fields;
};

struct GlobalVarDeclNode : DeclarationNode {
    std::shared_ptr<VarDeclStmtNode> declaration;
};

void printAstText(const ProgramNode& program, std::ostream& out);
void printAstDot(const ProgramNode& program, std::ostream& out);

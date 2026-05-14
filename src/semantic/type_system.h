#pragma once

#include <string>
#include <memory>
#include <vector>

namespace semantic {

enum class TypeKind {
    Int,
    Float,
    Bool,
    Void,
    String,
    Struct,
    Function,
    Unknown,
    Error
};

struct Type {
    TypeKind kind = TypeKind::Unknown;
    std::string name;
    std::vector<Type> parameters;
    std::shared_ptr<Type> returnType;

    Type() = default;
    explicit Type(TypeKind kind);
    Type(TypeKind kind, std::string name);

    static Type intType();
    static Type floatType();
    static Type boolType();
    static Type voidType();
    static Type stringType();
    static Type structType(const std::string& name);
    static Type functionType(std::vector<Type> params, Type ret);
    static Type unknown();
    static Type error();

    bool isNumeric() const;
    bool isBoolean() const;
    bool isVoid() const;
    bool isErrorLike() const;
    std::string toString() const;
};

bool equivalent(const Type& left, const Type& right);
bool canAssign(const Type& target, const Type& value);
Type arithmeticResult(const Type& left, const Type& right, const std::string& op);
Type comparisonResult(const Type& left, const Type& right, const std::string& op);
Type logicalResult(const Type& left, const Type& right, const std::string& op);
Type unaryResult(const Type& operand, const std::string& op);

} // namespace semantic

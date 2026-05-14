#include "type_system.h"

#include <sstream>
#include <utility>

namespace semantic {

Type::Type(TypeKind kind) : kind(kind) {
    switch (kind) {
    case TypeKind::Int: name = "int"; break;
    case TypeKind::Float: name = "float"; break;
    case TypeKind::Bool: name = "bool"; break;
    case TypeKind::Void: name = "void"; break;
    case TypeKind::String: name = "string"; break;
    case TypeKind::Unknown: name = "unknown"; break;
    case TypeKind::Error: name = "<error>"; break;
    case TypeKind::Struct: name = "<struct>"; break;
    case TypeKind::Function: name = "<function>"; break;
    }
}

Type::Type(TypeKind kind, std::string name) : kind(kind), name(std::move(name)) {}

Type Type::intType() { return Type(TypeKind::Int); }
Type Type::floatType() { return Type(TypeKind::Float); }
Type Type::boolType() { return Type(TypeKind::Bool); }
Type Type::voidType() { return Type(TypeKind::Void); }
Type Type::stringType() { return Type(TypeKind::String); }
Type Type::structType(const std::string& name) { return Type(TypeKind::Struct, name); }
Type Type::unknown() { return Type(TypeKind::Unknown); }
Type Type::error() { return Type(TypeKind::Error); }

Type Type::functionType(std::vector<Type> params, Type ret) {
    Type type(TypeKind::Function);
    type.parameters = std::move(params);
    type.returnType = std::make_shared<Type>(std::move(ret));
    return type;
}

bool Type::isNumeric() const { return kind == TypeKind::Int || kind == TypeKind::Float; }
bool Type::isBoolean() const { return kind == TypeKind::Bool; }
bool Type::isVoid() const { return kind == TypeKind::Void; }
bool Type::isErrorLike() const { return kind == TypeKind::Error || kind == TypeKind::Unknown; }

std::string Type::toString() const {
    if (kind != TypeKind::Function) return name;
    std::ostringstream out;
    out << "function(";
    for (size_t i = 0; i < parameters.size(); ++i) {
        if (i > 0) out << ", ";
        out << parameters[i].toString();
    }
    out << ") -> " << (returnType ? returnType->toString() : "void");
    return out.str();
}

bool equivalent(const Type& left, const Type& right) {
    if (left.kind != right.kind) return false;
    if (left.kind == TypeKind::Struct) return left.name == right.name;
    if (left.kind == TypeKind::Function) {
        if (!left.returnType || !right.returnType) return false;
        if (!equivalent(*left.returnType, *right.returnType)) return false;
        if (left.parameters.size() != right.parameters.size()) return false;
        for (size_t i = 0; i < left.parameters.size(); ++i) {
            if (!equivalent(left.parameters[i], right.parameters[i])) return false;
        }
    }
    return true;
}

bool canAssign(const Type& target, const Type& value) {
    if (target.isErrorLike() || value.isErrorLike()) return true;
    if (equivalent(target, value)) return true;
    return target.kind == TypeKind::Float && value.kind == TypeKind::Int;
}

Type arithmeticResult(const Type& left, const Type& right, const std::string& op) {
    if (left.isErrorLike() || right.isErrorLike()) return Type::error();
    if (!left.isNumeric() || !right.isNumeric()) return Type::error();
    if (op == "%" && (left.kind != TypeKind::Int || right.kind != TypeKind::Int)) return Type::error();
    if (left.kind == TypeKind::Float || right.kind == TypeKind::Float) return Type::floatType();
    return Type::intType();
}

Type comparisonResult(const Type& left, const Type& right, const std::string&) {
    if (left.isErrorLike() || right.isErrorLike()) return Type::error();
    if (left.isNumeric() && right.isNumeric()) return Type::boolType();
    if (equivalent(left, right) && (left.kind == TypeKind::Bool || left.kind == TypeKind::String)) return Type::boolType();
    return Type::error();
}

Type logicalResult(const Type& left, const Type& right, const std::string&) {
    if (left.isErrorLike() || right.isErrorLike()) return Type::error();
    if (left.kind == TypeKind::Bool && right.kind == TypeKind::Bool) return Type::boolType();
    return Type::error();
}

Type unaryResult(const Type& operand, const std::string& op) {
    if (operand.isErrorLike()) return Type::error();
    if (op == "-" && operand.isNumeric()) return operand;
    if (op == "!" && operand.kind == TypeKind::Bool) return Type::boolType();
    return Type::error();
}

} // namespace semantic

#pragma once

#include "type_system.h"

#include <optional>
#include <ostream>
#include <string>
#include <unordered_map>
#include <vector>

namespace semantic {

enum class SymbolKind {
    Variable,
    Function,
    Parameter,
    Struct,
    Field
};

std::string symbolKindToString(SymbolKind kind);

struct SymbolInfo {
    std::string name;
    Type type;
    SymbolKind kind = SymbolKind::Variable;
    int line = 1;
    int column = 1;
    bool initialized = true;
    std::vector<Type> parameters;
    Type returnType = Type::voidType();
    std::unordered_map<std::string, Type> fields;
};

class SymbolTable {
public:
    SymbolTable();

    void enter_scope(const std::string& label = "block");
    void exit_scope();

    bool insert(const std::string& name, const SymbolInfo& symbol_info);
    const SymbolInfo* lookup(const std::string& name) const;
    SymbolInfo* lookup(const std::string& name);
    const SymbolInfo* lookup_local(const std::string& name) const;
    int depth() const;

    std::string dump() const;

    struct Scope {
        std::string label;
        std::unordered_map<std::string, SymbolInfo> symbols;
    };

private:
    std::vector<Scope> scopes_;
    std::vector<Scope> closed_scopes_;
};

} // namespace semantic

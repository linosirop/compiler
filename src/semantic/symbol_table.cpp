#include "symbol_table.h"

#include <algorithm>
#include <sstream>

namespace semantic {

std::string symbolKindToString(SymbolKind kind) {
    switch (kind) {
    case SymbolKind::Variable: return "variable";
    case SymbolKind::Function: return "function";
    case SymbolKind::Parameter: return "parameter";
    case SymbolKind::Struct: return "struct";
    case SymbolKind::Field: return "field";
    }
    return "symbol";
}

SymbolTable::SymbolTable() {
    enter_scope("global");
}

void SymbolTable::enter_scope(const std::string& label) {
    scopes_.push_back(Scope{label, {}});
}

void SymbolTable::exit_scope() {
    if (scopes_.size() > 1) {
        closed_scopes_.push_back(scopes_.back());
        scopes_.pop_back();
    }
}

bool SymbolTable::insert(const std::string& name, const SymbolInfo& symbol_info) {
    auto& current = scopes_.back().symbols;
    if (current.find(name) != current.end()) return false;
    current.emplace(name, symbol_info);
    return true;
}

const SymbolInfo* SymbolTable::lookup(const std::string& name) const {
    for (auto it = scopes_.rbegin(); it != scopes_.rend(); ++it) {
        auto found = it->symbols.find(name);
        if (found != it->symbols.end()) return &found->second;
    }
    return nullptr;
}

SymbolInfo* SymbolTable::lookup(const std::string& name) {
    for (auto it = scopes_.rbegin(); it != scopes_.rend(); ++it) {
        auto found = it->symbols.find(name);
        if (found != it->symbols.end()) return &found->second;
    }
    return nullptr;
}

const SymbolInfo* SymbolTable::lookup_local(const std::string& name) const {
    const auto& current = scopes_.back().symbols;
    auto found = current.find(name);
    if (found == current.end()) return nullptr;
    return &found->second;
}

int SymbolTable::depth() const {
    return static_cast<int>(scopes_.size()) - 1;
}

namespace {
void dumpScope(std::ostream& out, const SymbolTable::Scope& scope, size_t index) {
    out << "  Scope " << index << " (" << scope.label << "):\n";
    if (scope.symbols.empty()) {
        out << "    []\n";
        return;
    }

    std::vector<std::string> names;
    names.reserve(scope.symbols.size());
    for (const auto& entry : scope.symbols) names.push_back(entry.first);
    std::sort(names.begin(), names.end());

    for (const auto& name : names) {
        const auto& symbol = scope.symbols.at(name);
        out << "    - " << symbol.name << ": " << symbolKindToString(symbol.kind)
            << ", type=" << symbol.type.toString()
            << ", declared at " << symbol.line << ":" << symbol.column;
        if (symbol.kind == SymbolKind::Function) {
            out << ", signature=" << Type::functionType(symbol.parameters, symbol.returnType).toString();
        }
        if (symbol.kind == SymbolKind::Struct && !symbol.fields.empty()) {
            out << ", fields={";
            bool first = true;
            for (const auto& field : symbol.fields) {
                if (!first) out << ", ";
                out << field.first << ": " << field.second.toString();
                first = false;
            }
            out << "}";
        }
        out << "\n";
    }
}
}

std::string SymbolTable::dump() const {
    std::ostringstream out;
    out << "Symbol Table:\n";
    size_t index = 0;
    for (const auto& scope : scopes_) dumpScope(out, scope, index++);
    for (const auto& scope : closed_scopes_) dumpScope(out, scope, index++);
    return out.str();
}

} // namespace semantic

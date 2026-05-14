#include "errors.h"

#include <sstream>

namespace semantic {

std::string SemanticError::toString(const std::string& fileName) const {
    std::ostringstream out;
    out << "semantic error: " << category << ": " << message << "\n";
    out << "  --> " << fileName << ":" << line << ":" << column;
    if (!context.empty()) out << "\n  = context: " << context;
    return out.str();
}

} // namespace semantic

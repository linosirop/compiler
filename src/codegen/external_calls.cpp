#include "codegen/external_calls.h"

#include <set>

namespace codegen {

bool ExternalCalls::isKnownLibCFunction(const std::string& name) {
    static const std::set<std::string> functions = {
        "printf", "scanf", "puts", "getchar", "malloc", "free", "memcpy", "memset",
        "strlen", "strcpy", "strcmp", "pow", "sqrt", "sin", "cos"
    };
    return functions.find(name) != functions.end();
}

bool ExternalCalls::isVariadicFunction(const std::string& name) {
    return name == "printf" || name == "scanf";
}

} // namespace codegen

#pragma once

#include <string>

namespace codegen {

class ExternalCalls {
public:
    static bool isKnownLibCFunction(const std::string& name);
    static bool isVariadicFunction(const std::string& name);
};

} // namespace codegen

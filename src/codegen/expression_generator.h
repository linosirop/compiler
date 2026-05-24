#pragma once

#include <string>

namespace codegen {

class ExpressionGenerator {
public:
    static bool isLogicalOperator(const std::string& op) { return op == "&&" || op == "||"; }
    static bool isUnaryLogicalNot(const std::string& op) { return op == "!"; }
};

} // namespace codegen

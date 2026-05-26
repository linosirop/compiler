#pragma once

#include <string>

namespace codegen {

class ArrayGenerator {
public:
    static int elementSizeBytes(const std::string& elementType);
    static std::string describeAddressFormula(const std::string& arrayName, const std::string& indexExpression);
};

} // namespace codegen

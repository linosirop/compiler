#include "codegen/array_generator.h"

namespace codegen {

int ArrayGenerator::elementSizeBytes(const std::string& elementType) {
    if (elementType == "bool") return 1;
    if (elementType == "int") return 8; // Backend stores MiniCompiler integer values in qword stack slots.
    if (elementType == "float") return 8;
    return 8;
}

std::string ArrayGenerator::describeAddressFormula(const std::string& arrayName, const std::string& indexExpression) {
    return "address(" + arrayName + "[" + indexExpression + "]) = base(" + arrayName + ") + index * element_size";
}

} // namespace codegen

#pragma once

#include <string>
#include <vector>

namespace codegen {

class SimpleRegisterAllocator {
public:
    const std::vector<std::string>& scratchRegisters() const;
    std::string scratch(size_t index) const;
};

} // namespace codegen

#include "codegen/register_allocator.h"

namespace codegen {

const std::vector<std::string>& SimpleRegisterAllocator::scratchRegisters() const {
    static const std::vector<std::string> registers = {"rax", "rbx", "rcx", "rdx", "r10", "r11"};
    return registers;
}

std::string SimpleRegisterAllocator::scratch(size_t index) const {
    const auto& registers = scratchRegisters();
    return registers[index % registers.size()];
}

} // namespace codegen

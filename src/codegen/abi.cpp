#include "codegen/abi.h"

namespace codegen {

const std::vector<std::string>& SystemVAbi::integerArgumentRegisters64() {
    static const std::vector<std::string> registers = {"rdi", "rsi", "rdx", "rcx", "r8", "r9"};
    return registers;
}

const std::vector<std::string>& SystemVAbi::integerArgumentRegisters32() {
    static const std::vector<std::string> registers = {"edi", "esi", "edx", "ecx", "r8d", "r9d"};
    return registers;
}

std::string SystemVAbi::returnRegister64() { return "rax"; }

std::string SystemVAbi::scratchRegister64(int index) {
    static const std::vector<std::string> registers = {"rax", "rbx", "rcx", "rdx", "r10", "r11"};
    if (index < 0) return registers.front();
    return registers[static_cast<size_t>(index) % registers.size()];
}

int SystemVAbi::slotSizeBytes() { return 8; }

int SystemVAbi::alignStackSize(int bytes) {
    if (bytes <= 0) return 0;
    const int remainder = bytes % 16;
    return remainder == 0 ? bytes : bytes + (16 - remainder);
}

} // namespace codegen

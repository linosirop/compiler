#pragma once

#include <string>
#include <vector>

namespace codegen {

struct SystemVAbi {
    static const std::vector<std::string>& integerArgumentRegisters64();
    static const std::vector<std::string>& integerArgumentRegisters32();
    static std::string returnRegister64();
    static std::string scratchRegister64(int index);
    static int slotSizeBytes();
    static int alignStackSize(int bytes);
};

} // namespace codegen

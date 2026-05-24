#pragma once

#include "ir/ir_instructions.h"

#include <string>

namespace codegen {

struct ConditionalJumpPlan {
    std::string jumpMnemonic;
    std::string targetLabel;
};

class ControlFlowGenerator {
public:
    static bool isComparison(ir::Opcode opcode);
    static std::string jumpForComparison(ir::Opcode opcode);
    static std::string invertedJump(const std::string& jumpMnemonic);
};

} // namespace codegen

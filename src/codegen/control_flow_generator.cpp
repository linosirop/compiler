#include "codegen/control_flow_generator.h"

namespace codegen {

bool ControlFlowGenerator::isComparison(ir::Opcode opcode) {
    using ir::Opcode;
    return opcode == Opcode::CMP_EQ || opcode == Opcode::CMP_NE || opcode == Opcode::CMP_LT ||
           opcode == Opcode::CMP_LE || opcode == Opcode::CMP_GT || opcode == Opcode::CMP_GE;
}

std::string ControlFlowGenerator::jumpForComparison(ir::Opcode opcode) {
    using ir::Opcode;
    switch (opcode) {
    case Opcode::CMP_EQ: return "je";
    case Opcode::CMP_NE: return "jne";
    case Opcode::CMP_LT: return "jl";
    case Opcode::CMP_LE: return "jle";
    case Opcode::CMP_GT: return "jg";
    case Opcode::CMP_GE: return "jge";
    default: return "jne";
    }
}

std::string ControlFlowGenerator::invertedJump(const std::string& jumpMnemonic) {
    if (jumpMnemonic == "je") return "jne";
    if (jumpMnemonic == "jne") return "je";
    if (jumpMnemonic == "jl") return "jge";
    if (jumpMnemonic == "jle") return "jg";
    if (jumpMnemonic == "jg") return "jle";
    if (jumpMnemonic == "jge") return "jl";
    return "je";
}

} // namespace codegen

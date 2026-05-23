#include "control_flow.h"

#include <set>
#include <sstream>

namespace ir {

namespace {
bool isJump(const IRInstruction& instruction) {
    return instruction.opcode == Opcode::JUMP || instruction.opcode == Opcode::JUMP_IF || instruction.opcode == Opcode::JUMP_IF_NOT;
}

std::string jumpTarget(const IRInstruction& instruction) {
    if (instruction.opcode == Opcode::JUMP && !instruction.operands.empty()) return instruction.operands[0];
    if ((instruction.opcode == Opcode::JUMP_IF || instruction.opcode == Opcode::JUMP_IF_NOT) && instruction.operands.size() >= 2) return instruction.operands[1];
    return "";
}
}

bool allJumpTargetsExist(const IRProgram& program, std::string* errorMessage) {
    for (const auto& function : program.functions) {
        std::set<std::string> labels;
        for (const auto& block : function.blocks) labels.insert(block.label);
        for (const auto& block : function.blocks) {
            for (const auto& instruction : block.instructions) {
                if (!isJump(instruction)) continue;
                std::string target = jumpTarget(instruction);
                if (!target.empty() && labels.find(target) == labels.end()) {
                    if (errorMessage) {
                        *errorMessage = "missing jump target '" + target + "' in function '" + function.name + "'";
                    }
                    return false;
                }
            }
        }
    }
    return true;
}

bool everyBlockHasTerminator(const IRProgram& program, std::string* errorMessage) {
    for (const auto& function : program.functions) {
        for (const auto& block : function.blocks) {
            if (!block.endsWithTerminator()) {
                if (errorMessage) {
                    *errorMessage = "basic block '" + block.label + "' in function '" + function.name + "' has no terminator";
                }
                return false;
            }
        }
    }
    return true;
}

std::string validateControlFlow(const IRProgram& program) {
    std::string error;
    std::ostringstream out;
    out << "IR Control Flow Validation:\n";
    if (allJumpTargetsExist(program, &error)) {
        out << "  jump targets: ok\n";
    } else {
        out << "  jump targets: failed: " << error << "\n";
    }
    if (everyBlockHasTerminator(program, &error)) {
        out << "  terminators: ok\n";
    } else {
        out << "  terminators: failed: " << error << "\n";
    }
    return out.str();
}

} // namespace ir

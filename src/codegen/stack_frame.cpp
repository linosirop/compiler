#include "codegen/stack_frame.h"

#include "codegen/abi.h"

#include <cctype>
#include <sstream>
#include <stdexcept>

namespace codegen {
namespace {
bool isIntegerLiteral(const std::string& text) {
    if (text.empty()) return false;
    size_t start = text[0] == '-' ? 1U : 0U;
    if (start == text.size()) return false;
    for (size_t i = start; i < text.size(); ++i) {
        if (!std::isdigit(static_cast<unsigned char>(text[i]))) return false;
    }
    return true;
}
}

void StackFrame::build(const ir::IRFunction& function) {
    offsets_.clear();
    nextOffset_ = 0;
    stackSize_ = 0;

    for (const auto& parameter : function.parameters) {
        ensureSlot(parameter.first);
    }

    for (const auto& mapping : function.variableLocations) {
        ensureSlot(mapping.second);
    }

    for (const auto& block : function.blocks) {
        for (const auto& instruction : block.instructions) {
            if (!instruction.dest.empty() && instruction.opcode != ir::Opcode::JUMP &&
                instruction.opcode != ir::Opcode::JUMP_IF && instruction.opcode != ir::Opcode::JUMP_IF_NOT) {
                ensureSlot(normalizeMemoryName(instruction.dest));
            }

            for (size_t i = 0; i < instruction.operands.size(); ++i) {
                if (instruction.opcode == ir::Opcode::CALL && i == 0) continue;
                if (instruction.opcode == ir::Opcode::JUMP) continue;
                if ((instruction.opcode == ir::Opcode::JUMP_IF || instruction.opcode == ir::Opcode::JUMP_IF_NOT) && i == 1) continue;
                const std::string normalized = normalizeMemoryName(instruction.operands[i]);
                if (isNamedStorage(normalized)) ensureSlot(normalized);
            }
        }
    }

    stackSize_ = SystemVAbi::alignStackSize(nextOffset_);
}

bool StackFrame::hasSlot(const std::string& name) const {
    return offsets_.find(normalizeMemoryName(name)) != offsets_.end();
}

int StackFrame::offsetOf(const std::string& name) const {
    const std::string normalized = normalizeMemoryName(name);
    auto found = offsets_.find(normalized);
    if (found == offsets_.end()) {
        throw std::runtime_error("no stack slot for operand: " + name);
    }
    return found->second;
}

std::string StackFrame::addressOf(const std::string& name) const {
    const int offset = offsetOf(name);
    return "[rbp-" + std::to_string(offset) + "]";
}

int StackFrame::stackSize() const { return stackSize_; }

std::string StackFrame::dump() const {
    std::ostringstream out;
    for (const auto& entry : offsets_) {
        out << ";   " << entry.first << " -> [rbp-" << entry.second << "]\n";
    }
    return out.str();
}

void StackFrame::ensureSlot(const std::string& name) {
    if (!isNamedStorage(name)) return;
    if (offsets_.find(name) != offsets_.end()) return;
    nextOffset_ += SystemVAbi::slotSizeBytes();
    offsets_[name] = nextOffset_;
}

std::string StackFrame::normalizeMemoryName(const std::string& operand) {
    if (operand.size() >= 2 && operand.front() == '[' && operand.back() == ']') {
        return operand.substr(1, operand.size() - 2);
    }
    return operand;
}

bool StackFrame::isNamedStorage(const std::string& value) {
    if (value.empty()) return false;
    if (value == "true" || value == "false") return false;
    if (isIntegerLiteral(value)) return false;
    if (value.find('.') != std::string::npos) return false;
    return std::isalpha(static_cast<unsigned char>(value[0])) || value[0] == '_';
}

} // namespace codegen

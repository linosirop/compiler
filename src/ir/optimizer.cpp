#include "ir/optimizer.h"

#include <algorithm>
#include <cctype>
#include <map>
#include <set>
#include <sstream>

namespace ir {
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

bool isBooleanLiteral(const std::string& text) { return text == "true" || text == "false"; }
bool isConstant(const std::string& text) { return isIntegerLiteral(text) || isBooleanLiteral(text); }
long long toInt(const std::string& text) {
    if (text == "true") return 1;
    if (text == "false") return 0;
    return std::stoll(text);
}
std::string fromBool(bool value) { return value ? "true" : "false"; }
std::string fromInt(long long value) { return std::to_string(value); }

bool isFoldableBinary(Opcode opcode) {
    switch (opcode) {
    case Opcode::ADD: case Opcode::SUB: case Opcode::MUL: case Opcode::DIV: case Opcode::MOD:
    case Opcode::AND: case Opcode::OR: case Opcode::XOR:
    case Opcode::CMP_EQ: case Opcode::CMP_NE: case Opcode::CMP_LT: case Opcode::CMP_LE:
    case Opcode::CMP_GT: case Opcode::CMP_GE:
        return true;
    default:
        return false;
    }
}

std::string foldBinary(Opcode opcode, const std::string& left, const std::string& right, bool& ok) {
    ok = true;
    const long long a = toInt(left);
    const long long b = toInt(right);
    switch (opcode) {
    case Opcode::ADD: return fromInt(a + b);
    case Opcode::SUB: return fromInt(a - b);
    case Opcode::MUL: return fromInt(a * b);
    case Opcode::DIV:
        if (b == 0) { ok = false; return {}; }
        return fromInt(a / b);
    case Opcode::MOD:
        if (b == 0) { ok = false; return {}; }
        return fromInt(a % b);
    case Opcode::AND: return fromBool((a != 0) && (b != 0));
    case Opcode::OR: return fromBool((a != 0) || (b != 0));
    case Opcode::XOR: return fromInt(a ^ b);
    case Opcode::CMP_EQ: return fromBool(a == b);
    case Opcode::CMP_NE: return fromBool(a != b);
    case Opcode::CMP_LT: return fromBool(a < b);
    case Opcode::CMP_LE: return fromBool(a <= b);
    case Opcode::CMP_GT: return fromBool(a > b);
    case Opcode::CMP_GE: return fromBool(a >= b);
    default: ok = false; return {};
    }
}

bool isFoldableUnary(Opcode opcode) { return opcode == Opcode::NEG || opcode == Opcode::NOT; }
std::string foldUnary(Opcode opcode, const std::string& operand) {
    const long long value = toInt(operand);
    if (opcode == Opcode::NEG) return fromInt(-value);
    return fromBool(value == 0);
}

bool isTemporary(const std::string& name) {
    if (name.size() < 2 || name[0] != 't') return false;
    return std::all_of(name.begin() + 1, name.end(), [](char c) { return std::isdigit(static_cast<unsigned char>(c)) != 0; });
}

bool hasSideEffects(const IRInstruction& inst) {
    return inst.opcode == Opcode::STORE || inst.opcode == Opcode::CALL || inst.opcode == Opcode::PARAM || inst.isTerminator() || inst.opcode == Opcode::ALLOCA;
}

} // namespace

int OptimizationReport::totalChanges() const {
    return constantFolds + constantsPropagated + deadInstructionsRemoved + branchSimplifications;
}

std::string OptimizationReport::toString() const {
    std::ostringstream out;
    out << "Optimization Report:\n";
    out << "  Iterations: " << iterations << "\n";
    out << "  Constant folding: " << constantFolds << " expression(s) folded\n";
    out << "  Constant propagation: " << constantsPropagated << " use(s) replaced\n";
    out << "  Branch simplification: " << branchSimplifications << " branch(es) simplified\n";
    out << "  Dead code elimination: " << deadInstructionsRemoved << " instruction(s) removed\n";
    out << "  Total changes: " << totalChanges() << "\n";
    return out.str();
}

OptimizationReport Optimizer::optimize(IRProgram& program, int maxIterations) {
    OptimizationReport report;
    for (int i = 0; i < maxIterations; ++i) {
        bool changed = false;
        changed = constantPropagate(program, report) || changed;
        changed = constantFold(program, report) || changed;
        changed = simplifyBranches(program, report) || changed;
        changed = eliminateDeadCode(program, report) || changed;
        ++report.iterations;
        if (!changed) break;
    }
    return report;
}

bool Optimizer::constantFold(IRProgram& program, OptimizationReport& report) {
    bool changed = false;
    for (auto& function : program.functions) {
        for (auto& block : function.blocks) {
            for (auto& inst : block.instructions) {
                if (isFoldableBinary(inst.opcode) && inst.operands.size() >= 2 && isConstant(inst.operands[0]) && isConstant(inst.operands[1])) {
                    bool ok = false;
                    const std::string folded = foldBinary(inst.opcode, inst.operands[0], inst.operands[1], ok);
                    if (ok) {
                        inst.opcode = Opcode::MOVE;
                        inst.operands = {folded};
                        inst.comment = "constant folded";
                        ++report.constantFolds;
                        changed = true;
                    }
                } else if (isFoldableUnary(inst.opcode) && !inst.operands.empty() && isConstant(inst.operands[0])) {
                    inst.opcode = Opcode::MOVE;
                    inst.operands = {foldUnary(inst.opcode, inst.operands[0])};
                    inst.comment = "constant folded";
                    ++report.constantFolds;
                    changed = true;
                }
            }
        }
    }
    return changed;
}

bool Optimizer::constantPropagate(IRProgram& program, OptimizationReport& report) {
    bool changed = false;
    for (auto& function : program.functions) {
        std::map<std::string, std::string> constants;
        for (auto& block : function.blocks) {
            for (auto& inst : block.instructions) {
                for (size_t i = 0; i < inst.operands.size(); ++i) {
                    if (inst.opcode == Opcode::CALL && i == 0) continue;
                    if (inst.opcode == Opcode::JUMP) continue;
                    if ((inst.opcode == Opcode::JUMP_IF || inst.opcode == Opcode::JUMP_IF_NOT) && i == 1) continue;
                    auto found = constants.find(inst.operands[i]);
                    if (found != constants.end()) {
                        inst.operands[i] = found->second;
                        ++report.constantsPropagated;
                        changed = true;
                    }
                }

                if (inst.opcode == Opcode::MOVE && inst.operands.size() == 1 && isConstant(inst.operands[0]) && !inst.dest.empty()) {
                    constants[inst.dest] = inst.operands[0];
                } else if (inst.opcode == Opcode::STORE && inst.operands.size() == 1 && isConstant(inst.operands[0])) {
                    constants[inst.dest] = inst.operands[0];
                } else if (inst.opcode == Opcode::LOAD && inst.operands.size() == 1) {
                    if (isConstant(inst.operands[0])) {
                        inst.opcode = Opcode::MOVE;
                        inst.comment = "constant propagated load";
                        constants[inst.dest] = inst.operands[0];
                        ++report.constantsPropagated;
                        changed = true;
                    } else {
                        auto found = constants.find(inst.operands[0]);
                        if (found != constants.end()) {
                            inst.opcode = Opcode::MOVE;
                            inst.operands = {found->second};
                            inst.comment = "constant propagated load";
                            constants[inst.dest] = found->second;
                            ++report.constantsPropagated;
                            changed = true;
                        } else {
                            constants.erase(inst.dest);
                        }
                    }
                } else {
                    if (!inst.dest.empty()) constants.erase(inst.dest);
                    if (inst.opcode == Opcode::CALL) constants.clear();
                }
            }
        }
    }
    return changed;
}

bool Optimizer::simplifyBranches(IRProgram& program, OptimizationReport& report) {
    bool changed = false;
    for (auto& function : program.functions) {
        for (auto& block : function.blocks) {
            for (auto& inst : block.instructions) {
                if ((inst.opcode == Opcode::JUMP_IF || inst.opcode == Opcode::JUMP_IF_NOT) && inst.operands.size() >= 2 && isConstant(inst.operands[0])) {
                    const bool value = toInt(inst.operands[0]) != 0;
                    const bool take = inst.opcode == Opcode::JUMP_IF ? value : !value;
                    if (take) {
                        inst.opcode = Opcode::JUMP;
                        inst.operands = {inst.operands[1]};
                    } else {
                        inst.opcode = Opcode::NOP;
                        inst.operands.clear();
                    }
                    inst.dest.clear();
                    inst.comment = "constant branch simplified";
                    ++report.branchSimplifications;
                    changed = true;
                }
            }
        }
    }
    return changed;
}

bool Optimizer::eliminateDeadCode(IRProgram& program, OptimizationReport& report) {
    bool changed = false;
    for (auto& function : program.functions) {
        std::set<std::string> used;
        for (const auto& block : function.blocks) {
            for (const auto& inst : block.instructions) {
                for (size_t i = 0; i < inst.operands.size(); ++i) {
                    if (inst.opcode == Opcode::CALL && i == 0) continue;
                    if (inst.opcode == Opcode::JUMP) continue;
                    if ((inst.opcode == Opcode::JUMP_IF || inst.opcode == Opcode::JUMP_IF_NOT) && i == 1) continue;
                    used.insert(inst.operands[i]);
                }
            }
        }

        for (auto& block : function.blocks) {
            std::vector<IRInstruction> kept;
            bool afterTerminator = false;
            for (const auto& inst : block.instructions) {
                if (afterTerminator) {
                    ++report.deadInstructionsRemoved;
                    changed = true;
                    continue;
                }
                if (inst.opcode == Opcode::NOP) {
                    ++report.deadInstructionsRemoved;
                    changed = true;
                    continue;
                }
                if (!hasSideEffects(inst) && !inst.dest.empty() && isTemporary(inst.dest) && used.find(inst.dest) == used.end()) {
                    ++report.deadInstructionsRemoved;
                    changed = true;
                    continue;
                }
                kept.push_back(inst);
                if (inst.isTerminator()) afterTerminator = true;
            }
            block.instructions = std::move(kept);
        }
    }
    return changed;
}

OptimizationReport optimizeProgram(IRProgram& program, int maxIterations) {
    Optimizer optimizer;
    return optimizer.optimize(program, maxIterations);
}

} // namespace ir

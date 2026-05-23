#include "ir_instructions.h"

#include <sstream>

namespace ir {

std::string opcodeToString(Opcode opcode) {
    switch (opcode) {
    case Opcode::NOP: return "NOP";
    case Opcode::ADD: return "ADD";
    case Opcode::SUB: return "SUB";
    case Opcode::MUL: return "MUL";
    case Opcode::DIV: return "DIV";
    case Opcode::MOD: return "MOD";
    case Opcode::NEG: return "NEG";
    case Opcode::AND: return "AND";
    case Opcode::OR: return "OR";
    case Opcode::NOT: return "NOT";
    case Opcode::XOR: return "XOR";
    case Opcode::CMP_EQ: return "CMP_EQ";
    case Opcode::CMP_NE: return "CMP_NE";
    case Opcode::CMP_LT: return "CMP_LT";
    case Opcode::CMP_LE: return "CMP_LE";
    case Opcode::CMP_GT: return "CMP_GT";
    case Opcode::CMP_GE: return "CMP_GE";
    case Opcode::LOAD: return "LOAD";
    case Opcode::STORE: return "STORE";
    case Opcode::ALLOCA: return "ALLOCA";
    case Opcode::MOVE: return "MOVE";
    case Opcode::JUMP: return "JUMP";
    case Opcode::JUMP_IF: return "JUMP_IF";
    case Opcode::JUMP_IF_NOT: return "JUMP_IF_NOT";
    case Opcode::PARAM: return "PARAM";
    case Opcode::CALL: return "CALL";
    case Opcode::RETURN: return "RETURN";
    case Opcode::PHI: return "PHI";
    }
    return "UNKNOWN";
}

IRInstruction::IRInstruction(Opcode opcodeValue, std::string destValue, std::vector<std::string> operandValues, std::string commentValue)
    : opcode(opcodeValue), dest(std::move(destValue)), operands(std::move(operandValues)), comment(std::move(commentValue)) {}

bool IRInstruction::isTerminator() const {
    return opcode == Opcode::JUMP || opcode == Opcode::JUMP_IF || opcode == Opcode::JUMP_IF_NOT || opcode == Opcode::RETURN;
}

std::string IRInstruction::toString() const {
    std::ostringstream out;
    const std::string op = opcodeToString(opcode);

    switch (opcode) {
    case Opcode::ADD:
    case Opcode::SUB:
    case Opcode::MUL:
    case Opcode::DIV:
    case Opcode::MOD:
    case Opcode::AND:
    case Opcode::OR:
    case Opcode::XOR:
    case Opcode::CMP_EQ:
    case Opcode::CMP_NE:
    case Opcode::CMP_LT:
    case Opcode::CMP_LE:
    case Opcode::CMP_GT:
    case Opcode::CMP_GE:
        out << dest << " = " << op << " " << operands.at(0) << ", " << operands.at(1);
        break;
    case Opcode::NEG:
    case Opcode::NOT:
    case Opcode::LOAD:
    case Opcode::MOVE:
        out << dest << " = " << op << " " << operands.at(0);
        break;
    case Opcode::ALLOCA:
        out << dest << " = ALLOCA " << (operands.empty() ? "auto" : operands.at(0));
        break;
    case Opcode::STORE:
        out << "STORE " << dest << ", " << operands.at(0);
        break;
    case Opcode::JUMP:
        out << "JUMP " << operands.at(0);
        break;
    case Opcode::JUMP_IF:
        out << "JUMP_IF " << operands.at(0) << ", " << operands.at(1);
        break;
    case Opcode::JUMP_IF_NOT:
        out << "JUMP_IF_NOT " << operands.at(0) << ", " << operands.at(1);
        break;
    case Opcode::PARAM:
        out << "PARAM " << dest << ", " << operands.at(0);
        break;
    case Opcode::CALL:
        out << dest << " = CALL " << operands.at(0);
        for (size_t i = 1; i < operands.size(); ++i) out << ", " << operands[i];
        break;
    case Opcode::RETURN:
        out << "RETURN";
        if (!operands.empty()) out << " " << operands.at(0);
        break;
    case Opcode::PHI:
        out << dest << " = PHI";
        for (const auto& operand : operands) out << " " << operand;
        break;
    case Opcode::NOP:
        out << "NOP";
        break;
    }

    if (!comment.empty()) out << "    # " << comment;
    return out.str();
}

std::ostream& operator<<(std::ostream& out, const IRInstruction& instruction) {
    out << instruction.toString();
    return out;
}

} // namespace ir

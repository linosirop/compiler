#pragma once

#include <ostream>
#include <string>
#include <vector>

namespace ir {

enum class Opcode {
    NOP,
    ADD, SUB, MUL, DIV, MOD, NEG,
    AND, OR, NOT, XOR,
    CMP_EQ, CMP_NE, CMP_LT, CMP_LE, CMP_GT, CMP_GE,
    LOAD, STORE, ALLOCA, MOVE,
    JUMP, JUMP_IF, JUMP_IF_NOT,
    PARAM, CALL, RETURN,
    PHI
};

std::string opcodeToString(Opcode opcode);

struct IRInstruction {
    Opcode opcode = Opcode::NOP;
    std::string dest;
    std::vector<std::string> operands;
    std::string comment;
    int line = 1;
    int column = 1;

    IRInstruction() = default;
    IRInstruction(Opcode opcode, std::string dest, std::vector<std::string> operands = {}, std::string comment = "");

    bool isTerminator() const;
    std::string toString() const;
};

std::ostream& operator<<(std::ostream& out, const IRInstruction& instruction);

} // namespace ir

#include "codegen/x86_generator.h"

#include "codegen/abi.h"

#include <algorithm>
#include <cctype>
#include <stdexcept>

namespace codegen {

X86Generator::X86Generator(X86GeneratorOptions options) : options_(options) {}

std::string X86Generator::generate(const ir::IRProgram& program) {
    out_.str("");
    out_.clear();
    out_ << "; MiniCompiler x86-64 assembly\n";
    out_ << "; Target: NASM, ELF64, System V AMD64 ABI\n";
    out_ << "default rel\n\n";
    out_ << "section .text\n";

    for (const auto& function : program.functions) {
        if (function.name == "__global") continue;
        out_ << "global " << sanitizeSymbol(function.name) << "\n";
    }
    out_ << "extern print_int\n";
    out_ << "extern print_string\n";
    out_ << "extern read_int\n";
    out_ << "extern exit\n\n";

    for (const auto& function : program.functions) {
        if (function.name != "__global") generateFunction(function);
    }
    return out_.str();
}

void X86Generator::generate(const ir::IRProgram& program, std::ostream& out) {
    out << generate(program);
}

void X86Generator::generateFunction(const ir::IRFunction& function) {
    currentFunction_ = sanitizeSymbol(function.name);
    frame_.build(function);

    out_ << currentFunction_ << ":\n";
    emitPrologue(function);

    for (const auto& block : function.blocks) {
        if (block.label != "entry") {
            out_ << labelFor(block.label) << ":\n";
        }
        for (const auto& instruction : block.instructions) {
            emitInstruction(instruction);
        }
    }

    if (options_.emitComments) {
        out_ << "; implicit epilogue safeguard\n";
    }
    if (function.returnType.isVoid()) out_ << "    xor rax, rax\n";
    emitEpilogue();
    out_ << "\n";
}

void X86Generator::emitPrologue(const ir::IRFunction& function) {
    out_ << "    push rbp\n";
    out_ << "    mov rbp, rsp\n";
    if (frame_.stackSize() > 0) out_ << "    sub rsp, " << frame_.stackSize() << "\n";

    const auto& argRegs = SystemVAbi::integerArgumentRegisters64();
    for (size_t i = 0; i < function.parameters.size() && i < argRegs.size(); ++i) {
        storeOperand(function.parameters[i].first, argRegs[i]);
    }

    for (size_t i = argRegs.size(); i < function.parameters.size(); ++i) {
        const int stackArgOffset = 16 + static_cast<int>((i - argRegs.size()) * SystemVAbi::slotSizeBytes());
        out_ << "    mov rax, [rbp+" << stackArgOffset << "]\n";
        storeOperand(function.parameters[i].first, "rax");
    }

    if (options_.emitStackMap && frame_.stackSize() > 0) {
        out_ << "; stack frame size: " << frame_.stackSize() << " bytes\n";
        out_ << frame_.dump();
    }
}

void X86Generator::emitEpilogue() {
    out_ << "    mov rsp, rbp\n";
    out_ << "    pop rbp\n";
    out_ << "    ret\n";
}

void X86Generator::emitInstruction(const ir::IRInstruction& instruction) {
    if (options_.emitComments && !instruction.comment.empty()) {
        out_ << "    ; " << instruction.comment << "\n";
    }

    using ir::Opcode;
    switch (instruction.opcode) {
    case Opcode::NOP:
    case Opcode::ALLOCA:
    case Opcode::PARAM:
        break;
    case Opcode::MOVE:
        loadOperand(instruction.operands.at(0), "rax");
        storeOperand(instruction.dest, "rax");
        break;
    case Opcode::LOAD:
        loadOperand(instruction.operands.at(0), "rax");
        storeOperand(instruction.dest, "rax");
        break;
    case Opcode::STORE:
        loadOperand(instruction.operands.at(0), "rax");
        storeOperand(instruction.dest, "rax");
        break;
    case Opcode::ADD:
        emitBinary(instruction, "add");
        break;
    case Opcode::SUB:
        emitBinary(instruction, "sub");
        break;
    case Opcode::MUL:
        emitBinary(instruction, "imul");
        break;
    case Opcode::AND:
        emitBinary(instruction, "and");
        break;
    case Opcode::OR:
        emitBinary(instruction, "or");
        break;
    case Opcode::XOR:
        emitBinary(instruction, "xor");
        break;
    case Opcode::DIV:
        emitDivision(instruction, false);
        break;
    case Opcode::MOD:
        emitDivision(instruction, true);
        break;
    case Opcode::NEG:
        loadOperand(instruction.operands.at(0), "rax");
        out_ << "    neg rax\n";
        storeOperand(instruction.dest, "rax");
        break;
    case Opcode::NOT:
        loadOperand(instruction.operands.at(0), "rax");
        out_ << "    cmp rax, 0\n";
        out_ << "    sete al\n";
        out_ << "    movzx rax, al\n";
        storeOperand(instruction.dest, "rax");
        break;
    case Opcode::CMP_EQ:
        emitComparison(instruction, "e");
        break;
    case Opcode::CMP_NE:
        emitComparison(instruction, "ne");
        break;
    case Opcode::CMP_LT:
        emitComparison(instruction, "l");
        break;
    case Opcode::CMP_LE:
        emitComparison(instruction, "le");
        break;
    case Opcode::CMP_GT:
        emitComparison(instruction, "g");
        break;
    case Opcode::CMP_GE:
        emitComparison(instruction, "ge");
        break;
    case Opcode::JUMP:
        out_ << "    jmp " << labelFor(instruction.operands.at(0)) << "\n";
        break;
    case Opcode::JUMP_IF:
        loadOperand(instruction.operands.at(0), "rax");
        out_ << "    cmp rax, 0\n";
        out_ << "    jne " << labelFor(instruction.operands.at(1)) << "\n";
        break;
    case Opcode::JUMP_IF_NOT:
        loadOperand(instruction.operands.at(0), "rax");
        out_ << "    cmp rax, 0\n";
        out_ << "    je " << labelFor(instruction.operands.at(1)) << "\n";
        break;
    case Opcode::CALL:
        emitCall(instruction);
        break;
    case Opcode::RETURN:
        if (!instruction.operands.empty()) loadOperand(instruction.operands.at(0), "rax");
        else out_ << "    xor rax, rax\n";
        emitEpilogue();
        break;
    case Opcode::PHI:
        if (!instruction.operands.empty()) {
            loadOperand(instruction.operands.front(), "rax");
            storeOperand(instruction.dest, "rax");
        }
        break;
    }
}

void X86Generator::emitBinary(const ir::IRInstruction& instruction, const std::string& mnemonic) {
    loadOperand(instruction.operands.at(0), "rax");
    loadOperand(instruction.operands.at(1), "r10");
    out_ << "    " << mnemonic << " rax, r10\n";
    storeOperand(instruction.dest, "rax");
}

void X86Generator::emitDivision(const ir::IRInstruction& instruction, bool remainder) {
    loadOperand(instruction.operands.at(0), "rax");
    loadOperand(instruction.operands.at(1), "r10");
    out_ << "    cqo\n";
    out_ << "    idiv r10\n";
    storeOperand(instruction.dest, remainder ? "rdx" : "rax");
}

void X86Generator::emitComparison(const ir::IRInstruction& instruction, const std::string& conditionCode) {
    loadOperand(instruction.operands.at(0), "rax");
    loadOperand(instruction.operands.at(1), "r10");
    out_ << "    cmp rax, r10\n";
    out_ << "    set" << conditionCode << " al\n";
    out_ << "    movzx rax, al\n";
    storeOperand(instruction.dest, "rax");
}

void X86Generator::emitCall(const ir::IRInstruction& instruction) {
    if (instruction.operands.empty()) return;
    const std::string callee = sanitizeSymbol(instruction.operands.at(0));
    const auto& argRegs = SystemVAbi::integerArgumentRegisters64();
    const size_t argCount = instruction.operands.size() - 1;
    const size_t stackArgCount = argCount > argRegs.size() ? argCount - argRegs.size() : 0;

    for (size_t i = argCount; i > argRegs.size(); --i) {
        loadOperand(instruction.operands.at(i), "rax");
        out_ << "    push rax\n";
    }

    for (size_t i = 0; i < argCount && i < argRegs.size(); ++i) {
        loadOperand(instruction.operands.at(i + 1), "rax");
        out_ << "    mov " << argRegs[i] << ", rax\n";
    }

    out_ << "    call " << callee << "\n";
    if (stackArgCount > 0) {
        out_ << "    add rsp, " << stackArgCount * static_cast<size_t>(SystemVAbi::slotSizeBytes()) << "\n";
    }
    if (!instruction.dest.empty()) storeOperand(instruction.dest, "rax");
}

void X86Generator::loadOperand(const std::string& operand, const std::string& reg) {
    if (isImmediate(operand)) {
        out_ << "    mov " << reg << ", " << immediateValue(operand) << "\n";
        return;
    }

    const std::string name = normalizeMemoryName(operand);
    if (frame_.hasSlot(name)) {
        out_ << "    mov " << reg << ", qword " << frame_.addressOf(name) << "\n";
        return;
    }

    out_ << "    ; unsupported operand treated as zero: " << operand << "\n";
    out_ << "    xor " << reg << ", " << reg << "\n";
}

void X86Generator::storeOperand(const std::string& destination, const std::string& reg) {
    if (destination.empty()) return;
    const std::string name = normalizeMemoryName(destination);
    if (!frame_.hasSlot(name)) return;
    out_ << "    mov qword " << frame_.addressOf(name) << ", " << reg << "\n";
}

std::string X86Generator::labelFor(const std::string& label) const {
    return "." + currentFunction_ + "_" + sanitizeSymbol(label);
}

std::string X86Generator::normalizeMemoryName(const std::string& operand) {
    if (operand.size() >= 2 && operand.front() == '[' && operand.back() == ']') {
        return operand.substr(1, operand.size() - 2);
    }
    return operand;
}

bool X86Generator::isIntegerLiteral(const std::string& text) {
    if (text.empty()) return false;
    size_t start = text[0] == '-' ? 1U : 0U;
    if (start == text.size()) return false;
    return std::all_of(text.begin() + static_cast<std::string::difference_type>(start), text.end(), [](char ch) {
        return std::isdigit(static_cast<unsigned char>(ch)) != 0;
    });
}

bool X86Generator::isImmediate(const std::string& operand) {
    return operand == "true" || operand == "false" || isIntegerLiteral(operand);
}

std::string X86Generator::immediateValue(const std::string& operand) {
    if (operand == "true") return "1";
    if (operand == "false") return "0";
    return operand;
}

std::string X86Generator::sanitizeSymbol(const std::string& symbol) {
    std::string result;
    result.reserve(symbol.size());
    for (char ch : symbol) {
        if (std::isalnum(static_cast<unsigned char>(ch)) || ch == '_') result.push_back(ch);
        else result.push_back('_');
    }
    if (result.empty()) return "anon";
    if (std::isdigit(static_cast<unsigned char>(result.front()))) result.insert(result.begin(), '_');
    return result;
}

std::string generateX86Assembly(const ir::IRProgram& program) {
    X86Generator generator;
    return generator.generate(program);
}

} // namespace codegen

#pragma once

#include "codegen/register_allocator.h"
#include "codegen/stack_frame.h"
#include "ir/basic_block.h"

#include <ostream>
#include <sstream>
#include <string>

namespace codegen {

struct X86GeneratorOptions {
    bool emitComments = true;
    bool emitStackMap = true;
};

class X86Generator {
public:
    explicit X86Generator(X86GeneratorOptions options = {});

    std::string generate(const ir::IRProgram& program);
    void generate(const ir::IRProgram& program, std::ostream& out);

private:
    void generateFunction(const ir::IRFunction& function);
    void emitInstruction(const ir::IRInstruction& instruction);
    void emitPrologue(const ir::IRFunction& function);
    void emitEpilogue();

    void emitBinary(const ir::IRInstruction& instruction, const std::string& mnemonic);
    void emitDivision(const ir::IRInstruction& instruction, bool remainder);
    void emitComparison(const ir::IRInstruction& instruction, const std::string& conditionCode);
    void emitCall(const ir::IRInstruction& instruction);

    void loadOperand(const std::string& operand, const std::string& reg);
    void storeOperand(const std::string& destination, const std::string& reg);
    std::string labelFor(const std::string& label) const;
    static std::string normalizeMemoryName(const std::string& operand);
    static bool isIntegerLiteral(const std::string& text);
    static bool isImmediate(const std::string& operand);
    static std::string immediateValue(const std::string& operand);
    static std::string sanitizeSymbol(const std::string& symbol);

    X86GeneratorOptions options_;
    SimpleRegisterAllocator registers_;
    StackFrame frame_;
    std::ostringstream out_;
    std::string currentFunction_;
};

std::string generateX86Assembly(const ir::IRProgram& program);

} // namespace codegen

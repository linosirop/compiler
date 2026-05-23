#pragma once

#include "ir_instructions.h"
#include "semantic/type_system.h"

#include <map>
#include <string>
#include <vector>

namespace ir {

struct BasicBlock {
    std::string label;
    std::vector<IRInstruction> instructions;
    std::vector<std::string> successors;

    explicit BasicBlock(std::string label = "entry");
    bool endsWithTerminator() const;
    void addInstruction(const IRInstruction& instruction);
    void addSuccessor(const std::string& label);
    std::string toText() const;
};

struct IRFunction {
    std::string name;
    semantic::Type returnType = semantic::Type::voidType();
    std::vector<std::pair<std::string, semantic::Type>> parameters;
    std::map<std::string, std::string> variableLocations;
    std::vector<BasicBlock> blocks;
    int tempCount = 0;
    int labelCount = 0;

    IRFunction() = default;
    explicit IRFunction(std::string functionName);

    BasicBlock& createBlock(const std::string& baseName);
    BasicBlock& currentBlock();
    const BasicBlock& currentBlock() const;
    std::string newTemp();
    std::string newLabel(const std::string& baseName);
    std::string localName(const std::string& sourceName);
    std::string toText() const;
    std::string toDot() const;
};

struct IRProgram {
    std::vector<IRFunction> functions;

    const IRFunction* getFunction(const std::string& name) const;
    IRFunction* getFunction(const std::string& name);
    std::string toText() const;
    std::string toDot() const;
    std::string statistics() const;
};

} // namespace ir

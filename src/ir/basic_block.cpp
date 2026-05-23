#include "basic_block.h"

#include <algorithm>
#include <sstream>

namespace ir {

BasicBlock::BasicBlock(std::string blockLabel) : label(std::move(blockLabel)) {}

bool BasicBlock::endsWithTerminator() const {
    return !instructions.empty() && instructions.back().isTerminator();
}

void BasicBlock::addInstruction(const IRInstruction& instruction) {
    instructions.push_back(instruction);
}

void BasicBlock::addSuccessor(const std::string& successorLabel) {
    if (std::find(successors.begin(), successors.end(), successorLabel) == successors.end()) {
        successors.push_back(successorLabel);
    }
}

std::string BasicBlock::toText() const {
    std::ostringstream out;
    out << "  " << label << ":\n";
    if (instructions.empty()) {
        out << "    NOP\n";
        return out.str();
    }
    for (const auto& instruction : instructions) {
        out << "    " << instruction.toString() << "\n";
    }
    return out.str();
}

IRFunction::IRFunction(std::string functionName) : name(std::move(functionName)) {
    blocks.emplace_back("entry");
}

BasicBlock& IRFunction::createBlock(const std::string& baseName) {
    blocks.emplace_back(baseName);
    return blocks.back();
}

BasicBlock& IRFunction::currentBlock() { return blocks.back(); }
const BasicBlock& IRFunction::currentBlock() const { return blocks.back(); }

std::string IRFunction::newTemp() {
    ++tempCount;
    return "t" + std::to_string(tempCount);
}

std::string IRFunction::newLabel(const std::string& baseName) {
    ++labelCount;
    return baseName + std::to_string(labelCount);
}

std::string IRFunction::localName(const std::string& sourceName) {
    auto found = variableLocations.find(sourceName);
    if (found != variableLocations.end()) return found->second;
    std::string location = sourceName + "_" + std::to_string(variableLocations.size());
    variableLocations[sourceName] = location;
    return location;
}

std::string IRFunction::toText() const {
    std::ostringstream out;
    out << "function " << name << ": " << returnType.toString() << " (";
    for (size_t i = 0; i < parameters.size(); ++i) {
        if (i > 0) out << ", ";
        out << parameters[i].second.toString() << " " << parameters[i].first;
    }
    out << ")\n";
    for (const auto& block : blocks) out << block.toText();
    if (!variableLocations.empty()) {
        out << "  # Variable mapping:\n";
        for (const auto& entry : variableLocations) {
            out << "  # " << entry.first << " -> [" << entry.second << "]\n";
        }
    }
    return out.str();
}

std::string IRFunction::toDot() const {
    std::ostringstream out;
    out << "  subgraph cluster_" << name << " {\n";
    out << "    label=\"function " << name << "\";\n";
    for (const auto& block : blocks) {
        out << "    \"" << name << "_" << block.label << "\" [shape=box, label=\"" << block.label;
        for (const auto& inst : block.instructions) {
            std::string text = inst.toString();
            for (char& ch : text) {
                if (ch == '"') ch = '\'';
            }
            out << "\\l" << text;
        }
        out << "\\l\"];\n";
    }
    for (const auto& block : blocks) {
        for (const auto& successor : block.successors) {
            out << "    \"" << name << "_" << block.label << "\" -> \"" << name << "_" << successor << "\";\n";
        }
    }
    out << "  }\n";
    return out.str();
}

const IRFunction* IRProgram::getFunction(const std::string& name) const {
    for (const auto& function : functions) {
        if (function.name == name) return &function;
    }
    return nullptr;
}

IRFunction* IRProgram::getFunction(const std::string& name) {
    for (auto& function : functions) {
        if (function.name == name) return &function;
    }
    return nullptr;
}

std::string IRProgram::toText() const {
    std::ostringstream out;
    out << "# MiniCompiler IR\n";
    for (const auto& function : functions) {
        out << "\n" << function.toText();
    }
    return out.str();
}

std::string IRProgram::toDot() const {
    std::ostringstream out;
    out << "digraph CFG {\n";
    out << "  rankdir=TB;\n";
    out << "  node [fontname=\"Consolas\"];\n";
    for (const auto& function : functions) out << function.toDot();
    out << "}\n";
    return out.str();
}

std::string IRProgram::statistics() const {
    std::ostringstream out;
    int blockCount = 0;
    int instructionCount = 0;
    int tempCount = 0;
    for (const auto& function : functions) {
        blockCount += static_cast<int>(function.blocks.size());
        tempCount += function.tempCount;
        for (const auto& block : function.blocks) {
            instructionCount += static_cast<int>(block.instructions.size());
        }
    }
    out << "IR Statistics:\n";
    out << "  Functions: " << functions.size() << "\n";
    out << "  Basic blocks: " << blockCount << "\n";
    out << "  Instructions: " << instructionCount << "\n";
    out << "  Temporaries: " << tempCount << "\n";
    return out.str();
}

} // namespace ir

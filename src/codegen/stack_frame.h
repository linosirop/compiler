#pragma once

#include "ir/basic_block.h"

#include <map>
#include <string>

namespace codegen {

class StackFrame {
public:
    void build(const ir::IRFunction& function);
    bool hasSlot(const std::string& name) const;
    int offsetOf(const std::string& name) const;
    std::string addressOf(const std::string& name) const;
    int stackSize() const;
    std::string dump() const;

private:
    void ensureSlot(const std::string& name);
    static std::string normalizeMemoryName(const std::string& operand);
    static bool isNamedStorage(const std::string& value);

    std::map<std::string, int> offsets_;
    int nextOffset_ = 0;
    int stackSize_ = 0;
};

} // namespace codegen

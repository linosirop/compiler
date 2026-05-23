#pragma once

#include "basic_block.h"

#include <string>

namespace ir {

bool allJumpTargetsExist(const IRProgram& program, std::string* errorMessage = nullptr);
bool everyBlockHasTerminator(const IRProgram& program, std::string* errorMessage = nullptr);
std::string validateControlFlow(const IRProgram& program);

} // namespace ir

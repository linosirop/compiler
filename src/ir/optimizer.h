#pragma once

#include "ir/basic_block.h"

#include <string>

namespace ir {

struct OptimizationReport {
    int constantFolds = 0;
    int constantsPropagated = 0;
    int deadInstructionsRemoved = 0;
    int branchSimplifications = 0;
    int iterations = 0;

    int totalChanges() const;
    std::string toString() const;
};

class Optimizer {
public:
    OptimizationReport optimize(IRProgram& program, int maxIterations = 4);

private:
    bool constantFold(IRProgram& program, OptimizationReport& report);
    bool constantPropagate(IRProgram& program, OptimizationReport& report);
    bool simplifyBranches(IRProgram& program, OptimizationReport& report);
    bool eliminateDeadCode(IRProgram& program, OptimizationReport& report);
};

OptimizationReport optimizeProgram(IRProgram& program, int maxIterations = 4);

} // namespace ir

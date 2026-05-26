#pragma once

#include "ir/optimizer.h"

namespace codegen {

using OptimizationReport = ir::OptimizationReport;

inline OptimizationReport runOptimizationPasses(ir::IRProgram& program) {
    return ir::optimizeProgram(program);
}

} // namespace codegen

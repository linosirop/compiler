#pragma once

#include <string>

namespace semantic {

struct SemanticError {
    std::string category;
    std::string message;
    int line = 1;
    int column = 1;
    std::string context;

    std::string toString(const std::string& fileName = "program.src") const;
};

} // namespace semantic

#pragma once

#include <string>

namespace codegen {

class LabelManager {
public:
    explicit LabelManager(std::string prefix = ".L") : prefix_(std::move(prefix)) {}

    std::string next(const std::string& hint) {
        return prefix_ + sanitize(hint) + "_" + std::to_string(counter_++);
    }

    void reset() { counter_ = 0; }

private:
    static std::string sanitize(const std::string& text);

    std::string prefix_;
    int counter_ = 0;
};

} // namespace codegen

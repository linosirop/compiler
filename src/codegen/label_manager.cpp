#include "codegen/label_manager.h"

#include <cctype>
#include <utility>

namespace codegen {

std::string LabelManager::sanitize(const std::string& text) {
    std::string result;
    result.reserve(text.size());
    for (char ch : text) {
        if (std::isalnum(static_cast<unsigned char>(ch)) || ch == '_') result.push_back(ch);
        else result.push_back('_');
    }
    return result.empty() ? "label" : result;
}

} // namespace codegen

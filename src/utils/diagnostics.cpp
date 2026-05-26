#include "utils/diagnostics.h"

#include <algorithm>
#include <sstream>

namespace diagnostics {

std::string severityName(Severity severity) {
    switch (severity) {
        case Severity::Note: return "note";
        case Severity::Warning: return "warning";
        case Severity::Error: return "error";
    }
    return "error";
}

namespace {
std::string colorPrefix(Severity severity) {
    switch (severity) {
        case Severity::Note: return "\033[36m";
        case Severity::Warning: return "\033[33m";
        case Severity::Error: return "\033[31m";
    }
    return "";
}
}

std::string formatDiagnostic(const Diagnostic& diagnostic, bool color) {
    std::ostringstream out;
    const std::string sev = severityName(diagnostic.severity);
    if (color) out << colorPrefix(diagnostic.severity);
    out << diagnostic.file << ":" << diagnostic.line << ":" << diagnostic.column << ": "
        << sev;
    if (!diagnostic.code.empty()) out << " " << diagnostic.code;
    out << ": " << diagnostic.message;
    if (color) out << "\033[0m";
    out << "\n";

    if (!diagnostic.sourceLine.empty()) {
        out << "   |\n";
        out << " " << diagnostic.line << " | " << diagnostic.sourceLine << "\n";
        out << "   | " << std::string(std::max(0, diagnostic.column - 1), ' ') << "^\n";
    }

    if (!diagnostic.note.empty()) {
        out << "note: " << diagnostic.note << "\n";
    }
    return out.str();
}

std::string formatSummary(int errors, int warnings) {
    std::ostringstream out;
    out << errors << " error" << (errors == 1 ? "" : "s") << ", "
        << warnings << " warning" << (warnings == 1 ? "" : "s") << " generated.\n";
    return out.str();
}

void DiagnosticBag::add(Diagnostic diagnostic) {
    diagnostics_.push_back(std::move(diagnostic));
}

void DiagnosticBag::error(const std::string& code,
                          const std::string& message,
                          const std::string& file,
                          int line,
                          int column,
                          const std::string& sourceLine,
                          const std::string& note) {
    add({Severity::Error, code, message, file, line, column, sourceLine, note});
}

void DiagnosticBag::warning(const std::string& code,
                            const std::string& message,
                            const std::string& file,
                            int line,
                            int column,
                            const std::string& sourceLine,
                            const std::string& note) {
    add({Severity::Warning, code, message, file, line, column, sourceLine, note});
}

bool DiagnosticBag::hasErrors() const {
    return errorCount() > 0;
}

int DiagnosticBag::errorCount() const {
    return static_cast<int>(std::count_if(diagnostics_.begin(), diagnostics_.end(), [](const Diagnostic& diagnostic) {
        return diagnostic.severity == Severity::Error;
    }));
}

int DiagnosticBag::warningCount() const {
    return static_cast<int>(std::count_if(diagnostics_.begin(), diagnostics_.end(), [](const Diagnostic& diagnostic) {
        return diagnostic.severity == Severity::Warning;
    }));
}

std::string DiagnosticBag::toString(bool color) const {
    std::ostringstream out;
    for (const auto& diagnostic : diagnostics_) {
        out << formatDiagnostic(diagnostic, color);
    }
    out << formatSummary(errorCount(), warningCount());
    return out.str();
}

} // namespace diagnostics

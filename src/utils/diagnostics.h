#pragma once

#include <string>
#include <vector>

namespace diagnostics {

enum class Severity {
    Note,
    Warning,
    Error
};

struct Diagnostic {
    Severity severity{Severity::Error};
    std::string code;
    std::string message;
    std::string file;
    int line{1};
    int column{1};
    std::string sourceLine;
    std::string note;
};

std::string severityName(Severity severity);
std::string formatDiagnostic(const Diagnostic& diagnostic, bool color = false);
std::string formatSummary(int errors, int warnings);

class DiagnosticBag {
public:
    void add(Diagnostic diagnostic);
    void error(const std::string& code,
               const std::string& message,
               const std::string& file,
               int line = 1,
               int column = 1,
               const std::string& sourceLine = "",
               const std::string& note = "");
    void warning(const std::string& code,
                 const std::string& message,
                 const std::string& file,
                 int line = 1,
                 int column = 1,
                 const std::string& sourceLine = "",
                 const std::string& note = "");

    bool hasErrors() const;
    int errorCount() const;
    int warningCount() const;
    std::string toString(bool color = false) const;

private:
    std::vector<Diagnostic> diagnostics_;
};

} // namespace diagnostics

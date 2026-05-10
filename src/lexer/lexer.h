#pragma once

#include "token.h"
#include <string>
#include <vector>
#include <unordered_map>

class Lexer {
public:
    explicit Lexer(const std::string& source);

    Token next_token();
    Token peek_token() const;
    bool is_at_end() const;

    int get_line() const { return current_line_; }
    int get_column() const { return current_column_; }

    const std::vector<Token>& all_tokens() const { return tokens_; }
    const std::vector<std::string>& get_errors() const { return errors_; }

private:
    std::string source_;
    size_t pos = 0;
    int current_line_ = 1;
    int current_column_ = 1;
    std::vector<Token> tokens_;
    size_t token_index_ = 0;
    std::vector<std::string> errors_;

    // Helper methods
    char advance();
    char peek() const;
    char peek_next() const;
    bool at_end() const;

    void add_token(TokenType type,
        std::string lexeme = "",
        std::variant<std::monostate, long long, double, bool, std::string> literal = {});

    void scan_token();
    void skip_whitespace_and_comments();

    void scan_number();
    void scan_identifier_or_keyword();
    void scan_string();

    void report_error(const std::string& message);
    void report_error_at(int line, int column, const std::string& message);
};
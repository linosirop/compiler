#include "lexer.h"
#include <cctype>
#include <sstream>
#include <limits>

static const std::unordered_map<std::string, TokenType> keywords = {
    {"if",     TokenType::KW_IF},
    {"else",   TokenType::KW_ELSE},
    {"while",  TokenType::KW_WHILE},
    {"for",    TokenType::KW_FOR},
    {"int",    TokenType::KW_INT},
    {"float",  TokenType::KW_FLOAT},
    {"bool",   TokenType::KW_BOOL},
    {"return", TokenType::KW_RETURN},
    {"void",   TokenType::KW_VOID},
    {"struct", TokenType::KW_STRUCT},
    {"fn",     TokenType::KW_FN},
    {"extern", TokenType::KW_EXTERN},
};

Lexer::Lexer(const std::string& source) : source_(source) {
    while (!at_end()) {
        skip_whitespace_and_comments();
        if (at_end()) break;
        scan_token();
    }
    add_token(TokenType::END_OF_FILE, "");
}

char Lexer::advance() {
    if (at_end()) return '\0';
    current_column_++;
    return source_[pos++];
}

char Lexer::peek() const {
    if (at_end()) return '\0';
    return source_[pos];
}

char Lexer::peek_next() const {
    if (pos + 1 >= source_.size()) return '\0';
    return source_[pos + 1];
}

bool Lexer::at_end() const {
    return pos >= source_.size();
}

void Lexer::add_token(TokenType type, std::string lexeme,
    std::variant<std::monostate, long long, double, bool, std::string> literal) {
    if (lexeme.empty()) {
        lexeme = source_.substr(tokens_.empty() ? 0 : pos - 1, 1);
    }
    int start_column = current_column_ - static_cast<int>(lexeme.length());
    tokens_.emplace_back(Token{ type, lexeme, literal, current_line_, start_column });
}

void Lexer::report_error(const std::string& message) {
    report_error_at(current_line_, current_column_, message);
}

void Lexer::report_error_at(int line, int column, const std::string& message) {
    std::ostringstream oss;
    oss << "Error at " << line << ":" << column << ": " << message;
    errors_.push_back(oss.str());

    // Простое восстановление — пропускаем до конца строки или пробела
    while (!at_end() && peek() != '\n' && !std::isspace(static_cast<unsigned char>(peek()))) {
        advance();
    }
}

void Lexer::skip_whitespace_and_comments() {
    while (!at_end()) {
        char c = peek();
        if (c == ' ' || c == '\t' || c == '\r') {
            advance();
            continue;
        }
        if (c == '\n') {
            advance();
            current_line_++;
            current_column_ = 1;
            continue;
        }

        // Комментарии
        if (c == '/' && peek_next() == '/') {  // //
            advance(); advance();
            while (!at_end() && peek() != '\n') advance();
            continue;
        }
        if (c == '/' && peek_next() == '*') {  // /*
            advance(); advance();
            bool closed = false;
            while (!at_end()) {
                if (peek() == '*' && peek_next() == '/') {
                    advance(); advance();
                    closed = true;
                    break;
                }
                if (peek() == '\n') {
                    advance();
                    current_line_++;
                    current_column_ = 1;
                    continue;
                }
                advance();
            }
            if (!closed) report_error("Unterminated multi-line comment");
            continue;
        }

        break;
    }
}

void Lexer::scan_token() {
    char c = advance();

    if (std::isdigit(c)) {
        scan_number();
        return;
    }

    if (std::isalpha(static_cast<unsigned char>(c))) {
        scan_identifier_or_keyword();
        return;
    }

    if (c == '"') {
        scan_string();
        return;
    }

    // Операторы и пунктуация
    switch (c) {
    case '+':
        if (peek() == '=') { advance(); add_token(TokenType::PLUS_EQUAL, "+="); }
        else add_token(TokenType::PLUS, "+");
        break;
    case '-':
        if (peek() == '=') { advance(); add_token(TokenType::MINUS_EQUAL, "-="); }
        else if (peek() == '>') { advance(); add_token(TokenType::ARROW, "->"); }
        else add_token(TokenType::MINUS, "-");
        break;
    case '*':
        if (peek() == '=') { advance(); add_token(TokenType::STAR_EQUAL, "*="); }
        else add_token(TokenType::STAR, "*");
        break;
    case '/':
        if (peek() == '=') { advance(); add_token(TokenType::SLASH_EQUAL, "/="); }
        else add_token(TokenType::SLASH, "/");
        break;
    case '%': add_token(TokenType::PERCENT, "%"); break;
    case '&':
        if (peek() == '&') { advance(); add_token(TokenType::AND_AND, "&&"); }
        else add_token(TokenType::AMPERSAND, "&");
        break;
    case '|':
        if (peek() == '|') { advance(); add_token(TokenType::OR_OR, "||"); }
        else report_error("Unexpected character '|'");
        break;

    case '=':
        if (peek() == '=') { advance(); add_token(TokenType::EQUAL_EQUAL, "=="); }
        else add_token(TokenType::ASSIGN, "=");
        break;

    case '!':
        if (peek() == '=') { advance(); add_token(TokenType::BANG_EQUAL, "!="); }
        else add_token(TokenType::BANG, "!");
        break;

    case '<':
        if (peek() == '=') { advance(); add_token(TokenType::LESS_EQUAL, "<="); }
        else add_token(TokenType::LESS, "<");
        break;

    case '>':
        if (peek() == '=') { advance(); add_token(TokenType::GREATER_EQUAL, ">="); }
        else add_token(TokenType::GREATER, ">");
        break;

    case '(': add_token(TokenType::LPAREN, "("); break;
    case ')': add_token(TokenType::RPAREN, ")"); break;
    case '{': add_token(TokenType::LBRACE, "{"); break;
    case '}': add_token(TokenType::RBRACE, "}"); break;
    case '[': add_token(TokenType::LBRACKET, "["); break;
    case ']': add_token(TokenType::RBRACKET, "]"); break;
    case ';': add_token(TokenType::SEMICOLON, ";"); break;
    case ',': add_token(TokenType::COMMA, ","); break;
    case ':': add_token(TokenType::COLON, ":"); break;

    default:
        report_error_at(current_line_, current_column_ - 1, std::string("Unexpected character '") + c + "'");
        break;
    }
}

void Lexer::scan_number() {
    size_t start = pos - 1;
    bool is_float = false;

    while (!at_end() && std::isdigit(peek())) advance();

    if (peek() == '.') {
        if (!std::isdigit(static_cast<unsigned char>(peek_next()))) {
            report_error_at(current_line_, current_column_, "Malformed floating-point literal: expected digit after '.'");
            return;
        }
        is_float = true;
        advance();
        while (!at_end() && std::isdigit(static_cast<unsigned char>(peek()))) advance();
    }

    std::string num_str = source_.substr(start, pos - start);

    if (is_float) {
        try {
            double val = std::stod(num_str);
            add_token(TokenType::FLOAT_LITERAL, num_str, val);
        }
        catch (...) {
            report_error("Invalid floating-point literal");
        }
    }
    else {
        try {
            long long val = std::stoll(num_str);
            if (val > std::numeric_limits<int>::max() || val < std::numeric_limits<int>::min()) {
                report_error("Integer literal out of 32-bit signed range");
                return;
            }
            add_token(TokenType::INT_LITERAL, num_str, val);
        }
        catch (...) {
            report_error("Invalid integer literal");
        }
    }
}

void Lexer::scan_identifier_or_keyword() {
    size_t start = pos - 1;
    while (!at_end() && (std::isalnum(peek()) || peek() == '_')) advance();

    std::string id = source_.substr(start, pos - start);

    if (id.length() > 255) {
        report_error("Identifier longer than 255 characters");
        return;
    }

    // Проверяем специальные литералы true / false
    if (id == "true") {
        add_token(TokenType::BOOL_LITERAL, id, true);
        return;
    }
    if (id == "false") {
        add_token(TokenType::BOOL_LITERAL, id, false);
        return;
    }

    // Обычные ключевые слова
    auto it = keywords.find(id);
    if (it != keywords.end()) {
        add_token(it->second, id);
    }
    else {
        add_token(TokenType::IDENTIFIER, id);
    }
}

void Lexer::scan_string() {
    size_t start = pos - 1;
    int start_line = current_line_;
    int start_col = current_column_ - 1;
    std::string value;

    while (!at_end() && peek() != '"') {
        if (peek() == '\n') {
            report_error("Unterminated string literal (newline inside)");
            return;
        }
        value += advance();
    }

    if (at_end() || peek() != '"') {
        report_error("Unterminated string literal");
        return;
    }

    advance();  // закрывающая "

    std::string lexeme = source_.substr(start, pos - start);
    add_token(TokenType::STRING_LITERAL, lexeme, value);
    tokens_.back().line = start_line;
    tokens_.back().column = start_col;
}

Token Lexer::next_token() {
    if (token_index_ >= tokens_.size()) {
        return Token{ TokenType::END_OF_FILE, "", {}, current_line_, current_column_ };
    }
    return tokens_[token_index_++];
}

Token Lexer::peek_token() const {
    if (token_index_ >= tokens_.size()) {
        return Token{ TokenType::END_OF_FILE, "", {}, current_line_, current_column_ };
    }
    return tokens_[token_index_];
}

bool Lexer::is_at_end() const {
    return token_index_ >= tokens_.size();
}
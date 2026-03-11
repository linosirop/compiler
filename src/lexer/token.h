#pragma once

#include <string>
#include <variant>

enum class TokenType {
    KW_IF, KW_ELSE, KW_WHILE, KW_FOR,
    KW_INT, KW_FLOAT, KW_BOOL, KW_RETURN,
    KW_VOID, KW_STRUCT, KW_FN,
    KW_TRUE, KW_FALSE,

    INT_LITERAL,
    FLOAT_LITERAL,
    STRING_LITERAL,
    BOOL_LITERAL,

    IDENTIFIER,

    PLUS, MINUS, STAR, SLASH, PERCENT,
    AMPERSAND,
    EQUAL_EQUAL, BANG_EQUAL,
    LESS, LESS_EQUAL, GREATER, GREATER_EQUAL,
    PLUS_EQUAL, ASSIGN,

    LPAREN, RPAREN, LBRACE, RBRACE,
    SEMICOLON, COMMA,

    END_OF_FILE
};

struct Token {
    TokenType type;
    std::string lexeme;
    std::variant<std::monostate, long long, double, bool, std::string> literal;
    int line;
    int column;  // позиция начала токена

    std::string to_string() const;
};
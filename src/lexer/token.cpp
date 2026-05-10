#include "token.h"
#include <sstream>

std::string Token::to_string() const {
    std::ostringstream oss;
    oss << line << ":" << column << " ";

    switch (type) {
    case TokenType::KW_IF:     oss << "KW_IF";     break;
    case TokenType::KW_ELSE:   oss << "KW_ELSE";   break;
    case TokenType::KW_WHILE:  oss << "KW_WHILE";  break;
    case TokenType::KW_FOR:    oss << "KW_FOR";    break;
    case TokenType::KW_INT:    oss << "KW_INT";    break;
    case TokenType::KW_FLOAT:  oss << "KW_FLOAT";  break;
    case TokenType::KW_BOOL:   oss << "KW_BOOL";   break;
    case TokenType::KW_RETURN: oss << "KW_RETURN"; break;
    case TokenType::KW_VOID:   oss << "KW_VOID";   break;
    case TokenType::KW_STRUCT: oss << "KW_STRUCT"; break;
    case TokenType::KW_FN:     oss << "KW_FN";     break;
    case TokenType::KW_TRUE:   oss << "KW_TRUE";   break;
    case TokenType::KW_FALSE:  oss << "KW_FALSE";  break;

    case TokenType::INT_LITERAL:    oss << "INT_LITERAL";    break;
    case TokenType::FLOAT_LITERAL:  oss << "FLOAT_LITERAL";  break;
    case TokenType::STRING_LITERAL: oss << "STRING_LITERAL"; break;
    case TokenType::BOOL_LITERAL:   oss << "BOOL_LITERAL";   break;
    case TokenType::IDENTIFIER:     oss << "IDENTIFIER";     break;

    case TokenType::PLUS:         oss << "PLUS";         break;
    case TokenType::MINUS:        oss << "MINUS";        break;
    case TokenType::STAR:         oss << "STAR";         break;
    case TokenType::SLASH:        oss << "SLASH";        break;
    case TokenType::PERCENT:      oss << "PERCENT";      break;
    case TokenType::AMPERSAND:    oss << "AMPERSAND";    break;
    case TokenType::AND_AND:      oss << "AND_AND";      break;
    case TokenType::OR_OR:        oss << "OR_OR";        break;
    case TokenType::BANG:         oss << "BANG";         break;
    case TokenType::EQUAL_EQUAL:  oss << "EQUAL_EQUAL";  break;
    case TokenType::BANG_EQUAL:   oss << "BANG_EQUAL";   break;
    case TokenType::LESS:         oss << "LESS";         break;
    case TokenType::LESS_EQUAL:   oss << "LESS_EQUAL";   break;
    case TokenType::GREATER:      oss << "GREATER";      break;
    case TokenType::GREATER_EQUAL:oss << "GREATER_EQUAL"; break;
    case TokenType::PLUS_EQUAL:   oss << "PLUS_EQUAL";   break;
    case TokenType::MINUS_EQUAL:  oss << "MINUS_EQUAL";  break;
    case TokenType::STAR_EQUAL:   oss << "STAR_EQUAL";   break;
    case TokenType::SLASH_EQUAL:  oss << "SLASH_EQUAL";  break;
    case TokenType::ASSIGN:       oss << "ASSIGN";       break;
    case TokenType::ARROW:        oss << "ARROW";        break;

    case TokenType::LPAREN:    oss << "LPAREN";    break;
    case TokenType::RPAREN:    oss << "RPAREN";    break;
    case TokenType::LBRACE:    oss << "LBRACE";    break;
    case TokenType::RBRACE:    oss << "RBRACE";    break;
    case TokenType::LBRACKET:  oss << "LBRACKET";  break;
    case TokenType::RBRACKET:  oss << "RBRACKET";  break;
    case TokenType::SEMICOLON: oss << "SEMICOLON"; break;
    case TokenType::COMMA:     oss << "COMMA";     break;
    case TokenType::COLON:     oss << "COLON";     break;

    case TokenType::END_OF_FILE: oss << "END_OF_FILE"; break;
    }

    if (type == TokenType::END_OF_FILE) {
        oss << " \"\"";
    }
    else {
        oss << " \"" << lexeme << "\"";
    }

    if (!std::holds_alternative<std::monostate>(literal)) {
        if (std::holds_alternative<long long>(literal)) {
            oss << " " << std::get<long long>(literal);
        }
        else if (std::holds_alternative<double>(literal)) {
            oss << " " << std::get<double>(literal);
        }
        else if (std::holds_alternative<bool>(literal)) {
            oss << " " << (std::get<bool>(literal) ? "true" : "false");
        }
        else if (std::holds_alternative<std::string>(literal)) {
            oss << " \"" << std::get<std::string>(literal) << "\"";
        }
    }

    return oss.str();
}
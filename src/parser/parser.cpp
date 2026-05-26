#include "parser.h"

#include <sstream>
#include <stdexcept>

Parser::Parser(const std::vector<Token>& tokens) : tokens_(tokens) {}

ProgramNode Parser::parse() {
    ProgramNode program;
    if (!tokens_.empty()) {
        program.line = tokens_.front().line;
        program.column = tokens_.front().column;
    }

    while (!isAtEnd()) {
        // ❗ пропускаем лишние }
        if (check(TokenType::RBRACE)) {
            advance();
            continue;
        }

        try {
            auto decl = parseDeclaration();
            if (decl) program.declarations.push_back(decl);
        }
        catch (const std::runtime_error&) {
            synchronize();
        }
    }

    return program;
}

bool Parser::isAtEnd() const { return peek().type == TokenType::END_OF_FILE; }
const Token& Parser::peek() const { return tokens_[current_]; }
const Token& Parser::previous() const { return tokens_[current_ - 1]; }

const Token& Parser::advance() {
    if (!isAtEnd()) ++current_;
    return previous();
}

bool Parser::check(TokenType type) const {
    if (isAtEnd()) return false;
    return peek().type == type;
}

bool Parser::match(TokenType type) {
    if (!check(type)) return false;
    advance();
    return true;
}

bool Parser::matchAny(std::initializer_list<TokenType> types) {
    for (TokenType type : types) {
        if (check(type)) {
            advance();
            return true;
        }
    }
    return false;
}

const Token& Parser::consume(TokenType type, const std::string& message) {
    if (check(type)) return advance();
    errorAtCurrent(message);
    throw std::runtime_error(message);
}

void Parser::errorAtCurrent(const std::string& message) {
    std::ostringstream oss;
    oss << "Syntax error at " << peek().line << ":" << peek().column << ": " << message
        << ", got '" << peek().lexeme << "'";
    errors_.push_back(oss.str());
}

void Parser::synchronize() {
    // всегда двигаемся хотя бы на 1 токен
    advance();

    while (!isAtEnd()) {
        // если нашли ; → конец statement
        if (previous().type == TokenType::SEMICOLON)
            return;

        switch (peek().type) {
        case TokenType::KW_FN:
        case TokenType::KW_EXTERN:
        case TokenType::KW_STRUCT:
        case TokenType::KW_IF:
        case TokenType::KW_WHILE:
        case TokenType::KW_FOR:
        case TokenType::KW_RETURN:
        case TokenType::KW_INT:
        case TokenType::KW_FLOAT:
        case TokenType::KW_BOOL:
        case TokenType::KW_VOID:
        case TokenType::RBRACE:
            return;
        default:
            advance();
            break;
        }
    }
}

bool Parser::isTypeToken(TokenType type) const {
    return type == TokenType::KW_INT || type == TokenType::KW_FLOAT ||
           type == TokenType::KW_BOOL || type == TokenType::KW_VOID ||
           type == TokenType::IDENTIFIER;
}

std::string Parser::parseTypeName() {
    if (!isTypeToken(peek().type)) {
        errorAtCurrent("Expected type name");
        throw std::runtime_error("Expected type name");
    }
    return advance().lexeme;
}

ParamNode Parser::parseParameter() {
    Token typeToken = peek();
    ParamNode param;
    param.type = parseTypeName();
    const Token& name = consume(TokenType::IDENTIFIER, "Expected parameter name");
    param.name = name.lexeme;
    if (match(TokenType::LBRACKET)) {
        consume(TokenType::RBRACKET, "Expected ] after array parameter");
        param.type += "[]";
    }
    param.line = typeToken.line;
    param.column = typeToken.column;
    return param;
}

DeclPtr Parser::parseDeclaration() {
    if (match(TokenType::KW_FN)) return parseFunctionDeclaration();
    if (match(TokenType::KW_EXTERN)) return parseExternalDeclaration();
    if (match(TokenType::KW_STRUCT)) return parseStructDeclaration();

    if (isTypeToken(peek().type)) {
        Token typeToken = advance();
        auto var = parseVariableDeclarationAfterType(typeToken.lexeme, typeToken);
        auto global = std::make_shared<GlobalVarDeclNode>();
        global->declaration = var;
        global->line = var->line;
        global->column = var->column;
        return global;
    }

    errorAtCurrent("Expected declaration");
    throw std::runtime_error("Expected declaration");
}

DeclPtr Parser::parseExternalDeclaration() {
    Token externToken = previous();

    // MiniCompiler accepts both forms:
    //   extern fn printf(string fmt, int value) -> int;
    //   extern int printf(string fmt, int value);
    bool sawFn = match(TokenType::KW_FN);
    std::string returnType = "void";
    if (!sawFn) {
        returnType = parseTypeName();
    }

    const Token& name = consume(TokenType::IDENTIFIER, "Expected external function name");
    consume(TokenType::LPAREN, "Expected '(' after external function name");

    std::vector<ParamNode> params;
    bool variadic = false;
    if (!check(TokenType::RPAREN)) {
        do {
            if (check(TokenType::RBRACE)) break;
            if (check(TokenType::COMMA) && tokens_[current_ + 1].lexeme == "...") {
                advance();
                variadic = true;
                break;
            }
            if (peek().lexeme == "...") {
                advance();
                variadic = true;
                break;
            }
            params.push_back(parseParameter());
        } while (match(TokenType::COMMA));
    }
    consume(TokenType::RPAREN, "Expected ')' after external parameters");

    if (sawFn && match(TokenType::ARROW)) {
        returnType = parseTypeName();
    }
    consume(TokenType::SEMICOLON, "Expected ';' after external declaration");

    auto fn = std::make_shared<FunctionDeclNode>();
    fn->line = externToken.line;
    fn->column = externToken.column;
    fn->name = name.lexeme;
    fn->returnType = returnType;
    fn->parameters = std::move(params);
    fn->isExtern = true;
    fn->isVariadic = variadic;
    return fn;
}

DeclPtr Parser::parseFunctionDeclaration() {
    Token fnToken = previous();
    const Token& name = consume(TokenType::IDENTIFIER, "Expected function name");
    consume(TokenType::LPAREN, "Expected '(' after function name");

    std::vector<ParamNode> params;
    if (!check(TokenType::RPAREN)) {
        do {
            params.push_back(parseParameter());
        } while (match(TokenType::COMMA));
    }
    consume(TokenType::RPAREN, "Expected ')' after parameters");

    std::string returnType = "void";
    if (match(TokenType::ARROW)) {
        returnType = parseTypeName();
    }

    auto body = parseBlock();

    auto fn = std::make_shared<FunctionDeclNode>();
    fn->line = fnToken.line;
    fn->column = fnToken.column;
    fn->name = name.lexeme;
    fn->returnType = returnType;
    fn->parameters = std::move(params);
    fn->body = body;
    return fn;
}

DeclPtr Parser::parseStructDeclaration() {
    Token structToken = previous();
    const Token& name = consume(TokenType::IDENTIFIER, "Expected struct name");
    consume(TokenType::LBRACE, "Expected '{' after struct name");

    auto st = std::make_shared<StructDeclNode>();
    st->line = structToken.line;
    st->column = structToken.column;
    st->name = name.lexeme;

    while (!check(TokenType::RBRACE) && !isAtEnd()) {
        Token typeToken = peek();
        std::string type = parseTypeName();
        st->fields.push_back(parseVariableDeclarationAfterType(type, typeToken));
    }

    consume(TokenType::RBRACE, "Expected '}' after struct fields");
    return st;
}

std::shared_ptr<VarDeclStmtNode> Parser::parseVariableDeclarationAfterType(const std::string& type, const Token& typeToken) {
    const Token& name = consume(TokenType::IDENTIFIER, "Expected variable name");
    ExprPtr initializer = nullptr;
    std::vector<int> dimensions;
    std::vector<ExprPtr> arrayInitializers;

    while (match(TokenType::LBRACKET)) {
        int size = 0;
        if (check(TokenType::INT_LITERAL)) {
            size = std::stoi(advance().lexeme);
        }
        consume(TokenType::RBRACKET, "Expected ']' after array size");
        dimensions.push_back(size);
    }

    if (match(TokenType::ASSIGN)) {
        if (!dimensions.empty() && match(TokenType::LBRACE)) {
            if (!check(TokenType::RBRACE)) {
                do {
                    arrayInitializers.push_back(parseExpression());
                } while (match(TokenType::COMMA));
            }
            consume(TokenType::RBRACE, "Expected '}' after array initializer");
        } else {
            initializer = parseExpression();
        }
    }
    consume(TokenType::SEMICOLON, "Expected ';' after variable declaration");

    auto var = std::make_shared<VarDeclStmtNode>(type, name.lexeme, initializer);
    var->arrayDimensions = std::move(dimensions);
    var->arrayInitializers = std::move(arrayInitializers);
    var->line = typeToken.line;
    var->column = typeToken.column;
    return var;
}

StmtPtr Parser::parseStatement() {
    if (check(TokenType::LBRACE)) {
        return parseBlock();
    }
    if (match(TokenType::KW_IF)) return parseIfStatement();
    if (match(TokenType::KW_WHILE)) return parseWhileStatement();
    if (match(TokenType::KW_FOR)) return parseForStatement();
    if (match(TokenType::KW_RETURN)) return parseReturnStatement();
    if (match(TokenType::SEMICOLON)) {
        auto stmt = std::make_shared<EmptyStmtNode>();
        stmt->line = previous().line;
        stmt->column = previous().column;
        return stmt;
    }
    if (isTypeToken(peek().type)) {
        Token next = tokens_[current_ + 1];
        if (next.type == TokenType::IDENTIFIER) {
            Token typeToken = advance();
            return parseVariableDeclarationAfterType(typeToken.lexeme, typeToken);
        }
    }
    return parseExpressionStatement();
}

std::shared_ptr<BlockStmtNode> Parser::parseBlock() {
    const Token& brace = consume(TokenType::LBRACE, "Expected '{'");

    auto block = std::make_shared<BlockStmtNode>();
    block->line = brace.line;
    block->column = brace.column;

    bool hadError = false; // ❗ новый флаг

    while (!check(TokenType::RBRACE) && !isAtEnd()) {
        try {
            block->statements.push_back(parseStatement());
        }
        catch (const std::runtime_error&) {
            hadError = true; // ❗ запоминаем
            synchronize();
        }
    }

    // ❗ ключевая логика
    if (!hadError) {
        consume(TokenType::RBRACE, "Expected '}' after block");
    }
    else {
        // если уже была ошибка — просто пытаемся съесть }
        if (check(TokenType::RBRACE)) {
            advance();
        }
    }

    return block;
}

StmtPtr Parser::parseIfStatement() {
    Token ifToken = previous();

    try {
        consume(TokenType::LPAREN, "Expected '(' after if");

        ExprPtr condition = parseExpression();

        consume(TokenType::RPAREN, "Expected ')' after if condition");

        StmtPtr thenBranch = parseStatement();

        StmtPtr elseBranch = nullptr;
        if (match(TokenType::KW_ELSE)) {
            elseBranch = parseStatement();
        }

        auto stmt = std::make_shared<IfStmtNode>();
        stmt->line = ifToken.line;
        stmt->column = ifToken.column;
        stmt->condition = condition;
        stmt->thenBranch = thenBranch;
        stmt->elseBranch = elseBranch;
        return stmt;
    }
    catch (const std::runtime_error&) {
        synchronize();

        // ❗ возвращаем пустой statement, чтобы не зациклиться
        auto stmt = std::make_shared<EmptyStmtNode>();
        stmt->line = ifToken.line;
        stmt->column = ifToken.column;
        return stmt;
    }
}

StmtPtr Parser::parseWhileStatement() {
    Token whileToken = previous();
    consume(TokenType::LPAREN, "Expected '(' after while");
    ExprPtr condition = parseExpression();
    consume(TokenType::RPAREN, "Expected ')' after while condition");
    StmtPtr body = parseStatement();

    auto stmt = std::make_shared<WhileStmtNode>();
    stmt->line = whileToken.line;
    stmt->column = whileToken.column;
    stmt->condition = condition;
    stmt->body = body;
    return stmt;
}

StmtPtr Parser::parseForStatement() {
    Token forToken = previous();
    consume(TokenType::LPAREN, "Expected '(' after for");

    StmtPtr init = nullptr;
    if (match(TokenType::SEMICOLON)) {
        auto empty = std::make_shared<EmptyStmtNode>();
        empty->line = previous().line;
        empty->column = previous().column;
        init = empty;
    } else if (isTypeToken(peek().type) && tokens_[current_ + 1].type == TokenType::IDENTIFIER) {
        Token typeToken = advance();
        init = parseVariableDeclarationAfterType(typeToken.lexeme, typeToken);
    } else {
        init = parseExpressionStatement();
    }

    ExprPtr condition = nullptr;
    if (!check(TokenType::SEMICOLON)) condition = parseExpression();
    consume(TokenType::SEMICOLON, "Expected ';' after for condition");

    ExprPtr update = nullptr;
    if (!check(TokenType::RPAREN)) update = parseExpression();
    consume(TokenType::RPAREN, "Expected ')' after for clauses");

    StmtPtr body = parseStatement();

    auto stmt = std::make_shared<ForStmtNode>();
    stmt->line = forToken.line;
    stmt->column = forToken.column;
    stmt->init = init;
    stmt->condition = condition;
    stmt->update = update;
    stmt->body = body;
    return stmt;
}

StmtPtr Parser::parseReturnStatement() {
    Token returnToken = previous();
    ExprPtr value = nullptr;
    if (!check(TokenType::SEMICOLON)) value = parseExpression();
    consume(TokenType::SEMICOLON, "Expected ';' after return value");

    auto stmt = std::make_shared<ReturnStmtNode>();
    stmt->line = returnToken.line;
    stmt->column = returnToken.column;
    stmt->value = value;
    return stmt;
}

StmtPtr Parser::parseExpressionStatement() {
    ExprPtr expr = parseExpression();
    consume(TokenType::SEMICOLON, "Expected ';' after expression");
    auto stmt = std::make_shared<ExprStmtNode>(expr);
    stmt->line = expr ? expr->line : peek().line;
    stmt->column = expr ? expr->column : peek().column;
    return stmt;
}

ExprPtr Parser::parseExpression() { return parseAssignment(); }

ExprPtr Parser::parseAssignment() {
    ExprPtr expr = parseLogicalOr();
    if (matchAny({TokenType::ASSIGN, TokenType::PLUS_EQUAL, TokenType::MINUS_EQUAL, TokenType::STAR_EQUAL, TokenType::SLASH_EQUAL})) {
        Token op = previous();

        if (!std::dynamic_pointer_cast<IdentifierExprNode>(expr) && !std::dynamic_pointer_cast<ArrayAccessExprNode>(expr)) {
            errors_.push_back("Syntax error at " + std::to_string(op.line) + ":" + std::to_string(op.column) +
                ": Invalid assignment target");
            throw std::runtime_error("Invalid assignment target");
        }

        ExprPtr value = parseAssignment();
        auto assign = std::make_shared<AssignmentExprNode>(expr, op.lexeme, value);
        assign->line = op.line;
        assign->column = op.column;
        return assign;
    }
    return expr;
}

ExprPtr Parser::parseLogicalOr() {
    ExprPtr expr = parseLogicalAnd();
    while (match(TokenType::OR_OR)) {
        Token op = previous();
        auto node = std::make_shared<BinaryExprNode>(expr, op.lexeme, parseLogicalAnd());
        node->line = op.line; node->column = op.column; expr = node;
    }
    return expr;
}

ExprPtr Parser::parseLogicalAnd() {
    ExprPtr expr = parseEquality();
    while (match(TokenType::AND_AND)) {
        Token op = previous();
        auto node = std::make_shared<BinaryExprNode>(expr, op.lexeme, parseEquality());
        node->line = op.line; node->column = op.column; expr = node;
    }
    return expr;
}

ExprPtr Parser::parseEquality() {
    ExprPtr expr = parseComparison();
    if (matchAny({TokenType::EQUAL_EQUAL, TokenType::BANG_EQUAL})) {
        Token op = previous();
        auto node = std::make_shared<BinaryExprNode>(expr, op.lexeme, parseComparison());
        node->line = op.line; node->column = op.column; expr = node;

        if (check(TokenType::EQUAL_EQUAL) || check(TokenType::BANG_EQUAL)) {
            errorAtCurrent("Chained equality operators are not allowed");
            throw std::runtime_error("Chained equality operators are not allowed");
        }
    }
    return expr;
}

ExprPtr Parser::parseComparison() {
    ExprPtr expr = parseTerm();
    if (matchAny({TokenType::LESS, TokenType::LESS_EQUAL, TokenType::GREATER, TokenType::GREATER_EQUAL})) {
        Token op = previous();
        auto node = std::make_shared<BinaryExprNode>(expr, op.lexeme, parseTerm());
        node->line = op.line; node->column = op.column; expr = node;

        if (check(TokenType::LESS) || check(TokenType::LESS_EQUAL) ||
            check(TokenType::GREATER) || check(TokenType::GREATER_EQUAL)) {
            errorAtCurrent("Chained comparison operators are not allowed");
            throw std::runtime_error("Chained comparison operators are not allowed");
        }
    }
    return expr;
}

ExprPtr Parser::parseTerm() {
    ExprPtr expr = parseFactor();
    while (matchAny({TokenType::PLUS, TokenType::MINUS})) {
        Token op = previous();
        auto node = std::make_shared<BinaryExprNode>(expr, op.lexeme, parseFactor());
        node->line = op.line; node->column = op.column; expr = node;
    }
    return expr;
}

ExprPtr Parser::parseFactor() {
    ExprPtr expr = parseUnary();
    while (matchAny({TokenType::STAR, TokenType::SLASH, TokenType::PERCENT})) {
        Token op = previous();
        auto node = std::make_shared<BinaryExprNode>(expr, op.lexeme, parseUnary());
        node->line = op.line; node->column = op.column; expr = node;
    }
    return expr;
}

ExprPtr Parser::parseUnary() {
    if (matchAny({TokenType::MINUS, TokenType::BANG})) {
        Token op = previous();
        auto node = std::make_shared<UnaryExprNode>(op.lexeme, parseUnary());
        node->line = op.line; node->column = op.column;
        return node;
    }
    return parsePrimary();
}

ExprPtr Parser::parsePrimary() {
    if (matchAny({TokenType::INT_LITERAL, TokenType::FLOAT_LITERAL, TokenType::STRING_LITERAL, TokenType::BOOL_LITERAL, TokenType::KW_TRUE, TokenType::KW_FALSE})) {
        Token token = previous();
        auto node = std::make_shared<LiteralExprNode>(token.lexeme);
        node->line = token.line; node->column = token.column;
        return node;
    }

    if (match(TokenType::IDENTIFIER)) {
        Token name = previous();
        ExprPtr expr;
        if (match(TokenType::LPAREN)) {
            std::vector<ExprPtr> args;
            if (!check(TokenType::RPAREN)) {
                do { args.push_back(parseExpression()); } while (match(TokenType::COMMA));
            }
            consume(TokenType::RPAREN, "Expected ')' after function arguments");
            auto call = std::make_shared<CallExprNode>(name.lexeme, std::move(args));
            call->line = name.line; call->column = name.column;
            expr = call;
        } else {
            auto node = std::make_shared<IdentifierExprNode>(name.lexeme);
            node->line = name.line; node->column = name.column;
            expr = node;
        }

        while (match(TokenType::LBRACKET)) {
            ExprPtr index = parseExpression();
            consume(TokenType::RBRACKET, "Expected ']' after array index");
            auto access = std::make_shared<ArrayAccessExprNode>(expr, index);
            access->line = name.line;
            access->column = name.column;
            expr = access;
        }
        return expr;
    }

    if (match(TokenType::LPAREN)) {
        ExprPtr expr = parseExpression();
        consume(TokenType::RPAREN, "Expected ')' after expression");
        return expr;
    }

    errorAtCurrent("Expected expression");
    throw std::runtime_error("Expected expression");
}

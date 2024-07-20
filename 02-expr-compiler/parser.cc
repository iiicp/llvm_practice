#include "parser.h"
/**
prog : (expr? ";")*
expr : term (("+" | "-") term)* ;
term : factor (("*" | "/") factor)* ;
factor : number | "(" expr ")" ;
number: ([0-9])+ ;
 */
std::shared_ptr<Program> Parser::ParseProgram() {
    /// while (true) ... exit??
    /// tok -> eof
    std::vector<std::shared_ptr<Expr>> exprVec;
    while (tok.tokenType != TokenType::eof) {
        if (tok.tokenType == TokenType::semi) {
            Advance();
            continue;
        }
        auto expr = ParseExpr();
        exprVec.push_back(expr);
    }
    auto program = std::make_shared<Program>();
    program->exprVec = std::move(exprVec);
    return program;
}

/// 左结合
std::shared_ptr<Expr> Parser::ParseExpr() {
    auto left = ParseTerm();
    while (tok.tokenType == TokenType::plus || tok.tokenType == TokenType::minus) {
        OpCode op;
        if (tok.tokenType == TokenType::plus) {
            op = OpCode::add;
        }else {
            op = OpCode::sub;
        }
        Advance();
        auto binaryExpr = std::make_shared<BinaryExpr>();
        binaryExpr->op = op;
        binaryExpr->left = left;
        binaryExpr->right = ParseTerm();

        left = binaryExpr;
    }
    return left;
}

/// 左结合
std::shared_ptr<Expr> Parser::ParseTerm() {
    auto left = ParseFactor();
    while (tok.tokenType == TokenType::star || tok.tokenType == TokenType::slash) {
        OpCode op;
        if (tok.tokenType == TokenType::star) {
            op = OpCode::mul;
        }else {
            op = OpCode::div;
        }
        Advance();
        auto binaryExpr = std::make_shared<BinaryExpr>();
        binaryExpr->op = op;
        binaryExpr->left = left;
        binaryExpr->right = ParseFactor();

        left = binaryExpr;
    }
    return left;
}

std::shared_ptr<Expr> Parser::ParseFactor() {
    if (tok.tokenType == TokenType::l_parent) {
        Advance();
        auto expr = ParseExpr();
        assert (Expect(TokenType::r_parent));
        Advance();
        return expr;
    }else {
        auto factor = std::make_shared<FactorExpr>();
        factor->number = tok.value;
        Advance();
        return factor;
    }
}

bool Parser::Expect(TokenType tokenType) {
    if (tok.tokenType == tokenType) {
        return true;
    }
    return false;
}

bool Parser::Consume(TokenType tokenType) {
    if (Expect(tokenType)) {
        Advance();
        return true;
    }
    return false;
}

void Parser::Advance() {
    lexer.NextToken(tok);
}
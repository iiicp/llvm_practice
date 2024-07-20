#pragma once
#include "lexer.h"
#include "ast.h"
class Parser {
private:
    Lexer &lexer;
public:
    Parser(Lexer &lexer) : lexer(lexer) {
        Advance();
    }

    std::shared_ptr<Program> ParseProgram();

private:
    std::shared_ptr<Expr> ParseExpr();
    std::shared_ptr<Expr> ParseTerm();
    std::shared_ptr<Expr> ParseFactor();

    /// 消费 token 的函数
    /// 检测当前 token是否是该类型，不会消费
    bool Expect(TokenType tokenType);
    /// 检测，并消费
    bool Consume(TokenType tokenType);
    /// 前进一个 token
    void Advance();

    Token tok;
};
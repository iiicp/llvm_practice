#pragma once
#include "lexer.h"
#include "ast.h"
#include "sema.h"
class Parser {
private:
    Lexer &lexer;
    Sema &sema;
public:
    Parser(Lexer &lexer, Sema &sema) : lexer(lexer), sema(sema) {
        Advance();
    }

    std::shared_ptr<Program> ParseProgram();

private:
    std::vector<std::shared_ptr<AstNode>> ParseDecl();
    std::shared_ptr<AstNode> ParseExpr();
    std::shared_ptr<AstNode> ParseTerm();
    std::shared_ptr<AstNode> ParseFactor();

    /// 消费 token 的函数
    /// 检测当前 token是否是该类型，不会消费
    bool Expect(TokenType tokenType);
    /// 检测，并消费
    bool Consume(TokenType tokenType);
    /// 前进一个 token
    void Advance();

    Token tok;
};
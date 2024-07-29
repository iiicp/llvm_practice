#pragma once
#include "lexer.h"
#include "ast.h"
#include "sema.h"
class Parser {
private:
    Lexer &lexer;
    Sema &sema;
    std::vector<std::shared_ptr<AstNode>> breakNodes;
    std::vector<std::shared_ptr<AstNode>> continueNodes;
public:
    Parser(Lexer &lexer, Sema &sema) : lexer(lexer), sema(sema) {
        Advance();
    }

    std::shared_ptr<Program> ParseProgram();

private:
    std::shared_ptr<AstNode> ParseStmt();
    std::shared_ptr<AstNode> ParseBlockStmt();
    std::shared_ptr<AstNode> ParseDeclStmt();
    std::shared_ptr<AstNode> ParseIfStmt();
    std::shared_ptr<AstNode> ParseForStmt();
    std::shared_ptr<AstNode> ParseBreakStmt();
    std::shared_ptr<AstNode> ParseContinueStmt();
    std::shared_ptr<AstNode> ParseExprStmt();
    std::shared_ptr<AstNode> ParseExpr();
    std::shared_ptr<AstNode> ParseAssignExpr();
    std::shared_ptr<AstNode> ParseEqualExpr();
    std::shared_ptr<AstNode> ParseRelationalExpr();
    std::shared_ptr<AstNode> ParseAddExpr();
    std::shared_ptr<AstNode> ParseMultiExpr();
    std::shared_ptr<AstNode> ParsePrimary();

    bool IsTypeName();

    /// 消费 token 的函数
    /// 检测当前 token是否是该类型，不会消费
    bool Expect(TokenType tokenType);
    /// 检测，并消费
    bool Consume(TokenType tokenType);
    /// 前进一个 token
    void Advance();

    DiagEngine &GetDiagEngine() {
        return lexer.GetDiagEngine();
    }

    Token tok;
};
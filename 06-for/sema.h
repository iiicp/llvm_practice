#pragma once
#include "scope.h"
#include "ast.h"
#include "diag_engine.h"
class Sema {
private:
    DiagEngine &diagEngine;
public:
    Sema(DiagEngine &diagEngine):diagEngine(diagEngine) {}
    std::shared_ptr<AstNode> SemaVariableDeclNode(Token tok, CType *ty);
    std::shared_ptr<AstNode> SemaVariableAccessNode(Token tok);
    std::shared_ptr<AstNode> SemaNumberExprNode(Token tok, CType *ty);
    std::shared_ptr<AstNode> SemaAssignExprNode( std::shared_ptr<AstNode> left,std::shared_ptr<AstNode> right, Token tok);
    std::shared_ptr<AstNode> SemaBinaryExprNode( std::shared_ptr<AstNode> left,std::shared_ptr<AstNode> right, OpCode op);
    std::shared_ptr<AstNode> SemaIfStmtNode(std::shared_ptr<AstNode> condNode, std::shared_ptr<AstNode> thenNode, std::shared_ptr<AstNode> elseNode);

    void EnterScope();
    void ExitScope();
private:
    Scope scope;
};
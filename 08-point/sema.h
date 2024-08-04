#pragma once
#include "scope.h"
#include "ast.h"
#include "diag_engine.h"
class Sema {
private:
    DiagEngine &diagEngine;
public:
    Sema(DiagEngine &diagEngine):diagEngine(diagEngine) {}
    std::shared_ptr<AstNode> SemaVariableDeclNode(Token tok, std::shared_ptr<CType> ty);
    std::shared_ptr<AstNode> SemaVariableAccessNode(Token tok);
    std::shared_ptr<AstNode> SemaNumberExprNode(Token tok, std::shared_ptr<CType> ty);
    std::shared_ptr<AstNode> SemaBinaryExprNode( std::shared_ptr<AstNode> left,std::shared_ptr<AstNode> right, BinaryOp op);
    std::shared_ptr<AstNode> SemaIfStmtNode(std::shared_ptr<AstNode> condNode, std::shared_ptr<AstNode> thenNode, std::shared_ptr<AstNode> elseNode);

    void EnterScope();
    void ExitScope();
private:
    Scope scope;
};
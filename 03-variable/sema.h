#pragma once
#include "scope.h"
#include "ast.h"
class Sema {
public:
    std::shared_ptr<AstNode> SemaVariableDeclNode(llvm::StringRef name, CType *ty);
    std::shared_ptr<AstNode> SemaVariableAccessNode(llvm::StringRef name);
    std::shared_ptr<AstNode> SemaNumberExprNode(int number, CType *ty);
    std::shared_ptr<AstNode> SemaAssignExprNode( std::shared_ptr<AstNode> left,std::shared_ptr<AstNode> right);
    std::shared_ptr<AstNode> SemaBinaryExprNode( std::shared_ptr<AstNode> left,std::shared_ptr<AstNode> right, OpCode op);
private:
    Scope scope;
};
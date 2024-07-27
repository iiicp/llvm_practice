#include "sema.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/Support/Casting.h"

std::shared_ptr<AstNode> Sema::SemaVariableDeclNode(Token tok, CType *ty) {
    // 1. 检测是否出现重定义
    llvm::StringRef text(tok.ptr, tok.len);
    std::shared_ptr<Symbol> symbol = scope.FindVarSymbolInCurEnv(text);
    if (symbol) {
        diagEngine.Report(llvm::SMLoc::getFromPointer(tok.ptr), diag::err_redefined, text);
    }

    /// 2. 添加到符号表
    scope.AddSymbol(SymbolKind::LocalVariable, ty, text);

    /// 3. 返回结点
    auto decl = std::make_shared<VariableDecl>();
    decl->tok = tok;
    decl->ty = ty;

    return decl;
}

std::shared_ptr<AstNode> Sema::SemaVariableAccessNode(Token tok)  {

    llvm::StringRef text(tok.ptr, tok.len);
    std::shared_ptr<Symbol> symbol = scope.FindVarSymbol(text);
    if (symbol == nullptr) {
        diagEngine.Report(llvm::SMLoc::getFromPointer(tok.ptr), diag::err_undefined, text);
    }

    auto expr = std::make_shared<VariableAccessExpr>();
    expr->tok = tok;
    expr->ty = symbol->GetTy();
    return expr;
}

std::shared_ptr<AstNode> Sema::SemaAssignExprNode( std::shared_ptr<AstNode> left,std::shared_ptr<AstNode> right, Token tok) {
    assert(left && right);

    if (!llvm::isa<VariableAccessExpr>(left.get())) {
        diagEngine.Report(llvm::SMLoc::getFromPointer(tok.ptr), diag::err_lvalue);
    }

    auto assignExpr = std::make_shared<AssignExpr>();
    assignExpr->left = left;
    assignExpr->right = right;
    return assignExpr;
}

std::shared_ptr<AstNode> Sema::SemaBinaryExprNode( std::shared_ptr<AstNode> left,std::shared_ptr<AstNode> right, OpCode op) {
    auto binaryExpr = std::make_shared<BinaryExpr>();
    binaryExpr->op = op;
    binaryExpr->left = left;
    binaryExpr->right = right;
    return binaryExpr;
}

std::shared_ptr<AstNode> Sema::SemaNumberExprNode(Token tok, CType *ty) {
    auto expr = std::make_shared<NumberExpr>();
    expr->tok = tok;
    expr->ty = ty;
    return expr;
}
#include "sema.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/Support/Casting.h"

std::shared_ptr<AstNode> Sema::SemaVariableDeclNode(Token tok, std::shared_ptr<CType> ty) {
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
    decl->isLValue = true;
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
    expr->isLValue = true;
    return expr;
}

std::shared_ptr<AstNode> Sema::SemaBinaryExprNode( std::shared_ptr<AstNode> left,std::shared_ptr<AstNode> right, BinaryOp op) {
    auto binaryExpr = std::make_shared<BinaryExpr>();
    binaryExpr->op = op;
    binaryExpr->left = left;
    binaryExpr->right = right;
    binaryExpr->ty = left->ty;

    if (op == BinaryOp::add || op == BinaryOp::sub || op == BinaryOp::add_assign || op == BinaryOp::sub_assign) {
        /// int a = 3; int *p = &a; 3+p;
        if ((left->ty->GetKind() == CType::TY_Int) && (right->ty->GetKind() == CType::TY_Point)) {
            binaryExpr->ty = right->ty;
        }
    }
    return binaryExpr;
}

std::shared_ptr<AstNode> Sema::SemaUnaryExprNode( std::shared_ptr<AstNode> unary, UnaryOp op, Token tok) {
    auto node = std::make_shared<UnaryExpr>();
    node->op = op;
    node->node = unary;

    switch (op)
    {
    case UnaryOp::positive:
    case UnaryOp::negative:
    case UnaryOp::logical_not:
    case UnaryOp::bitwise_not:
    {
        if (unary->ty->GetKind() != CType::TY_Int) {
            diagEngine.Report(llvm::SMLoc::getFromPointer(tok.ptr), diag::err_expected_ype, "int type");
        }
        node->ty = unary->ty;
        break;
    }
    case UnaryOp::addr: {
        /// &a; 
        if (!unary->isLValue) {
            diagEngine.Report(llvm::SMLoc::getFromPointer(tok.ptr), diag::err_expected_lvalue);
        }
        node->ty = std::make_shared<CPointType>(unary->ty);
        break;
    }
    case UnaryOp::deref: {
        /// *a;
        /// 语义判断 must be pointer
        if (unary->ty->GetKind() != CType::TY_Point) {
            diagEngine.Report(llvm::SMLoc::getFromPointer(tok.ptr), diag::err_expected_ype, "pointer type");
        }
        // if (!unary->isLValue) {
        //     diagEngine.Report(llvm::SMLoc::getFromPointer(tok.ptr), diag::err_expected_lvalue);
        // }
        CPointType *pty = llvm::dyn_cast<CPointType>(unary->ty.get());
        node->ty = pty->GetBaseType();
        node->isLValue = true;
        break;   
    }
    case UnaryOp::dec:
    case UnaryOp::inc: {
        if (!unary->isLValue) {
            diagEngine.Report(llvm::SMLoc::getFromPointer(tok.ptr), diag::err_expected_lvalue);
        }
        node->ty = unary->ty;
        break;          
    }                       
    default:
        break;
    }

    return node;
}

std::shared_ptr<AstNode> Sema::SemaThreeExprNode( std::shared_ptr<AstNode> cond,std::shared_ptr<AstNode> then, std::shared_ptr<AstNode> els, Token tok) {
    auto node = std::make_shared<ThreeExpr>();
    node->cond = cond;
    node->then = then;
    node->els = els;
    if (then->ty->GetKind() != els->ty->GetKind()) {
        diagEngine.Report(llvm::SMLoc::getFromPointer(tok.ptr), diag::err_same_type);
    }
    node->ty = then->ty;
    return node;
}

// sizeof a;
std::shared_ptr<AstNode> Sema::SemaSizeofExprNode( std::shared_ptr<AstNode> unary,std::shared_ptr<CType> ty) {
    auto node = std::make_shared<SizeOfExpr>();
    node->type = ty;
    node->node = unary;
    node->ty = CType::IntType;
    return node;
}

std::shared_ptr<AstNode> Sema::SemaPostIncExprNode(std::shared_ptr<AstNode> left, Token tok) {
    if (!left->isLValue) {
        diagEngine.Report(llvm::SMLoc::getFromPointer(tok.ptr), diag::err_expected_lvalue);
    }
    auto node = std::make_shared<PostIncExpr>();
    node->left = left;
    node->ty = left->ty;
    return node;
}

/// a--
std::shared_ptr<AstNode> Sema::SemaPostDecExprNode( std::shared_ptr<AstNode> left, Token tok) {
    if (!left->isLValue) {
        diagEngine.Report(llvm::SMLoc::getFromPointer(tok.ptr), diag::err_expected_lvalue);
    }
    auto node = std::make_shared<PostDecExpr>();
    node->left = left;
    node->ty = left->ty;
    return node;
}

std::shared_ptr<AstNode> Sema::SemaNumberExprNode(Token tok, std::shared_ptr<CType> ty) {
    auto expr = std::make_shared<NumberExpr>();
    expr->tok = tok;
    expr->ty = ty;
    return expr;
}

std::shared_ptr<AstNode> Sema::SemaIfStmtNode(std::shared_ptr<AstNode> condNode, std::shared_ptr<AstNode> thenNode, std::shared_ptr<AstNode> elseNode) {
    auto node = std::make_shared<IfStmt>();
    node->condNode = condNode;
    node->thenNode = thenNode;
    node->elseNode = elseNode;
    return node;
}

void Sema::EnterScope() {
    scope.EnterScope();
}

void Sema::ExitScope() {
    scope.ExitScope();
}
#pragma once
#include "ast.h"
#include "parser.h"
#include "llvm/IR/LLVMContext.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/ADT/DenseMap.h"

class CodeGen : public Visitor, public TypeVisitor {
public:
    CodeGen(std::shared_ptr<Program> p) {
        module = std::make_unique<llvm::Module>("expr", context);
        VisitProgram(p.get());
    }

    std::unique_ptr<llvm::Module> &GetModule() {
        return module;
    }

private:
    llvm::Value * VisitProgram(Program *p) override;
    llvm::Value * VisitBlockStmt(BlockStmt *p) override;
    llvm::Value * VisitDeclStmt(DeclStmt *p) override;
    llvm::Value * VisitIfStmt(IfStmt *p) override;
    llvm::Value * VisitForStmt(ForStmt *p) override;
    llvm::Value * VisitContinueStmt(ContinueStmt *p) override;
    llvm::Value * VisitBreakStmt(BreakStmt *p) override;
    llvm::Value * VisitVariableDecl(VariableDecl *decl) override;
    llvm::Value * VisitBinaryExpr(BinaryExpr *binaryExpr) override;
    llvm::Value * VisitNumberExpr(NumberExpr *factorExpr) override;
    llvm::Value * VisitUnaryExpr(UnaryExpr *expr) override;
    llvm::Value * VisitSizeOfExpr(SizeOfExpr *expr) override;
    llvm::Value * VisitPostIncExpr(PostIncExpr *expr) override;
    llvm::Value * VisitPostDecExpr(PostDecExpr *expr) override;
    llvm::Value * VisitThreeExpr(ThreeExpr *expr) override;
    llvm::Value * VisitVariableAccessExpr(VariableAccessExpr *factorExpr) override;

    llvm::Type * VisitPrimaryType(CPrimaryType *ty) override;
    llvm::Type * VisitPointType(CPointType *ty) override;

private:
    llvm::LLVMContext context;
    llvm::IRBuilder<> irBuilder{context};
    std::unique_ptr<llvm::Module> module;
    llvm::Function *curFunc{nullptr};

    llvm::DenseMap<AstNode *, llvm::BasicBlock *> breakBBs;
    llvm::DenseMap<AstNode *, llvm::BasicBlock *> continueBBs;

    llvm::StringMap<std::pair<llvm::Value *, llvm::Type *>> varAddrTypeMap;
};
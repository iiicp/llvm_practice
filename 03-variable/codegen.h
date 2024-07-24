#pragma once
#include "ast.h"
#include "parser.h"
#include "llvm/IR/LLVMContext.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/IRBuilder.h"

class CodeGen : public Visitor {
public:
    CodeGen(std::shared_ptr<Program> p) {
        module = std::make_shared<llvm::Module>("expr", context);
        VisitProgram(p.get());
    }

private:
    llvm::Value * VisitProgram(Program *p) override;
    llvm::Value * VisitVariableDecl(VariableDecl *decl) override;
    llvm::Value * VisitAssignExpr(AssignExpr *expr) override;
    llvm::Value * VisitBinaryExpr(BinaryExpr *binaryExpr) override;
    llvm::Value * VisitNumberExpr(NumberExpr *factorExpr) override;
    llvm::Value * VisitVariableAccessExpr(VariableAccessExpr *factorExpr) override;

private:
    llvm::LLVMContext context;
    llvm::IRBuilder<> irBuilder{context};
    std::shared_ptr<llvm::Module> module;

    llvm::StringMap<llvm::Value *> varAddrMap;
};
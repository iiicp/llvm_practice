#include "codegen.h"
#include "llvm/IR/Verifier.h"

using namespace llvm;

/// ir 常量折叠 
/// 编译的版本是 Release and Debug Symbol
/*
define i32 @main() {
entry:
  %0 = call i32 (ptr, ...) @printf(ptr @0, i32 4)
  %1 = call i32 (ptr, ...) @printf(ptr @1, i32 10)
  %2 = call i32 (ptr, ...) @printf(ptr @2, i32 -1)
  ret i32 0
}
*/
llvm::Value * CodeGen::VisitProgram(Program *p) {
    /// printf
    auto printfFuncType = FunctionType::get(irBuilder.getInt32Ty(), {irBuilder.getInt8PtrTy()}, true);
    auto printFunc = Function::Create(printfFuncType, GlobalValue::ExternalLinkage, "printf", module.get());

    /// main 
    auto mFuncType = FunctionType::get(irBuilder.getInt32Ty(), false);
    auto mFunc = Function::Create(mFuncType, GlobalValue::ExternalLinkage, "main", module.get());
    BasicBlock *entryBB = BasicBlock::Create(context, "entry", mFunc);
    irBuilder.SetInsertPoint(entryBB);

    llvm::Value *lastVal = nullptr;
    for (auto &expr : p->exprVec) {
        lastVal = expr->Accept(this);
    }
    irBuilder.CreateCall(printFunc, {irBuilder.CreateGlobalStringPtr("expr val: %d\n"), lastVal});

    /// 返回指令
    llvm::Value *ret = irBuilder.CreateRet(irBuilder.getInt32(0));
    verifyFunction(*mFunc);

    module->print(llvm::outs(), nullptr);
    return ret;
}

llvm::Value * CodeGen::VisitBinaryExpr(BinaryExpr *binaryExpr) {
    llvm::Value *left = binaryExpr->left->Accept(this);
    llvm::Value *right = binaryExpr->right->Accept(this);

 switch (binaryExpr->op)
    {
    case OpCode::add: {
        return irBuilder.CreateNSWAdd(left, right);
    }
    case OpCode::sub:{
        return irBuilder.CreateNSWSub(left, right);
    }
    case OpCode::mul:{
        return irBuilder.CreateNSWMul(left, right);
    }
    case OpCode::div:{
        return irBuilder.CreateSDiv(left, right);
    }           
    default:
        break;
    }
    return nullptr;
}

llvm::Value * CodeGen::VisitNumberExpr(NumberExpr *numberExpr) {
    return irBuilder.getInt32(numberExpr->number);
}

llvm::Value * CodeGen::VisitVariableDecl(VariableDecl *decl) {
    llvm::Type *ty = nullptr;
    if (decl->ty == CType::GetIntTy()) {
        ty = irBuilder.getInt32Ty();
    }
    llvm::Value *value = irBuilder.CreateAlloca(ty, nullptr, decl->name);
    varAddrMap.insert({decl->name, value});
    return value;
}
llvm::Value * CodeGen::VisitAssignExpr(AssignExpr *expr) {
    auto left = expr->left;
    VariableAccessExpr *varAccessExpr = (VariableAccessExpr *)left.get();
    llvm::Value *leftValAddr = varAddrMap[varAccessExpr->name];
    llvm::Value *rightValue = expr->right->Accept(this);
    return irBuilder.CreateStore(rightValue, leftValAddr);
}
llvm::Value * CodeGen::VisitVariableAccessExpr(VariableAccessExpr *factorExpr) {
    llvm::Value *varAddr = varAddrMap[factorExpr->name];
    llvm::Type *ty = nullptr;
    if (factorExpr->ty == CType::GetIntTy()) {
        ty = irBuilder.getInt32Ty();
    }
    return irBuilder.CreateLoad(ty, varAddr, factorExpr->name);
}
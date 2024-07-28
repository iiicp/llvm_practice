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

    /// 记录当前函数
    curFunc = mFunc;

    llvm::Value *lastVal = nullptr;
    for (auto &node : p->nodeVec) {
        lastVal = node->Accept(this);
    }
    if (lastVal)
        irBuilder.CreateCall(printFunc, {irBuilder.CreateGlobalStringPtr("expr val: %d\n"), lastVal});
    else 
        irBuilder.CreateCall(printFunc, {irBuilder.CreateGlobalStringPtr("last inst is not expr!!!")});

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
    return irBuilder.getInt32(numberExpr->tok.value);
}

llvm::Value * CodeGen::VisitBlockStmt(BlockStmt *p) {
    llvm::Value *lastVal = nullptr;
    for (const auto &stmt : p->nodeVec) {
        lastVal = stmt->Accept(this);
    }
    return lastVal;
}

llvm::Value * CodeGen::VisitDeclStmt(DeclStmt *p) {
    llvm::Value *lastVal = nullptr;
    for (const auto &node : p->nodeVec) {
        lastVal = node->Accept(this);
    }
    return lastVal;
}

/// 划分基本块 
llvm::Value * CodeGen::VisitIfStmt(IfStmt *p) {
    llvm::BasicBlock *condBB = llvm::BasicBlock::Create(context, "cond", curFunc);
    llvm::BasicBlock *thenBB = llvm::BasicBlock::Create(context, "then", curFunc);
    llvm::BasicBlock *elseBB = nullptr;
    if (p->elseNode)
        elseBB = llvm::BasicBlock::Create(context, "else", curFunc);
    llvm::BasicBlock *lastBB = llvm::BasicBlock::Create(context, "last", curFunc);

    irBuilder.CreateBr(condBB);
    irBuilder.SetInsertPoint(condBB);
    llvm::Value *val = p->condNode->Accept(this);
    /// 整型比较指令
    llvm::Value *condVal = irBuilder.CreateICmpNE(val, irBuilder.getInt32(0));
    if (p->elseNode) {
        irBuilder.CreateCondBr(condVal, thenBB, elseBB);

        /// handle then bb
        irBuilder.SetInsertPoint(thenBB);
        p->thenNode->Accept(this);
        irBuilder.CreateBr(lastBB);

        irBuilder.SetInsertPoint(elseBB);
        p->elseNode->Accept(this);
        irBuilder.CreateBr(lastBB);
    }else {
        irBuilder.CreateCondBr(condVal, thenBB, lastBB);

        /// handle then bb
        irBuilder.SetInsertPoint(thenBB);
        p->thenNode->Accept(this);
        irBuilder.CreateBr(lastBB);
    }

    irBuilder.SetInsertPoint(lastBB);

    return nullptr;
}

llvm::Value * CodeGen::VisitVariableDecl(VariableDecl *decl) {
    llvm::Type *ty = nullptr;
    if (decl->ty == CType::GetIntTy()) {
        ty = irBuilder.getInt32Ty();
    }
    llvm::StringRef text(decl->tok.ptr, decl->tok.len);
    llvm::Value *value = irBuilder.CreateAlloca(ty, nullptr, text);
    varAddrTypeMap.insert({text, {value, ty}});
    return value;
}
//  a = 3; => rValue
llvm::Value * CodeGen::VisitAssignExpr(AssignExpr *expr) {
    auto left = expr->left;
    VariableAccessExpr *variableExpr = (VariableAccessExpr *)left.get();
    llvm::StringRef text(variableExpr->tok.ptr, variableExpr->tok.len);
    std::pair pair = varAddrTypeMap[text];
    llvm::Value *addr = pair.first;
    llvm::Type *ty = pair.second;

    llvm::Value *rightValue = expr->right->Accept(this);
    irBuilder.CreateStore(rightValue, addr);
    return irBuilder.CreateLoad(ty, addr, text);
}
llvm::Value * CodeGen::VisitVariableAccessExpr(VariableAccessExpr *expr) {
    llvm::StringRef text(expr->tok.ptr, expr->tok.len);
    std::pair pair = varAddrTypeMap[text];
    llvm::Value *addr = pair.first;
    llvm::Type *ty = pair.second;
    return irBuilder.CreateLoad(ty, addr, text);
}
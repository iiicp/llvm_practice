#include "codegen.h"
#include "llvm/IR/Verifier.h"
#include "llvm/IR/Function.h"

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
    lastVal = p->node->Accept(this);
    // if (lastVal)
    //     irBuilder.CreateCall(printFunc, {irBuilder.CreateGlobalStringPtr("expr val: %d\n"), lastVal});
    // else 
    //     irBuilder.CreateCall(printFunc, {irBuilder.CreateGlobalStringPtr("last inst is not expr!!!")});

    /// 返回指令
    llvm::Value *ret = irBuilder.CreateRet(lastVal);
    verifyFunction(*mFunc);

    if (verifyModule(*module, &llvm::outs())) {
        module->print(llvm::outs(), nullptr);
    }
    return ret;
}

llvm::Value * CodeGen::VisitBinaryExpr(BinaryExpr *binaryExpr) {
    llvm::Value *left = nullptr;
    llvm::Value *right = nullptr;
    if (binaryExpr->op != BinaryOp::logical_or && binaryExpr->op != BinaryOp::logical_and) {
        left = binaryExpr->left->Accept(this);
        right = binaryExpr->right->Accept(this);
    }
    switch (binaryExpr->op)
    {
    case BinaryOp::add: {
        llvm::Type *ty = binaryExpr->left->ty->Accept(this);
        if (ty->isPointerTy()) {
            llvm::Value *newVal = irBuilder.CreateInBoundsGEP(ty, left, {right});
            return newVal;
        }else {
            return irBuilder.CreateNSWAdd(left, right);
        }
    }
    case BinaryOp::sub:{
        llvm::Type *ty = binaryExpr->left->ty->Accept(this);
        if (ty->isPointerTy()) {
            llvm::Value *newVal = irBuilder.CreateInBoundsGEP(ty, left, {irBuilder.CreateNeg(right)});
            return newVal;
        }else {
            return irBuilder.CreateNSWSub(left, right);
        }
    }
    case BinaryOp::mul:{
        return irBuilder.CreateNSWMul(left, right);
    }
    case BinaryOp::div:{
        return irBuilder.CreateSDiv(left, right);
    }
    case BinaryOp::mod: {
        return irBuilder.CreateSRem(left, right);
    }
    case BinaryOp::bitwise_and:{      
        return irBuilder.CreateAnd(left, right);
    }
    case BinaryOp::bitwise_or:{      
        return irBuilder.CreateOr(left, right);
    }
    case BinaryOp::bitwise_xor:{     
        return irBuilder.CreateXor(left, right);
    }   
    case BinaryOp::left_shift:{    
        return irBuilder.CreateShl(left, right);
    }
    case BinaryOp::right_shift:{  
        return irBuilder.CreateAShr(left, right);
    }        
    case BinaryOp::equal: {      
        /// getInt1Ty()
        llvm::Value *val = irBuilder.CreateICmpEQ(left, right);
        return irBuilder.CreateIntCast(val, irBuilder.getInt32Ty(), true);
    }
    case BinaryOp::not_equal:{      
        llvm::Value *val = irBuilder.CreateICmpNE(left, right);
        return irBuilder.CreateIntCast(val, irBuilder.getInt32Ty(), true);
    }
    case BinaryOp::less:{     
        llvm::Value *val = irBuilder.CreateICmpSLT(left, right);
        return irBuilder.CreateIntCast(val, irBuilder.getInt32Ty(), true);
    }
    case BinaryOp::less_equal:{      
        llvm::Value *val = irBuilder.CreateICmpSLE(left, right);
        return irBuilder.CreateIntCast(val, irBuilder.getInt32Ty(), true);
    }      
    case BinaryOp::greater:{      
        llvm::Value *val = irBuilder.CreateICmpSGT(left, right);
        return irBuilder.CreateIntCast(val, irBuilder.getInt32Ty(), true);
    }
    case BinaryOp::greater_equal:{ 
        llvm::Value *val = irBuilder.CreateICmpSGE(left, right);
        return irBuilder.CreateIntCast(val, irBuilder.getInt32Ty(), true);
    }  
    case BinaryOp::logical_and:{
        /// A && B

        llvm::BasicBlock *nextBB = llvm::BasicBlock::Create(context, "nextBB", curFunc);
        llvm::BasicBlock *falseBB = llvm::BasicBlock::Create(context, "falseBB");
        llvm::BasicBlock *mergeBB = llvm::BasicBlock::Create(context, "mergeBB");

        llvm::Value *left = binaryExpr->left->Accept(this);
        llvm::Value *val = irBuilder.CreateICmpNE(left, irBuilder.getInt32(0));
        irBuilder.CreateCondBr(val, nextBB, falseBB);

        irBuilder.SetInsertPoint(nextBB);
        llvm::Value *right = binaryExpr->right->Accept(this);
        right = irBuilder.CreateICmpNE(right, irBuilder.getInt32(0));
        /// 32位 0 或着 1
        right = irBuilder.CreateZExt(right, irBuilder.getInt32Ty());
        irBuilder.CreateBr(mergeBB);

        /// right 这个值，所在的基本块，并不一定是 之前的nextBB了.
        /// 原因是：binaryExpr->right->Accept(this) 内部会生成新的基本块

        /// 拿到当前插入的block, 建立一个值和基本块的关系 {right, nextBB}
        nextBB = irBuilder.GetInsertBlock();
        
        falseBB->insertInto(curFunc);
        irBuilder.SetInsertPoint(falseBB);
        irBuilder.CreateBr(mergeBB);

        mergeBB->insertInto(curFunc);
        irBuilder.SetInsertPoint(mergeBB);
        llvm::PHINode *phi = irBuilder.CreatePHI(irBuilder.getInt32Ty(), 2);
        phi->addIncoming(right, nextBB);
        phi->addIncoming(irBuilder.getInt32(0), falseBB);

        return phi;
    }  
    case BinaryOp::logical_or: {
        /// A || B && C

        llvm::BasicBlock *nextBB = llvm::BasicBlock::Create(context, "nextBB", curFunc);
        llvm::BasicBlock *trueBB = llvm::BasicBlock::Create(context, "trueBB");
        llvm::BasicBlock *mergeBB = llvm::BasicBlock::Create(context, "mergeBB");

        llvm::Value *left = binaryExpr->left->Accept(this);
        llvm::Value *val = irBuilder.CreateICmpNE(left, irBuilder.getInt32(0));
        irBuilder.CreateCondBr(val, trueBB, nextBB);

        irBuilder.SetInsertPoint(nextBB);
        /// 右子树内部也生成了基本块
        llvm::Value *right = binaryExpr->right->Accept(this);
        right = irBuilder.CreateICmpNE(right, irBuilder.getInt32(0));
        /// 32位 0 或着 1
        right = irBuilder.CreateZExt(right, irBuilder.getInt32Ty());
        irBuilder.CreateBr(mergeBB);
        /// right 这个值，所在的基本块，并不一定是 之前的nextBB了.
        /// 原因是：binaryExpr->right->Accept(this) 内部会生成新的基本块

        /// 拿到当前插入的block, 建立一个值和基本块的关系 {right, nextBB}
        nextBB = irBuilder.GetInsertBlock();

        trueBB->insertInto(curFunc);
        irBuilder.SetInsertPoint(trueBB);
        irBuilder.CreateBr(mergeBB);

        mergeBB->insertInto(curFunc);
        irBuilder.SetInsertPoint(mergeBB);
        llvm::PHINode *phi = irBuilder.CreatePHI(irBuilder.getInt32Ty(), 2);
        phi->addIncoming(right, nextBB);
        phi->addIncoming(irBuilder.getInt32(1), trueBB);

        return phi;
    }
    case BinaryOp::assign: {
        llvm::LoadInst *load = llvm::dyn_cast<llvm::LoadInst>(left);
        assert(load);
        irBuilder.CreateStore(right, load->getPointerOperand());
        return right;
    }
    case BinaryOp::add_assign: {
        llvm::LoadInst *load = llvm::dyn_cast<llvm::LoadInst>(left);
        assert(load);
        llvm::Type *ty = binaryExpr->left->ty->Accept(this);
        if (ty->isPointerTy()) {
             llvm::Value *newVal = irBuilder.CreateInBoundsGEP(ty, left, {right});
             irBuilder.CreateStore(newVal, load->getPointerOperand());
             return newVal;
        }else {
            /// a+=3; => a = a + 3;
            llvm::Value *tmp = irBuilder.CreateAdd(left, right);
            irBuilder.CreateStore(tmp, load->getPointerOperand());
            return tmp;
        }
    }
    case BinaryOp::sub_assign: {
        llvm::LoadInst *load = llvm::dyn_cast<llvm::LoadInst>(left);
        assert(load);
        llvm::Type *ty = binaryExpr->left->ty->Accept(this);
        if (ty->isPointerTy()) {
             llvm::Value *newVal = irBuilder.CreateInBoundsGEP(ty, left, {irBuilder.CreateNeg(right)});
             irBuilder.CreateStore(newVal, load->getPointerOperand());
             return newVal;
        }else {
            llvm::Value *tmp = irBuilder.CreateSub(left, right);
            irBuilder.CreateStore(tmp, load->getPointerOperand());
            return tmp;
        }
    }
    case BinaryOp::mul_assign: {
        llvm::LoadInst *load = llvm::dyn_cast<llvm::LoadInst>(left);
        assert(load);
        llvm::Value *tmp = irBuilder.CreateMul(left, right);
        irBuilder.CreateStore(tmp, load->getPointerOperand());
        return tmp;
    }
    case BinaryOp::div_assign: {
        llvm::LoadInst *load = llvm::dyn_cast<llvm::LoadInst>(left);
        assert(load);
        llvm::Value *tmp = irBuilder.CreateSDiv(left, right);
        irBuilder.CreateStore(tmp, load->getPointerOperand());
        return tmp;        
    }
    case BinaryOp::mod_assign: {
        llvm::LoadInst *load = llvm::dyn_cast<llvm::LoadInst>(left);
        assert(load);
        llvm::Value *tmp = irBuilder.CreateSRem(left, right);
        irBuilder.CreateStore(tmp, load->getPointerOperand());
        return tmp;
    }
    case BinaryOp::bitwise_and_assign: {
        llvm::LoadInst *load = llvm::dyn_cast<llvm::LoadInst>(left);
        assert(load);
        llvm::Value *tmp = irBuilder.CreateAnd(left, right);
        irBuilder.CreateStore(tmp, load->getPointerOperand());
        return tmp;
    }
    case BinaryOp::bitwise_or_assign: {
        llvm::LoadInst *load = llvm::dyn_cast<llvm::LoadInst>(left);
        assert(load);
        llvm::Value *tmp = irBuilder.CreateOr(left, right);
        irBuilder.CreateStore(tmp, load->getPointerOperand());
        return tmp;
    }
    case BinaryOp::bitwise_xor_assign: {
        llvm::LoadInst *load = llvm::dyn_cast<llvm::LoadInst>(left);
        assert(load);
        llvm::Value *tmp = irBuilder.CreateXor(left, right);
        irBuilder.CreateStore(tmp, load->getPointerOperand());
        return tmp;
    }
    case BinaryOp::left_shift_assign: {
        llvm::LoadInst *load = llvm::dyn_cast<llvm::LoadInst>(left);
        assert(load);
        llvm::Value *tmp = irBuilder.CreateShl(left, right);
        irBuilder.CreateStore(tmp, load->getPointerOperand());
        return tmp;
    }
    case BinaryOp::right_shift_assign: {
        llvm::LoadInst *load = llvm::dyn_cast<llvm::LoadInst>(left);
        assert(load);
        llvm::Value *tmp = irBuilder.CreateAShr(left, right);
        irBuilder.CreateStore(tmp, load->getPointerOperand());
        return tmp;
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

llvm::Value * CodeGen::VisitForStmt(ForStmt *p) {
    llvm::BasicBlock *initBB = llvm::BasicBlock::Create(context, "for.init", curFunc);
    llvm::BasicBlock *condBB = llvm::BasicBlock::Create(context, "for.cond", curFunc);
    llvm::BasicBlock *incBB = llvm::BasicBlock::Create(context, "for.inc", curFunc);
    llvm::BasicBlock *bodyBB = llvm::BasicBlock::Create(context, "for.body", curFunc);
    llvm::BasicBlock *lastBB = llvm::BasicBlock::Create(context, "for.last", curFunc);

    breakBBs.insert({p, lastBB});
    continueBBs.insert({p, incBB});

    irBuilder.CreateBr(initBB);
    irBuilder.SetInsertPoint(initBB);
    if (p->initNode) {
        p->initNode->Accept(this);
    }
    irBuilder.CreateBr(condBB);

    irBuilder.SetInsertPoint(condBB);
    if (p->condNode) {
        llvm::Value *val = p->condNode->Accept(this);
        llvm::Value *condVal = irBuilder.CreateICmpNE(val, irBuilder.getInt32(0));
        irBuilder.CreateCondBr(condVal, bodyBB, lastBB);
    }else {
        irBuilder.CreateBr(bodyBB);
    }

    irBuilder.SetInsertPoint(bodyBB);
    if (p->bodyNode) {
        p->bodyNode->Accept(this);
    }
    irBuilder.CreateBr(incBB);

    irBuilder.SetInsertPoint(incBB);
    if (p->incNode) {
        p->incNode->Accept(this);
    }
    irBuilder.CreateBr(condBB);

    breakBBs.erase(p);
    continueBBs.erase(p);

    irBuilder.SetInsertPoint(lastBB);

    return nullptr;
}

llvm::Value * CodeGen::VisitContinueStmt(ContinueStmt *p) {
    /// jump incBB
    llvm::BasicBlock *bb = continueBBs[p->target.get()];
    irBuilder.CreateBr(bb);

    llvm::BasicBlock *out = llvm::BasicBlock::Create(context, "for.continue.death", curFunc);
    irBuilder.SetInsertPoint(out);
    return nullptr;
}

llvm::Value * CodeGen::VisitBreakStmt(BreakStmt *p) {
    /// jump lastBB
    llvm::BasicBlock *bb = breakBBs[p->target.get()];
    irBuilder.CreateBr(bb);

    llvm::BasicBlock *out = llvm::BasicBlock::Create(context, "for.break.death", curFunc);
    irBuilder.SetInsertPoint(out);
    return nullptr;
}

llvm::Value * CodeGen::VisitVariableDecl(VariableDecl *decl) {
    llvm::Type *ty = decl->ty->Accept(this);
    llvm::StringRef text(decl->tok.ptr, decl->tok.len);
    llvm::Value *value = irBuilder.CreateAlloca(ty, nullptr, text);
    varAddrTypeMap.insert({text, {value, ty}});

    if (decl->init) {
        llvm::Value *initValue = decl->init->Accept(this);
        irBuilder.CreateStore(initValue, value);
    }
    return value;
}

llvm::Value * CodeGen::VisitUnaryExpr(UnaryExpr *expr) {
    llvm::Value *val = expr->node->Accept(this);
    llvm::Type *ty = expr->node->ty->Accept(this);

    switch (expr->op)
    {
    case UnaryOp::positive:
        return val;
    case UnaryOp::negative:{
        return irBuilder.CreateNeg(val);
    }
    case UnaryOp::logical_not: {
        llvm::Value *tmp = irBuilder.CreateICmpNE(val, irBuilder.getInt32(0));
        return irBuilder.CreateZExt(irBuilder.CreateNot(tmp), irBuilder.getInt32Ty());
    }
    case UnaryOp::bitwise_not:
        return irBuilder.CreateNot(val);
    case UnaryOp::addr:
        return llvm::dyn_cast<LoadInst>(val)->getPointerOperand();
    case UnaryOp::deref: {
        llvm::Type *nodeTy = expr->ty->Accept(this);
        return irBuilder.CreateLoad(nodeTy, val);
    }
    case UnaryOp::inc: {
        /// ++a => a+1 -> a;
        if (ty->isPointerTy()) {
            llvm::Value *newVal = irBuilder.CreateInBoundsGEP(ty, val, {irBuilder.getInt32(1)});
            irBuilder.CreateStore(newVal, llvm::dyn_cast<LoadInst>(val)->getPointerOperand());
            return newVal;
        }else if (ty->isIntegerTy()) {
            llvm::Value *newVal = irBuilder.CreateAdd(val, irBuilder.getInt32(1));
            irBuilder.CreateStore(newVal, llvm::dyn_cast<LoadInst>(val)->getPointerOperand());
            return newVal;
        }else {
            assert(0);
            return nullptr;
        }
    }
    case UnaryOp::dec:{
        /// --a => a-1 -> a;
        if (ty->isPointerTy()) {
            llvm::Value *newVal = irBuilder.CreateInBoundsGEP(ty, val, {irBuilder.getInt32(-1)});
            irBuilder.CreateStore(newVal, llvm::dyn_cast<LoadInst>(val)->getPointerOperand());
            return newVal;
        }else if (ty->isIntegerTy()) {
            llvm::Value *newVal = irBuilder.CreateSub(val, irBuilder.getInt32(1));
            irBuilder.CreateStore(newVal, llvm::dyn_cast<LoadInst>(val)->getPointerOperand());
            return newVal;
        }else {
            assert(0);
            return nullptr;
        }
    }                                           
    default:
        break;
    }
    return nullptr;
}

llvm::Value * CodeGen::VisitSizeOfExpr(SizeOfExpr *expr) {
    llvm::Type *ty = nullptr;
    if (expr->type) {
        ty = expr->type->Accept(this);
    }else {
        ty = expr->node->ty->Accept(this);
    }
    if (ty->isPointerTy()) {
        return irBuilder.getInt32(8);
    }else if (ty->isIntegerTy()) {
        return irBuilder.getInt32(4);
    }else {
        assert(0);
        return nullptr;
    }
}

llvm::Value * CodeGen::VisitPostIncExpr(PostIncExpr *expr) {
    /// p++;
    /// p = p+1;
    llvm::Value *val = expr->left->Accept(this);
    llvm::Type *ty = expr->left->ty->Accept(this);

    if (ty->isPointerTy()) {
        /// p = p + 1
        llvm::Value *newVal = irBuilder.CreateInBoundsGEP(ty, val, {irBuilder.getInt32(1)});
        irBuilder.CreateStore(newVal, llvm::dyn_cast<LoadInst>(val)->getPointerOperand());
        return val;
    }else if (ty->isIntegerTy()) {
        llvm::Value *newVal = irBuilder.CreateAdd(val, irBuilder.getInt32(1));
        irBuilder.CreateStore(newVal, llvm::dyn_cast<LoadInst>(val)->getPointerOperand());
        return val;
    }else {
        assert(0);
        return nullptr;
    }
}

llvm::Value * CodeGen::VisitPostDecExpr(PostDecExpr *expr) {
    llvm::Value *val = expr->left->Accept(this);
    llvm::Type *ty = expr->left->ty->Accept(this);

    if (ty->isPointerTy()) {
        /// p = p - 1
        llvm::Value *newVal = irBuilder.CreateInBoundsGEP(ty, val, {irBuilder.getInt32(-1)});
        irBuilder.CreateStore(newVal, llvm::dyn_cast<LoadInst>(val)->getPointerOperand());
        return val;
    }else if (ty->isIntegerTy()) {
        llvm::Value *newVal = irBuilder.CreateSub(val, irBuilder.getInt32(1));
        irBuilder.CreateStore(newVal, llvm::dyn_cast<LoadInst>(val)->getPointerOperand());
        return val;
    }else {
        assert(0);
        return nullptr;
    }
}

llvm::Value * CodeGen::VisitThreeExpr(ThreeExpr *expr) {
    llvm::Value *val = expr->cond->Accept(this);
    llvm::Value *cond = irBuilder.CreateICmpNE(val, irBuilder.getInt32(0));

    llvm::BasicBlock *thenBB = llvm::BasicBlock::Create(context, "then", curFunc);
    llvm::BasicBlock *elsBB = llvm::BasicBlock::Create(context, "els");
    llvm::BasicBlock *mergeBB = llvm::BasicBlock::Create(context, "merge");
    irBuilder.CreateCondBr(cond, thenBB, elsBB);

    irBuilder.SetInsertPoint(thenBB);
    llvm::Value *thenVal = expr->then->Accept(this);
    thenBB = irBuilder.GetInsertBlock();
    irBuilder.CreateBr(mergeBB);

    elsBB->insertInto(curFunc);
    irBuilder.SetInsertPoint(elsBB);
    llvm::Value *elsVal = expr->els->Accept(this);
    elsBB = irBuilder.GetInsertBlock();
    irBuilder.CreateBr(mergeBB);

    mergeBB->insertInto(curFunc);
    irBuilder.SetInsertPoint(mergeBB);

    llvm::PHINode *phi = irBuilder.CreatePHI(expr->then->ty->Accept(this), 2);
    phi->addIncoming(thenVal, thenBB);
    phi->addIncoming(elsVal, elsBB);
    return phi;
}


llvm::Value * CodeGen::VisitVariableAccessExpr(VariableAccessExpr *expr) {
    llvm::StringRef text(expr->tok.ptr, expr->tok.len);
    std::pair pair = varAddrTypeMap[text];
    llvm::Value *addr = pair.first;
    llvm::Type *ty = pair.second;
    return irBuilder.CreateLoad(ty, addr, text);
}

llvm::Type * CodeGen::VisitPrimaryType(CPrimaryType *ty) {
    if (ty->GetKind() == CType::TY_Int) {
        return irBuilder.getInt32Ty();
    }
    assert(0);
    return nullptr;
}

llvm::Type * CodeGen::VisitPointType(CPointType *ty) {
    llvm::Type *baseType = ty->GetBaseType()->Accept(this);
    return llvm::PointerType::getUnqual(baseType);
}
#include "llvm/IR/Module.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/LLVMContext.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/IR/Verifier.h"

using namespace llvm;
// ; ModuleID = 'helloworld'
// source_filename = "helloworld
// @gStr = private constant [12 x i8] c"hello,world\00"

// declare i32 @puts(ptr)

// define i32 @main() {
// entry:
//   %call_puts = call i32 @puts(ptr @gStr)
//   ret i32 0
// }
int main() {
    LLVMContext context;
    Module m("helloworld", context);
    IRBuilder<> irBuilder(context);

    /// int puts(char *);
    FunctionType *putsFuncType = FunctionType::get(irBuilder.getInt32Ty(), {irBuilder.getInt8PtrTy()}, false);
    Function*putsFunc = Function::Create(putsFuncType, GlobalValue::LinkageTypes::ExternalLinkage, "puts", m);

    llvm::Constant *c = ConstantDataArray::getString(context, "hello,world");
    GlobalVariable *gvar = new GlobalVariable(m, c->getType(), true, GlobalValue::LinkageTypes::PrivateLinkage, c, "gStr");

    FunctionType *mainFuncType = FunctionType::get(irBuilder.getInt32Ty(), false);
    Function *mainFunc = Function::Create(mainFuncType, GlobalValue::LinkageTypes::ExternalLinkage, "main", m);
    BasicBlock *entryBB = BasicBlock::Create(context, "entry", mainFunc);
    irBuilder.SetInsertPoint(entryBB);

    llvm::Value *gep = irBuilder.CreateGEP(gvar->getType(), gvar, {irBuilder.getInt64(0), irBuilder.getInt64(0)});

    irBuilder.CreateCall(putsFunc, {gep}, "call_puts");
    irBuilder.CreateRet(irBuilder.getInt32(0));

    verifyFunction(*mainFunc, &errs());

    verifyModule(m, &errs());

    m.print(outs(), nullptr);    

    return 0;
}
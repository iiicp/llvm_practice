#pragma once
#include "ast.h"
#include "parser.h"
class PrintVisitor : public Visitor {
public:
    PrintVisitor(std::shared_ptr<Program> program);

    llvm::Value * VisitProgram(Program *p) override;
    llvm::Value * VisitBinaryExpr(BinaryExpr *binaryExpr) override;
    llvm::Value * VisitFactorExpr(FactorExpr *factorExpr) override;
};
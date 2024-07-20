#pragma once
#include <memory>
#include <vector>
#include "llvm/IR/Value.h"

class Program;
class Expr;
class BinaryExpr;
class FactorExpr;

class Visitor {
public:
    virtual ~Visitor() {}
    virtual llvm::Value * VisitProgram(Program *p) = 0;
    virtual llvm::Value * VisitExpr(Expr *expr) {return nullptr;}
    virtual llvm::Value * VisitBinaryExpr(BinaryExpr *binaryExpr) = 0;
    virtual llvm::Value * VisitFactorExpr(FactorExpr *factorExpr) = 0;
};

class Expr {
public:
    virtual ~Expr() {}
    virtual llvm::Value * Accept(Visitor *v) {return nullptr;}
};

enum class OpCode {
    add,
    sub,
    mul,
    div
};

class BinaryExpr : public Expr{
public:
    OpCode op;
    std::shared_ptr<Expr> left;
    std::shared_ptr<Expr> right;

    llvm::Value * Accept(Visitor *v) override {
        return v->VisitBinaryExpr(this);
    }
};

class FactorExpr : public Expr{
public:
    int number;
    llvm::Value * Accept(Visitor *v) override {
        return v->VisitFactorExpr(this);
    }
};

class Program {
public:
    std::vector<std::shared_ptr<Expr>> exprVec;
};

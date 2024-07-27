#pragma once
#include <memory>
#include <vector>
#include "llvm/IR/Value.h"
#include "type.h"
#include "lexer.h"

class Program;
class VariableDecl;
class BinaryExpr;
class AssignExpr;
class NumberExpr;
class VariableAccessExpr;

class Visitor {
public:
    virtual ~Visitor() {}
    virtual llvm::Value * VisitProgram(Program *p) = 0;
    virtual llvm::Value * VisitVariableDecl(VariableDecl *decl) = 0;
    virtual llvm::Value * VisitAssignExpr(AssignExpr *expr) = 0;
    virtual llvm::Value * VisitNumberExpr(NumberExpr *expr) = 0;
    virtual llvm::Value * VisitBinaryExpr(BinaryExpr *binaryExpr) = 0;
    virtual llvm::Value * VisitVariableAccessExpr(VariableAccessExpr *factorExpr) = 0;
};

/// llvm rtti
class AstNode {
public:
    enum Kind{
        ND_VariableDecl,
        ND_BinaryExpr,
        ND_NumberExpr,
        ND_VariableAccessExpr,
        ND_AssignExpr
    };
private:
    const Kind kind;
public:
    virtual ~AstNode() {}
    CType *ty;
    Token tok;
    AstNode(Kind kind): kind(kind) {}
    const Kind GetKind() const {return kind;}
    virtual llvm::Value * Accept(Visitor *v) {return nullptr;}
};

class VariableDecl : public AstNode {
public:
    VariableDecl():AstNode(ND_VariableDecl) {}

    llvm::Value * Accept(Visitor *v) override {
        return v->VisitVariableDecl(this);
    }

    static bool classof(const AstNode *node) {
        return node->GetKind() == ND_VariableDecl;
    }
};

enum class OpCode {
    add,
    sub,
    mul,
    div
};

class BinaryExpr : public AstNode{
public:
    OpCode op;
    std::shared_ptr<AstNode> left;
    std::shared_ptr<AstNode> right;
    BinaryExpr() : AstNode(ND_BinaryExpr) {}
    llvm::Value * Accept(Visitor *v) override {
        return v->VisitBinaryExpr(this);
    }

    static bool classof(const AstNode *node) {
        return node->GetKind() == ND_BinaryExpr;
    }
};

class NumberExpr : public AstNode{
public:
    NumberExpr():AstNode(ND_NumberExpr){}
    llvm::Value * Accept(Visitor *v) override {
        return v->VisitNumberExpr(this);
    }
    static bool classof(const AstNode *node) {
        return node->GetKind() == ND_NumberExpr;
    }
};

class VariableAccessExpr : public AstNode {
public:
    VariableAccessExpr():AstNode(ND_VariableAccessExpr){}
    llvm::Value * Accept(Visitor *v) override {
        return v->VisitVariableAccessExpr(this);
    }    
    static bool classof(const AstNode *node) {
        return node->GetKind() == ND_VariableAccessExpr;
    }
};

class AssignExpr : public AstNode {
public:
    std::shared_ptr<AstNode> left;
    std::shared_ptr<AstNode> right;
    AssignExpr() : AstNode(ND_AssignExpr) {}

    llvm::Value * Accept(Visitor *v) override {
        return v->VisitAssignExpr(this);
    }  
    static bool classof(const AstNode *node) {
        return node->GetKind() == ND_AssignExpr;
    }
};

class Program {
public:
    std::vector<std::shared_ptr<AstNode>> exprVec;
};

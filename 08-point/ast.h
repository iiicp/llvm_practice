#pragma once
#include <memory>
#include <vector>
#include "llvm/IR/Value.h"
#include "type.h"
#include "lexer.h"

class Program;
class VariableDecl;
class BinaryExpr;
class ThreeExpr;
class UnaryExpr;
class SizeOfExpr;
class PostIncExpr;
class PostDecExpr;
class NumberExpr;
class VariableAccessExpr;
class IfStmt;
class DeclStmt;
class BlockStmt;
class ForStmt;
class BreakStmt;
class ContinueStmt;

class Visitor {
public:
    virtual ~Visitor() {}
    virtual llvm::Value * VisitProgram(Program *p) = 0;
    virtual llvm::Value * VisitBlockStmt(BlockStmt *p) = 0;
    virtual llvm::Value * VisitDeclStmt(DeclStmt *p) = 0;
    virtual llvm::Value * VisitIfStmt(IfStmt *p) = 0;
    virtual llvm::Value * VisitForStmt(ForStmt *p) = 0;
    virtual llvm::Value * VisitBreakStmt(BreakStmt *p) = 0;
    virtual llvm::Value * VisitContinueStmt(ContinueStmt *p) = 0;
    virtual llvm::Value * VisitVariableDecl(VariableDecl *decl) = 0;
    virtual llvm::Value * VisitNumberExpr(NumberExpr *expr) = 0;
    virtual llvm::Value * VisitBinaryExpr(BinaryExpr *binaryExpr) = 0;
    virtual llvm::Value * VisitUnaryExpr(UnaryExpr *expr) = 0;
    virtual llvm::Value * VisitSizeOfExpr(SizeOfExpr *expr) = 0;
    virtual llvm::Value * VisitPostIncExpr(PostIncExpr *expr) = 0;
    virtual llvm::Value * VisitPostDecExpr(PostDecExpr *expr) = 0;
    virtual llvm::Value * VisitThreeExpr(ThreeExpr *expr) = 0;
    virtual llvm::Value * VisitVariableAccessExpr(VariableAccessExpr *factorExpr) = 0;
};

/// llvm rtti
class AstNode {
public:
    enum Kind{
        ND_BlockStmt,
        ND_DeclStmt,
        ND_ForStmt,
        ND_BreakStmt,
        ND_ContinueStmt,
        ND_IfStmt,
        ND_VariableDecl,
        ND_BinaryExpr,
        ND_ThreeExpr,
        ND_UnaryExpr,
        ND_SizeOfExpr,
        ND_PostIncExpr,
        ND_PostDecExpr,
        ND_NumberExpr,
        ND_VariableAccessExpr
    };
private:
    const Kind kind;
public:
    virtual ~AstNode() {}
    std::shared_ptr<CType> ty;
    Token tok;
    bool isLValue{false};
    AstNode(Kind kind): kind(kind) {}
    const Kind GetKind() const {return kind;}
    virtual llvm::Value * Accept(Visitor *v) {return nullptr;}
};

class BlockStmt : public AstNode {
public:
    std::vector<std::shared_ptr<AstNode>> nodeVec;
public:
    BlockStmt():AstNode(ND_BlockStmt) {}

    llvm::Value * Accept(Visitor *v) override {
        return v->VisitBlockStmt(this);
    }

    static bool classof(const AstNode *node) {
        return node->GetKind() == ND_BlockStmt;
    }
};


class DeclStmt : public AstNode {
public:
    std::vector<std::shared_ptr<AstNode>> nodeVec;
public:
    DeclStmt():AstNode(ND_DeclStmt) {}

    llvm::Value * Accept(Visitor *v) override {
        return v->VisitDeclStmt(this);
    }

    static bool classof(const AstNode *node) {
        return node->GetKind() == ND_DeclStmt;
    }
};

class IfStmt : public AstNode {
public:
    std::shared_ptr<AstNode> condNode;
    std::shared_ptr<AstNode> thenNode;
    std::shared_ptr<AstNode> elseNode;
public:
    IfStmt():AstNode(ND_IfStmt) {}

    llvm::Value * Accept(Visitor *v) override {
        return v->VisitIfStmt(this);
    }

    static bool classof(const AstNode *node) {
        return node->GetKind() == ND_IfStmt;
    }
};

/*
for (int i = 0; i < 100; i = i + 1) {
    aa = aa + 1;
}
*/
class ForStmt : public AstNode {
public:
    std::shared_ptr<AstNode> initNode;
    std::shared_ptr<AstNode> condNode;
    std::shared_ptr<AstNode> incNode;
    std::shared_ptr<AstNode> bodyNode;
public:
    ForStmt():AstNode(ND_ForStmt) {}

    llvm::Value * Accept(Visitor *v) override {
        return v->VisitForStmt(this);
    }

    static bool classof(const AstNode *node) {
        return node->GetKind() == ND_ForStmt;
    }
};

class BreakStmt : public AstNode {
public:
    std::shared_ptr<AstNode> target;
public:
    BreakStmt():AstNode(ND_BreakStmt) {}

    llvm::Value * Accept(Visitor *v) override {
        return v->VisitBreakStmt(this);
    }

    static bool classof(const AstNode *node) {
        return node->GetKind() == ND_BreakStmt;
    }
};

class ContinueStmt : public AstNode {
public:
    std::shared_ptr<AstNode> target;
public:
    ContinueStmt():AstNode(ND_ContinueStmt) {}

    llvm::Value * Accept(Visitor *v) override {
        return v->VisitContinueStmt(this);
    }

    static bool classof(const AstNode *node) {
        return node->GetKind() == ND_ContinueStmt;
    }
};

class VariableDecl : public AstNode {
public:
    std::shared_ptr<AstNode> init;
    VariableDecl():AstNode(ND_VariableDecl) {}

    llvm::Value * Accept(Visitor *v) override {
        return v->VisitVariableDecl(this);
    }

    static bool classof(const AstNode *node) {
        return node->GetKind() == ND_VariableDecl;
    }
};

enum class BinaryOp {
    add,
    sub,
    mul,
    div,
    mod,
    equal, // ==
    not_equal,
    less,
    less_equal,
    greater,
    greater_equal,
    logical_or,
    logical_and,
    bitwise_or,
    bitwise_and,
    bitwise_xor,
    left_shift,
    right_shift,

    comma,

    assign,
    add_assign,
    sub_assign,
    mul_assign,
    div_assign,
    mod_assign,
    bitwise_or_assign,
    bitwise_xor_assign,
    bitwise_and_assign,
    left_shift_assign,
    right_shift_assign
};

class BinaryExpr : public AstNode{
public:
    BinaryOp op;
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

class ThreeExpr : public AstNode {
public:
    std::shared_ptr<AstNode> cond;
    std::shared_ptr<AstNode> then;
    std::shared_ptr<AstNode> els;

    ThreeExpr() : AstNode(ND_ThreeExpr) {}
    llvm::Value * Accept(Visitor *v) override {
        return v->VisitThreeExpr(this);
    }

    static bool classof(const AstNode *node) {
        return node->GetKind() == ND_ThreeExpr;
    }
};

enum class UnaryOp {
    positive,
    negative,
    deref,
    addr,
    inc,
    dec,
    logical_not,
    bitwise_not
};

class UnaryExpr : public AstNode {
public:
    UnaryOp op;
    std::shared_ptr<AstNode> node;

    UnaryExpr() : AstNode(ND_UnaryExpr) {}
    llvm::Value * Accept(Visitor *v) override {
        return v->VisitUnaryExpr(this);
    }

    static bool classof(const AstNode *node) {
        return node->GetKind() == ND_UnaryExpr;
    }
};

class SizeOfExpr : public AstNode {
public:
    std::shared_ptr<AstNode> node;
    std::shared_ptr<CType> type;

    SizeOfExpr() : AstNode(ND_SizeOfExpr) {}
    llvm::Value * Accept(Visitor *v) override {
        return v->VisitSizeOfExpr(this);
    }

    static bool classof(const AstNode *node) {
        return node->GetKind() == ND_SizeOfExpr;
    }
};

class PostIncExpr : public AstNode {
public:
    std::shared_ptr<AstNode> left;
    PostIncExpr() : AstNode(ND_PostIncExpr) {}
    llvm::Value * Accept(Visitor *v) override {
        return v->VisitPostIncExpr(this);
    }

    static bool classof(const AstNode *node) {
        return node->GetKind() == ND_PostIncExpr;
    }
};

class PostDecExpr : public AstNode {
public:
    std::shared_ptr<AstNode> left;
    PostDecExpr() : AstNode(ND_PostDecExpr) {}
    llvm::Value * Accept(Visitor *v) override {
        return v->VisitPostDecExpr(this);
    }

    static bool classof(const AstNode *node) {
        return node->GetKind() == ND_PostDecExpr;
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

class Program {
public:
    std::shared_ptr<AstNode> node;
};

#include "printVisitor.h"

PrintVisitor::PrintVisitor(std::shared_ptr<Program> program, llvm::raw_ostream *out) {
    this->out = out;
    VisitProgram(program.get());
}

llvm::Value * PrintVisitor::VisitProgram(Program *p) {
    for (const auto &node : p->nodeVec) {
        node->Accept(this);
        *out << ";";
    }
    return nullptr;
}

llvm::Value * PrintVisitor::VisitBlockStmt(BlockStmt *p) {
    *out << "{";
    for (const auto &stmt : p->nodeVec) {
        stmt->Accept(this);
        *out << ";";
    }
    *out << "}";
    return nullptr;
}

llvm::Value * PrintVisitor::VisitDeclStmt(DeclStmt *p) {
    int i = 0, size = p->nodeVec.size();
    for (const auto &node : p->nodeVec) {
        node->Accept(this);
        if (i != size-1) {
            *out << ";";
        }
        ++i;
    }
    return nullptr;
}

llvm::Value * PrintVisitor::VisitIfStmt(IfStmt *p) {
    *out << "if(";
    p->condNode->Accept(this);
    *out << ")";
    p->thenNode->Accept(this);
    if (p->elseNode) {
        *out << "else";
        p->elseNode->Accept(this);
    }
    return nullptr;
}

llvm::Value * PrintVisitor::VisitForStmt(ForStmt *p) {
    *out << "for(";
    if (p->initNode) {
        p->initNode->Accept(this);
    }
    *out << ";";
    if (p->condNode) {
        p->condNode->Accept(this);
    }
    *out << ";";
    if (p->incNode) {
        p->incNode->Accept(this);
    }
    *out << ")";

    if (p->bodyNode) {
        p->bodyNode->Accept(this);
    }

    return nullptr;
}

llvm::Value * PrintVisitor::VisitContinueStmt(ContinueStmt *p) {
    *out << "continue";
    return nullptr;
}

llvm::Value * PrintVisitor::VisitBreakStmt(BreakStmt *p) {
    *out << "break";
    return nullptr;
}

llvm::Value * PrintVisitor::VisitVariableDecl(VariableDecl *decl) {
    if (decl->ty == CType::GetIntTy()) {
        llvm::StringRef text(decl->tok.ptr, decl->tok.len);
        *out << "int " << text;
    }
    return nullptr;
}

llvm::Value * PrintVisitor::VisitAssignExpr(AssignExpr *expr) {
    expr->left->Accept(this);
    *out << "=";
    expr->right->Accept(this);
    return nullptr;
}

llvm::Value * PrintVisitor::VisitBinaryExpr(BinaryExpr *binaryExpr) {
    binaryExpr->left->Accept(this);

    switch (binaryExpr->op)
    {
    case OpCode::add: {
        *out << "+";
        break;
    }
    case OpCode::sub:{
        *out << "-";
        break;
    }
    case OpCode::mul:{
        *out << "*";
        break;
    }
    case OpCode::div:{
        *out << "/";
        break;
    }     
    case OpCode::equal_equal:{
        *out << "==";
        break;
    } 
    case OpCode::not_equal:{
        *out << "!=";
        break;
    } 
    case OpCode::less:{
        *out << "<";
        break;
    } 
    case OpCode::less_equal:{
        *out << "<=";
        break;
    } 
    case OpCode::greater:{
        *out << ">";
        break;
    } 
    case OpCode::greater_equal:{
        *out << ">=";
        break;
    }  
    case OpCode::mod:{
        *out << "%";
        break;
    } 
    case OpCode::logOr:{
        *out << "||";
        break;
    } 
    case OpCode::logAnd:{
        *out << "&&";
        break;
    } 
    case OpCode::bitAnd:{
        *out << "&";
        break;
    } 
    case OpCode::bitOr:{
        *out << "|";
        break;
    } 
    case OpCode::bitXor:{
        *out << "^";
        break;
    }   
    case OpCode::leftShift:{
        *out << "<<";
        break;
    }  
    case OpCode::rightShift:{
        *out << ">>";
        break;
    }     
    default:
        break;
    }

    binaryExpr->right->Accept(this);

    return nullptr;
}

llvm::Value * PrintVisitor::VisitNumberExpr(NumberExpr *expr) {

    *out << llvm::StringRef(expr->tok.ptr, expr->tok.len);

    return nullptr;
}

llvm::Value * PrintVisitor::VisitVariableAccessExpr(VariableAccessExpr *expr) {
    *out << llvm::StringRef(expr->tok.ptr, expr->tok.len);
    return nullptr;
}
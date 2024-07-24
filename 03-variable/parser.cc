#include "parser.h"
/**
prog : (expr? ";")*
expr : term (("+" | "-") term)* ;
term : factor (("*" | "/") factor)* ;
factor : number | "(" expr ")" ;
number: ([0-9])+ ;
 */
std::shared_ptr<Program> Parser::ParseProgram() {

    std::vector<std::shared_ptr<AstNode>> exprVec;
    while (tok.tokenType != TokenType::eof) {
        if (tok.tokenType == TokenType::semi) {
            Advance();
            continue;
        }
        if (tok.tokenType == TokenType::kw_int) {
            const auto &exprs = ParseDecl();
            for (auto &expr : exprs) {
                exprVec.push_back(expr);
            }
        }else {
            auto expr = ParseExpr();
            exprVec.push_back(expr);
        }
    }
    auto program = std::make_shared<Program>();
    program->exprVec = std::move(exprVec);
    return program;
}

std::vector<std::shared_ptr<AstNode>> Parser::ParseDecl() {
    Consume(TokenType::kw_int);
    CType *baseTy = CType::GetIntTy();

    std::vector<std::shared_ptr<AstNode>> astArr;
    /// int a,b=3;
    /// a,b=3; 
    
    int i = 0;
    while (tok.tokenType != TokenType::semi) {
        if (i++ > 0) {
            assert(Consume(TokenType::comma));
        }
        auto variableName = tok.content;
        /// int a = 3; => int a; a = 3;
        ///变量声明的节点
        auto variableDecl = sema.SemaVariableDeclNode(variableName, baseTy);
        astArr.push_back(variableDecl);

        Consume(TokenType::identifier);

        if (tok.tokenType == TokenType::equal) {
            Advance();
            
            auto left = sema.SemaVariableAccessNode(variableName);
            /// a = 3;
            auto right = ParseExpr();
            auto assignExpr = sema.SemaAssignExprNode(left, right);

            astArr.push_back(assignExpr);
        }
    }

    Consume(TokenType::semi);

    return astArr;
}

/// 左结合
std::shared_ptr<AstNode> Parser::ParseExpr() {
    auto left = ParseTerm();
    while (tok.tokenType == TokenType::plus || tok.tokenType == TokenType::minus) {
        OpCode op;
        if (tok.tokenType == TokenType::plus) {
            op = OpCode::add;
        }else {
            op = OpCode::sub;
        }
        Advance();
        auto right = ParseTerm();
        auto binaryExpr = sema.SemaBinaryExprNode(left, right, op);

        left = binaryExpr;
    }
    return left;
}

/// 左结合
std::shared_ptr<AstNode> Parser::ParseTerm() {
    auto left = ParseFactor();
    while (tok.tokenType == TokenType::star || tok.tokenType == TokenType::slash) {
        OpCode op;
        if (tok.tokenType == TokenType::star) {
            op = OpCode::mul;
        }else {
            op = OpCode::div;
        }
        Advance();
        auto right = ParseFactor();
        auto binaryExpr = sema.SemaBinaryExprNode(left, right, op);

        left = binaryExpr;
    }
    return left;
}

std::shared_ptr<AstNode> Parser::ParseFactor() {
    if (tok.tokenType == TokenType::l_parent) {
        Advance();
        auto expr = ParseExpr();
        assert (Expect(TokenType::r_parent));
        Advance();
        return expr;
    } else if (tok.tokenType == TokenType::identifier) {
        auto expr = sema.SemaVariableAccessNode(tok.content);
        Advance();
        return expr;
    }
    else {
        auto factor = sema.SemaNumberExprNode(tok.value, tok.ty);
        Advance();
        return factor;
    }
}

bool Parser::Expect(TokenType tokenType) {
    if (tok.tokenType == tokenType) {
        return true;
    }
    return false;
}

bool Parser::Consume(TokenType tokenType) {
    if (Expect(tokenType)) {
        Advance();
        return true;
    }
    return false;
}

void Parser::Advance() {
    lexer.NextToken(tok);
}
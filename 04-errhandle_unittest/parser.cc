#include "parser.h"
/**
prog : stmt*
stmt : decl-stmt | expr-stmt | null-stmt
null-stmt : ";"
decl-stmt : "int" identifier ("," identifier (= expr)?)* ";"
expr-stmt : expr ";"
expr : assign-expr | add-expr
assign-expr: identifier "=" expr
add-expr : mult-expr (("+" | "-") mult-expr)* 
mult-expr : primary-expr (("*" | "/") primary-expr)* 
primary-expr : identifier | number | "(" expr ")" 
number: ([0-9])+ 
identifier : (a-zA-Z_)(a-zA-Z0-9_)*
 */
std::shared_ptr<Program> Parser::ParseProgram() {

    std::vector<std::shared_ptr<AstNode>> exprVec;
    while (tok.tokenType != TokenType::eof) {
        /// null stmt
        if (tok.tokenType == TokenType::semi) {
            Advance();
            continue;
        }

        /// decl stmt
        if (tok.tokenType == TokenType::kw_int) {
            const auto &exprs = ParseDeclStmt();
            for (auto &expr : exprs) {
                exprVec.push_back(expr);
            }
        }else {
            /// expr stmt
            auto expr = ParseExprStmt();
            exprVec.push_back(expr);
        }
    }
    auto program = std::make_shared<Program>();
    program->exprVec = std::move(exprVec);
    return program;
}

std::vector<std::shared_ptr<AstNode>> Parser::ParseDeclStmt() {
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
        Token tmp = tok;
        /// int a = 3; => int a; a = 3;
        ///变量声明的节点
        auto variableDecl = sema.SemaVariableDeclNode(tmp, baseTy);
        astArr.push_back(variableDecl);

        Consume(TokenType::identifier);

        if (tok.tokenType == TokenType::equal) {
            Token opTok = tok;
            Advance();
            
            auto left = sema.SemaVariableAccessNode(tmp);
            /// a = 3;
            auto right = ParseExpr();
            auto assignExpr = sema.SemaAssignExprNode(left, right, opTok);

            astArr.push_back(assignExpr);
        }
    }

    Consume(TokenType::semi);

    return astArr;
}

std::shared_ptr<AstNode> Parser::ParseExprStmt() {
    auto expr = ParseExpr();
    Consume(TokenType::semi);
    return expr;
}

/// 左结合
/// expr : assign-expr | add-expr
/// assign-expr: identifier "=" expr
/// add-expr : mult-expr (("+" | "-") mult-expr)* 

/// LLn  
std::shared_ptr<AstNode> Parser::ParseExpr() {

    bool isAssignExpr = false;
    
    lexer.SaveState();
    if (tok.tokenType == TokenType::identifier) {
        Token tmp;
        lexer.NextToken(tmp);
        if (tmp.tokenType == TokenType::equal) {
            isAssignExpr = true;
        }
    }
    lexer.RestoreState();

    if (isAssignExpr) {
        /// assign-expr: identifier "=" expr
        return ParseAssignExpr();
    }

    /// add-expr : mult-expr (("+" | "-") mult-expr)* 
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
        auto expr = sema.SemaVariableAccessNode(tok);
        Advance();
        return expr;
    }
    else {
        Expect(TokenType::number);
        auto factor = sema.SemaNumberExprNode(tok, tok.ty);
        Advance();
        return factor;
    }
}

/// a = b = 3;
std::shared_ptr<AstNode> Parser::ParseAssignExpr() {
    Expect(TokenType::identifier);
    Token tmp = tok;
    Advance();
    auto expr = sema.SemaVariableAccessNode(tmp);
    Token opTok = tok;
    Consume(TokenType::equal);
    return sema.SemaAssignExprNode(expr, ParseExpr(), opTok);
}

bool Parser::Expect(TokenType tokenType) {
    if (tok.tokenType == tokenType) {
        return true;
    }
    GetDiagEngine().Report(
        llvm::SMLoc::getFromPointer(tok.ptr), 
        diag::err_expected, 
        Token::GetSpellingText(tokenType), 
        llvm::StringRef(tok.ptr, tok.len));
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
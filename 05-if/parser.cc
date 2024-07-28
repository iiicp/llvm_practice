#include "parser.h"
/**
prog : stmt*
stmt : decl-stmt | expr-stmt | null-stmt | if-stmt
null-stmt : ";"
decl-stmt : "int" identifier ("," identifier (= expr)?)* ";"
if-stmt : "if" "(" expr ")" stmt ( "else" stmt )?
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

    std::vector<std::shared_ptr<AstNode>> nodeVec;
    while (tok.tokenType != TokenType::eof) {
       auto stmt = ParseStmt();
       if (stmt)
        nodeVec.push_back(stmt);
    }
    auto program = std::make_shared<Program>();
    program->nodeVec = std::move(nodeVec);
    return program;
}

std::shared_ptr<AstNode> Parser::ParseStmt() {
    /// null stmt
    if (tok.tokenType == TokenType::semi) {
        Advance();
        return nullptr;
    }
    /// decl stmt
    if (tok.tokenType == TokenType::kw_int) {
        return ParseDeclStmt();
    }
    /// if stmt
    else if(tok.tokenType == TokenType::kw_if) {
        return ParseIfStmt();
    }
    /// block stmt
    else if(tok.tokenType == TokenType::l_brace) {
        return ParseBlockStmt();
    }
    /// expr stmt
    else {
        return ParseExprStmt();
    }
}

std::shared_ptr<AstNode> Parser::ParseBlockStmt() {
    sema.EnterScope();
    auto blockStmt = std::make_shared<BlockStmt>();

    Consume(TokenType::l_brace);
    while (tok.tokenType != TokenType::r_brace) {
        blockStmt->nodeVec.push_back(ParseStmt());
    }
    Consume(TokenType::r_brace);

    sema.ExitScope();
    
    return blockStmt;
}

std::shared_ptr<AstNode> Parser::ParseDeclStmt() {
    Consume(TokenType::kw_int);
    CType *baseTy = CType::GetIntTy();

    auto decl = std::make_shared<DeclStmt>();
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
        decl->nodeVec.push_back(variableDecl);

        Consume(TokenType::identifier);

        if (tok.tokenType == TokenType::equal) {
            Token opTok = tok;
            Advance();
            
            auto left = sema.SemaVariableAccessNode(tmp);
            /// a = 3;
            auto right = ParseExpr();
            auto assignExpr = sema.SemaAssignExprNode(left, right, opTok);

            decl->nodeVec.push_back(assignExpr);
        }
    }

    Consume(TokenType::semi);

    return decl;
}

std::shared_ptr<AstNode> Parser::ParseExprStmt() {
    auto expr = ParseExpr();
    Consume(TokenType::semi);
    return expr;
}

// if-stmt : "if" "(" expr ")" stmt ( "else" stmt )?
std::shared_ptr<AstNode> Parser::ParseIfStmt() {
    Consume(TokenType::kw_if);
    Consume(TokenType::l_parent);
    auto condExpr = ParseExpr();
    Consume(TokenType::r_parent);
    auto thenStmt = ParseStmt();
    std::shared_ptr<AstNode> elseStmt = nullptr;
    /// peek tok is 'else'
    if (tok.tokenType == TokenType::kw_else) {
        Consume(TokenType::kw_else);
        elseStmt = ParseStmt();
    }
    return sema.SemaIfStmtNode(condExpr, thenStmt, elseStmt);
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
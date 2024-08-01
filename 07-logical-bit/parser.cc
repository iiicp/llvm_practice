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
    if (IsTypeName()) {
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
    /// for stmt
    else if (tok.tokenType == TokenType::kw_for) {
        return ParseForStmt();
    }
    /// break stmt
    else if (tok.tokenType == TokenType::kw_break) {
        return ParseBreakStmt();
    }
    /// continue stmt
    else if (tok.tokenType == TokenType::kw_continue) {
        return ParseContinueStmt();
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

std::shared_ptr<AstNode> Parser::ParseForStmt() {
    Consume(TokenType::kw_for);
    Consume(TokenType::l_parent);

    sema.EnterScope();
    auto node = std::make_shared<ForStmt>();
    
    breakNodes.push_back(node);
    continueNodes.push_back(node);

    std::shared_ptr<AstNode> initNode = nullptr;
    std::shared_ptr<AstNode> condNode = nullptr;
    std::shared_ptr<AstNode> incNode = nullptr;
    std::shared_ptr<AstNode> bodyNode = nullptr;

    if (IsTypeName()) {
        initNode = ParseDeclStmt();
    }else {
        if (tok.tokenType != TokenType::semi) {
            initNode = ParseExpr();
        }
        Consume(TokenType::semi);
    }

    if (tok.tokenType != TokenType::semi) {
        condNode = ParseExpr();
    }
    Consume(TokenType::semi);

    if (tok.tokenType != TokenType::r_parent) {
        incNode = ParseExpr();
    }
    Consume(TokenType::r_parent);

    bodyNode = ParseStmt();

    node->initNode = initNode;
    node->condNode = condNode;
    node->incNode = incNode;
    node->bodyNode = bodyNode;

    breakNodes.pop_back();
    continueNodes.pop_back();

    sema.ExitScope();
    return node;
}

std::shared_ptr<AstNode> Parser::ParseBreakStmt() {
    if (breakNodes.size() == 0) {
        GetDiagEngine().Report(llvm::SMLoc::getFromPointer(tok.ptr), diag::err_break_stmt);
    }
    Consume(TokenType::kw_break);
    auto node = std::make_shared<BreakStmt>();
    node->target = breakNodes.back(); 
    Consume(TokenType::semi);
    return node;
}

std::shared_ptr<AstNode> Parser::ParseContinueStmt() {
    if (breakNodes.size() == 0) {
        GetDiagEngine().Report(llvm::SMLoc::getFromPointer(tok.ptr), diag::err_continue_stmt);
    }
    Consume(TokenType::kw_continue);
    auto node = std::make_shared<ContinueStmt>();
    node->target = continueNodes.back();
    Consume(TokenType::semi);
    return node;
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

    return ParseLogOrExpr();
}

std::shared_ptr<AstNode> Parser::ParseEqualExpr() {
    auto left = ParseRelationalExpr();
    while (tok.tokenType == TokenType::equal_equal || tok.tokenType == TokenType::not_equal) {
        OpCode op;
        if (tok.tokenType == TokenType::equal_equal) {
            op = OpCode::equal_equal;
        }else {
            op = OpCode::not_equal;
        }
        Advance();
        auto right = ParseRelationalExpr();
        auto binaryExpr = sema.SemaBinaryExprNode(left, right, op);

        left = binaryExpr;
    }
    return left;
}

std::shared_ptr<AstNode> Parser::ParseRelationalExpr() {
    auto left = ParseShiftExpr();
    while (tok.tokenType == TokenType::less || tok.tokenType == TokenType::less_equal || 
        tok.tokenType == TokenType::greater || tok.tokenType == TokenType::greater_equal) {
        OpCode op;
        if (tok.tokenType == TokenType::less) {
            op = OpCode::less;
        }else if (tok.tokenType == TokenType::less_equal){
            op = OpCode::less_equal;
        }else if (tok.tokenType == TokenType::greater){
            op = OpCode::greater;
        }else if (tok.tokenType == TokenType::greater_equal){
            op = OpCode::greater_equal;
        }
        Advance();
        auto right = ParseShiftExpr();
        auto binaryExpr = sema.SemaBinaryExprNode(left, right, op);

        left = binaryExpr;
    }
    return left;
}

std::shared_ptr<AstNode> Parser::ParseAddExpr() {
     /// add-expr : mult-expr (("+" | "-") mult-expr)* 
    auto left = ParseMultiExpr();
    while (tok.tokenType == TokenType::plus || tok.tokenType == TokenType::minus) {
        OpCode op;
        if (tok.tokenType == TokenType::plus) {
            op = OpCode::add;
        }else {
            op = OpCode::sub;
        }
        Advance();
        auto right = ParseMultiExpr();
        auto binaryExpr = sema.SemaBinaryExprNode(left, right, op);

        left = binaryExpr;
    }
    return left;
}

/// 左结合
std::shared_ptr<AstNode> Parser::ParseMultiExpr() {
    auto left = ParsePrimary();
    while (tok.tokenType == TokenType::star || 
            tok.tokenType == TokenType::slash || 
            tok.tokenType == TokenType::percent) {
        OpCode op;
        if (tok.tokenType == TokenType::star) {
            op = OpCode::mul;
        }else if (tok.tokenType == TokenType::slash) {
            op = OpCode::div;
        }
        else {
            op = OpCode::mod;
        }
        Advance();
        auto right = ParsePrimary();
        auto binaryExpr = sema.SemaBinaryExprNode(left, right, op);

        left = binaryExpr;
    }
    return left;
}

std::shared_ptr<AstNode> Parser::ParsePrimary() {
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

std::shared_ptr<AstNode> Parser::ParseLogOrExpr() {
    auto left = ParseLogAndExpr();
    while (tok.tokenType == TokenType::pipepipe) {
        OpCode op = OpCode::logOr;
        Advance();
        auto right = ParseLogAndExpr();
        auto binaryExpr = sema.SemaBinaryExprNode(left, right, op);

        left = binaryExpr;
    }
    return left;
}

std::shared_ptr<AstNode> Parser::ParseLogAndExpr() {
    auto left = ParseBitOrExpr();
    while (tok.tokenType == TokenType::ampamp) {
        OpCode op = OpCode::logAnd;
        Advance();
        auto right = ParseBitOrExpr();
        auto binaryExpr = sema.SemaBinaryExprNode(left, right, op);

        left = binaryExpr;
    }
    return left;
}
std::shared_ptr<AstNode> Parser::ParseBitOrExpr() {
    auto left = ParseBitXorExpr();
    while (tok.tokenType == TokenType::pipe) {
        OpCode op = OpCode::bitOr;
        Advance();
        auto right = ParseBitXorExpr();
        auto binaryExpr = sema.SemaBinaryExprNode(left, right, op);

        left = binaryExpr;
    }
    return left;
}
std::shared_ptr<AstNode> Parser::ParseBitXorExpr() {
    auto left = ParseBitAndExpr();
    while (tok.tokenType == TokenType::caret) {
        OpCode op = OpCode::bitXor;
        Advance();
        auto right = ParseBitAndExpr();
        auto binaryExpr = sema.SemaBinaryExprNode(left, right, op);

        left = binaryExpr;
    }
    return left;
}
std::shared_ptr<AstNode> Parser::ParseBitAndExpr() {
    auto left = ParseEqualExpr();
    while (tok.tokenType == TokenType::amp) {
        OpCode op = OpCode::bitAnd;
        Advance();
        auto right = ParseEqualExpr();
        auto binaryExpr = sema.SemaBinaryExprNode(left, right, op);

        left = binaryExpr;
    }
    return left;
}
std::shared_ptr<AstNode> Parser::ParseShiftExpr() {
    auto left = ParseAddExpr();
    while (tok.tokenType == TokenType::less_less || tok.tokenType == TokenType::greater_greater) {
        OpCode op;
        if (tok.tokenType == TokenType::less_less) {
            op = OpCode::leftShift;
        }else {
            op = OpCode::rightShift;
        }
        Advance();
        auto right = ParseAddExpr();
        auto binaryExpr = sema.SemaBinaryExprNode(left, right, op);

        left = binaryExpr;
    }
    return left;
}

bool Parser::IsTypeName() {
    if (tok.tokenType == TokenType::kw_int) {
        return true;
    }
    return false;
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
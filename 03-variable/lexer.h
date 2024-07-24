#pragma once
#include "llvm/ADT/StringRef.h"
#include "llvm/Support/raw_ostream.h"
#include "type.h"

/// char stream -> Token

enum class TokenType {
    unknown,
    number,
    identifier,
    kw_int, // int
    minus, // -
    plus, // +
    star, // *
    slash, // '/'
    l_parent, // {
    r_parent, // }
    semi, // ';'
    equal, // =
    comma, // ,
    eof // end
};

class Token {
public:
    int row, col;
    TokenType tokenType;
    int value;
    llvm::StringRef content;
    CType *ty;
    Token() {
        row = col = -1;
        tokenType = TokenType::unknown;
        value = -1;
    }

    void Dump() {
        llvm::outs() << "{ " << content << ", row = " << row << ", col = " << col << "}\n";
    }
};

class Lexer {
public:
    Lexer(llvm::StringRef sourceCode);

    void NextToken(Token &tok);
private:
    const char *BufPtr;
    const char *LineHeadPtr;
    const char *BufEnd;
    int row;
};
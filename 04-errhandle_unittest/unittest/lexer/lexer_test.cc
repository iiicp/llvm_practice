#include <gtest/gtest.h>
#include "lexer.h"


class LexerTest : public ::testing::Test
{
public:
    void SetUp() override {
        static llvm::ErrorOr<std::unique_ptr<llvm::MemoryBuffer>> buf = llvm::MemoryBuffer::getFile("../testset/lexer_01.txt");
        if (!buf) {
            llvm::errs() << "can't open file!!!\n";
            return;
        }

        llvm::SourceMgr mgr;
        DiagEngine diagEngine(mgr);

        mgr.AddNewSourceBuffer(std::move(*buf), llvm::SMLoc());

        lexer = new Lexer(mgr, diagEngine);
    }

    void TearDown() override {
        delete lexer;
    }
    Lexer *lexer;
};

/*
int aa, b = 4;
aa=1 ;
*/

TEST_F(LexerTest, NextToken) {
    /// 正确集
    /// 当前集
    std::vector<Token> expectedVec, curVec;
    expectedVec.push_back(Token{TokenType::kw_int, 1, 1});
    expectedVec.push_back(Token{TokenType::identifier, 1, 5});
    expectedVec.push_back(Token{TokenType::comma, 1, 7});
    expectedVec.push_back(Token{TokenType::identifier, 1, 9});
    expectedVec.push_back(Token{TokenType::equal, 1, 11});
    expectedVec.push_back(Token{TokenType::number, 1, 13});
    expectedVec.push_back(Token{TokenType::semi, 1, 14});
    expectedVec.push_back(Token{TokenType::identifier, 2, 1});
    expectedVec.push_back(Token{TokenType::equal, 2, 3});
    expectedVec.push_back(Token{TokenType::number, 2, 4});
    expectedVec.push_back(Token{TokenType::semi, 2, 6});

    Token tok;
    while (true) {
        lexer->NextToken(tok);
        if (tok.tokenType == TokenType::eof)
            break;
        curVec.push_back(tok);
    }

    ASSERT_EQ(expectedVec.size(), curVec.size());
    for (int i = 0; i < expectedVec.size(); i++) {
        const auto &expected_tok = expectedVec[i];
        const auto &cur_tok = curVec[i];

        EXPECT_EQ(expected_tok.tokenType, cur_tok.tokenType);
        EXPECT_EQ(expected_tok.row, cur_tok.row);
        EXPECT_EQ(expected_tok.col, cur_tok.col);
    }
}
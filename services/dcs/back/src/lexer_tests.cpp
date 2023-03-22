#include <string>
#include <sstream>
#include <string_view>
#include <initializer_list>

#include <gtest/gtest.h>

#include "parser/lexer.h"

using namespace std::literals;

static std::string tokensToString(std::vector<Token> tokens) {
    std::stringstream ss;

    for (auto t : tokens) {
        ss << ", " << t.String();
    }

    return ss.str();
}

static void assertTokenizeResult(std::string_view text, std::initializer_list<Token> expectedTokens, std::string expectedError) {
    auto result = TokenizeString(text);

    EXPECT_EQ(tokensToString(result.tokens), tokensToString(expectedTokens));
    EXPECT_EQ(result.errorMessage, expectedError);
    if (expectedError.empty()) {
        EXPECT_TRUE(result.success);
    } else {
        EXPECT_FALSE(result.success);
    }
}

TEST(Lexer, EmptyString) {
    assertTokenizeResult(""s, {Token(TokenType::Eof, "")}, "");
}

TEST(Lexer, WhitespaceString) {
    assertTokenizeResult("  \n \t    \t\t\t\n\n\n   \n"s, {Token(TokenType::Eof, "")}, "");
}

TEST(Lexer, FunKeyword) {
    assertTokenizeResult("fun"s, {Token(TokenType::Fun, "fun"), Token(TokenType::Eof, "")}, "");

    assertTokenizeResult("fun  "s, {Token(TokenType::Fun, "fun"), Token(TokenType::Eof, "")}, "");

    assertTokenizeResult("   fun"s, {Token(TokenType::Fun, "fun"), Token(TokenType::Eof, "")}, "");

    assertTokenizeResult(" \n fun\n"s, {Token(TokenType::Fun, "fun"), Token(TokenType::Eof, "")}, "");
}

TEST(Lexer, ReturnKeyword) {
    assertTokenizeResult("return"s, {Token(TokenType::Return, "return"), Token(TokenType::Eof, "")}, "");

    assertTokenizeResult("return  "s, {Token(TokenType::Return, "return"), Token(TokenType::Eof, "")}, "");

    assertTokenizeResult("   return"s, {Token(TokenType::Return, "return"), Token(TokenType::Eof, "")}, "");

    assertTokenizeResult(" \n return\n"s, {Token(TokenType::Return, "return"), Token(TokenType::Eof, "")}, "");
}

TEST(Lexer, IfKeyword) {
    assertTokenizeResult("if"s, {Token(TokenType::If, "if"), Token(TokenType::Eof, "")}, "");

    assertTokenizeResult("if  "s, {Token(TokenType::If, "if"), Token(TokenType::Eof, "")}, "");

    assertTokenizeResult("   if"s, {Token(TokenType::If, "if"), Token(TokenType::Eof, "")}, "");

    assertTokenizeResult(" \n if\n"s, {Token(TokenType::If, "if"), Token(TokenType::Eof, "")}, "");
}

TEST(Lexer, ElseKeyword) {
    assertTokenizeResult("else"s, {Token(TokenType::Else, "else"), Token(TokenType::Eof, "")}, "");

    assertTokenizeResult("else  "s, {Token(TokenType::Else, "else"), Token(TokenType::Eof, "")}, "");

    assertTokenizeResult("   else"s, {Token(TokenType::Else, "else"), Token(TokenType::Eof, "")}, "");

    assertTokenizeResult(" \n else\n"s, {Token(TokenType::Else, "else"), Token(TokenType::Eof, "")}, "");
}

TEST(Lexer, AllKeywords) {
    assertTokenizeResult(
        "\t if else fun   fun \t\t return \n if else fun return"s,
        {
            Token(TokenType::If, "if"),
            Token(TokenType::Else, "else"),
            Token(TokenType::Fun, "fun"),
            Token(TokenType::Fun, "fun"),
            Token(TokenType::Return, "return"),
            Token(TokenType::If, "if"),
            Token(TokenType::Else, "else"),
            Token(TokenType::Fun, "fun"),
            Token(TokenType::Return, "return"),
            Token(TokenType::Eof, ""),
        },
        ""
    );
}

TEST(Lexer, OneName) {
    assertTokenizeResult("Fun"s, {Token(TokenType::Name, "Fun"), Token(TokenType::Eof, "")}, "");

    assertTokenizeResult("reTurn"s, {Token(TokenType::Name, "reTurn"), Token(TokenType::Eof, "")}, "");

    assertTokenizeResult("iF"s, {Token(TokenType::Name, "iF"), Token(TokenType::Eof, "")}, "");

    assertTokenizeResult("elsE"s, {Token(TokenType::Name, "elsE"), Token(TokenType::Eof, "")}, "");

    assertTokenizeResult("_sdfsdf"s, {Token(TokenType::Name, "_sdfsdf"), Token(TokenType::Eof, "")}, "");

    assertTokenizeResult("_sdfs234df"s, {Token(TokenType::Name, "_sdfs234df"), Token(TokenType::Eof, "")}, "");

    assertTokenizeResult("_99sdfs234df"s, {Token(TokenType::Name, "_99sdfs234df"), Token(TokenType::Eof, "")}, "");

    assertTokenizeResult("_99sdfFDFSDFs234df"s, {Token(TokenType::Name, "_99sdfFDFSDFs234df"), Token(TokenType::Eof, "")}, "");

    assertTokenizeResult("_99sd_fFDFSDFs_234df"s, {Token(TokenType::Name, "_99sd_fFDFSDFs_234df"), Token(TokenType::Eof, "")}, "");

    assertTokenizeResult(
        "\n Fun reTurn iF \t\t\t elsE  _sdfsdf\n\n_sdfs234df  _99sdfs234df\t\t_99sdfFDFSDFs234df\t\t_99sd_fFDFSDFs_234df\n\n\n"s,
        {
            Token(TokenType::Name, "Fun"),
            Token(TokenType::Name, "reTurn"),
            Token(TokenType::Name, "iF"),
            Token(TokenType::Name, "elsE"),
            Token(TokenType::Name, "_sdfsdf"),
            Token(TokenType::Name, "_sdfs234df"),
            Token(TokenType::Name, "_99sdfs234df"),
            Token(TokenType::Name, "_99sdfFDFSDFs234df"),
            Token(TokenType::Name, "_99sd_fFDFSDFs_234df"),
            Token(TokenType::Eof, ""),
        },
        ""
    );
}

TEST(Lexer, KeywordWithNames) {
    assertTokenizeResult(
        "if FSDF else POPO9 return fun"s,
        {
            Token(TokenType::If, "if"),
            Token(TokenType::Name, "FSDF"),
            Token(TokenType::Else, "else"),
            Token(TokenType::Name, "POPO9"),
            Token(TokenType::Return, "return"),
            Token(TokenType::Fun, "fun"),
            Token(TokenType::Eof, ""),
        },
        ""
    );
}

TEST(Lexer, Braces) {
    assertTokenizeResult(
        "( ) { } }} ))"s,
        {
            Token(TokenType::LeftParen, "("),
            Token(TokenType::RightParen, ")"),
            Token(TokenType::LeftBrace, "{"),
            Token(TokenType::RightBrace, "}"),
            Token(TokenType::RightBrace, "}"),
            Token(TokenType::RightBrace, "}"),
            Token(TokenType::RightParen, ")"),
            Token(TokenType::RightParen, ")"),
            Token(TokenType::Eof, ""),
        },
        ""
    );
}

TEST(Lexer, Comparisons) {
    assertTokenizeResult(
        "> >= < <= = == != ==="s,
        {
            Token(TokenType::Great, ">"),
            Token(TokenType::Ge, ">="),
            Token(TokenType::Less, "<"),
            Token(TokenType::Le, "<="),
            Token(TokenType::Assign, "="),
            Token(TokenType::Eq, "=="),
            Token(TokenType::Neq, "!="),
            Token(TokenType::Eq, "=="),
            Token(TokenType::Assign, "="),
            Token(TokenType::Eof, ""),
        },
        ""
    );
}

TEST(Lexer, InvalidNeq) {
    assertTokenizeResult("! =", {}, "expected '=' after '!'");
}

TEST(Lexer, Arithmetical) {
    assertTokenizeResult(
        "+-*/ + - * /"s,
        {
            Token(TokenType::Plus, "+"),
            Token(TokenType::Minus, "-"),
            Token(TokenType::Mul, "*"),
            Token(TokenType::Div, "/"),
            Token(TokenType::Plus, "+"),
            Token(TokenType::Minus, "-"),
            Token(TokenType::Mul, "*"),
            Token(TokenType::Div, "/"),
            Token(TokenType::Eof, ""),
        },
        ""
    );
}

TEST(Lexer, Comma) {
    assertTokenizeResult(
        ",  ,,,"s,
        {
            Token(TokenType::Comma, ","),
            Token(TokenType::Comma, ","),
            Token(TokenType::Comma, ","),
            Token(TokenType::Comma, ","),
            Token(TokenType::Eof, ""),
        },
        ""s);
}

TEST(Lexer, Semicolon) {
    assertTokenizeResult(
        ";  ;;;"s,
        {
            Token(TokenType::Semicolon, ";"),
            Token(TokenType::Semicolon, ";"),
            Token(TokenType::Semicolon, ";"),
            Token(TokenType::Semicolon, ";"),
            Token(TokenType::Eof, ""),
        },
        ""s);
}

TEST(Lexer, UnexpectedSymbol) {
    assertTokenizeResult(
        ";  ;;;@"s,
        {
        },
        "unexpected symbol @ at 6"s);
}

TEST(Lexer, Numbers) {
    assertTokenizeResult("42.323", {Token(TokenType::Number, "42.323"), Token(TokenType::Eof, "")}, "");

    assertTokenizeResult("42", {Token(TokenType::Number, "42"), Token(TokenType::Eof, "")}, "");

    assertTokenizeResult("4212312313231231231231231", {Token(TokenType::Number, "4212312313231231231231231"), Token(TokenType::Eof, "")}, "");

    assertTokenizeResult("4212312313231231231231231.1231231243234325234534225", {Token(TokenType::Number, "4212312313231231231231231.1231231243234325234534225"), Token(TokenType::Eof, "")}, "");

    assertTokenizeResult("0.2132312", {Token(TokenType::Number, "0.2132312"), Token(TokenType::Eof, "")}, "");

    assertTokenizeResult("+0.2132312", {Token(TokenType::Number, "+0.2132312"), Token(TokenType::Eof, "")}, "");

    assertTokenizeResult("-0.2132312", {Token(TokenType::Number, "-0.2132312"), Token(TokenType::Eof, "")}, "");

    assertTokenizeResult("-0.2132.312", {}, "too many dots for '-0.2132.312'");

    assertTokenizeResult(
        "-0.2132312+ 1337",
        {
            Token(TokenType::Number, "-0.2132312"),
            Token(TokenType::Plus, "+"),
            Token(TokenType::Number, "1337"),
            Token(TokenType::Eof, ""),
        },
        ""
    );
}

TEST(Lexer, FullProgram) {
    const char *text = R"(

y = 42;
x = +43.00;
p = -1337.13123123;

fun some_func(x1, x2) {
    return x1 + x2;
}

fun main() {
    l = (x + y) * p / some_func(x, y);
    if (l < 1) {
        l = 3;
    } else {
        if (l <= 2)  {
            l = 3;
        }

        if (l > 3) {
            l = 3;
        }

        if (l >= 3) {
            l = 3;
        }

        if (l != 3) {
            l = 3;
        }

        if (l == 3) {
            l = 3;
        }
    }

    return 0;
}

)";
    auto textS = std::string(text);

    assertTokenizeResult(
        std::string(text),
        {
            Token(TokenType::Name, "y"), Token(TokenType::Assign, "="), Token(TokenType::Number, "42"), Token(TokenType::Semicolon, ";"),
            Token(TokenType::Name, "x"), Token(TokenType::Assign, "="), Token(TokenType::Number, "+43.00"), Token(TokenType::Semicolon, ";"),
            Token(TokenType::Name, "p"), Token(TokenType::Assign, "="), Token(TokenType::Number, "-1337.13123123"), Token(TokenType::Semicolon, ";"),
            Token(TokenType::Fun, "fun"), Token(TokenType::Name, "some_func"),
            Token(TokenType::LeftParen, "("), Token(TokenType::Name, "x1"), Token(TokenType::Comma, ","), Token(TokenType::Name, "x2"), Token(TokenType::RightParen, ")"),
            Token(TokenType::LeftBrace, "{"),
            Token(TokenType::Return, "return"), Token(TokenType::Name, "x1"), Token(TokenType::Plus, "+"), Token(TokenType::Name, "x2"), Token(TokenType::Semicolon, ";"),
            Token(TokenType::RightBrace, "}"),
            Token(TokenType::Fun, "fun"), Token(TokenType::Name, "main"), Token(TokenType::LeftParen, "("), Token(TokenType::RightParen, ")"),
            Token(TokenType::LeftBrace, "{"),
            Token(TokenType::Name, "l"), Token(TokenType::Assign, "="),
            Token(TokenType::LeftParen, "("), Token(TokenType::Name, "x"), Token(TokenType::Plus, "+"), Token(TokenType::Name, "y"), Token(TokenType::RightParen, ")"),
            Token(TokenType::Mul, "*"),
            Token(TokenType::Name, "p"),
            Token(TokenType::Div, "/"),
            Token(TokenType::Name, "some_func"),
            Token(TokenType::LeftParen, "("), Token(TokenType::Name, "x"), Token(TokenType::Comma, ","), Token(TokenType::Name, "y"), Token(TokenType::RightParen, ")"),
            Token(TokenType::Semicolon, ";"),
            Token(TokenType::If, "if"),
            Token(TokenType::LeftParen, "("), Token(TokenType::Name, "l"), Token(TokenType::Less, "<"), Token(TokenType::Number, "1"), Token(TokenType::RightParen, ")"),
            Token(TokenType::LeftBrace, "{"), Token(TokenType::Name, "l"), Token(TokenType::Assign, "="), Token(TokenType::Number, "3"), Token(TokenType::Semicolon, ";"), Token(TokenType::RightBrace, "}"),
            Token(TokenType::Else, "else"),
            Token(TokenType::LeftBrace, "{"),
            Token(TokenType::If, "if"),
            Token(TokenType::LeftParen, "("), Token(TokenType::Name, "l"), Token(TokenType::Le, "<="), Token(TokenType::Number, "2"), Token(TokenType::RightParen, ")"),
            Token(TokenType::LeftBrace, "{"), Token(TokenType::Name, "l"), Token(TokenType::Assign, "="), Token(TokenType::Number, "3"), Token(TokenType::Semicolon, ";"), Token(TokenType::RightBrace, "}"),
            Token(TokenType::If, "if"),
            Token(TokenType::LeftParen, "("), Token(TokenType::Name, "l"), Token(TokenType::Great, ">"), Token(TokenType::Number, "3"), Token(TokenType::RightParen, ")"),
            Token(TokenType::LeftBrace, "{"), Token(TokenType::Name, "l"), Token(TokenType::Assign, "="), Token(TokenType::Number, "3"), Token(TokenType::Semicolon, ";"), Token(TokenType::RightBrace, "}"),
            Token(TokenType::If, "if"),
            Token(TokenType::LeftParen, "("), Token(TokenType::Name, "l"), Token(TokenType::Ge, ">="), Token(TokenType::Number, "3"), Token(TokenType::RightParen, ")"),
            Token(TokenType::LeftBrace, "{"), Token(TokenType::Name, "l"), Token(TokenType::Assign, "="), Token(TokenType::Number, "3"), Token(TokenType::Semicolon, ";"), Token(TokenType::RightBrace, "}"),
            Token(TokenType::If, "if"), Token(TokenType::LeftParen, "("), Token(TokenType::Name, "l"), Token(TokenType::Neq, "!="), Token(TokenType::Number, "3"), Token(TokenType::RightParen, ")"),
            Token(TokenType::LeftBrace, "{"), Token(TokenType::Name, "l"), Token(TokenType::Assign, "="), Token(TokenType::Number, "3"), Token(TokenType::Semicolon, ";"), Token(TokenType::RightBrace, "}"),
            Token(TokenType::If, "if"),
            Token(TokenType::LeftParen, "("), Token(TokenType::Name, "l"), Token(TokenType::Eq, "=="), Token(TokenType::Number, "3"), Token(TokenType::RightParen, ")"),
            Token(TokenType::LeftBrace, "{"), Token(TokenType::Name, "l"), Token(TokenType::Assign, "="), Token(TokenType::Number, "3"), Token(TokenType::Semicolon, ";"), Token(TokenType::RightBrace, "}"),
            Token(TokenType::RightBrace, "}"),
            Token(TokenType::Return, "return"), Token(TokenType::Number, "0"), Token(TokenType::Semicolon, ";"),
            Token(TokenType::RightBrace, "}"),
            Token(TokenType::Eof, "")
        },
        ""
    );
}

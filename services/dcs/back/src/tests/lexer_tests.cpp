#include <string>
#include <sstream>
#include <string_view>
#include <initializer_list>

#include <gtest/gtest.h>

#include "compiler/lexer.h"

using namespace std::literals;

static std::string tokensToString(const std::vector<Token>& tokens) {
    std::stringstream ss;

    for (auto t : tokens) {
        ss << ", " << t.String();
    }

    return ss.str();
}

static void assertTokenizeResult(std::string_view text, std::initializer_list<Token> expectedTokens, const std::string &expectedError) {
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
    assertTokenizeResult(""s, {Token(Token::Type::Eof, "")}, "");
}

TEST(Lexer, WhitespaceString) {
    assertTokenizeResult("  \n \t    \t\t\t\n\n\n   \n"s, {Token(Token::Type::Eof, "")}, "");
}

TEST(Lexer, FunKeyword) {
    assertTokenizeResult("fun"s, {Token(Token::Type::Fun, "fun"), Token(Token::Type::Eof, "")}, "");

    assertTokenizeResult("fun  "s, {Token(Token::Type::Fun, "fun"), Token(Token::Type::Eof, "")}, "");

    assertTokenizeResult("   fun"s, {Token(Token::Type::Fun, "fun"), Token(Token::Type::Eof, "")}, "");

    assertTokenizeResult(" \n fun\n"s, {Token(Token::Type::Fun, "fun"), Token(Token::Type::Eof, "")}, "");
}

TEST(Lexer, ReturnKeyword) {
    assertTokenizeResult("return"s, {Token(Token::Type::Return, "return"), Token(Token::Type::Eof, "")}, "");

    assertTokenizeResult("return  "s, {Token(Token::Type::Return, "return"), Token(Token::Type::Eof, "")}, "");

    assertTokenizeResult("   return"s, {Token(Token::Type::Return, "return"), Token(Token::Type::Eof, "")}, "");

    assertTokenizeResult(" \n return\n"s, {Token(Token::Type::Return, "return"), Token(Token::Type::Eof, "")}, "");
}

TEST(Lexer, IfKeyword) {
    assertTokenizeResult("if"s, {Token(Token::Type::If, "if"), Token(Token::Type::Eof, "")}, "");

    assertTokenizeResult("if  "s, {Token(Token::Type::If, "if"), Token(Token::Type::Eof, "")}, "");

    assertTokenizeResult("   if"s, {Token(Token::Type::If, "if"), Token(Token::Type::Eof, "")}, "");

    assertTokenizeResult(" \n if\n"s, {Token(Token::Type::If, "if"), Token(Token::Type::Eof, "")}, "");
}

TEST(Lexer, ElseKeyword) {
    assertTokenizeResult("else"s, {Token(Token::Type::Else, "else"), Token(Token::Type::Eof, "")}, "");

    assertTokenizeResult("else  "s, {Token(Token::Type::Else, "else"), Token(Token::Type::Eof, "")}, "");

    assertTokenizeResult("   else"s, {Token(Token::Type::Else, "else"), Token(Token::Type::Eof, "")}, "");

    assertTokenizeResult(" \n else\n"s, {Token(Token::Type::Else, "else"), Token(Token::Type::Eof, "")}, "");
}

TEST(Lexer, AllKeywords) {
    assertTokenizeResult(
        "\t if else fun   fun \t\t return \n if else fun return"s,
        {
            Token(Token::Type::If, "if"),
            Token(Token::Type::Else, "else"),
            Token(Token::Type::Fun, "fun"),
            Token(Token::Type::Fun, "fun"),
            Token(Token::Type::Return, "return"),
            Token(Token::Type::If, "if"),
            Token(Token::Type::Else, "else"),
            Token(Token::Type::Fun, "fun"),
            Token(Token::Type::Return, "return"),
            Token(Token::Type::Eof, ""),
        },
        ""
    );
}

TEST(Lexer, NameTooLong) {
    assertTokenizeResult("qwertyuiopasdfghjklzxcvbnm = 42.43;", {}, "name 'qwertyuiopasdfghjklzxcvbnm' too long");
}

TEST(Lexer, OneName) {
    assertTokenizeResult("Fun"s, {Token(Token::Type::Name, "Fun"), Token(Token::Type::Eof, "")}, "");

    assertTokenizeResult("reTurn"s, {Token(Token::Type::Name, "reTurn"), Token(Token::Type::Eof, "")}, "");

    assertTokenizeResult("iF"s, {Token(Token::Type::Name, "iF"), Token(Token::Type::Eof, "")}, "");

    assertTokenizeResult("elsE"s, {Token(Token::Type::Name, "elsE"), Token(Token::Type::Eof, "")}, "");

    assertTokenizeResult("_sdfsdf"s, {Token(Token::Type::Name, "_sdfsdf"), Token(Token::Type::Eof, "")}, "");

    assertTokenizeResult("_sdfs234df"s, {Token(Token::Type::Name, "_sdfs234df"), Token(Token::Type::Eof, "")}, "");

    assertTokenizeResult("_99sdfs234df"s, {Token(Token::Type::Name, "_99sdfs234df"), Token(Token::Type::Eof, "")}, "");

    assertTokenizeResult("_99sdfFDFSDFs234df"s, {Token(Token::Type::Name, "_99sdfFDFSDFs234df"), Token(Token::Type::Eof, "")}, "");

    assertTokenizeResult("_99sd_fFDFSDFs_234df"s, {Token(Token::Type::Name, "_99sd_fFDFSDFs_234df"), Token(Token::Type::Eof, "")}, "");

    assertTokenizeResult(
        "\n Fun reTurn iF \t\t\t elsE  _sdfsdf\n\n_sdfs234df  _99sdfs234df\t\t_99sdfFDFSDFs234df\t\t_99sd_fFDFSDFs_234df\n\n\n"s,
        {
            Token(Token::Type::Name, "Fun"),
            Token(Token::Type::Name, "reTurn"),
            Token(Token::Type::Name, "iF"),
            Token(Token::Type::Name, "elsE"),
            Token(Token::Type::Name, "_sdfsdf"),
            Token(Token::Type::Name, "_sdfs234df"),
            Token(Token::Type::Name, "_99sdfs234df"),
            Token(Token::Type::Name, "_99sdfFDFSDFs234df"),
            Token(Token::Type::Name, "_99sd_fFDFSDFs_234df"),
            Token(Token::Type::Eof, ""),
        },
        ""
    );
}

TEST(Lexer, KeywordWithNames) {
    assertTokenizeResult(
        "if FSDF else POPO9 return fun"s,
        {
            Token(Token::Type::If, "if"),
            Token(Token::Type::Name, "FSDF"),
            Token(Token::Type::Else, "else"),
            Token(Token::Type::Name, "POPO9"),
            Token(Token::Type::Return, "return"),
            Token(Token::Type::Fun, "fun"),
            Token(Token::Type::Eof, ""),
        },
        ""
    );
}

TEST(Lexer, Braces) {
    assertTokenizeResult(
        "( ) { } }} ))"s,
        {
            Token(Token::Type::LeftParen, "("),
            Token(Token::Type::RightParen, ")"),
            Token(Token::Type::LeftBrace, "{"),
            Token(Token::Type::RightBrace, "}"),
            Token(Token::Type::RightBrace, "}"),
            Token(Token::Type::RightBrace, "}"),
            Token(Token::Type::RightParen, ")"),
            Token(Token::Type::RightParen, ")"),
            Token(Token::Type::Eof, ""),
        },
        ""
    );
}

TEST(Lexer, Comparisons) {
    assertTokenizeResult(
        "> >= < <= = == != ==="s,
        {
            Token(Token::Type::Great, ">"),
            Token(Token::Type::Ge, ">="),
            Token(Token::Type::Less, "<"),
            Token(Token::Type::Le, "<="),
            Token(Token::Type::Assign, "="),
            Token(Token::Type::Eq, "=="),
            Token(Token::Type::Neq, "!="),
            Token(Token::Type::Eq, "=="),
            Token(Token::Type::Assign, "="),
            Token(Token::Type::Eof, ""),
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
            Token(Token::Type::Plus, "+"),
            Token(Token::Type::Minus, "-"),
            Token(Token::Type::Mul, "*"),
            Token(Token::Type::Div, "/"),
            Token(Token::Type::Plus, "+"),
            Token(Token::Type::Minus, "-"),
            Token(Token::Type::Mul, "*"),
            Token(Token::Type::Div, "/"),
            Token(Token::Type::Eof, ""),
        },
        ""
    );
}

TEST(Lexer, Comma) {
    assertTokenizeResult(
        ",  ,,,"s,
        {
            Token(Token::Type::Comma, ","),
            Token(Token::Type::Comma, ","),
            Token(Token::Type::Comma, ","),
            Token(Token::Type::Comma, ","),
            Token(Token::Type::Eof, ""),
        },
        ""s);
}

TEST(Lexer, Semicolon) {
    assertTokenizeResult(
        ";  ;;;"s,
        {
            Token(Token::Type::Semicolon, ";"),
            Token(Token::Type::Semicolon, ";"),
            Token(Token::Type::Semicolon, ";"),
            Token(Token::Type::Semicolon, ";"),
            Token(Token::Type::Eof, ""),
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
    assertTokenizeResult("42.323", {Token(Token::Type::Number, "42.323"), Token(Token::Type::Eof, "")}, "");

    assertTokenizeResult("42", {Token(Token::Type::Number, "42"), Token(Token::Type::Eof, "")}, "");

    assertTokenizeResult("4212312313231231231231231", {Token(Token::Type::Number, "4212312313231231231231231"), Token(Token::Type::Eof, "")}, "");

    assertTokenizeResult("4212312313231231231231231.1231231243234325234534225", {Token(Token::Type::Number, "4212312313231231231231231.1231231243234325234534225"), Token(Token::Type::Eof, "")}, "");

    assertTokenizeResult("0.2132312", {Token(Token::Type::Number, "0.2132312"), Token(Token::Type::Eof, "")}, "");

    assertTokenizeResult("+0.2132312", {Token(Token::Type::Number, "+0.2132312"), Token(Token::Type::Eof, "")}, "");

    assertTokenizeResult("-0.2132312", {Token(Token::Type::Number, "-0.2132312"), Token(Token::Type::Eof, "")}, "");

    assertTokenizeResult("-0.2132.312", {}, "unexpected symbol . at 7");

    assertTokenizeResult(
        "-0.2132312+ 1337",
        {
            Token(Token::Type::Number, "-0.2132312"),
            Token(Token::Type::Plus, "+"),
            Token(Token::Type::Number, "1337"),
            Token(Token::Type::Eof, ""),
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
            Token(Token::Type::Name, "y"), Token(Token::Type::Assign, "="), Token(Token::Type::Number, "42"), Token(Token::Type::Semicolon, ";"),
            Token(Token::Type::Name, "x"), Token(Token::Type::Assign, "="), Token(Token::Type::Number, "+43.00"), Token(Token::Type::Semicolon, ";"),
            Token(Token::Type::Name, "p"), Token(Token::Type::Assign, "="), Token(Token::Type::Number, "-1337.13123123"), Token(Token::Type::Semicolon, ";"),
            Token(Token::Type::Fun, "fun"), Token(Token::Type::Name, "some_func"),
            Token(Token::Type::LeftParen, "("), Token(Token::Type::Name, "x1"), Token(Token::Type::Comma, ","), Token(Token::Type::Name, "x2"), Token(Token::Type::RightParen, ")"),
            Token(Token::Type::LeftBrace, "{"),
            Token(Token::Type::Return, "return"), Token(Token::Type::Name, "x1"), Token(Token::Type::Plus, "+"), Token(Token::Type::Name, "x2"), Token(Token::Type::Semicolon, ";"),
            Token(Token::Type::RightBrace, "}"),
            Token(Token::Type::Fun, "fun"), Token(Token::Type::Name, "main"), Token(Token::Type::LeftParen, "("), Token(Token::Type::RightParen, ")"),
            Token(Token::Type::LeftBrace, "{"),
            Token(Token::Type::Name, "l"), Token(Token::Type::Assign, "="),
            Token(Token::Type::LeftParen, "("), Token(Token::Type::Name, "x"), Token(Token::Type::Plus, "+"), Token(Token::Type::Name, "y"), Token(Token::Type::RightParen, ")"),
            Token(Token::Type::Mul, "*"),
            Token(Token::Type::Name, "p"),
            Token(Token::Type::Div, "/"),
            Token(Token::Type::Name, "some_func"),
            Token(Token::Type::LeftParen, "("), Token(Token::Type::Name, "x"), Token(Token::Type::Comma, ","), Token(Token::Type::Name, "y"), Token(Token::Type::RightParen, ")"),
            Token(Token::Type::Semicolon, ";"),
            Token(Token::Type::If, "if"),
            Token(Token::Type::LeftParen, "("), Token(Token::Type::Name, "l"), Token(Token::Type::Less, "<"), Token(Token::Type::Number, "1"), Token(Token::Type::RightParen, ")"),
            Token(Token::Type::LeftBrace, "{"), Token(Token::Type::Name, "l"), Token(Token::Type::Assign, "="), Token(Token::Type::Number, "3"), Token(Token::Type::Semicolon, ";"), Token(Token::Type::RightBrace, "}"),
            Token(Token::Type::Else, "else"),
            Token(Token::Type::LeftBrace, "{"),
            Token(Token::Type::If, "if"),
            Token(Token::Type::LeftParen, "("), Token(Token::Type::Name, "l"), Token(Token::Type::Le, "<="), Token(Token::Type::Number, "2"), Token(Token::Type::RightParen, ")"),
            Token(Token::Type::LeftBrace, "{"), Token(Token::Type::Name, "l"), Token(Token::Type::Assign, "="), Token(Token::Type::Number, "3"), Token(Token::Type::Semicolon, ";"), Token(Token::Type::RightBrace, "}"),
            Token(Token::Type::If, "if"),
            Token(Token::Type::LeftParen, "("), Token(Token::Type::Name, "l"), Token(Token::Type::Great, ">"), Token(Token::Type::Number, "3"), Token(Token::Type::RightParen, ")"),
            Token(Token::Type::LeftBrace, "{"), Token(Token::Type::Name, "l"), Token(Token::Type::Assign, "="), Token(Token::Type::Number, "3"), Token(Token::Type::Semicolon, ";"), Token(Token::Type::RightBrace, "}"),
            Token(Token::Type::If, "if"),
            Token(Token::Type::LeftParen, "("), Token(Token::Type::Name, "l"), Token(Token::Type::Ge, ">="), Token(Token::Type::Number, "3"), Token(Token::Type::RightParen, ")"),
            Token(Token::Type::LeftBrace, "{"), Token(Token::Type::Name, "l"), Token(Token::Type::Assign, "="), Token(Token::Type::Number, "3"), Token(Token::Type::Semicolon, ";"), Token(Token::Type::RightBrace, "}"),
            Token(Token::Type::If, "if"), Token(Token::Type::LeftParen, "("), Token(Token::Type::Name, "l"), Token(Token::Type::Neq, "!="), Token(Token::Type::Number, "3"), Token(Token::Type::RightParen, ")"),
            Token(Token::Type::LeftBrace, "{"), Token(Token::Type::Name, "l"), Token(Token::Type::Assign, "="), Token(Token::Type::Number, "3"), Token(Token::Type::Semicolon, ";"), Token(Token::Type::RightBrace, "}"),
            Token(Token::Type::If, "if"),
            Token(Token::Type::LeftParen, "("), Token(Token::Type::Name, "l"), Token(Token::Type::Eq, "=="), Token(Token::Type::Number, "3"), Token(Token::Type::RightParen, ")"),
            Token(Token::Type::LeftBrace, "{"), Token(Token::Type::Name, "l"), Token(Token::Type::Assign, "="), Token(Token::Type::Number, "3"), Token(Token::Type::Semicolon, ";"), Token(Token::Type::RightBrace, "}"),
            Token(Token::Type::RightBrace, "}"),
            Token(Token::Type::Return, "return"), Token(Token::Type::Number, "0"), Token(Token::Type::Semicolon, ";"),
            Token(Token::Type::RightBrace, "}"),
            Token(Token::Type::Eof, "")
        },
        ""
    );
}

TEST(Lexer, NumerScientificFormat1) {
    assertTokenizeResult(
            "-5.70322518485311095648396561471e+306"s,
            {
                Token(Token::Type::Number, "-5.70322518485311095648396561471e+306"),
                Token(Token::Type::Eof, ""),
            },
            ""s);
}

TEST(Lexer, NumerScientificFormat2) {
    assertTokenizeResult(
            "-5.70322518485311095648396561471e306"s,
            {
                    Token(Token::Type::Number, "-5.70322518485311095648396561471e306"),
                    Token(Token::Type::Eof, ""),
            },
            ""s);
}

TEST(Lexer, NumerScientificFormat3) {
    assertTokenizeResult(
            "-5.70322518485311095648396561471e+306"s,
            {
                    Token(Token::Type::Number, "-5.70322518485311095648396561471e+306"),
                    Token(Token::Type::Eof, ""),
            },
            ""s);
}

TEST(Lexer, NumerScientificFormat4) {
    assertTokenizeResult(
            "5.70322518485311095648396561471e+306"s,
            {
                    Token(Token::Type::Number, "5.70322518485311095648396561471e+306"),
                    Token(Token::Type::Eof, ""),
            },
            ""s);
}

TEST(Lexer, NumerScientificFormat5) {
    assertTokenizeResult(
            "5.70322518485311095648396561471e+30e6"s,
            {
                Token(Token::Type::Number, "5.70322518485311095648396561471e+30"),
                Token(Token::Type::Name, "e6"),
                Token(Token::Type::Eof, ""),
            },
            ""s);
}

TEST(Lexer, NumerScientificFormat6) {
    assertTokenizeResult(
            "5.70322518485311095648396561471e+306+1337"s,
            {
                    Token(Token::Type::Number, "5.70322518485311095648396561471e+306"),
                    Token(Token::Type::Number, "+1337"),
                    Token(Token::Type::Eof, ""),
            },
            ""s);
}
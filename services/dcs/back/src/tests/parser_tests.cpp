#include <sstream>

#include <gtest/gtest.h>

#include "compiler/parser.h"
#include "utils/utils.h"

void assertNodeValues(const std::shared_ptr<AstNode> &expected, const std::shared_ptr<AstNode> &actual) {
    ASSERT_EQ(expected->NodeType, actual->NodeType);

    if (expected->NodeType == AstNode::Type::ConstantValue) {
        auto e = dynamic_cast<ConstantValueNode*>(expected.get());
        auto a = dynamic_cast<ConstantValueNode*>(actual.get());

        ASSERT_EQ(e->Value, a->Value);
    }

    if (expected->NodeType == AstNode::Type::Id) {
        auto e = dynamic_cast<IdNode*>(expected.get());
        auto a = dynamic_cast<IdNode*>(actual.get());

        ASSERT_EQ(e->Name, a->Name);
    }

    if (expected->NodeType == AstNode::Type::ConditionalStatement) {
        auto e = dynamic_cast<ConditionalStatementNode*>(expected.get());
        auto a = dynamic_cast<ConditionalStatementNode*>(actual.get());

        ASSERT_EQ(e->Condition, a->Condition);
    }

    if (expected->NodeType == AstNode::Type::AdditiveExpression) {
        auto e = dynamic_cast<AdditiveExpressionNode*>(expected.get());
        auto a = dynamic_cast<AdditiveExpressionNode*>(actual.get());

        ASSERT_EQ(e->Operations, a->Operations);
    }

    if (expected->NodeType == AstNode::Type::MultiplicativeExpression) {
        auto e = dynamic_cast<MultiplicativeExpressionNode*>(expected.get());
        auto a = dynamic_cast<MultiplicativeExpressionNode*>(actual.get());

        ASSERT_EQ(e->Operations, a->Operations);
    }
}

void assertTreeEquals(const std::shared_ptr<AstNode> &expected, const std::shared_ptr<AstNode> &actual) {
    if (expected == nullptr) {
        ASSERT_EQ(actual, nullptr);
    }

    if (actual == nullptr) {
        ASSERT_EQ(expected, nullptr);
    }

    assertNodeValues(expected, actual);

    auto expectedChildren = expected->GetChildNodes();
    auto actualChildren = actual->GetChildNodes();

    ASSERT_EQ(expectedChildren.size(), actualChildren.size());

    int l = static_cast<int>(expectedChildren.size());

    for (int i = 0; i < l; ++i) {
        assertTreeEquals(expectedChildren[i], actualChildren[i]);
    }
}

void assertTokenParsing(const std::vector<Token> &tokens, const std::shared_ptr<AstNode> &expectedTree, const std::string &expectedErrorMessage) {
    auto sut = ParseTokens(tokens);

    if (expectedErrorMessage.empty()) {
        EXPECT_TRUE(sut.Success);
        assertTreeEquals(expectedTree, sut.ProgramNode);
    } else {
        EXPECT_FALSE(sut.Success);
        ASSERT_EQ(expectedErrorMessage, sut.ErrorMessage);
    }
}

void assertTextParsing(const std::string &text, const std::shared_ptr<AstNode> &expectedTree, const std::string &expectedErrorMessage) {
    auto tokensResult = TokenizeString(text);
    ASSERT_TRUE(tokensResult.Success);

    assertTokenParsing(tokensResult.Tokens, expectedTree, expectedErrorMessage);
}

static std::shared_ptr<DcsProgramNode> Program(
        std::initializer_list<std::shared_ptr<ConstantDefinitionNode>> constants,
        std::initializer_list<std::shared_ptr<FunctionDefinitionNode>> functions) {
    return std::make_shared<DcsProgramNode>(constants, functions);
}

static std::shared_ptr<IdNode> Id(const std::string &name) {
    return std::make_shared<IdNode>(name);
}

static std::shared_ptr<ConstantValueNode> Constant(double value) {
    return std::make_shared<ConstantValueNode>(value);
}

static std::shared_ptr<FunctionCallNode> FunctionCall(const std::shared_ptr<IdNode> &id, std::initializer_list<std::shared_ptr<ExpressionNode>> args) {
    return std::make_shared<FunctionCallNode>(id, args);
}

static std::shared_ptr<ConstantDefinitionNode> ConstantDefinition(const std::shared_ptr<IdNode> &id, const std::shared_ptr<ConstantValueNode> &value) {
    return std::make_shared<ConstantDefinitionNode>(id, value);
}

static std::shared_ptr<StatementListNode> StatementsList(std::initializer_list<std::shared_ptr<StatementNode>> statements) {
    return std::make_shared<StatementListNode>(statements);
}

static std::shared_ptr<ExpressionNode> Expression(const std::shared_ptr<AdditiveExpressionNode> &additive) {
    return std::make_shared<ExpressionNode>(additive);
}

static std::shared_ptr<AdditiveExpressionNode> Additive(std::initializer_list<std::shared_ptr<MultiplicativeExpressionNode>> mul, std::initializer_list<AdditiveExpressionNode::OperationType> ops) {
    return std::make_shared<AdditiveExpressionNode>(mul, ops);
}

static std::shared_ptr<MultiplicativeExpressionNode> Multiplicative(std::initializer_list<std::shared_ptr<UnaryExpressionNode>> s, std::initializer_list<MultiplicativeExpressionNode::OperationType> ops) {
    return std::make_shared<MultiplicativeExpressionNode>(s, ops);
}

static std::shared_ptr<UnaryExpressionNode> UnaryId(const std::shared_ptr<IdNode> &id) {
    return std::make_shared<UnaryExpressionNode>(id, nullptr, nullptr, nullptr, nullptr);
}

static std::shared_ptr<UnaryExpressionNode> UnaryConst(const std::shared_ptr<ConstantValueNode> &c) {
    return std::make_shared<UnaryExpressionNode>(nullptr, c, nullptr, nullptr, nullptr);
}

static std::shared_ptr<UnaryExpressionNode> UnaryCall(const std::shared_ptr<FunctionCallNode> &fc) {
    return std::make_shared<UnaryExpressionNode>(nullptr, nullptr, fc, nullptr, nullptr);
}

static std::shared_ptr<UnaryExpressionNode> UnaryParenthesis(const std::shared_ptr<ExpressionNode> &e) {
    return std::make_shared<UnaryExpressionNode>(nullptr, nullptr, nullptr, e, nullptr);
}

static std::shared_ptr<ReturnStatementNode> Return(const std::shared_ptr<ExpressionNode> &e) {
    return std::make_shared<ReturnStatementNode>(e);
}

static std::shared_ptr<AssignStatementNode> AssignStatement(const std::shared_ptr<IdNode> &id, const std::shared_ptr<ExpressionNode> &expression) {
    return std::make_shared<AssignStatementNode>(id, expression);
}

static std::shared_ptr<StatementNode> StatementAssign(const std::shared_ptr<AssignStatementNode> &assign) {
    return std::make_shared<StatementNode>(nullptr, nullptr, assign);
}

static std::shared_ptr<StatementNode> StatementReturn(const std::shared_ptr<ReturnStatementNode> &e) {
    return std::make_shared<StatementNode>(nullptr, e, nullptr);
}

static std::shared_ptr<ArgumentsDefinitionListNode> ArgumentsDefinitionList(std::initializer_list<std::shared_ptr<IdNode>> ids) {
    return std::make_shared<ArgumentsDefinitionListNode>(ids);
}

static std::shared_ptr<FunctionDefinitionNode> FunctionDefinition(
        const std::shared_ptr<IdNode> &id,
        const std::shared_ptr<ArgumentsDefinitionListNode> &arguments,
        const std::shared_ptr<StatementListNode> &body) {
    return std::make_shared<FunctionDefinitionNode>(id, arguments, body);
}

static double Double(const std::string &v) {
    return std::stod(v);
}

TEST(Parser, EmptyTokens) {
    assertTokenParsing(
        {},
        nullptr,
        "expected EOF, but got <nothing>"
    );
}

TEST(Parser, OnlyEof) {
    assertTokenParsing(
        {Token(Token::Type::Eof, "")},
        Program({}, {}),
        ""
    );
}

// next tests assume that lexer works correctly

TEST(Parser, SingleConstantDefinition) {
    assertTextParsing(
R"(
x = -42.43;
)",
        Program(
            {
                ConstantDefinition(Id("x"), Constant(Double("-42.43")))
            },
            {
            }
        ),
        ""
    );

    assertTextParsing(
            R"(
x = +42.43;
)",
        Program(
            {
                ConstantDefinition(Id("x"), Constant(Double("+42.43")))
            },
            {
            }
        ),
        ""
    );

    assertTextParsing(
            R"(
x = 42.43;
)",
        Program(
            {
                ConstantDefinition(Id("x"), Constant(Double("42.43")))
            },
            {
            }
        ),
        ""
    );
}

TEST(Parser, ConstantDefinitionNameDuplicated) {
    assertTextParsing(
        R"(
x x = 42.43;
)",
        nullptr,
        "expected ASSIGN, but got Token(Type::Name, \"x\")"
    );
}

TEST(Parser, ConstantDefinitionNoAssign) {
    assertTextParsing(
        R"(
x 42.43;
)",
        nullptr,
        "expected ASSIGN, but got Token(Type::Number, \"42.43\")"
    );
}

TEST(Parser, ConstantDefinitionNoSemicolon) {
    assertTextParsing(
        R"(
x = 42.43
)",
        nullptr,
        "expected SEMICOLON, but got Token(Type::Eof, \"\")"
    );
}

TEST(Parser, ConstantDefinitionNoNumber) {
    assertTextParsing(
        R"(
x = x;
)",
        nullptr,
        "expected NUMBER, but got Token(Type::Name, \"x\")"
    );
}

TEST(Parser, FunctionDefinitionNoArguments) {
    assertTextParsing(
        R"(
fun main() {}
)",
        Program(
            {
            },
            {
                FunctionDefinition(Id("main"), ArgumentsDefinitionList({}), StatementsList({}))
            }
        ),
        ""
    );
}

TEST(Parser, FunctionDefinitionOneArgument) {
    assertTextParsing(
        R"(
fun main(_some_arg) {}
)",
        Program(
            {
            },
            {
                FunctionDefinition(Id("main"), ArgumentsDefinitionList({Id("_some_arg")}), StatementsList({}))
            }
        ),
        ""
    );
}

TEST(Parser, FunctionDefinitionTwoArguments) {
    assertTextParsing(
        R"(
fun main(_some_arg, x99) {}
)",
        Program(
            {
            },
            {
                FunctionDefinition(Id("main"), ArgumentsDefinitionList({Id("_some_arg"), Id("x99")}), StatementsList({}))
            }
        ),
        ""
    );
}

TEST(Parser, FunctionDefinitionThreeArguments) {
    assertTextParsing(
        R"(
fun main(_some_arg, x99, _123123_234234) {}
)",
        Program(
            {
            },
            {
                FunctionDefinition(Id("main"), ArgumentsDefinitionList({Id("_some_arg"), Id("x99"), Id("_123123_234234")}), StatementsList({}))
            }
        ),
        ""
    );
}

TEST(Parser, FunctionDefinitionTooManyArguments) {
    assertTextParsing(
        R"(
fun main(_some_arg, x99, _123123_234234, F) {}
)",
        nullptr,
        "too many arguments for function 'main', max is 3"
    );

    assertTextParsing(
        R"(
fun main(_some_arg, x99, _123123_234234, F, t) {}
)",
        nullptr,
        "too many arguments for function 'main', max is 3"
    );

    assertTextParsing(
        R"(
fun main(_some_arg, x99, _123123_234234, F, t, y) {}
)",
        nullptr,
        "too many arguments for function 'main', max is 3"
    );
}

TEST(Parser, FunctionWithNoArgumentsList) {
    assertTextParsing(
        R"(
fun main {}
)",
        nullptr,
        "expected LEFT_PAREN, but got Token(Type::LeftBrace, \"{\")"
    );
}


TEST(Parser, FunctionWithNoRightBraceAtArgumentsList) {
    assertTextParsing(
        R"(
fun main(x {}
)",
        nullptr,
        "expected RIGHT_PAREN, but got Token(Type::LeftBrace, \"{\")"
    );
}

TEST(Parser, FunctionNoBody) {
    assertTextParsing(
        R"(
fun main(x)
)",
        nullptr,
        "expected LEFT_BRACE, but got Token(Type::Eof, \"\")"
    );
}

TEST(Parser, ConstantWithEmptyFunction) {
    assertTextParsing(
        R"(
x = +42.43;

fun main(_some_arg) {}
)",
        Program(
            {
                ConstantDefinition(Id("x"), Constant(Double("+42.43")))
            },
            {
                FunctionDefinition(Id("main"), ArgumentsDefinitionList({Id("_some_arg")}), StatementsList({}))
            }
        ),
        ""
    );
}

TEST(Parser, AssignExpression) {
    assertTextParsing(
        R"(
fun main() {
    x = 42;
}
)",
        Program(
            {
            },
            {
                FunctionDefinition(
                    Id("main"),
                    ArgumentsDefinitionList({}),
                    StatementsList({
                                           StatementAssign(AssignStatement(Id("x"), Expression(
                                                   Additive({Multiplicative({UnaryConst(Constant(Double("42")))},
                                                                            {})}, {})))
                                           )
                    })
                )
            }
        ),
        ""
    );
}

TEST(Parser, ReturnStatement) {
    assertTextParsing(
        R"(
fun main() {
return 42;
}
)",
        Program(
            {
            },
            {
                FunctionDefinition(
                    Id("main"),
                    ArgumentsDefinitionList({}),
                    StatementsList({
                               StatementReturn(Return(Expression(
                                       Additive({Multiplicative({UnaryConst(Constant(Double("42")))}, {})}, {}))
                               ))
                    })
                )
            }
        ),
        ""
    );
}

TEST(Parser, FunctionCall) {
    auto expression = Expression(
            Additive({Multiplicative({UnaryCall(FunctionCall(Id("func"), {}))}, {})}, {})
    );

    assertTextParsing(
        R"(
fun main() {
    return func();
}
)",
        Program(
            {
            },
            {
                FunctionDefinition(
                    Id("main"),
                    ArgumentsDefinitionList({}),
                    StatementsList({
                           StatementReturn(Return(expression))
                   })
                )
            }
        ),
        ""
    );
}

TEST(Parser, FunctionCallWithArguments) {
    auto arg1 = Expression(Additive({Multiplicative({UnaryId(Id("x1"))}, {})}, {}));
    auto arg2 = Expression(Additive({
        Multiplicative({UnaryConst(Constant(Double("4")))}, {}),
        Multiplicative({UnaryConst(Constant(Double("3")))}, {})}, {AdditiveExpressionNode::Sum}));
    auto arg3 = Expression(Additive({
        Multiplicative({UnaryId(Id("x3")), UnaryConst(Constant(Double("7.43")))}, {MultiplicativeExpressionNode::Mul})}, {}));

    auto expression = Expression(
        Additive({Multiplicative({UnaryCall(FunctionCall(Id("func"), {arg1, arg2, arg3}))}, {})}, {})
    );

    assertTextParsing(
        R"(
fun main() {
    return func(x1, 4 + 3, x3 * 7.43);
}
)",
        Program(
            {
            },
            {
                FunctionDefinition(
                    Id("main"),
                    ArgumentsDefinitionList({}),
                    StatementsList({
                                           StatementReturn(Return(expression))
                                   })
                )
            }
        ),
        ""
    );
}

TEST(Parser, ParenthesisExpressions) {
    auto par = Expression(Additive({Multiplicative({UnaryConst(Constant(Double("4")))}, {}), Multiplicative({UnaryConst(Constant(Double("3")))}, {})}, {AdditiveExpressionNode::Sub}));
    auto expression = Expression(
            Additive({
                Multiplicative({UnaryParenthesis(par), UnaryId(Id("x"))}, {MultiplicativeExpressionNode::Div})}, {})
    );

    assertTextParsing(
        R"(
fun main() {
    return (4 - 3) / x;
}
)",
        Program(
            {
            },
            {
                FunctionDefinition(
                    Id("main"),
                    ArgumentsDefinitionList({}),
                    StatementsList({
                                           StatementReturn(Return(expression))
                                   })
                )
            }
        ),
        ""
    );
}

static void assertParsingByPrint(const std::string &text, const std::string &expectedTree, const std::string &errorMessage) {
    auto tokensResult = TokenizeString(text);
    ASSERT_TRUE(tokensResult.Success);
    auto sut = ParseTokens(tokensResult.Tokens);

    ASSERT_EQ(errorMessage, sut.ErrorMessage);

    if (errorMessage.empty()) {
        ASSERT_TRUE(sut.Success);

        std::stringstream dumpStream;
        sut.ProgramNode->Print(dumpStream, "", true);
        auto sutS = dumpStream.str();

        Trim(sutS);
        auto trimmedExpected = TrimCopy(expectedTree);

        ASSERT_EQ(sutS, trimmedExpected);
    } else {
        EXPECT_FALSE(sut.Success);
    }
}

TEST(Parser, SeveralConstants) {
    assertParsingByPrint(R"(
x = 1;
x = 2;
e=+3.1415927;
pi=-12323423.23452345245;
)",
R"(
DcsProgramNode
 ├ConstantDefinitionNode
 │├IdNode 'x'
 │└ConstantValueNode 1.000000
 ├ConstantDefinitionNode
 │├IdNode 'x'
 │└ConstantValueNode 2.000000
 ├ConstantDefinitionNode
 │├IdNode 'e'
 │└ConstantValueNode 3.141593
 └ConstantDefinitionNode
  ├IdNode 'pi'
  └ConstantValueNode -12323423.234523
)",
"");
}

TEST(Parser, SeveralFunctions) {
    assertParsingByPrint(R"(
fun main() {}
fun lol(yyyyy) {}
fun ses(x, y, z) {}
fun _dsfdsf(_, _, _323) {}
)",
R"(
DcsProgramNode
 ├FunctionDefinitionNode
 │├IdNode 'main'
 │├ArgumentsDefinitionListNode
 │└StatementListNode
 ├FunctionDefinitionNode
 │├IdNode 'lol'
 │├ArgumentsDefinitionListNode
 ││└IdNode 'yyyyy'
 │└StatementListNode
 ├FunctionDefinitionNode
 │├IdNode 'ses'
 │├ArgumentsDefinitionListNode
 ││├IdNode 'x'
 ││├IdNode 'y'
 ││└IdNode 'z'
 │└StatementListNode
 └FunctionDefinitionNode
  ├IdNode '_dsfdsf'
  ├ArgumentsDefinitionListNode
  │├IdNode '_'
  │├IdNode '_'
  │└IdNode '_323'
  └StatementListNode
)",
"");
}

TEST(Parser, MainFunctionWithAssignAndReturnStatement) {
        assertParsingByPrint(R"(
pi = 3.141592;
e = 2.7;

fun main() {
    l = (3 + 4) * pi / e;
    return l * l * pi * e - 228 / 1337;
}
)", R"(
DcsProgramNode
 ├ConstantDefinitionNode
 │├IdNode 'pi'
 │└ConstantValueNode 3.141592
 ├ConstantDefinitionNode
 │├IdNode 'e'
 │└ConstantValueNode 2.700000
 └FunctionDefinitionNode
  ├IdNode 'main'
  ├ArgumentsDefinitionListNode
  └StatementListNode
   ├StatementNode
   │└AssignStatementNode
   │ ├IdNode 'l'
   │ └ExpressionNode
   │  └AdditiveExpressionNode
   │   └MultiplicativeExpressionNode
   │    ├UnaryExpressionNode
   │    │└ExpressionNode
   │    │ └AdditiveExpressionNode
   │    │  ├MultiplicativeExpressionNode
   │    │  │└UnaryExpressionNode
   │    │  │ └ConstantValueNode 3.000000
   │    │  ├SUM
   │    │  └MultiplicativeExpressionNode
   │    │   └UnaryExpressionNode
   │    │    └ConstantValueNode 4.000000
   │    ├MUL
   │    ├UnaryExpressionNode
   │    │└IdNode 'pi'
   │    ├DIV
   │    └UnaryExpressionNode
   │     └IdNode 'e'
   └StatementNode
    └ReturnStatementNode
     └ExpressionNode
      └AdditiveExpressionNode
       ├MultiplicativeExpressionNode
       │├UnaryExpressionNode
       ││└IdNode 'l'
       │├MUL
       │├UnaryExpressionNode
       ││└IdNode 'l'
       │├MUL
       │├UnaryExpressionNode
       ││└IdNode 'pi'
       │├MUL
       │└UnaryExpressionNode
       │ └IdNode 'e'
       ├SUB
       └MultiplicativeExpressionNode
        ├UnaryExpressionNode
        │└ConstantValueNode 228.000000
        ├DIV
        └UnaryExpressionNode
         └ConstantValueNode 1337.000000
)",
"");
}

TEST(Parser, AnotherFunctionTest) {
    assertParsingByPrint(R"(
pi = 3.141592;
e = 2.7;

fun another(x, y) {
    return pi * x * y / e;
}

fun main() {
    l = 1;
    l = 1 + 1;
    l = 1 + another(l, l);
    return l;
}
)", R"(
DcsProgramNode
 ├ConstantDefinitionNode
 │├IdNode 'pi'
 │└ConstantValueNode 3.141592
 ├ConstantDefinitionNode
 │├IdNode 'e'
 │└ConstantValueNode 2.700000
 ├FunctionDefinitionNode
 │├IdNode 'another'
 │├ArgumentsDefinitionListNode
 ││├IdNode 'x'
 ││└IdNode 'y'
 │└StatementListNode
 │ └StatementNode
 │  └ReturnStatementNode
 │   └ExpressionNode
 │    └AdditiveExpressionNode
 │     └MultiplicativeExpressionNode
 │      ├UnaryExpressionNode
 │      │└IdNode 'pi'
 │      ├MUL
 │      ├UnaryExpressionNode
 │      │└IdNode 'x'
 │      ├MUL
 │      ├UnaryExpressionNode
 │      │└IdNode 'y'
 │      ├DIV
 │      └UnaryExpressionNode
 │       └IdNode 'e'
 └FunctionDefinitionNode
  ├IdNode 'main'
  ├ArgumentsDefinitionListNode
  └StatementListNode
   ├StatementNode
   │└AssignStatementNode
   │ ├IdNode 'l'
   │ └ExpressionNode
   │  └AdditiveExpressionNode
   │   └MultiplicativeExpressionNode
   │    └UnaryExpressionNode
   │     └ConstantValueNode 1.000000
   ├StatementNode
   │└AssignStatementNode
   │ ├IdNode 'l'
   │ └ExpressionNode
   │  └AdditiveExpressionNode
   │   ├MultiplicativeExpressionNode
   │   │└UnaryExpressionNode
   │   │ └ConstantValueNode 1.000000
   │   ├SUM
   │   └MultiplicativeExpressionNode
   │    └UnaryExpressionNode
   │     └ConstantValueNode 1.000000
   ├StatementNode
   │└AssignStatementNode
   │ ├IdNode 'l'
   │ └ExpressionNode
   │  └AdditiveExpressionNode
   │   ├MultiplicativeExpressionNode
   │   │└UnaryExpressionNode
   │   │ └ConstantValueNode 1.000000
   │   ├SUM
   │   └MultiplicativeExpressionNode
   │    └UnaryExpressionNode
   │     └FunctionCallNode
   │      ├IdNode 'another'
   │      ├ExpressionNode
   │      │└AdditiveExpressionNode
   │      │ └MultiplicativeExpressionNode
   │      │  └UnaryExpressionNode
   │      │   └IdNode 'l'
   │      └ExpressionNode
   │       └AdditiveExpressionNode
   │        └MultiplicativeExpressionNode
   │         └UnaryExpressionNode
   │          └IdNode 'l'
   └StatementNode
    └ReturnStatementNode
     └ExpressionNode
      └AdditiveExpressionNode
       └MultiplicativeExpressionNode
        └UnaryExpressionNode
         └IdNode 'l'
)",
"");
}

TEST(Parser, ConditionalNoElse) {
    assertParsingByPrint(R"(
pi = 3.141592;
e = 2.7;

fun main() {
    l = -1;
    if (pi == e) { l = 1; }
    if (pi != e) { l = 2; }
    if (pi < e) { l = 3; }
    if (pi <= e) { l = 4; }
    if (pi > e) { l = 5; }
    if (pi >= e) { l = 6; }

    return l;
}
)", R"(
DcsProgramNode
 ├ConstantDefinitionNode
 │├IdNode 'pi'
 │└ConstantValueNode 3.141592
 ├ConstantDefinitionNode
 │├IdNode 'e'
 │└ConstantValueNode 2.700000
 └FunctionDefinitionNode
  ├IdNode 'main'
  ├ArgumentsDefinitionListNode
  └StatementListNode
   ├StatementNode
   │└AssignStatementNode
   │ ├IdNode 'l'
   │ └ExpressionNode
   │  └AdditiveExpressionNode
   │   └MultiplicativeExpressionNode
   │    └UnaryExpressionNode
   │     └ConstantValueNode -1.000000
   ├StatementNode
   │└ConditionalStatementNode EQ
   │ ├ExpressionNode
   │ │└AdditiveExpressionNode
   │ │ └MultiplicativeExpressionNode
   │ │  └UnaryExpressionNode
   │ │   └IdNode 'pi'
   │ ├ExpressionNode
   │ │└AdditiveExpressionNode
   │ │ └MultiplicativeExpressionNode
   │ │  └UnaryExpressionNode
   │ │   └IdNode 'e'
   │ └StatementListNode
   │  └StatementNode
   │   └AssignStatementNode
   │    ├IdNode 'l'
   │    └ExpressionNode
   │     └AdditiveExpressionNode
   │      └MultiplicativeExpressionNode
   │       └UnaryExpressionNode
   │        └ConstantValueNode 1.000000
   ├StatementNode
   │└ConditionalStatementNode NEQ
   │ ├ExpressionNode
   │ │└AdditiveExpressionNode
   │ │ └MultiplicativeExpressionNode
   │ │  └UnaryExpressionNode
   │ │   └IdNode 'pi'
   │ ├ExpressionNode
   │ │└AdditiveExpressionNode
   │ │ └MultiplicativeExpressionNode
   │ │  └UnaryExpressionNode
   │ │   └IdNode 'e'
   │ └StatementListNode
   │  └StatementNode
   │   └AssignStatementNode
   │    ├IdNode 'l'
   │    └ExpressionNode
   │     └AdditiveExpressionNode
   │      └MultiplicativeExpressionNode
   │       └UnaryExpressionNode
   │        └ConstantValueNode 2.000000
   ├StatementNode
   │└ConditionalStatementNode LESS
   │ ├ExpressionNode
   │ │└AdditiveExpressionNode
   │ │ └MultiplicativeExpressionNode
   │ │  └UnaryExpressionNode
   │ │   └IdNode 'pi'
   │ ├ExpressionNode
   │ │└AdditiveExpressionNode
   │ │ └MultiplicativeExpressionNode
   │ │  └UnaryExpressionNode
   │ │   └IdNode 'e'
   │ └StatementListNode
   │  └StatementNode
   │   └AssignStatementNode
   │    ├IdNode 'l'
   │    └ExpressionNode
   │     └AdditiveExpressionNode
   │      └MultiplicativeExpressionNode
   │       └UnaryExpressionNode
   │        └ConstantValueNode 3.000000
   ├StatementNode
   │└ConditionalStatementNode LE
   │ ├ExpressionNode
   │ │└AdditiveExpressionNode
   │ │ └MultiplicativeExpressionNode
   │ │  └UnaryExpressionNode
   │ │   └IdNode 'pi'
   │ ├ExpressionNode
   │ │└AdditiveExpressionNode
   │ │ └MultiplicativeExpressionNode
   │ │  └UnaryExpressionNode
   │ │   └IdNode 'e'
   │ └StatementListNode
   │  └StatementNode
   │   └AssignStatementNode
   │    ├IdNode 'l'
   │    └ExpressionNode
   │     └AdditiveExpressionNode
   │      └MultiplicativeExpressionNode
   │       └UnaryExpressionNode
   │        └ConstantValueNode 4.000000
   ├StatementNode
   │└ConditionalStatementNode GREAT
   │ ├ExpressionNode
   │ │└AdditiveExpressionNode
   │ │ └MultiplicativeExpressionNode
   │ │  └UnaryExpressionNode
   │ │   └IdNode 'pi'
   │ ├ExpressionNode
   │ │└AdditiveExpressionNode
   │ │ └MultiplicativeExpressionNode
   │ │  └UnaryExpressionNode
   │ │   └IdNode 'e'
   │ └StatementListNode
   │  └StatementNode
   │   └AssignStatementNode
   │    ├IdNode 'l'
   │    └ExpressionNode
   │     └AdditiveExpressionNode
   │      └MultiplicativeExpressionNode
   │       └UnaryExpressionNode
   │        └ConstantValueNode 5.000000
   ├StatementNode
   │└ConditionalStatementNode GE
   │ ├ExpressionNode
   │ │└AdditiveExpressionNode
   │ │ └MultiplicativeExpressionNode
   │ │  └UnaryExpressionNode
   │ │   └IdNode 'pi'
   │ ├ExpressionNode
   │ │└AdditiveExpressionNode
   │ │ └MultiplicativeExpressionNode
   │ │  └UnaryExpressionNode
   │ │   └IdNode 'e'
   │ └StatementListNode
   │  └StatementNode
   │   └AssignStatementNode
   │    ├IdNode 'l'
   │    └ExpressionNode
   │     └AdditiveExpressionNode
   │      └MultiplicativeExpressionNode
   │       └UnaryExpressionNode
   │        └ConstantValueNode 6.000000
   └StatementNode
    └ReturnStatementNode
     └ExpressionNode
      └AdditiveExpressionNode
       └MultiplicativeExpressionNode
        └UnaryExpressionNode
         └IdNode 'l'
)",
"");
}

TEST(Parser, ConditionalWithElse) {
    assertParsingByPrint(R"(
pi = 3.141592;
e = 2.7;

fun main() {
    if (pi > e) {
        return 1;
    } else {
        if (pi == e) {
            return 0;
        } else {
            return -1;
        }
    }

}
)", R"(
DcsProgramNode
 ├ConstantDefinitionNode
 │├IdNode 'pi'
 │└ConstantValueNode 3.141592
 ├ConstantDefinitionNode
 │├IdNode 'e'
 │└ConstantValueNode 2.700000
 └FunctionDefinitionNode
  ├IdNode 'main'
  ├ArgumentsDefinitionListNode
  └StatementListNode
   └StatementNode
    └ConditionalStatementNode GREAT
     ├ExpressionNode
     │└AdditiveExpressionNode
     │ └MultiplicativeExpressionNode
     │  └UnaryExpressionNode
     │   └IdNode 'pi'
     ├ExpressionNode
     │└AdditiveExpressionNode
     │ └MultiplicativeExpressionNode
     │  └UnaryExpressionNode
     │   └IdNode 'e'
     ├StatementListNode
     │└StatementNode
     │ └ReturnStatementNode
     │  └ExpressionNode
     │   └AdditiveExpressionNode
     │    └MultiplicativeExpressionNode
     │     └UnaryExpressionNode
     │      └ConstantValueNode 1.000000
     └StatementListNode
      └StatementNode
       └ConditionalStatementNode EQ
        ├ExpressionNode
        │└AdditiveExpressionNode
        │ └MultiplicativeExpressionNode
        │  └UnaryExpressionNode
        │   └IdNode 'pi'
        ├ExpressionNode
        │└AdditiveExpressionNode
        │ └MultiplicativeExpressionNode
        │  └UnaryExpressionNode
        │   └IdNode 'e'
        ├StatementListNode
        │└StatementNode
        │ └ReturnStatementNode
        │  └ExpressionNode
        │   └AdditiveExpressionNode
        │    └MultiplicativeExpressionNode
        │     └UnaryExpressionNode
        │      └ConstantValueNode 0.000000
        └StatementListNode
         └StatementNode
          └ReturnStatementNode
           └ExpressionNode
            └AdditiveExpressionNode
             └MultiplicativeExpressionNode
              └UnaryExpressionNode
               └ConstantValueNode -1.000000
)",
"");
}

TEST(Parser, AnotherExample) {
    assertParsingByPrint(R"(
fun main() {
    l = 1;
    t = 2;
    if ((l + 1) / (t + 2) == 1) {
        return 1;
    }
    return 0;
}
)", R"(
DcsProgramNode
 └FunctionDefinitionNode
  ├IdNode 'main'
  ├ArgumentsDefinitionListNode
  └StatementListNode
   ├StatementNode
   │└AssignStatementNode
   │ ├IdNode 'l'
   │ └ExpressionNode
   │  └AdditiveExpressionNode
   │   └MultiplicativeExpressionNode
   │    └UnaryExpressionNode
   │     └ConstantValueNode 1.000000
   ├StatementNode
   │└AssignStatementNode
   │ ├IdNode 't'
   │ └ExpressionNode
   │  └AdditiveExpressionNode
   │   └MultiplicativeExpressionNode
   │    └UnaryExpressionNode
   │     └ConstantValueNode 2.000000
   ├StatementNode
   │└ConditionalStatementNode EQ
   │ ├ExpressionNode
   │ │└AdditiveExpressionNode
   │ │ └MultiplicativeExpressionNode
   │ │  ├UnaryExpressionNode
   │ │  │└ExpressionNode
   │ │  │ └AdditiveExpressionNode
   │ │  │  ├MultiplicativeExpressionNode
   │ │  │  │└UnaryExpressionNode
   │ │  │  │ └IdNode 'l'
   │ │  │  ├SUM
   │ │  │  └MultiplicativeExpressionNode
   │ │  │   └UnaryExpressionNode
   │ │  │    └ConstantValueNode 1.000000
   │ │  ├DIV
   │ │  └UnaryExpressionNode
   │ │   └ExpressionNode
   │ │    └AdditiveExpressionNode
   │ │     ├MultiplicativeExpressionNode
   │ │     │└UnaryExpressionNode
   │ │     │ └IdNode 't'
   │ │     ├SUM
   │ │     └MultiplicativeExpressionNode
   │ │      └UnaryExpressionNode
   │ │       └ConstantValueNode 2.000000
   │ ├ExpressionNode
   │ │└AdditiveExpressionNode
   │ │ └MultiplicativeExpressionNode
   │ │  └UnaryExpressionNode
   │ │   └ConstantValueNode 1.000000
   │ └StatementListNode
   │  └StatementNode
   │   └ReturnStatementNode
   │    └ExpressionNode
   │     └AdditiveExpressionNode
   │      └MultiplicativeExpressionNode
   │       └UnaryExpressionNode
   │        └ConstantValueNode 1.000000
   └StatementNode
    └ReturnStatementNode
     └ExpressionNode
      └AdditiveExpressionNode
       └MultiplicativeExpressionNode
        └UnaryExpressionNode
         └ConstantValueNode 0.000000
)",
"");
}


TEST(Parser, ConditionalStatementWithoutElse) {
    assertParsingByPrint(R"(
fun main() {
    if (1 > 2) {
        return 1;
    }
    return 2;
}
)", R"(
DcsProgramNode
 └FunctionDefinitionNode
  ├IdNode 'main'
  ├ArgumentsDefinitionListNode
  └StatementListNode
   ├StatementNode
   │└ConditionalStatementNode GREAT
   │ ├ExpressionNode
   │ │└AdditiveExpressionNode
   │ │ └MultiplicativeExpressionNode
   │ │  └UnaryExpressionNode
   │ │   └ConstantValueNode 1.000000
   │ ├ExpressionNode
   │ │└AdditiveExpressionNode
   │ │ └MultiplicativeExpressionNode
   │ │  └UnaryExpressionNode
   │ │   └ConstantValueNode 2.000000
   │ └StatementListNode
   │  └StatementNode
   │   └ReturnStatementNode
   │    └ExpressionNode
   │     └AdditiveExpressionNode
   │      └MultiplicativeExpressionNode
   │       └UnaryExpressionNode
   │        └ConstantValueNode 1.000000
   └StatementNode
    └ReturnStatementNode
     └ExpressionNode
      └AdditiveExpressionNode
       └MultiplicativeExpressionNode
        └UnaryExpressionNode
         └ConstantValueNode 2.000000
)",
 "");
}

TEST(Parser, NegativeNumber) {
    assertParsingByPrint(R"(
fun main() {
    return -3.14;
}
)",
R"(
DcsProgramNode
 └FunctionDefinitionNode
  ├IdNode 'main'
  ├ArgumentsDefinitionListNode
  └StatementListNode
   └StatementNode
    └ReturnStatementNode
     └ExpressionNode
      └AdditiveExpressionNode
       └MultiplicativeExpressionNode
        └UnaryExpressionNode
         └ConstantValueNode -3.140000
)",
"");
}

TEST(Parser, NegativeExpression) {
    assertParsingByPrint(R"(
fun main() {
    return -(3.14 + 5);
}
)",
R"(
DcsProgramNode
 └FunctionDefinitionNode
  ├IdNode 'main'
  ├ArgumentsDefinitionListNode
  └StatementListNode
   └StatementNode
    └ReturnStatementNode
     └ExpressionNode
      └AdditiveExpressionNode
       └MultiplicativeExpressionNode
        └UnaryExpressionNode
         ├NEG
         └ExpressionNode
          └AdditiveExpressionNode
           └MultiplicativeExpressionNode
            └UnaryExpressionNode
             └ExpressionNode
              └AdditiveExpressionNode
               ├MultiplicativeExpressionNode
               │└UnaryExpressionNode
               │ └ConstantValueNode 3.140000
               ├SUM
               └MultiplicativeExpressionNode
                └UnaryExpressionNode
                 └ConstantValueNode 5.000000
)",
"");
}

TEST(Parser, Sub) {
    assertParsingByPrint(R"(
fun main() {
    return 3.14-5;
}
)",
R"(
DcsProgramNode
 └FunctionDefinitionNode
  ├IdNode 'main'
  ├ArgumentsDefinitionListNode
  └StatementListNode
   └StatementNode
    └ReturnStatementNode
     └ExpressionNode
      └AdditiveExpressionNode
       ├MultiplicativeExpressionNode
       │└UnaryExpressionNode
       │ └ConstantValueNode 3.140000
       ├SUB
       └MultiplicativeExpressionNode
        └UnaryExpressionNode
         └ConstantValueNode 5.000000
)",
"");
}

TEST(Parser, NegExpr) {
    assertParsingByPrint(R"(
fun main() {
    return -(5+6);
}
)",
R"(
DcsProgramNode
 └FunctionDefinitionNode
  ├IdNode 'main'
  ├ArgumentsDefinitionListNode
  └StatementListNode
   └StatementNode
    └ReturnStatementNode
     └ExpressionNode
      └AdditiveExpressionNode
       └MultiplicativeExpressionNode
        └UnaryExpressionNode
         ├NEG
         └ExpressionNode
          └AdditiveExpressionNode
           └MultiplicativeExpressionNode
            └UnaryExpressionNode
             └ExpressionNode
              └AdditiveExpressionNode
               ├MultiplicativeExpressionNode
               │└UnaryExpressionNode
               │ └ConstantValueNode 5.000000
               ├SUM
               └MultiplicativeExpressionNode
                └UnaryExpressionNode
                 └ConstantValueNode 6.000000
)",
"");
}

TEST(Parser, PlusExpr) {
    assertParsingByPrint(R"(
fun main() {
    return +(5+6);
}
)",
R"(
DcsProgramNode
 └FunctionDefinitionNode
  ├IdNode 'main'
  ├ArgumentsDefinitionListNode
  └StatementListNode
   └StatementNode
    └ReturnStatementNode
     └ExpressionNode
      └AdditiveExpressionNode
       └MultiplicativeExpressionNode
        └UnaryExpressionNode
         └ExpressionNode
          └AdditiveExpressionNode
           └MultiplicativeExpressionNode
            └UnaryExpressionNode
             └ExpressionNode
              └AdditiveExpressionNode
               ├MultiplicativeExpressionNode
               │└UnaryExpressionNode
               │ └ConstantValueNode 5.000000
               ├SUM
               └MultiplicativeExpressionNode
                └UnaryExpressionNode
                 └ConstantValueNode 6.000000
)",
"");
}

TEST(Parser, NegMinus) {
    assertParsingByPrint(R"(
fun main() {
    return -5-6;
}
)",
R"(
DcsProgramNode
 └FunctionDefinitionNode
  ├IdNode 'main'
  ├ArgumentsDefinitionListNode
  └StatementListNode
   └StatementNode
    └ReturnStatementNode
     └ExpressionNode
      └AdditiveExpressionNode
       ├MultiplicativeExpressionNode
       │└UnaryExpressionNode
       │ └ConstantValueNode -5.000000
       ├SUB
       └MultiplicativeExpressionNode
        └UnaryExpressionNode
         └ConstantValueNode 6.000000
)",
                         "");
}

TEST(Parser, NegNegNeg) {
    assertParsingByPrint(R"(
fun main() {
    return ---5;
}
)",
R"(
DcsProgramNode
 └FunctionDefinitionNode
  ├IdNode 'main'
  ├ArgumentsDefinitionListNode
  └StatementListNode
   └StatementNode
    └ReturnStatementNode
     └ExpressionNode
      └AdditiveExpressionNode
       └MultiplicativeExpressionNode
        └UnaryExpressionNode
         ├NEG
         └ExpressionNode
          └AdditiveExpressionNode
           └MultiplicativeExpressionNode
            └UnaryExpressionNode
             ├NEG
             └ExpressionNode
              └AdditiveExpressionNode
               └MultiplicativeExpressionNode
                └UnaryExpressionNode
                 └ConstantValueNode -5.000000
)",
"");
}
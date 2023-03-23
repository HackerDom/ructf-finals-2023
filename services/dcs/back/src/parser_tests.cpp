#include <gtest/gtest.h>

#include "parser.h"

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
        EXPECT_TRUE(sut.success);
        assertTreeEquals(expectedTree, sut.programNode);
    } else {
        EXPECT_FALSE(sut.success);
        ASSERT_EQ(expectedErrorMessage, sut.errorMessage);
    }
}

void assertTextParsing(const std::string &text, const std::shared_ptr<AstNode> &expectedTree, const std::string &expectedErrorMessage) {
    auto tokensResult = TokenizeString(text);
    ASSERT_TRUE(tokensResult.success);

    assertTokenParsing(tokensResult.tokens, expectedTree, expectedErrorMessage);
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

static std::shared_ptr<ConstantDefinitionNode> ConstantDefinition(const std::shared_ptr<IdNode> &id, const std::shared_ptr<ConstantValueNode> &value) {
    return std::make_shared<ConstantDefinitionNode>(id, value);
}

static std::shared_ptr<StatementListNode> StatementsList(std::initializer_list<std::shared_ptr<StatementNode>> statements) {
    return std::make_shared<StatementListNode>(statements);
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
        {Token(TokenType::Eof, "")},
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
        "expected ASSIGN, but got Token(TokenType::Name, \"x\")"
    );
}

TEST(Parser, ConstantDefinitionNoAssign) {
    assertTextParsing(
        R"(
x 42.43;
)",
        nullptr,
        "expected ASSIGN, but got Token(TokenType::Number, \"42.43\")"
    );
}

TEST(Parser, ConstantDefinitionNoSemicolon) {
    assertTextParsing(
        R"(
x = 42.43
)",
        nullptr,
        "expected SEMICOLON, but got Token(TokenType::Eof, \"\")"
    );
}

TEST(Parser, ConstantDefinitionNoNumber) {
    assertTextParsing(
        R"(
x = x;
)",
        nullptr,
        "expected NUMBER, but got Token(TokenType::Name, \"x\")"
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
        "expected LEFT_PAREN, but got Token(TokenType::LeftBrace, \"{\")"
    );
}


TEST(Parser, FunctionWithNoRightBraceAtArgumentsList) {
    assertTextParsing(
        R"(
fun main(x {}
)",
        nullptr,
        "expected RIGHT_PAREN, but got Token(TokenType::LeftBrace, \"{\")"
    );
}

TEST(Parser, FunctionNoBody) {
    assertTextParsing(
        R"(
fun main(x)
)",
        nullptr,
        "expected LEFT_BRACE, but got Token(TokenType::Eof, \"\")"
    );
}
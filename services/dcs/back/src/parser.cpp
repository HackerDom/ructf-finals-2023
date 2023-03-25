#include "parser.h"

#include <string>

#include "utils.h"

template<class T>
struct Parsed {
    std::shared_ptr<T> node;
    std::string errorMessage;
};

class ParserWithContext {
public:
    explicit ParserWithContext(const std::vector<Token> &tokens)
            : tokens(tokens), tokenIndex(0), tokensCount(static_cast<int>(tokens.size())) {
    }

    ParseResult ParseTokens();

private:
    const std::vector<Token> &tokens;
    int tokenIndex;
    int tokensCount;

    [[nodiscard]] const Token &currentToken() const {
        return tokens[tokenIndex];
    }

    [[nodiscard]] std::string currentTokenAbout() const {
        return tokenIndex < tokensCount ? currentToken().String() : "<nothing>";
    }

    [[nodiscard]] bool currentTokenIs(Token::Type t) {
        return tokenIndex < tokensCount && currentToken().type == t;
    }

    [[nodiscard]] bool nextTokenIs(Token::Type t) {
        return (tokenIndex + 1) < tokensCount && tokens[tokenIndex + 1].type == t;
    }

    void acceptToken() {
        ++tokenIndex;
    }

    Parsed<DcsProgramNode> readProgramNode();
    Parsed<ConstantDefinitionNode> readConstantDefinition();
    Parsed<IdNode> readIdNode();
    Parsed<ConstantValueNode> readConstantValueNode();
    Parsed<FunctionDefinitionNode> readFunctionDefinitionNode();
    Parsed<StatementListNode> readStatementsListNode();
    Parsed<ArgumentsDefinitionListNode> readArgumentsDefinitionListNode();
    Parsed<ReturnStatementNode> readReturnNode();
    Parsed<ExpressionNode> readExpressionNode();
    Parsed<AdditiveExpressionNode> readAdditiveExpressionNode();
    Parsed<StatementNode> readStatementNode();
    Parsed<MultiplicativeExpressionNode> readMultiplicativeExpressionNode();
    Parsed<UnaryExpressionNode> readUnaryExpressionNode();
    Parsed<FunctionCallNode> readFunctionCallNode();
    Parsed<AssignStatementNode> readAssigmentStatementNode();
    Parsed<ConditionalStatementNode> readConditionalStatementNode();

    static constexpr std::size_t kMaxFunctionArgumentsCount = 3;
};

Parsed<ConditionalStatementNode> ParserWithContext::readConditionalStatementNode() {
    if (!currentTokenIs(Token::Type::If)) {
        return {nullptr, Format("expected IF, but got %s", currentTokenAbout().c_str())};
    }
    acceptToken();

    if (!currentTokenIs(Token::Type::LeftParen)) {
        return {nullptr, Format("expected LEFT_PAREN, but got %s", currentTokenAbout().c_str())};
    }
    acceptToken();

    auto leftExpression = readExpressionNode();
    if (!leftExpression.errorMessage.empty()) {
        return {nullptr, leftExpression.errorMessage};
    }

    if (tokenIndex >= tokensCount) {
        return {nullptr, "expected conditional operator, but got <nothing>"};
    }

    auto cond = ConditionalStatementNode::ConditionType();

    switch (currentToken().type) {
        case Token::Less:
            cond = ConditionalStatementNode::Less;
            break;
        case Token::Great:
            cond = ConditionalStatementNode::Great;
            break;
        case Token::Eq:
            cond = ConditionalStatementNode::Eq;
            break;
        case Token::Neq:
            cond = ConditionalStatementNode::Neq;
            break;
        case Token::Le:
            cond = ConditionalStatementNode::Le;
            break;
        case Token::Ge:
            cond = ConditionalStatementNode::Ge;
            break;
        default:
            return {nullptr, Format("expected LESS | GREAT| EQ | NEQ | LE | GE, but got %s", currentTokenAbout().c_str())};
    }
    acceptToken();

    auto rightExpression = readExpressionNode();
    if (!rightExpression.errorMessage.empty()) {
        return {nullptr, rightExpression.errorMessage};
    }

    if (!currentTokenIs(Token::Type::RightParen)) {
        return {nullptr, Format("expected RIGHT_PAREN, but got %s", currentTokenAbout().c_str())};
    }
    acceptToken();

    auto thenBlock = readStatementsListNode();
    if (!thenBlock.errorMessage.empty()) {
        return {nullptr, thenBlock.errorMessage};
    }

    auto elseBlock = std::shared_ptr<StatementListNode>(nullptr);

    if (currentTokenIs(Token::Type::Else)) {
        acceptToken();

        auto e = readStatementsListNode();
        if (!e.errorMessage.empty()) {
            return {nullptr, e.errorMessage};
        }

        elseBlock = e.node;
    }

    return {std::make_shared<ConditionalStatementNode>(leftExpression.node, rightExpression.node, cond, thenBlock.node, elseBlock)};
}

Parsed<AssignStatementNode> ParserWithContext::readAssigmentStatementNode() {
    auto id = readIdNode();
    if (!id.errorMessage.empty()) {
        return {nullptr, id.errorMessage};
    }

    if (!currentTokenIs(Token::Type::Assign)) {
        return {nullptr, Format("expected ASSIGN, but got %s", currentTokenAbout().c_str())};
    }
    acceptToken();

    auto expression = readExpressionNode();
    if (!expression.errorMessage.empty()) {
        return {nullptr, expression.errorMessage};
    }

    if (!currentTokenIs(Token::Type::Semicolon)) {
        return {nullptr, Format("expected SEMICOLON, but got %s", currentTokenAbout().c_str())};
    }
    acceptToken();

    return {std::make_shared<AssignStatementNode>(id.node, expression.node)};
}

Parsed<FunctionCallNode> ParserWithContext::readFunctionCallNode() {
    auto id = readIdNode();
    if (!id.errorMessage.empty()) {
        return {nullptr, id.errorMessage};
    }

    if (!currentTokenIs(Token::Type::LeftParen)) {
        return {nullptr, Format("expected LEFT_PAREN, but got %s", currentTokenAbout().c_str())};
    }
    acceptToken();

    auto expressions = FunctionCallNode::ExpressionList();

    if (currentTokenIs(Token::Type::Name) ||
        currentTokenIs(Token::Type::Number) ||
        currentTokenIs(Token::Type::LeftParen)) {
        auto expression = readExpressionNode();
        if (!expression.errorMessage.empty()) {
            return {nullptr, expression.errorMessage};
        }
        expressions.push_back(expression.node);

        while (tokenIndex < tokensCount && currentTokenIs(Token::Type::Comma)) {
            acceptToken();

            expression = readExpressionNode();
            if (!expression.errorMessage.empty()) {
                return {nullptr, expression.errorMessage};
            }
            expressions.push_back(expression.node);
        }
    }

    if (!currentTokenIs(Token::Type::RightParen)) {
        return {nullptr, Format("expected RIGHT_PAREN, but got %s", currentTokenAbout().c_str())};
    }
    acceptToken();

    if (expressions.size() > kMaxFunctionArgumentsCount) {
        return {nullptr, Format("too many call arguments %u, expected no more than %u", expressions.size(), kMaxFunctionArgumentsCount)};
    }

    return {std::make_shared<FunctionCallNode>(id.node, expressions)};
}

Parsed<UnaryExpressionNode> ParserWithContext::readUnaryExpressionNode() {
    if (currentTokenIs(Token::Type::Name)) {
        if (nextTokenIs(Token::Type::LeftParen)) {
            auto fc = readFunctionCallNode();
            if (!fc.errorMessage.empty()) {
                return {nullptr, fc.errorMessage};
            }

            return {std::make_shared<UnaryExpressionNode>(nullptr, nullptr, fc.node, nullptr)};
        }

        auto id = readIdNode();
        if (!id.errorMessage.empty()) {
            return {nullptr, id.errorMessage};
        }

        return {std::make_shared<UnaryExpressionNode>(id.node, nullptr, nullptr, nullptr)};
    }

    if (currentTokenIs(Token::Type::Number)) {
        auto c = readConstantValueNode();
        if (!c.errorMessage.empty()) {
            return {nullptr, c.errorMessage};
        }

        return {std::make_shared<UnaryExpressionNode>(nullptr, c.node, nullptr, nullptr)};
    }

    if (currentTokenIs(Token::Type::LeftParen)) {
        acceptToken();

        auto e = readExpressionNode();
        if (!e.errorMessage.empty()) {
            return {nullptr, e.errorMessage};
        }

        if (currentTokenIs(Token::Type::RightParen)) {
            acceptToken();

            return {std::make_shared<UnaryExpressionNode>(nullptr, nullptr, nullptr, e.node)};
        }

    }

    return {nullptr, Format("expected NAME | NUMBER | LEFT_PAREN, but got %s", currentTokenAbout().c_str())};
}

Parsed<MultiplicativeExpressionNode> ParserWithContext::readMultiplicativeExpressionNode() {
    auto unaryExpressions = MultiplicativeExpressionNode::UnaryExpressionList();
    auto operations = MultiplicativeExpressionNode::OperationListT();

    auto unary = readUnaryExpressionNode();
    if (!unary.errorMessage.empty()) {
        return {nullptr, unary.errorMessage};
    }
    unaryExpressions.push_back(unary.node);

    while (tokenIndex < tokensCount) {
        if (currentTokenIs(Token::Type::Mul)) {
            acceptToken();

            unary = readUnaryExpressionNode();
            if (!unary.errorMessage.empty()) {
                return {nullptr, unary.errorMessage};
            }

            operations.push_back(MultiplicativeExpressionNode::OperationType::Mul);
            unaryExpressions.push_back(unary.node);
        } else if (currentTokenIs(Token::Type::Div)) {
            acceptToken();

            unary = readUnaryExpressionNode();
            if (!unary.errorMessage.empty()) {
                return {nullptr, unary.errorMessage};
            }

            operations.push_back(MultiplicativeExpressionNode::OperationType::Div);
            unaryExpressions.push_back(unary.node);
        } else {
            break;
        }
    }

    return {std::make_shared<MultiplicativeExpressionNode>(unaryExpressions, operations)};
}

Parsed<AdditiveExpressionNode> ParserWithContext::readAdditiveExpressionNode() {
    auto mulExpressions = AdditiveExpressionNode::MultiplicativeExpressionList();
    auto operations = AdditiveExpressionNode::OperationsListT();

    auto mul = readMultiplicativeExpressionNode();
    if (!mul.errorMessage.empty()) {
        return {nullptr, mul.errorMessage};
    }
    mulExpressions.push_back(mul.node);

    while (tokenIndex < tokensCount) {
        if (currentTokenIs(Token::Type::Plus)) {
            acceptToken();

            mul = readMultiplicativeExpressionNode();
            if (!mul.errorMessage.empty()) {
                return {nullptr, mul.errorMessage};
            }

            operations.push_back(AdditiveExpressionNode::OperationType::Sum);
            mulExpressions.push_back(mul.node);
        } else if (currentTokenIs(Token::Type::Minus)) {
            acceptToken();

            mul = readMultiplicativeExpressionNode();
            if (!mul.errorMessage.empty()) {
                return {nullptr, mul.errorMessage};
            }

            operations.push_back(AdditiveExpressionNode::OperationType::Sub);
            mulExpressions.push_back(mul.node);
        } else {
            break;
        }
    }

    return {std::make_shared<AdditiveExpressionNode>(mulExpressions, operations)};
}

Parsed<StatementNode> ParserWithContext::readStatementNode() {
    if (currentTokenIs(Token::Type::If)) {
        auto conditional = readConditionalStatementNode();
        if (!conditional.errorMessage.empty()) {
            return {nullptr, conditional.errorMessage};
        }

        return {std::make_shared<StatementNode>(conditional.node, nullptr, nullptr)};
    }

    if (currentTokenIs(Token::Type::Name)) {
        auto assigment = readAssigmentStatementNode();
        if (!assigment.errorMessage.empty()) {
            return {nullptr, assigment.errorMessage};
        }

        return {std::make_shared<StatementNode>(nullptr, nullptr, assigment.node)};
    }

    if (currentTokenIs(Token::Type::Return)) {
        auto r = readReturnNode();
        if (!r.errorMessage.empty()) {
            return {nullptr, r.errorMessage};
        }

        return {std::make_shared<StatementNode>(nullptr, r.node, nullptr)};
    }

    return {nullptr, Format("expected IF | NAME | RETURN, but got %s", currentTokenAbout().c_str())};
}

Parsed<ExpressionNode> ParserWithContext::readExpressionNode() {
    auto additive = readAdditiveExpressionNode();
    if (!additive.errorMessage.empty()) {
        return {nullptr, additive.errorMessage};
    }

    return {std::make_shared<ExpressionNode>(additive.node)};
}

Parsed<ReturnStatementNode> ParserWithContext::readReturnNode() {
    if (!currentTokenIs(Token::Type::Return)) {
        return {nullptr, Format("expected RETURN, but got %s", currentTokenAbout().c_str())};
    }
    acceptToken();

    auto expression = readExpressionNode();
    if (!expression.errorMessage.empty()) {
        return {nullptr, expression.errorMessage};
    }

    if (!currentTokenIs(Token::Type::Semicolon)) {
        return {nullptr, Format("expected SEMICOLON, but got %s", currentTokenAbout().c_str())};
    }
    acceptToken();

    return {std::make_shared<ReturnStatementNode>(expression.node)};
}

Parsed<ArgumentsDefinitionListNode> ParserWithContext::readArgumentsDefinitionListNode() {
    if (!currentTokenIs(Token::Type::LeftParen)) {
        return {nullptr, Format("expected LEFT_PAREN, but got %s", currentTokenAbout().c_str())};
    }
    acceptToken();

    auto ids = ArgumentsDefinitionListNode::IdList();

    if (currentTokenIs(Token::Type::Name)) {
        auto r = readIdNode();
        if (!r.errorMessage.empty()) {
            return {nullptr, r.errorMessage};
        }

        ids.push_back(r.node);

        while (tokenIndex < tokensCount && currentTokenIs(Token::Type::Comma)) {
            acceptToken();

            r = readIdNode();
            if (!r.errorMessage.empty()) {
                return {nullptr, r.errorMessage};
            }

            ids.push_back(r.node);
        }
    }

    if (!currentTokenIs(Token::Type::RightParen)) {
        return {nullptr, Format("expected RIGHT_PAREN, but got %s", currentTokenAbout().c_str())};
    }
    acceptToken();

    return {std::make_shared<ArgumentsDefinitionListNode>(ids)};
}

Parsed<StatementListNode> ParserWithContext::readStatementsListNode() {
    if (!currentTokenIs(Token::Type::LeftBrace)) {
        return {nullptr, Format("expected LEFT_BRACE, but got %s", currentTokenAbout().c_str())};
    }
    acceptToken();

    auto statements = StatementListNode::StatementNodeList();

    while (tokenIndex < tokensCount && !currentTokenIs(Token::Type::RightBrace)) {
        auto st = readStatementNode();
        if (!st.errorMessage.empty()) {
            return {nullptr, st.errorMessage};
        }

        statements.push_back(st.node);
    }

    if (!currentTokenIs(Token::Type::RightBrace)) {
        return {nullptr, Format("expected LEFT_BRACE, but got %s", currentTokenAbout().c_str())};
    }
    acceptToken();

    return {std::make_shared<StatementListNode>(statements)};
}

Parsed<FunctionDefinitionNode> ParserWithContext::readFunctionDefinitionNode() {
    if (!currentTokenIs(Token::Type::Fun)) {
        return {nullptr, Format("expected FUN, but got %s", currentTokenAbout().c_str())};
    }
    acceptToken();

    auto id = readIdNode();
    if (!id.errorMessage.empty()) {
        return {nullptr, id.errorMessage};
    }

    auto arguments = readArgumentsDefinitionListNode();
    if (!arguments.errorMessage.empty()) {
        return {nullptr, arguments.errorMessage};
    }

    auto statementsList = readStatementsListNode();
    if (!statementsList.errorMessage.empty()) {
        return {nullptr, statementsList.errorMessage};
    }

    if (arguments.node->Ids.size() > kMaxFunctionArgumentsCount) {
        return {nullptr, Format("too many arguments for function '%s', max is %u", id.node->Name.c_str(), kMaxFunctionArgumentsCount)};
    }

    return {std::make_shared<FunctionDefinitionNode>(id.node, arguments.node, statementsList.node)};
}

Parsed<IdNode> ParserWithContext::readIdNode() {
    const auto& t = currentToken();

    if (currentTokenIs(Token::Type::Name)) {
        acceptToken();
        return {std::make_shared<IdNode>(std::string{t.value})};
    }

    return {nullptr, Format("expected NAME, but got %s", currentTokenAbout().c_str())};
}

Parsed<ConstantValueNode> ParserWithContext::readConstantValueNode() {
    const auto &t = currentToken();

    if (!currentTokenIs(Token::Type::Number)) {
        return {nullptr, Format("expected NUMBER, but got %s", currentTokenAbout().c_str())};
    }

    std::string s{t.value};
    std::size_t p;
    auto d = std::stod(s, &p);

    if (p != s.size()) {
        return {nullptr, Format("can't read double from '%s'", s.c_str())};
    }

    acceptToken();

    return {std::make_shared<ConstantValueNode>(d)};
}

Parsed<ConstantDefinitionNode> ParserWithContext::readConstantDefinition() {
    auto id = readIdNode();
    if (!id.errorMessage.empty()) {
        return {nullptr, id.errorMessage};
    }

    if (!currentTokenIs(Token::Type::Assign)) {
        return {nullptr, Format("expected ASSIGN, but got %s", currentTokenAbout().c_str())};
    }
    acceptToken();

    auto v = readConstantValueNode();
    if (!v.errorMessage.empty()) {
        return {nullptr, v.errorMessage};
    }

    if (!currentTokenIs(Token::Type::Semicolon)) {
        return {nullptr, Format("expected SEMICOLON, but got %s", currentTokenAbout().c_str())};
    }
    acceptToken();

    return {std::make_shared<ConstantDefinitionNode>(id.node, v.node)};
}

Parsed<DcsProgramNode> ParserWithContext::readProgramNode() {
    auto constants = DcsProgramNode::ConstantsListT();
    auto functions = DcsProgramNode::FunctionsListT();

    while (tokenIndex < tokensCount) {
        if (currentTokenIs(Token::Type::Name)) {
            auto constant = readConstantDefinition();
            if (!constant.errorMessage.empty()) {
                return {nullptr, constant.errorMessage};
            }

            constants.push_back(constant.node);
        } else if (currentTokenIs(Token::Type::Fun)) {
            auto function = readFunctionDefinitionNode();
            if (!function.errorMessage.empty()) {
                return {nullptr, function.errorMessage};
            }

            functions.push_back(function.node);
        } else {
            break;
        }
    }

    if (currentTokenIs(Token::Type::Eof)) {
        return {std::make_shared<DcsProgramNode>(constants, functions)};
    }

    return {nullptr, Format("expected EOF, but got %s", currentTokenAbout().c_str())};
}

ParseResult ParserWithContext::ParseTokens() {
    auto programReadResult = readProgramNode();

    if (!programReadResult.errorMessage.empty()) {
        return {false, nullptr, programReadResult.errorMessage};
    }

    return {true, programReadResult.node};
}

ParseResult ParseTokens(const std::vector<Token> &tokens) {
    auto parser = ParserWithContext(tokens);

    return parser.ParseTokens();
}

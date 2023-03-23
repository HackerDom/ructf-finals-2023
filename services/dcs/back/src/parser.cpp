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

    [[nodiscard]] bool currentTokenIs(TokenType t) {
        return tokenIndex < tokensCount && currentToken().type == t;
    }

    [[nodiscard]] bool nextTokenIs(TokenType t) {
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
};

Parsed<AssignStatementNode> ParserWithContext::readAssigmentStatementNode() {
    auto id = readIdNode();
    if (!id.errorMessage.empty()) {
        return {nullptr, id.errorMessage};
    }

    if (!currentTokenIs(TokenType::Assign)) {
        return {nullptr, format("expected ASSIGN, but got %s", currentTokenAbout().c_str())};
    }
    acceptToken();

    auto expression = readExpressionNode();
    if (!expression.errorMessage.empty()) {
        return {nullptr, expression.errorMessage};
    }

    if (!currentTokenIs(TokenType::Semicolon)) {
        return {nullptr, format("expected SEMICOLON, but got %s", currentTokenAbout().c_str())};
    }
    acceptToken();

    return {std::make_shared<AssignStatementNode>(id.node, expression.node)};
}

Parsed<FunctionCallNode> ParserWithContext::readFunctionCallNode() {
    auto id = readIdNode();
    if (!id.errorMessage.empty()) {
        return {nullptr, id.errorMessage};
    }

    if (!currentTokenIs(TokenType::LeftParen)) {
        return {nullptr, format("expected LEFT_PAREN, but got %s", currentTokenAbout().c_str())};
    }
    acceptToken();

    auto expressions = FunctionCallNode::ExpressionList();

    if (currentTokenIs(TokenType::Name) ||
        currentTokenIs(TokenType::Number) ||
        currentTokenIs(TokenType::LeftParen)) {
        auto expression = readExpressionNode();
        if (!expression.errorMessage.empty()) {
            return {nullptr, expression.errorMessage};
        }
        expressions.push_back(expression.node);

        while (tokenIndex < tokensCount && currentTokenIs(TokenType::Comma)) {
            acceptToken();

            expression = readExpressionNode();
            if (!expression.errorMessage.empty()) {
                return {nullptr, expression.errorMessage};
            }
            expressions.push_back(expression.node);
        }
    }

    if (!currentTokenIs(TokenType::RightParen)) {
        return {nullptr, format("expected RIGHT_PAREN, but got %s", currentTokenAbout().c_str())};
    }
    acceptToken();

    return {std::make_shared<FunctionCallNode>(id.node, expressions)};
}

Parsed<UnaryExpressionNode> ParserWithContext::readUnaryExpressionNode() {
    if (currentTokenIs(TokenType::Name)) {
        if (nextTokenIs(TokenType::LeftParen)) {
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

    if (currentTokenIs(TokenType::Number)) {
        auto c = readConstantValueNode();
        if (!c.errorMessage.empty()) {
            return {nullptr, c.errorMessage};
        }

        return {std::make_shared<UnaryExpressionNode>(nullptr, c.node, nullptr, nullptr)};
    }

    if (currentTokenIs(TokenType::LeftParen)) {
        auto e = readExpressionNode();
        if (!e.errorMessage.empty()) {
            return {nullptr, e.errorMessage};
        }

        if (currentTokenIs(TokenType::RightParen)) {
            acceptToken();

            return {std::make_shared<UnaryExpressionNode>(nullptr, nullptr, nullptr, e.node)};
        }

    }

    return {nullptr, format("expected NAME | NUMBER | LEFT_PAREN, but got %s", currentTokenAbout().c_str())};
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
        if (currentTokenIs(TokenType::Mul)) {
            acceptToken();

            unary = readUnaryExpressionNode();
            if (!unary.errorMessage.empty()) {
                return {nullptr, unary.errorMessage};
            }

            operations.push_back(MultiplicativeExpressionNode::OperationType::Mul);
            unaryExpressions.push_back(unary.node);
        } else if (currentTokenIs(TokenType::Div)) {
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
        if (currentTokenIs(TokenType::Plus)) {
            acceptToken();

            mul = readMultiplicativeExpressionNode();
            if (!mul.errorMessage.empty()) {
                return {nullptr, mul.errorMessage};
            }

            operations.push_back(AdditiveExpressionNode::OperationType::Sum);
            mulExpressions.push_back(mul.node);
        } else if (currentTokenIs(TokenType::Minus)) {
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
    if (currentTokenIs(TokenType::If)) {
        return {nullptr, "if node read not implemented"};
    }

    if (currentTokenIs(TokenType::Name)) {
        auto assigment = readAssigmentStatementNode();
        if (!assigment.errorMessage.empty()) {
            return {nullptr, assigment.errorMessage};
        }

        return {std::make_shared<StatementNode>(nullptr, nullptr, assigment.node)};
    }

    if (currentTokenIs(TokenType::Return)) {
        auto r = readReturnNode();
        if (!r.errorMessage.empty()) {
            return {nullptr, r.errorMessage};
        }

        return {std::make_shared<StatementNode>(nullptr, r.node, nullptr)};
    }

    return {nullptr, format("expected IF | NAME | RETURN, but got %s", currentTokenAbout().c_str())};
}

Parsed<ExpressionNode> ParserWithContext::readExpressionNode() {
    auto additive = readAdditiveExpressionNode();
    if (!additive.errorMessage.empty()) {
        return {nullptr, additive.errorMessage};
    }

    return {std::make_shared<ExpressionNode>(additive.node)};
}

Parsed<ReturnStatementNode> ParserWithContext::readReturnNode() {
    if (!currentTokenIs(TokenType::Return)) {
        return {nullptr, format("expected RETURN, but got %s", currentTokenAbout().c_str())};
    }
    acceptToken();

    auto expression = readExpressionNode();
    if (!expression.errorMessage.empty()) {
        return {nullptr, expression.errorMessage};
    }

    if (!currentTokenIs(TokenType::Semicolon)) {
        return {nullptr, format("expected SEMICOLON, but got %s", currentTokenAbout().c_str())};
    }
    acceptToken();

    return {std::make_shared<ReturnStatementNode>(expression.node)};
}

Parsed<ArgumentsDefinitionListNode> ParserWithContext::readArgumentsDefinitionListNode() {
    if (!currentTokenIs(TokenType::LeftParen)) {
        return {nullptr, format("expected LEFT_PAREN, but got %s", currentTokenAbout().c_str())};
    }
    acceptToken();

    auto ids = ArgumentsDefinitionListNode::IdList();

    if (currentTokenIs(TokenType::Name)) {
        auto r = readIdNode();
        if (!r.errorMessage.empty()) {
            return {nullptr, r.errorMessage};
        }

        ids.push_back(r.node);

        while (tokenIndex < tokensCount && currentTokenIs(TokenType::Comma)) {
            acceptToken();

            r = readIdNode();
            if (!r.errorMessage.empty()) {
                return {nullptr, r.errorMessage};
            }

            ids.push_back(r.node);
        }
    }

    if (!currentTokenIs(TokenType::RightParen)) {
        return {nullptr, format("expected RIGHT_PAREN, but got %s", currentTokenAbout().c_str())};
    }
    acceptToken();

    return {std::make_shared<ArgumentsDefinitionListNode>(ids)};
}

Parsed<StatementListNode> ParserWithContext::readStatementsListNode() {
    if (!currentTokenIs(TokenType::LeftBrace)) {
        return {nullptr, format("expected LEFT_BRACE, but got %s", currentTokenAbout().c_str())};
    }
    acceptToken();

    auto statements = StatementListNode::StatementNodeList();

    while (tokenIndex < tokensCount && !currentTokenIs(TokenType::RightBrace)) {
        auto st = readStatementNode();
        if (!st.errorMessage.empty()) {
            return {nullptr, st.errorMessage};
        }

        statements.push_back(st.node);
    }

    if (!currentTokenIs(TokenType::RightBrace)) {
        return {nullptr, format("expected LEFT_BRACE, but got %s", currentTokenAbout().c_str())};
    }
    acceptToken();

    return {std::make_shared<StatementListNode>(statements)};
}

Parsed<FunctionDefinitionNode> ParserWithContext::readFunctionDefinitionNode() {
    if (!currentTokenIs(TokenType::Fun)) {
        return {nullptr, format("expected FUN, but got %s", currentTokenAbout().c_str())};
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

    const auto maxArguments = 3;

    if (arguments.node->Ids.size() > maxArguments) {
        return {nullptr, format("too many arguments for function '%s', max is %d", id.node->Name.c_str(), maxArguments)};
    }

    return {std::make_shared<FunctionDefinitionNode>(id.node, arguments.node, statementsList.node)};
}

Parsed<IdNode> ParserWithContext::readIdNode() {
    const auto& t = currentToken();

    if (currentTokenIs(TokenType::Name)) {
        acceptToken();
        return {std::make_shared<IdNode>(std::string{t.value})};
    }

    return {nullptr, format("expected NAME, but got %s", currentTokenAbout().c_str())};
}

Parsed<ConstantValueNode> ParserWithContext::readConstantValueNode() {
    const auto &t = currentToken();

    if (!currentTokenIs(TokenType::Number)) {
        return {nullptr, format("expected NUMBER, but got %s", currentTokenAbout().c_str())};
    }

    std::string s{t.value};
    std::size_t p;
    auto d = std::stod(s, &p);

    if (p != s.size()) {
        return {nullptr, format("can't read double from '%s'", s.c_str())};
    }

    acceptToken();

    return {std::make_shared<ConstantValueNode>(d)};
}

Parsed<ConstantDefinitionNode> ParserWithContext::readConstantDefinition() {
    auto id = readIdNode();
    if (!id.errorMessage.empty()) {
        return {nullptr, id.errorMessage};
    }

    if (!currentTokenIs(TokenType::Assign)) {
        return {nullptr, format("expected ASSIGN, but got %s", currentTokenAbout().c_str())};
    }
    acceptToken();

    auto v = readConstantValueNode();
    if (!v.errorMessage.empty()) {
        return {nullptr, v.errorMessage};
    }

    if (!currentTokenIs(TokenType::Semicolon)) {
        return {nullptr, format("expected SEMICOLON, but got %s", currentTokenAbout().c_str())};
    }
    acceptToken();

    return {std::make_shared<ConstantDefinitionNode>(id.node, v.node)};
}

Parsed<DcsProgramNode> ParserWithContext::readProgramNode() {
    auto constants = DcsProgramNode::ConstantsListT();
    auto functions = DcsProgramNode::FunctionsListT();

    while (tokenIndex < tokensCount) {
        const auto &next = currentToken();

        if (next.isName()) {
            auto constant = readConstantDefinition();
            if (!constant.errorMessage.empty()) {
                return {nullptr, constant.errorMessage};
            }

            constants.push_back(constant.node);
        } else if (next.isFun()) {
            auto function = readFunctionDefinitionNode();
            if (!function.errorMessage.empty()) {
                return {nullptr, function.errorMessage};
            }

            functions.push_back(function.node);
        } else {
            break;
        }
    }

    if (currentTokenIs(TokenType::Eof)) {
        return {std::make_shared<DcsProgramNode>(constants, functions)};
    }

    return {nullptr, format("expected EOF, but got %s", currentTokenAbout().c_str())};
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

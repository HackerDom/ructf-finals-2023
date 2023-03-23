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

    [[nodiscard]] std::string getCurrentTokenDescription() const {
        return tokenIndex < tokensCount ? currentToken().String() : "<nothing>";
    }

    [[nodiscard]] bool currentTokenIs(TokenType t) {
        return tokenIndex < tokensCount && currentToken().type == t;
    }

    void acceptToken(int count = 1) {
        tokenIndex += count;
    }

    Parsed<DcsProgramNode> readProgramNode();
    Parsed<ConstantDefinitionNode> readConstantDefinition();
    Parsed<IdNode> readIdNode();
    Parsed<ConstantValueNode> readConstantValueNode();
    Parsed<FunctionDefinitionNode> readFunctionDefinitionNode();
    Parsed<StatementListNode> readStatementsListNode();
    Parsed<ArgumentsDefinitionListNode> readArgumentsDefinitionListNode();
};

Parsed<ArgumentsDefinitionListNode> ParserWithContext::readArgumentsDefinitionListNode() {
    if (!currentTokenIs(TokenType::LeftParen)) {
        return {nullptr, format("expected LEFT_PAREN, but got %s", getCurrentTokenDescription().c_str())};
    }
    acceptToken();

    auto ids = ArgumentsDefinitionListNode::IdList();

    if (currentTokenIs(TokenType::Name)) {
        auto r = readIdNode();
        if (!r.errorMessage.empty()) {
            return {nullptr, r.errorMessage};
        }

        ids.push_back(r.node);

        while (currentTokenIs(TokenType::Comma)) {
            acceptToken();

            r = readIdNode();
            if (!r.errorMessage.empty()) {
                return {nullptr, r.errorMessage};
            }

            ids.push_back(r.node);
        }
    }

    if (!currentTokenIs(TokenType::RightParen)) {
        return {nullptr, format("expected RIGHT_PAREN, but got %s", getCurrentTokenDescription().c_str())};
    }
    acceptToken();

    return {std::make_shared<ArgumentsDefinitionListNode>(ids)};
}

Parsed<StatementListNode> ParserWithContext::readStatementsListNode() {
    if (!currentTokenIs(TokenType::LeftBrace)) {
        return {nullptr, format("expected LEFT_BRACE, but got %s", getCurrentTokenDescription().c_str())};
    }
    acceptToken();

    auto statements = StatementListNode::StatementNodeList();

    if (!currentTokenIs(TokenType::RightBrace)) {
        return {nullptr, format("expected LEFT_BRACE, but got %s", getCurrentTokenDescription().c_str())};
    }
    acceptToken();

    return {std::make_shared<StatementListNode>(statements)};
}

Parsed<FunctionDefinitionNode> ParserWithContext::readFunctionDefinitionNode() {
    if (!currentTokenIs(TokenType::Fun)) {
        return {nullptr, format("expected FUN, but got %s", getCurrentTokenDescription().c_str())};
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

    return {nullptr, format("expected NAME, but got %s", getCurrentTokenDescription().c_str())};
}

Parsed<ConstantValueNode> ParserWithContext::readConstantValueNode() {
    const auto &t = currentToken();

    if (!currentTokenIs(TokenType::Number)) {
        return {nullptr, format("expected NUMBER, but got %s", getCurrentTokenDescription().c_str())};
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
        return {nullptr, format("expected ASSIGN, but got %s", getCurrentTokenDescription().c_str())};
    }
    acceptToken();

    auto v = readConstantValueNode();
    if (!v.errorMessage.empty()) {
        return {nullptr, v.errorMessage};
    }

    if (!currentTokenIs(TokenType::Semicolon)) {
        return {nullptr, format("expected SEMICOLON, but got %s", getCurrentTokenDescription().c_str())};
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

    return {nullptr, format("expected EOF, but got %s", getCurrentTokenDescription().c_str())};
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

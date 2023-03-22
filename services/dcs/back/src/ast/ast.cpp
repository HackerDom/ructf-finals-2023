#include "ast.h"

NodesList FunctionCallNode::GetChildNodes() const {
    NodesList result;

    result.reserve(1 + Expressions.size());

    result.push_back(Id);

    for (const auto &p : Expressions) {
        result.push_back(p);
    }

    return result;
}

NodesList StatementListNode::GetChildNodes() const {
    NodesList result;

    result.reserve(Statements.size());

    for (const auto &p : Statements) {
        result.push_back(p);
    }

    return result;
}

NodesList AdditiveExpressionNode::GetChildNodes() const {
    NodesList result;

    result.reserve(MultiplicativeExpressions.size());

    for (const auto &p : MultiplicativeExpressions) {
        result.push_back(p);
    }

    return result;
}

NodesList MultiplicativeExpression::GetChildNodes() const {
    NodesList result;

    result.reserve(UnaryExpressions.size());

    for (const auto &p : UnaryExpressions) {
        result.push_back(p);
    }

    return result;
}

NodesList ArgumentsDefinitionListNode::GetChildNodes() const {
    NodesList result;

    result.reserve(Ids.size());

    for (const auto &p : Ids) {
        result.push_back(p);
    }

    return result;
}

NodesList DcsProgramNode::GetChildNodes() const {
    NodesList result;

    result.reserve(Constants.size() + Functions.size());

    for (const auto &p : Constants) {
        result.push_back(p);
    }

    for (const auto &p : Functions) {
        result.push_back(p);
    }

    return result;
}

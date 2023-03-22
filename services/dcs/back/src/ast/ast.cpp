#include "ast.h"

NodesList ConstantDefinitionNode::GetChildNodes() const {
    NodesList result;
    result.reserve(2);

    result.push_back(Id);
    result.push_back(Value);

    return result;
}

NodesList AssignStatementNode::GetChildNodes() const {
    NodesList result;
    result.reserve(2);

    result.push_back(Id);
    result.push_back(Expression);

    return result;
}

NodesList ParenthesisExpressionNode::GetChildNodes() const {
    NodesList result;
    result.push_back(Expression);

    return result;
}

NodesList UnaryExpressionNode::GetChildNodes() const {
    NodesList result;

    if (Id != nullptr) {
        result.push_back(Id);
    }

    if (Constant != nullptr) {
        result.push_back(Constant);
    }

    if (FunctionCall != nullptr) {
        result.push_back(FunctionCall);
    }

    if (ParenthesisExpression != nullptr) {
        result.push_back(ParenthesisExpression);
    }

    return result;
}

NodesList ExpressionNode::GetChildNodes() const {
    NodesList result;
    result.push_back(AdditiveExpression);

    return result;
}

NodesList ReturnStatementNode::GetChildNodes() const {
    NodesList result;
    result.push_back(Expression);

    return result;
}

NodesList ConditionalStatementNode::GetChildNodes() const {
    NodesList result;

    result.push_back(LeftExpression);
    result.push_back(RightExpression);
    result.push_back(ThenStatements);

    if (ElseStatements != nullptr) {
        result.push_back(ElseStatements);
    }

    return result;
}

NodesList StatementNode::GetChildNodes() const {
    NodesList result;

    if (Conditional != nullptr) {
        result.push_back(Conditional);
    }

    if (Return != nullptr) {
        result.push_back(Return);
    }

    if (Assign != nullptr) {
        result.push_back(Assign);
    }

    return result;
}

NodesList FunctionDefinitionNode::GetChildNodes() const {
    NodesList result;

    result.push_back(Id);
    result.push_back(Arguments);
    result.push_back(Body);

    return result;
}

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

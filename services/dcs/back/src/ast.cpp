#include <iostream>

#include "ast.h"

static std::string getNextTab(const std::string &tab, bool last) {
    return tab + (last ? " " : "│");
}

static void printHeader(std::ostream &stream, const std::string &tab, const std::string &name, bool leaf, bool newline = true) {
    if (!tab.empty()) {
        stream << tab << (leaf ? "└" : "├") << name;
    } else {
        stream << name;
    }
    if (newline) {
        stream << std::endl;
    }
}

void ConstantValueNode::Print(std::ostream &stream, const std::string &tab, bool last) const {
    printHeader(stream, tab, "ConstantValueNode", last, false);
    stream << " " << std::fixed << Value << std::endl;
}

void IdNode::Print(std::ostream &stream, const std::string &tab, bool last) const {
    printHeader(stream, tab, "IdNode '" + Name + "'", last);
}

void ConstantDefinitionNode::Print(std::ostream &stream, const std::string &tab, bool last) const {
    std::string nextTab = getNextTab(tab, last);
    printHeader(stream, tab, "ConstantDefinitionNode", last);
    Id->Print(stream, nextTab, false);
    Value->Print(stream, nextTab, true);
}

void AssignStatementNode::Print(std::ostream &stream, const std::string &tab, bool last) const {
    std::string nextTab = getNextTab(tab, last);
    printHeader(stream, tab, "AssignStatementNode", last);
    Id->Print(stream, nextTab, false);
    Expression->Print(stream, nextTab, true);
}

void FunctionCallNode::Print(std::ostream &stream, const std::string &tab, bool last) const {
    std::string nextTab = getNextTab(tab, last);
    printHeader(stream, tab, "FunctionCallNode", last);
    Id->Print(stream, nextTab, Expressions.empty());
    for (std::size_t i = 0; i < Expressions.size(); ++i) {
        Expressions[i]->Print(stream, nextTab, i == Expressions.size() - 1);
    }
}

void UnaryExpressionNode::Print(std::ostream &stream, const std::string &tab, bool last) const {
    std::string nextTab = getNextTab(tab, last);
    printHeader(stream, tab, "UnaryExpressionNode", last);
    if (Id != nullptr) {
        Id->Print(stream, nextTab, true);
    } else if (Constant != nullptr) {
        Constant->Print(stream, nextTab, true);
    } else if (FunctionCall != nullptr) {
        FunctionCall->Print(stream, nextTab, true);
    } else if (ParenthesisExpression != nullptr) {
        ParenthesisExpression->Print(stream, nextTab, true);
    }
}

void MultiplicativeExpressionNode::Print(std::ostream &stream, const std::string &tab, bool last) const {
    std::string nextTab = getNextTab(tab, last);
    printHeader(stream, tab, "MultiplicativeExpressionNode", last);

    if (!UnaryExpressions.empty()) {
        UnaryExpressions[0]->Print(stream, nextTab, UnaryExpressions.size() == 1);

        for (size_t i = 0; i < Operations.size(); ++i) {
            if (Operations[i] == Mul) {
                printHeader(stream, nextTab, "MUL", false);
            } else if (Operations[i] == Div) {
                printHeader(stream, nextTab, "DIV", false);
            }
            UnaryExpressions[i + 1]->Print(stream, nextTab, i + 1 == UnaryExpressions.size() - 1);
        }
    }
}

void AdditiveExpressionNode::Print(std::ostream &stream, const std::string &tab, bool last) const {
    std::string nextTab = getNextTab(tab, last);
    printHeader(stream, tab, "AdditiveExpressionNode", last);

    if (!MultiplicativeExpressions.empty()) {
        MultiplicativeExpressions[0]->Print(stream, nextTab, MultiplicativeExpressions.size() == 1);

        for (size_t i = 0; i < Operations.size(); ++i) {
            if (Operations[i] == Sum) {
                printHeader(stream, nextTab, "SUM", false);
            } else if (Operations[i] == Sub) {
                printHeader(stream, nextTab, "SUB", false);
            }
            MultiplicativeExpressions[i + 1]->Print(stream, nextTab, i + 1 == MultiplicativeExpressions.size() - 1);
        }
    }
}

void ExpressionNode::Print(std::ostream &stream, const std::string &tab, bool last) const {
    std::string nextTab = getNextTab(tab, last);
    printHeader(stream, tab, "ExpressionNode", last);
    AdditiveExpression->Print(stream, nextTab, true);
}

void ReturnStatementNode::Print(std::ostream &stream, const std::string &tab, bool last) const {
    std::string nextTab = getNextTab(tab, last);
    printHeader(stream, tab, "ReturnStatementNode", last);
    Expression->Print(stream, nextTab, true);
}

void StatementListNode::Print(std::ostream &stream, const std::string &tab, bool last) const {
    std::string nextTab = getNextTab(tab, last);
    printHeader(stream, tab, "StatementListNode", last);
    if (!Statements.empty()) {
        Statements[0]->Print(stream, nextTab, Statements.size() == 1);

        for (std::size_t i = 1; i < Statements.size(); ++i) {
            Statements[i]->Print(stream, nextTab, i == Statements.size() - 1);
        }
    }
}

void ConditionalStatementNode::Print(std::ostream &stream, const std::string &tab, bool last) const {
    std::string nextTab = getNextTab(tab, last);
    printHeader(stream, tab, "ConditionalStatementNode ", last, false);
    switch(Condition) {
        case Eq:
            stream << "EQ";
            break;
        case Neq:
            stream << "NEQ";
            break;
        case Less:
            stream << "LESS";
            break;
        case Le:
            stream << "LE";
            break;
        case Great:
            stream << "GREAT";
            break;
        case Ge:
            stream << "GE";
            break;
    }
    stream << std::endl;
    LeftExpression->Print(stream, nextTab, false);
    RightExpression->Print(stream, nextTab, false);
    ThenStatements->Print(stream, nextTab, ElseStatements == nullptr);
    if (ElseStatements != nullptr) {
        ElseStatements->Print(stream, nextTab, true);
    }
}

void StatementNode::Print(std::ostream &stream, const std::string &tab, bool last) const {
    std::string nextTab = getNextTab(tab, last);
    printHeader(stream, tab, "StatementNode", last);
    if (Conditional != nullptr) {
        Conditional->Print(stream, nextTab, true);
    } else if (Return != nullptr) {
        Return->Print(stream, nextTab, true);
    } else if (Assign != nullptr) {
        Assign->Print(stream, nextTab, true);
    }
}

void ArgumentsDefinitionListNode::Print(std::ostream &stream, const std::string &tab, bool last) const {
    std::string nextTab = getNextTab(tab, last);
    printHeader(stream, tab, "ArgumentsDefinitionListNode", last);

    if (!Ids.empty()) {
        Ids[0]->Print(stream, nextTab, Ids.size() == 1);

        for (std::size_t i = 1; i < Ids.size(); ++i) {
            Ids[i]->Print(stream, nextTab, i == Ids.size() - 1);
        }
    }
}

void FunctionDefinitionNode::Print(std::ostream &stream, const std::string &tab, bool last) const {
    std::string nextTab = getNextTab(tab, last);
    printHeader(stream, tab, "FunctionDefinitionNode", last);
    Id->Print(stream, nextTab, false);
    Arguments->Print(stream, nextTab, false);
    Body->Print(stream, nextTab, true);
}

void DcsProgramNode::Print(std::ostream &stream, const std::string &tab, bool last) const {
    std::string nextTab = getNextTab(tab, last);
    printHeader(stream, tab, "DcsProgramNode", last);

    if (!Constants.empty()) {
        Constants[0]->Print(stream, nextTab, (Constants.size() == 1) && Functions.empty());

        for (std::size_t i = 1; i < Constants.size(); ++i) {
            Constants[i]->Print(stream, nextTab, (i == Constants.size() - 1) && Functions.empty());
        }
    }

    if (!Functions.empty()) {
        Functions[0]->Print(stream, nextTab, Functions.size() == 1);

        for (std::size_t i = 1; i < Functions.size(); ++i) {
            Functions[i]->Print(stream, nextTab, i == Functions.size() - 1);
        }
    }
}

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

NodesList MultiplicativeExpressionNode::GetChildNodes() const {
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

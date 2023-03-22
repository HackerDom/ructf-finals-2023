#pragma once

#include <vector>
#include <ostream>
#include <memory>

class AstNode;
typedef std::vector<std::shared_ptr<AstNode>> NodesList;

class ExpressionNode;
class StatementNode;

class AstNode {
public:
    virtual ~AstNode() = default;
    virtual void Print(std::ostream &stream, int depth) const = 0;
    virtual NodesList GetChildNodes() const = 0;
};

class LeafAstNode : public AstNode {
public:
    NodesList GetChildNodes() const override {
        return NodesList();
    }
};

class ConstantValueNode : public LeafAstNode {
public:
    ConstantValueNode(double value) : Value(value) {
    }
    ~ConstantValueNode() override = default;
    void Print(std::ostream &stream, int depth) const override;

    double Value;
};

class IdNode : public LeafAstNode {
public:
    IdNode(std::string name) : Name(std::move(name)) {
    }
    ~IdNode() override = default;
    void Print(std::ostream &stream, int depth) const override;

    std::string Name;
};

class ConstantDefinitionNode : public AstNode {
public:
    ConstantDefinitionNode(std::shared_ptr<IdNode> id, std::shared_ptr<ConstantValueNode> value)
        : Id(std::move(id)), Value(std::move(value)) {
    }
    ~ConstantDefinitionNode() override = default;
    void Print(std::ostream &stream, int depth) const override;
    NodesList GetChildNodes() const override {
        return {Id, Value};
    }

    std::shared_ptr<IdNode> Id;
    std::shared_ptr<ConstantValueNode> Value;
};

class AssignStatementNode : public AstNode {
public:
    AssignStatementNode(std::shared_ptr<IdNode> id, std::shared_ptr<ExpressionNode> expression)
        : Id(std::move(id)), Expression(std::move(expression)) {
    }
    ~AssignStatementNode() override = default;
    void Print(std::ostream &stream, int depth) const override;
    NodesList GetChildNodes() const override {
        return {Id, Expression};
    }

    std::shared_ptr<IdNode> Id;
    std::shared_ptr<ExpressionNode> Expression;
};

class ParenthesisExpressionNode : public AstNode {
public:
    ParenthesisExpressionNode(std::shared_ptr<ExpressionNode> expression) : Expression(std::move(expression)) {
    }
    ~ParenthesisExpressionNode() override = default;
    void Print(std::ostream &stream, int depth) const override;
    NodesList GetChildNodes() const override {
        return {Expression};
    }

    std::shared_ptr<ExpressionNode> Expression;
};

class FunctionCallNode : public AstNode {
public:
    typedef std::vector<std::shared_ptr<ExpressionNode>> ExpressionList;

    FunctionCallNode(std::shared_ptr<IdNode> id, ExpressionList expressions) : Id(std::move(id)), Expressions(std::move(expressions)) {
    }
    ~FunctionCallNode() override = default;
    void Print(std::ostream &stream, int depth) const override;
    NodesList GetChildNodes() const override;

    std::shared_ptr<IdNode> Id;
    ExpressionList Expressions;
};

class UnaryExpressionNode : public AstNode {
public:
    UnaryExpressionNode(
        std::shared_ptr<IdNode> id,
        std::shared_ptr<ConstantValueNode> constant,
        std::shared_ptr<FunctionCallNode> functionCall,
        std::shared_ptr<ParenthesisExpressionNode> parenthesisExpression
    ) : Id(std::move(id)), Constant(std::move(constant)), FunctionCall(std::move(functionCall)), ParenthesisExpression(std::move(parenthesisExpression)) {
    }
    ~UnaryExpressionNode() override = default;
    void Print(std::ostream &stream, int depth) const override;
    NodesList GetChildNodes() const override {
        return {Id, Constant, FunctionCall, ParenthesisExpression};
    }

    std::shared_ptr<IdNode> Id;
    std::shared_ptr<ConstantValueNode> Constant;
    std::shared_ptr<FunctionCallNode> FunctionCall;
    std::shared_ptr<ParenthesisExpressionNode> ParenthesisExpression;
};

class MultiplicativeExpression : public AstNode {
public:
    typedef std::vector<std::shared_ptr<UnaryExpressionNode>> UnaryExpressionList;

    MultiplicativeExpression(UnaryExpressionList unaryExpressions) : UnaryExpressions(std::move(unaryExpressions)) {
    }
    ~MultiplicativeExpression() override = default;
    void Print(std::ostream &stream, int depth) const override;
    NodesList GetChildNodes() const override;

    UnaryExpressionList UnaryExpressions;
};

class AdditiveExpressionNode : public AstNode {
public:
    typedef std::vector<std::shared_ptr<MultiplicativeExpression>> MultiplicativeExpressionList;

    AdditiveExpressionNode(MultiplicativeExpressionList multiplicativeExpressions)
        : MultiplicativeExpressions(std::move(multiplicativeExpressions)) {
    }
    ~AdditiveExpressionNode() override = default;
    void Print(std::ostream &stream, int depth) const override;
    NodesList GetChildNodes() const override;

    MultiplicativeExpressionList MultiplicativeExpressions;
};

class ExpressionNode : public AstNode {
public:
    ExpressionNode(std::shared_ptr<AdditiveExpressionNode> additive) : AdditiveExpression(std::move(additive)) {
    }
    ~ExpressionNode() override = default;
    void Print(std::ostream &stream, int depth) const override;
    NodesList GetChildNodes() const override {
        return {AdditiveExpression};
    }

    std::shared_ptr<AdditiveExpressionNode> AdditiveExpression;
};

class ReturnStatementNode : public AstNode {
public:
    ReturnStatementNode(std::shared_ptr<ExpressionNode> expression) : Expression(std::move(expression)) {
    }
    ~ReturnStatementNode() override = default;
    void Print(std::ostream &stream, int depth) const override;
    NodesList GetChildNodes() const override {
        return {Expression};
    }

    std::shared_ptr<ExpressionNode> Expression;
};

class StatementListNode : public AstNode {
public:
    typedef std::vector<std::shared_ptr<StatementNode>> StatementNodeList;

    StatementListNode(StatementNodeList statements) : Statements(std::move(statements)) {
    }
    ~StatementListNode() override = default;
    void Print(std::ostream &stream, int depth) const override;
    NodesList GetChildNodes() const override;

    StatementNodeList Statements;
};

class ConditionalStatementNode : public AstNode {
public:
    enum ConditionType {
        Eq,
        Neq,
        Less,
        Le,
        Great,
        Ge
    };

    ConditionalStatementNode(
        std::shared_ptr<ExpressionNode> leftExpression,
        std::shared_ptr<ExpressionNode> rightExpression,
        ConditionType condition,
        std::shared_ptr<StatementListNode> thenStatements,
        std::shared_ptr<StatementListNode> elseStatements
    ) : LeftExpression(std::move(leftExpression)),
        RightExpression(std::move(rightExpression)),
        Condition(condition),
        ThenStatements(std::move(thenStatements)),
        ElseStatements(std::move(elseStatements)) {
    }
    ~ConditionalStatementNode() override = default;
    void Print(std::ostream &stream, int depth) const override;
    NodesList GetChildNodes() const override {
        return {LeftExpression, RightExpression, ThenStatements, ElseStatements};
    }

    std::shared_ptr<ExpressionNode> LeftExpression;
    std::shared_ptr<ExpressionNode> RightExpression;
    ConditionType Condition;
    std::shared_ptr<StatementListNode> ThenStatements;
    std::shared_ptr<StatementListNode> ElseStatements;
};

class StatementNode : public AstNode {
public:
    StatementNode(
        std::shared_ptr<ConditionalStatementNode> conditional,
        std::shared_ptr<ReturnStatementNode> return_,
        std::shared_ptr<AssignStatementNode> assign
    ) : Conditional(std::move(conditional)), Return(std::move(return_)), Assign(std::move(assign)) {
    }
    ~StatementNode() override = default;
    void Print(std::ostream &stream, int depth) const override;
    NodesList GetChildNodes() const override {
        return {Conditional, Return, Assign};
    }

    std::shared_ptr<ConditionalStatementNode> Conditional;
    std::shared_ptr<ReturnStatementNode> Return;
    std::shared_ptr<AssignStatementNode> Assign;
};


class ArgumentsDefinitionListNode : public AstNode {
public:
    typedef std::vector<std::shared_ptr<IdNode>> IdList;

    ArgumentsDefinitionListNode(IdList ids) : Ids(std::move(ids)) {
    }
    ~ArgumentsDefinitionListNode() override = default;
    void Print(std::ostream &stream, int depth) const override;
    NodesList GetChildNodes() const override;

    IdList Ids;
};

class FunctionDefinitionNode : public AstNode {
public:
    FunctionDefinitionNode(
        std::shared_ptr<IdNode> id,
        std::shared_ptr<ArgumentsDefinitionListNode> arguments,
        std::shared_ptr<StatementListNode> body
    ) : Id(std::move(id)), Arguments(std::move(arguments)), Body(std::move(body)) {
    }
    ~FunctionDefinitionNode() override = default;
    void Print(std::ostream &stream, int depth) const override;
    NodesList GetChildNodes() const override {
        return {Arguments, Body};
    }

    std::shared_ptr<IdNode> Id;
    std::shared_ptr<ArgumentsDefinitionListNode> Arguments;
    std::shared_ptr<StatementListNode> Body;
};

class DcsProgramNode : public AstNode {
public:
    typedef std::vector<std::shared_ptr<FunctionDefinitionNode>> FunctionsListT;
    typedef std::vector<std::shared_ptr<ConstantDefinitionNode>> ConstantsListT;

    DcsProgramNode(ConstantsListT constants, FunctionsListT functions)
        : Constants(std::move(constants)), Functions(std::move(functions)) {
    }
    ~DcsProgramNode() override = default;
    void Print(std::ostream &stream, int depth) const override;
    NodesList GetChildNodes() const override;

    ConstantsListT Constants;
    FunctionsListT Functions;
};

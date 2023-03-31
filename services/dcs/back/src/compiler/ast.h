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
    enum Type {
        ConstantValue,
        Id,
        ConstantDefinition,
        AssignStatement,
        FunctionCall,
        UnaryExpression,
        AdditiveExpression,
        MultiplicativeExpression,
        Expression,
        ReturnStatement,
        StatementList,
        ConditionalStatement,
        Statement,
        ArgumentsDefinitionList,
        FunctionDefinition,
        DcsProgram
    };

    explicit AstNode(Type nodeType) : NodeType(nodeType) {
    }
    virtual ~AstNode() = default;
    virtual void Print(std::ostream &stream, const std::string &tab, bool last) const = 0;
    [[nodiscard]] virtual NodesList GetChildNodes() const = 0;

    const Type NodeType;
};

class LeafAstNode : public AstNode {
public:
    explicit LeafAstNode(Type nodeType) : AstNode(nodeType) {
    }
    [[nodiscard]] NodesList GetChildNodes() const override {
        return {};
    }
};

class ConstantValueNode : public LeafAstNode {
public:
    explicit ConstantValueNode(double value) : LeafAstNode(Type::ConstantValue), Value(value) {
    }
    ~ConstantValueNode() override = default;
    void Print(std::ostream &stream, const std::string &tab, bool last) const override;

    double Value;
};

class IdNode : public LeafAstNode {
public:
    explicit IdNode(std::string name) : LeafAstNode(Type::Id), Name(std::move(name)) {
    }
    ~IdNode() override = default;
     void Print(std::ostream &stream, const std::string &tab, bool last) const override;

    std::string Name;
};

class ConstantDefinitionNode : public AstNode {
public:
    ConstantDefinitionNode(std::shared_ptr<IdNode> id, std::shared_ptr<ConstantValueNode> value)
        : AstNode(Type::ConstantDefinition), Id(std::move(id)), Value(std::move(value)) {
    }
    ~ConstantDefinitionNode() override = default;
    void Print(std::ostream &stream, const std::string &tab, bool last) const override;
    [[nodiscard]] NodesList GetChildNodes() const override;

    std::shared_ptr<IdNode> Id;
    std::shared_ptr<ConstantValueNode> Value;
};

class AssignStatementNode : public AstNode {
public:
    AssignStatementNode(std::shared_ptr<IdNode> id, std::shared_ptr<ExpressionNode> expression)
        : AstNode(Type::AssignStatement), Id(std::move(id)), Expression(std::move(expression)) {
    }
    ~AssignStatementNode() override = default;
    void Print(std::ostream &stream, const std::string &tab, bool last) const override;
    [[nodiscard]] NodesList GetChildNodes() const override;

    std::shared_ptr<IdNode> Id;
    std::shared_ptr<ExpressionNode> Expression;
};

class FunctionCallNode : public AstNode {
public:
    typedef std::vector<std::shared_ptr<ExpressionNode>> ExpressionList;

    FunctionCallNode(std::shared_ptr<IdNode> id, ExpressionList expressions)
        : AstNode(Type::FunctionCall), Id(std::move(id)), Expressions(std::move(expressions)) {
    }
    ~FunctionCallNode() override = default;
    void Print(std::ostream &stream, const std::string &tab, bool last) const override;
    [[nodiscard]] NodesList GetChildNodes() const override;

    std::shared_ptr<IdNode> Id;
    ExpressionList Expressions;
};

class UnaryExpressionNode : public AstNode {
public:
    UnaryExpressionNode(
        std::shared_ptr<IdNode> id,
        std::shared_ptr<ConstantValueNode> constant,
        std::shared_ptr<FunctionCallNode> functionCall,
        std::shared_ptr<ExpressionNode> parenthesisExpression,
        std::shared_ptr<ExpressionNode> negativeExpression
    ) : AstNode(Type::UnaryExpression),
        Id(std::move(id)),
        Constant(std::move(constant)),
        FunctionCall(std::move(functionCall)),
        ParenthesisExpression(std::move(parenthesisExpression)),
        NegationExression(std::move(negativeExpression)) {
    }
    ~UnaryExpressionNode() override = default;
    void Print(std::ostream &stream, const std::string &tab, bool last) const override;
    [[nodiscard]] NodesList GetChildNodes() const override;

    std::shared_ptr<IdNode> Id;
    std::shared_ptr<ConstantValueNode> Constant;
    std::shared_ptr<FunctionCallNode> FunctionCall;
    std::shared_ptr<ExpressionNode> ParenthesisExpression;
    std::shared_ptr<ExpressionNode> NegationExression;
};

class MultiplicativeExpressionNode : public AstNode {
public:
    enum OperationType {
        Mul,
        Div
    };

    typedef std::vector<std::shared_ptr<UnaryExpressionNode>> UnaryExpressionList;
    typedef std::vector<OperationType> OperationListT;

    explicit MultiplicativeExpressionNode(UnaryExpressionList unaryExpressions, OperationListT operations)
        : AstNode(Type::MultiplicativeExpression), UnaryExpressions(std::move(unaryExpressions)), Operations(std::move(operations)) {
    }
    ~MultiplicativeExpressionNode() override = default;
    void Print(std::ostream &stream, const std::string &tab, bool last) const override;
    [[nodiscard]] NodesList GetChildNodes() const override;

    UnaryExpressionList UnaryExpressions;
    OperationListT Operations;
};

class AdditiveExpressionNode : public AstNode {
public:
    enum OperationType {
        Sum,
        Sub
    };

    typedef std::vector<std::shared_ptr<MultiplicativeExpressionNode>> MultiplicativeExpressionList;
    typedef std::vector<OperationType> OperationsListT;

    explicit AdditiveExpressionNode(MultiplicativeExpressionList multiplicativeExpressions, OperationsListT operations)
        : AstNode(Type::AdditiveExpression), MultiplicativeExpressions(std::move(multiplicativeExpressions)), Operations(std::move(operations)) {
    }
    ~AdditiveExpressionNode() override = default;
    void Print(std::ostream &stream, const std::string &tab, bool last) const override;
    [[nodiscard]] NodesList GetChildNodes() const override;

    MultiplicativeExpressionList MultiplicativeExpressions;
    OperationsListT Operations;
};

class ExpressionNode : public AstNode {
public:
    explicit ExpressionNode(std::shared_ptr<AdditiveExpressionNode> additive)
        : AstNode(Type::Expression), AdditiveExpression(std::move(additive)) {
    }
    ~ExpressionNode() override = default;
    void Print(std::ostream &stream, const std::string &tab, bool last) const override;
    [[nodiscard]] NodesList GetChildNodes() const override;

    std::shared_ptr<AdditiveExpressionNode> AdditiveExpression;
};

class ReturnStatementNode : public AstNode {
public:
    explicit ReturnStatementNode(std::shared_ptr<ExpressionNode> expression)
        : AstNode(Type::ReturnStatement), Expression(std::move(expression)) {
    }
    ~ReturnStatementNode() override = default;
    void Print(std::ostream &stream, const std::string &tab, bool last) const override;
    [[nodiscard]] NodesList GetChildNodes() const override;

    std::shared_ptr<ExpressionNode> Expression;
};

class StatementListNode : public AstNode {
public:
    typedef std::vector<std::shared_ptr<StatementNode>> StatementNodeList;

    explicit StatementListNode(StatementNodeList statements)
        : AstNode(Type::StatementList), Statements(std::move(statements)) {
    }
    ~StatementListNode() override = default;
    void Print(std::ostream &stream, const std::string &tab, bool last) const override;
    [[nodiscard]] NodesList GetChildNodes() const override;

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
    ) : AstNode(Type::ConditionalStatement),
        LeftExpression(std::move(leftExpression)),
        RightExpression(std::move(rightExpression)),
        Condition(condition),
        ThenStatements(std::move(thenStatements)),
        ElseStatements(std::move(elseStatements)) {
    }
    ~ConditionalStatementNode() override = default;
    void Print(std::ostream &stream, const std::string &tab, bool last) const override;
    [[nodiscard]] NodesList GetChildNodes() const override;

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
    ) : AstNode(Type::Statement), Conditional(std::move(conditional)), Return(std::move(return_)), Assign(std::move(assign)) {
    }
    ~StatementNode() override = default;
    void Print(std::ostream &stream, const std::string &tab, bool last) const override;
    [[nodiscard]] NodesList GetChildNodes() const override;

    std::shared_ptr<ConditionalStatementNode> Conditional;
    std::shared_ptr<ReturnStatementNode> Return;
    std::shared_ptr<AssignStatementNode> Assign;
};

class ArgumentsDefinitionListNode : public AstNode {
public:
    typedef std::vector<std::shared_ptr<IdNode>> IdList;

    explicit ArgumentsDefinitionListNode(IdList ids) : AstNode(Type::ArgumentsDefinitionList), Ids(std::move(ids)) {
    }
    ~ArgumentsDefinitionListNode() override = default;
    void Print(std::ostream &stream, const std::string &tab, bool last) const override;
    [[nodiscard]] NodesList GetChildNodes() const override;

    IdList Ids;
};

class FunctionDefinitionNode : public AstNode {
public:
    FunctionDefinitionNode(
        std::shared_ptr<IdNode> id,
        std::shared_ptr<ArgumentsDefinitionListNode> arguments,
        std::shared_ptr<StatementListNode> body
    ) : AstNode(Type::FunctionDefinition), Id(std::move(id)), Arguments(std::move(arguments)), Body(std::move(body)) {
    }
    ~FunctionDefinitionNode() override = default;
    void Print(std::ostream &stream, const std::string &tab, bool last) const override;
    [[nodiscard]] NodesList GetChildNodes() const override;

    std::shared_ptr<IdNode> Id;
    std::shared_ptr<ArgumentsDefinitionListNode> Arguments;
    std::shared_ptr<StatementListNode> Body;
};

class DcsProgramNode : public AstNode {
public:
    typedef std::vector<std::shared_ptr<FunctionDefinitionNode>> FunctionsListT;
    typedef std::vector<std::shared_ptr<ConstantDefinitionNode>> ConstantsListT;

    DcsProgramNode(ConstantsListT constants, FunctionsListT functions)
        : AstNode(Type::DcsProgram), Constants(std::move(constants)), Functions(std::move(functions)) {
    }
    ~DcsProgramNode() override = default;
    void Print(std::ostream &stream, const std::string &tab, bool last) const override;
    [[nodiscard]] NodesList GetChildNodes() const override;

    ConstantsListT Constants;
    FunctionsListT Functions;
};

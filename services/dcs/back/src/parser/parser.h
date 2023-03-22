#pragma once

#include <vector>
#include <memory>

#include "lexer.h"

class AstNode {
public:
    virtual ~AstNode() = default;

    virtual void Print(int indent) = 0;

    virtual std::vector<std::shared_ptr<AstNode>> GetChildNodes() const = 0;
};

class ConstantDefinitionNode : public AstNode {
public:
    ~ConstantDefinitionNode() = default;

    void Print(int indent);

    std::vector<std::shared_ptr<AstNode>> GetChildNodes() const;
};

class FunctionDefinitionNode : public AstNode {
public:
    ~FunctionDefinitionNode() = default;

    void Print(int indent);

    std::vector<std::shared_ptr<AstNode>> GetChildNodes() const;
};

class DcsProgramNode : public AstNode {
public:
    ~DcsProgramNode() = default;

    void Print(int indent);

    std::vector<std::shared_ptr<AstNode>> GetChildNodes() const;

    std::vector<std::shared_ptr<ConstantDefinitionNode>> ConstantDefinitions;
    std::vector<std::shared_ptr<FunctionDefinitionNode>> FunctionDefinitionNodes;
};


struct ParseResult {
    bool success;
    std::shared_ptr<DcsProgramNode> programNode;
    std::string errorMessage;
};

ParseResult ParseTokens(const std::vector<Token> &tokens);

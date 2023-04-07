#pragma once

#include <lexer/lexer.h>
#include <ast/ast.h>

struct ParseResult {
    bool Success;
    std::shared_ptr<DcsProgramNode> ProgramNode;
    std::string ErrorMessage;
};

ParseResult ParseTokens(const std::vector<Token> &tokens);

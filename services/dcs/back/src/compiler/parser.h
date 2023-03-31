#pragma once

#include "lexer.h"
#include "ast.h"

struct ParseResult {
    bool Success;
    std::shared_ptr<DcsProgramNode> ProgramNode;
    std::string ErrorMessage;
};

ParseResult ParseTokens(const std::vector<Token> &tokens);

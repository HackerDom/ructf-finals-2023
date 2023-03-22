#pragma once

#include "lexer.h"
#include "ast.h"


struct ParseResult {
    bool success;
    std::shared_ptr<DcsProgramNode> programNode;
    std::string errorMessage;
};

ParseResult ParseTokens(const std::vector<Token> &tokens);

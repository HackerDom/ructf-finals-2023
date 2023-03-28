#pragma once

#include <string>
#include <filesystem>

#include "ast.h"

struct CompilationResult {
    bool success;
    std::string assemblyCode;
    std::string errorMessage;
};

CompilationResult CompileToAssembly(const std::shared_ptr<DcsProgramNode> &program);

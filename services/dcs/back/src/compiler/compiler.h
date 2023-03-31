#pragma once

#include <string>
#include <filesystem>

#include "ast.h"

struct CompilationResult {
    bool Success;
    std::string AssemblyCode;
    std::string ErrorMessage;
};

CompilationResult CompileToAssembly(const std::shared_ptr<DcsProgramNode> &program);

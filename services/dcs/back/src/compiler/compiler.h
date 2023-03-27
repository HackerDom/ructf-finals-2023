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

struct FullCompilationResult {
    bool success;
    std::filesystem::path soPath;
    std::filesystem::path sourcePath;
    std::filesystem::path objectPath;
    std::string errorMessage;
};
// if out is "", some temporarily file will be used
FullCompilationResult CompileToSharedLibrary(const std::shared_ptr<DcsProgramNode> &program, const std::filesystem::path &out = "");

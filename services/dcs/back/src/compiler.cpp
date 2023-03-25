#include <sstream>
#include <map>
#include <unordered_map>
#include <vector>
#include <queue>

#include "compiler.h"
#include "utils.h"

class CompilerWithContext {
public:
    explicit CompilerWithContext(std::shared_ptr<DcsProgramNode> root) : root(std::move(root)) {
    }

    [[nodiscard]] CompilationResult Compile();

private:
    std::shared_ptr<DcsProgramNode> root;

    struct EmitResult {
        std::string assembly;
        std::string error;
    };

    EmitResult emitFunctions();
    EmitResult emitFunction(const std::shared_ptr<FunctionDefinitionNode> &function);
    EmitResult emitConstants();
    EmitResult emitHeader();

    struct ConstantCompilationContext {
        std::shared_ptr<ConstantDefinitionNode> definitionNode;
    };

    struct VariableCompilationContext {
        int Num = -1;
        int ArgumentRegNum = -1;
    };

    struct FunctionCompilationContext {
        std::shared_ptr<FunctionDefinitionNode> definitionNode;
        std::unordered_map<std::shared_ptr<ConstantValueNode>, std::shared_ptr<ConstantDefinitionNode>> constantToItsDefinition;
        std::unordered_map<std::shared_ptr<IdNode>, std::shared_ptr<ConstantDefinitionNode>> constantUsageToItsDefinition;
        std::unordered_map<std::string, std::shared_ptr<VariableCompilationContext>> variableNameToItsContext;
        std::unordered_map<std::shared_ptr<IdNode>, std::shared_ptr<VariableCompilationContext>> variableUsageToItsContext;
        std::unordered_map<std::shared_ptr<IdNode>, std::shared_ptr<FunctionCompilationContext>> functionUsageToItsContext;
    };

    std::map<std::string, std::shared_ptr<ConstantCompilationContext>> constantNameToContext;
    std::map<std::string, std::shared_ptr<FunctionCompilationContext>> functionNameToContext;
    std::shared_ptr<FunctionCompilationContext> currentCompilationFunction;
    std::string functionEmitError;

    bool emitFunctionBody(std::ostream &out, const std::shared_ptr<StatementListNode> &body);
    bool emitStatement(std::ostream &out, const std::shared_ptr<StatementNode> &statement);
    bool emitReturnStatement(std::ostream &out, const std::shared_ptr<ReturnStatementNode> &returnStatement);
    bool emitExpression(std::ostream &out, const std::shared_ptr<ExpressionNode> &expression);
    bool emitAdditive(std::ostream &out, const std::shared_ptr<AdditiveExpressionNode> &expression);
    bool emitMultiplicative(std::ostream &out, const std::shared_ptr<MultiplicativeExpressionNode> &mul);
    bool emitUnary(std::ostream &out, const std::shared_ptr<UnaryExpressionNode> &unary);
    bool emitConstant(std::ostream &out, const std::shared_ptr<ConstantValueNode> &constant);
    bool emitId(std::ostream &out, const std::shared_ptr<IdNode> &id);

    std::string fillContexts();
    std::string appendUserDefinedConstantsToContext();
    std::string createFunctionContexts();
    std::string defineConstantsFromFunctions();
    std::string defineConstantsFromFunction(const std::shared_ptr<FunctionDefinitionNode> &function);
    std::string fillConstantUsages();
    std::string fillConstantUsageToItsDefinition(const std::shared_ptr<FunctionDefinitionNode> &function);
    std::string createVariableContexts();
    std::string createVariableContexts(const std::shared_ptr<FunctionDefinitionNode> &function);
    std::string fillVariableUsages();
    std::string fillVariableUsages(const std::shared_ptr<FunctionDefinitionNode> &function);
    std::string fillFunctionUsages();
    std::string fillFunctionUsages(const std::shared_ptr<FunctionDefinitionNode> &function);

    std::string makeChecks();

    static constexpr std::string_view kTab              = "    ";
    static constexpr std::string_view kPushRbp          = "push    %rbp";
    static constexpr std::string_view kMovRspRbp        = "mov     %rsp,%rbp";
    static constexpr const char *kSubRspFmt             = "sub     $%#x,%%rsp"; // ex: sub $0x10,%rsp
    static constexpr const char *kMovSdXmmRbpFmt        = "movsd   %%xmm%d,-%#x(%%rbp)"; // ex: movsd  %xmm0,-0x8(%rbp)
    static constexpr const char *kMovSdRbpXmm0Fmt       = "movsd   -%#x(%%rbp),%%xmm0"; // ex: movsd -0x8(%rbp),%xmm0
    static constexpr std::string_view kLeaveq           = "leaveq";
    static constexpr std::string_view kRetq             = "retq";
    static constexpr std::string_view kSub0x10Rsp       = "sub     $0x10,%rsp";
    static constexpr std::string_view kMovSdXmm0Rsp     = "movsd   %xmm0,(%rsp)";
    static constexpr std::string_view kMovSdRspXmm0     = "movsd   (%rsp),%xmm0";
    static constexpr std::string_view kAdd0x10Rsp       = "add     $0x10,%rsp";
    static constexpr std::string_view kMovXmm0Xmm1      = "movaps  %xmm0,%xmm1";
    static constexpr std::string_view kAddSdXmm1Xmm0    = "addsd   %xmm1,%xmm0";
    static constexpr std::string_view kSubSdXmm1Xmm0    = "subsd   %xmm1,%xmm0";
    static constexpr std::string_view kMulSdXmm1Xmm0    = "mulsd   %xmm1,%xmm0";
    static constexpr std::string_view kDivSdXmm1Xmm0    = "divsd   %xmm1,%xmm0";
    static constexpr const char *kMovSdAddrXmm0Fmt      = "movsd   %s(%%rip),%%xmm0"; // ex: movsd pi(%rip),%xmm0
};

std::string CompilerWithContext::makeChecks() {
    auto mainIt = functionNameToContext.find("main");
    if (mainIt == functionNameToContext.end()) {
        return "there is no main function in program";
    }

    if (!mainIt->second->definitionNode->Arguments->Ids.empty()) {
        return "main function cant have any arguments";
    }

    return "";
}

std::string CompilerWithContext::fillFunctionUsages(const std::shared_ptr<FunctionDefinitionNode> &function) {
    auto fit = functionNameToContext.find(function->Id->Name);
    if (fit == functionNameToContext.end()) {
        return Format("context for function '%s' is undefined", function->Id->Name.c_str());
    }
    auto &functionContext = fit->second;
    auto &idToFunctionContext = functionContext->functionUsageToItsContext;

    std::queue<std::shared_ptr<AstNode>> q;
    q.push(function->Body);

    while (!q.empty()) {
        auto next = q.front();
        q.pop();

        if (next->NodeType == AstNode::FunctionCall) {
            auto fc = std::dynamic_pointer_cast<FunctionCallNode>(next);
            if (auto it = functionNameToContext.find(fc->Id->Name); it != functionNameToContext.end()) {
                if (fc->Expressions.size() != it->second->definitionNode->Arguments->Ids.size()) {
                    return Format(
                            "invalid arguments count for function call '%s': expected %u, but got %u (in function '%s')",
                            fc->Id->Name.c_str(), it->second->definitionNode->Arguments->Ids.size(),
                            fc->Expressions.size(), function->Id->Name.c_str());
                }

                idToFunctionContext[fc->Id] = it->second;
            } else {
                return Format("unknown function call '%s' in '%s'", fc->Id->Name.c_str(), function->Id->Name.c_str());
            }

            for (const auto &c : fc->Expressions) {
                q.push(c);
            }
        } else {
            for (const auto &c : next->GetChildNodes()) {
                q.push(c);
            }
        }
    }

    return "";
}

std::string CompilerWithContext::fillFunctionUsages() {
    for (const auto &fd : root->Functions) {
        auto err = fillFunctionUsages(fd);
        if (!err.empty()) {
            return err;
        }
    }

    return "";
}

std::string CompilerWithContext::fillVariableUsages() {
    for (const auto &fd : root->Functions) {
        auto err = fillVariableUsages(fd);
        if (!err.empty()) {
            return err;
        }
    }

    return "";
}

std::string CompilerWithContext::fillVariableUsages(const std::shared_ptr<FunctionDefinitionNode> &function) {
    auto fit = functionNameToContext.find(function->Id->Name);
    if (fit == functionNameToContext.end()) {
        return Format("context for function '%s' is undefined", function->Id->Name.c_str());
    }
    auto &functionContext = fit->second;
    auto &nameToVariableContext = functionContext->variableNameToItsContext;
    auto &idToVarContext = functionContext->variableUsageToItsContext;

    std::queue<std::shared_ptr<AstNode>> q;
    q.push(function->Body);

    while (!q.empty()) {
        auto next = q.front();
        q.pop();

        if (next->NodeType == AstNode::AssignStatement) {
            auto as = std::dynamic_pointer_cast<AssignStatementNode>(next);
            q.push(as->Expression);
            continue;
        } else if (next->NodeType == AstNode::FunctionCall) {
            auto as = std::dynamic_pointer_cast<FunctionCallNode>(next);
            for (const auto &c : as->Expressions) {
                q.push(c);
            }
            continue;
        } else if (next->NodeType == AstNode::Id) {
            auto id = std::dynamic_pointer_cast<IdNode>(next);
            if (auto it = constantNameToContext.find(id->Name); it != constantNameToContext.end()) {
                continue;
            }
            if (auto it = nameToVariableContext.find(id->Name); it != nameToVariableContext.end()) {
                idToVarContext[id] = it->second;
            } else {
                return Format("unknown variable '%s' in function '%s'", id->Name.c_str(), function->Id->Name.c_str());
            }

            continue;
        }

        for (const auto &c : next->GetChildNodes()) {
            q.push(c);
        }
    }

    return "";
}

std::string CompilerWithContext::createVariableContexts(const std::shared_ptr<FunctionDefinitionNode> &function) {
    auto fit = functionNameToContext.find(function->Id->Name);
    if (fit == functionNameToContext.end()) {
        return Format("context for function '%s' is undefined", function->Id->Name.c_str());
    }
    auto &functionContext = fit->second;
    auto &nameToVariableContext = functionContext->variableNameToItsContext;

    auto count = 1;

    for (const auto &arg : function->Arguments->Ids) {
        if (auto it = nameToVariableContext.find(arg->Name); it == nameToVariableContext.end()) {
            auto c = std::make_shared<VariableCompilationContext>();
            c->ArgumentRegNum = count - 1;
            c->Num = (count++) * static_cast<int>(sizeof(double));

            nameToVariableContext.emplace_hint(it, arg->Name, c);
        } else {
            return Format("redefinition of argument '%s' in function '%s'", arg->Name.c_str(), function->Id->Name.c_str());
        }
    }

    std::queue<std::shared_ptr<AstNode>> q;
    q.push(function->Body);

    while (!q.empty()) {
        auto next = q.front();
        q.pop();

        if (next->NodeType == AstNode::AssignStatement) {
            auto as = std::dynamic_pointer_cast<AssignStatementNode>(next);
            if (auto it = constantNameToContext.find(as->Id->Name); it != constantNameToContext.end()) {
                return Format("cant assign to constant '%s' in function '%s'", as->Id->Name.c_str(), function->Id->Name.c_str());
            }

            if (auto it = nameToVariableContext.find(as->Id->Name); it == nameToVariableContext.end()) {
                auto c = std::make_shared<VariableCompilationContext>();
                c->Num = (count++) * static_cast<int>(sizeof(double));
                nameToVariableContext.emplace_hint(it, as->Id->Name, c);
            }

            continue;
        }

        for (const auto &c : next->GetChildNodes()) {
            q.push(c);
        }
    }

    return "";
}

std::string CompilerWithContext::createVariableContexts() {
    for (const auto &fd : root->Functions) {
        auto err = createVariableContexts(fd);
        if (!err.empty()) {
            return err;
        }
    }

    return "";
}

std::string CompilerWithContext::fillConstantUsageToItsDefinition(
        const std::shared_ptr<FunctionDefinitionNode> &function) {
    auto fit = functionNameToContext.find(function->Id->Name);
    if (fit == functionNameToContext.end()) {
        return Format("context for function '%s' is undefined", function->Id->Name.c_str());
    }
    auto &functionContext = fit->second;

    std::queue<std::shared_ptr<AstNode>> q;
    q.push(function);

    while (!q.empty()) {
        auto next = q.front();
        q.pop();

        if (next->NodeType == AstNode::AssignStatement) {
            auto as = std::dynamic_pointer_cast<AssignStatementNode>(next);
            if (auto it = constantNameToContext.find(as->Id->Name); it != constantNameToContext.end()) {
                return Format("cant create local variable with name '%s': there is constant with that name", as->Id->Name.c_str());
            }

            if (auto it = functionNameToContext.find(as->Id->Name); it != functionNameToContext.end()) {
                return Format("cant create local variable with name '%s': there is function with that name", as->Id->Name.c_str());
            }

            q.push(as->Expression);

            continue;
        } else if (next->NodeType == AstNode::ArgumentsDefinitionList) {
            auto dl = std::dynamic_pointer_cast<ArgumentsDefinitionListNode>(next);
            for (const auto &arg : dl->Ids) {
                if (auto it = constantNameToContext.find(arg->Name); it != constantNameToContext.end()) {
                    return Format("cant create argument for '%s' with name '%s': there is constant with that name", function->Id->Name.c_str(), arg->Name.c_str());
                }

                if (auto it = functionNameToContext.find(arg->Name); it != functionNameToContext.end()) {
                    return Format("cant create argument for '%s' with name '%s': there is function with that name", function->Id->Name.c_str(), arg->Name.c_str());
                }
            }

            continue;
        } else if (next->NodeType == AstNode::Id) {
            auto id = std::dynamic_pointer_cast<IdNode>(next);
            if (auto it = constantNameToContext.find(id->Name); it != constantNameToContext.end()) {
                functionContext->constantUsageToItsDefinition[id] = it->second->definitionNode;
            }

            continue;
        }

        for (const auto &c : next->GetChildNodes()) {
            q.push(c);
        }
    }

    return "";
}

std::string CompilerWithContext::fillConstantUsages() {
    for (const auto &fd : root->Functions) {
        auto err = fillConstantUsageToItsDefinition(fd);
        if (!err.empty()) {
            return err;
        }
    }

    return "";
}

std::string CompilerWithContext::defineConstantsFromFunction(const std::shared_ptr<FunctionDefinitionNode> &function) {
    auto fit = functionNameToContext.find(function->Id->Name);
    if (fit == functionNameToContext.end()) {
        return Format("context for function '%s' is undefined", function->Id->Name.c_str());
    }
    auto &functionContext = fit->second;

    auto constantsPrefix = Format("_c_const_%s_", function->Id->Name.c_str());
    auto count = 0;

    std::queue<std::shared_ptr<AstNode>> q;
    q.push(function);

    while (!q.empty()) {
        auto next = q.front();
        q.pop();

        if (next->NodeType == AstNode::ConstantValue) {
            auto value = std::dynamic_pointer_cast<ConstantValueNode>(next);
            auto constantName = Format("%s%d", constantsPrefix.c_str(), count++);
            auto id = std::make_shared<IdNode>(constantName);
            auto constDef = std::make_shared<ConstantDefinitionNode>(id, value);
            auto it = constantNameToContext.find(constDef->Id->Name);
            if (it == constantNameToContext.end()) {
                auto c = std::make_shared<ConstantCompilationContext>(ConstantCompilationContext{constDef});
                constantNameToContext.emplace_hint(it, constDef->Id->Name, c);
                functionContext->constantToItsDefinition[value] = constDef;
            } else {
                return Format("cant define constant '%s' (do not define it manually)", constantName.c_str());
            }
        }

        for (const auto &c : next->GetChildNodes()) {
            q.push(c);
        }
    }

    return "";
}

std::string CompilerWithContext::defineConstantsFromFunctions() {
    for (const auto &fd : root->Functions) {
        auto err = defineConstantsFromFunction(fd);
        if (!err.empty()) {
            return err;
        }
    }

    return "";
}

std::string CompilerWithContext::appendUserDefinedConstantsToContext() {
    for (const auto &p : root->Constants) {
        auto it = constantNameToContext.find(p->Id->Name);

        if (it == constantNameToContext.end()) {
            auto c = std::make_shared<ConstantCompilationContext>(ConstantCompilationContext{p});
            constantNameToContext.emplace_hint(it, p->Id->Name, c);
        } else {
            return Format("constant '%s' is defined twice", p->Id->Name.c_str());
        }
    }

    return "";
}

std::string CompilerWithContext::createFunctionContexts() {
    for (const auto &f : root->Functions) {
        auto it = functionNameToContext.find(f->Id->Name);
        if (it == functionNameToContext.end()) {
            if (auto itc = constantNameToContext.find(f->Id->Name); itc != constantNameToContext.end()) {
                return Format("cant define function '%s': there is constant with that name", f->Id->Name.c_str());
            }

            auto c = std::make_shared<FunctionCompilationContext>(FunctionCompilationContext{.definitionNode=f});

            functionNameToContext.emplace_hint(it, f->Id->Name, c);
        } else {
            return Format("function '%s' is defined twice", f->Id->Name.c_str());
        }
    }

    return "";
}

std::string CompilerWithContext::fillContexts() {
    auto err = appendUserDefinedConstantsToContext();
    if (!err.empty()) {
        return err;
    }

    err = createFunctionContexts();
    if (!err.empty()) {
        return err;
    }

    err = defineConstantsFromFunctions();
    if (!err.empty()) {
        return err;
    }

    err = fillConstantUsages();
    if (!err.empty()) {
        return err;
    }

    err = createVariableContexts();
    if (!err.empty()) {
        return err;
    }

    err = fillVariableUsages();
    if (!err.empty()) {
        return err;
    }

    err = fillFunctionUsages();
    if (!err.empty()) {
        return err;
    }

    return "";
}

CompilerWithContext::EmitResult CompilerWithContext::emitHeader() {
    auto text = std::string(".section .text\n");

    if (functionNameToContext.find("main") != functionNameToContext.end()) {
        text += ".globl main\n";
    }

    return {text};
}

CompilerWithContext::EmitResult CompilerWithContext::emitFunctions() {
    std::stringstream ss;

    for (const auto &fd : root->Functions) {
        auto er = emitFunction(fd);
        if (!er.error.empty()) {
            return {"", er.error};
        }

        ss << er.assembly << std::endl;
    }

    return {ss.str()};
}

bool CompilerWithContext::emitId(std::ostream &out, const std::shared_ptr<IdNode> &id) {
    if (currentCompilationFunction == nullptr) {
        functionEmitError = "call emitId outside of function compilation";
        return false;
    }

    if (auto it = currentCompilationFunction->constantUsageToItsDefinition.find(id); it != currentCompilationFunction->constantUsageToItsDefinition.end()) {
        out << kTab << Format(kMovSdAddrXmm0Fmt, it->second->Id->Name.c_str()) << std::endl;
        return true;
    }

    if (auto it = currentCompilationFunction->variableNameToItsContext.find(id->Name); it != currentCompilationFunction->variableNameToItsContext.end()) {
        out << kTab << Format(kMovSdRbpXmm0Fmt, it->second->Num) << std::endl;
        return true;
    }

    functionEmitError = Format("context for '%s' not found in constants and variable (in '%s')", id->Name.c_str(), currentCompilationFunction->definitionNode->Id->Name.c_str());
    return false;
}

bool CompilerWithContext::emitConstant(std::ostream &out, const std::shared_ptr<ConstantValueNode> &constant) {
    if (currentCompilationFunction == nullptr) {
        functionEmitError = "call emitConstant outside of function compilation";
        return false;
    }

    if (auto it = currentCompilationFunction->constantToItsDefinition.find(constant); it != currentCompilationFunction->constantToItsDefinition.end()) {
        out << kTab << Format(kMovSdAddrXmm0Fmt, it->second->Id->Name.c_str()) << std::endl;
        return true;
    }

    functionEmitError = Format("context for constant '%f' not found (in '%s')", constant->Value, currentCompilationFunction->definitionNode->Id->Name.c_str());
    return false;
}

bool CompilerWithContext::emitUnary(std::ostream &out, const std::shared_ptr<UnaryExpressionNode> &unary) {
    if (unary->Constant != nullptr) {
        return emitConstant(out, unary->Constant);
    }

    if (unary->ParenthesisExpression != nullptr) {
        return emitExpression(out, unary->ParenthesisExpression);
    }

    if (unary->Id != nullptr) {
        return emitId(out, unary->Id);
    }

    if (unary->FunctionCall != nullptr) {
        out << kTab << "# unary function call" << std::endl;
        return true;
    }

    functionEmitError = "empty unary node";
    return false;
}

bool CompilerWithContext::emitMultiplicative(std::ostream &out,
                                             const std::shared_ptr<MultiplicativeExpressionNode> &mul) {
    if (mul->UnaryExpressions.empty()) {
        functionEmitError = "empty multiplicative node";
        return false;
    }

    if (!emitUnary(out, mul->UnaryExpressions[0])) {
        return false;
    }

    if (mul->UnaryExpressions.size() == 1) {
        return true;
    }

    // save xmm0 into stack, to preserve value
    out << kTab << kSub0x10Rsp << std::endl;
    out << kTab << kMovSdXmm0Rsp << std::endl;

    for (std::size_t i = 1; i < mul->UnaryExpressions.size(); ++i) {
        if (!emitUnary(out, mul->UnaryExpressions[i])) {
            return false;
        }

        // xmm0 now is next multiplicative
        // save it to xmm1
        out << kTab << kMovXmm0Xmm1 << std::endl;
        // restore xmm0 from stack
        out << kTab << kMovSdRspXmm0 << std::endl;
        out << kTab << kAdd0x10Rsp << std::endl;

        if (mul->Operations[i - 1] == MultiplicativeExpressionNode::Mul) {
            out << kTab << kMulSdXmm1Xmm0 << std::endl;
        } else if (mul->Operations[i - 1] == MultiplicativeExpressionNode::Div) {
            out << kTab << kDivSdXmm1Xmm0 << std::endl;
        }

        if (i != mul->UnaryExpressions.size() - 1) {
            // save xmm0 before next multiplicative
            out << kTab << kSub0x10Rsp << std::endl;
            out << kTab << kMovSdXmm0Rsp << std::endl;
        }
    }

    return true;
}

bool CompilerWithContext::emitAdditive(std::ostream &out, const std::shared_ptr<AdditiveExpressionNode> &expression) {
    if (expression->MultiplicativeExpressions.empty()) {
        functionEmitError = "empty additive node";
        return false;
    }

    if (!emitMultiplicative(out, expression->MultiplicativeExpressions[0])) {
        return false;
    }

    if (expression->MultiplicativeExpressions.size() == 1) {
        return true;
    }

    // save xmm0 into stack, to preserve value
    out << kTab << kSub0x10Rsp << std::endl;
    out << kTab << kMovSdXmm0Rsp << std::endl;

    for (std::size_t i = 1; i < expression->MultiplicativeExpressions.size(); ++i) {
        if (!emitMultiplicative(out, expression->MultiplicativeExpressions[i])) {
            return false;
        }

        // xmm0 now is next multiplicative
        // save it to xmm1
        out << kTab << kMovXmm0Xmm1 << std::endl;
        // restore xmm0 from stack
        out << kTab << kMovSdRspXmm0 << std::endl;
        out << kTab << kAdd0x10Rsp << std::endl;

        if (expression->Operations[i - 1] == AdditiveExpressionNode::Sum) {
            out << kTab << kAddSdXmm1Xmm0 << std::endl;
        } else if (expression->Operations[i - 1] == AdditiveExpressionNode::Sub) {
            out << kTab << kSubSdXmm1Xmm0 << std::endl;
        }

        if (i != expression->MultiplicativeExpressions.size() - 1) {
            // save xmm0 before next multiplicative
            out << kTab << kSub0x10Rsp << std::endl;
            out << kTab << kMovSdXmm0Rsp << std::endl;
        }
    }

    return true;
}

bool CompilerWithContext::emitExpression(std::ostream &out, const std::shared_ptr<ExpressionNode> &expression) {
    return emitAdditive(out, expression->AdditiveExpression);
}

bool CompilerWithContext::emitReturnStatement(std::ostream &out,
                                              const std::shared_ptr<ReturnStatementNode> &returnStatement) {
    if (!emitExpression(out, returnStatement->Expression)) {
        return false;
    }

    out << kTab << kLeaveq << std::endl;
    out << kTab << kRetq << std::endl;

    return true;
}

bool CompilerWithContext::emitStatement(std::ostream &out, const std::shared_ptr<StatementNode> &statement) {
    if (currentCompilationFunction == nullptr) {
        functionEmitError = "call emitStatement outside of function compilation";
        return false;
    }

    if (statement->Assign != nullptr) {
        return true;
    }

    if (statement->Conditional != nullptr) {
        return true;
    }

    if (statement->Return != nullptr) {
        return emitReturnStatement(out, statement->Return);
    }

    functionEmitError = Format("function '%s' got empty statement node", currentCompilationFunction->definitionNode->Id->Name.c_str());
    return false;
}

bool CompilerWithContext::emitFunctionBody(std::ostream &out, const std::shared_ptr<StatementListNode> &body) {
    for (const auto &statement : body->Statements) {
        if (!emitStatement(out, statement)) {
            return false;
        }
    }

    return true;
}

CompilerWithContext::EmitResult CompilerWithContext::emitFunction(
        const std::shared_ptr<FunctionDefinitionNode> &function) {
    if (currentCompilationFunction != nullptr) {
        return {"", Format("try to compile function '%s' while compiling function '%s'", function->Id->Name.c_str(), currentCompilationFunction->definitionNode->Id->Name.c_str())};
    }
    auto fit = functionNameToContext.find(function->Id->Name);
    if (fit == functionNameToContext.end()) {
        return {"", Format("there is no context for function '%s'", function->Id->Name.c_str())};
    }

    currentCompilationFunction = fit->second;

    std::stringstream functionCode;

    functionCode << function->Id->Name << ":" << std::endl;

    // function enter
    functionCode << kTab << kPushRbp << std::endl;
    functionCode << kTab << kMovRspRbp << std::endl;
    // reserve space for variables (arguments and locals)
    if (!currentCompilationFunction->variableNameToItsContext.empty()) {
        functionCode << kTab << Format(kSubRspFmt, currentCompilationFunction->variableNameToItsContext.size() * sizeof(double)) << std::endl;
    }

    // store arguments into variables
    for (const auto &arg : function->Arguments->Ids) {
        if (auto it = currentCompilationFunction->variableNameToItsContext.find(arg->Name); it != currentCompilationFunction->variableNameToItsContext.end()){
            functionCode << kTab << Format(kMovSdXmmRbpFmt, it->second->ArgumentRegNum, it->second->Num) << std::endl;
        } else {
            return {"", Format("no context for argument '%s' of function '%s'", arg->Name.c_str(), function->Id->Name.c_str())};
        }
    }

    if (!emitFunctionBody(functionCode, function->Body)) {
        return {"", functionEmitError};
    }

    currentCompilationFunction = nullptr;

    return {functionCode.str()};
}

CompilerWithContext::EmitResult CompilerWithContext::emitConstants() {
    std::stringstream result;

    for (const auto &it : constantNameToContext) {
        result << it.first << ": .double " << std::setprecision(15) << it.second->definitionNode->Value->Value << std::endl;
    }

    return {result.str()};
}

CompilationResult CompilerWithContext::Compile() {
    auto err = fillContexts();
    if (!err.empty()) {
        return {false, "", err};
    }

    err = makeChecks();
    if (!err.empty()) {
        return {false, "", err};
    }

    auto header = emitHeader();
    if (!header.error.empty()) {
        return {false, "", header.assembly};
    }

    auto functionsEmitResult = emitFunctions();
    if (!functionsEmitResult.error.empty()) {
        return {false, "", functionsEmitResult.error};
    }

    auto constantsEmitResult = emitConstants();
    if (!constantsEmitResult.error.empty()) {
        return {false, "", constantsEmitResult.error};
    }

    return {true, header.assembly + "\n" + functionsEmitResult.assembly + "\n" + constantsEmitResult.assembly, ""};
}

CompilationResult CompileToAssembly(const std::shared_ptr<DcsProgramNode> &program) {
    auto compiler = CompilerWithContext(program);

    return compiler.Compile();
}

FullCompilationResult CompileToSharedLibrary(const std::shared_ptr<DcsProgramNode> &program, const std::filesystem::path &out) {
    FullCompilationResult result{};
    result.success = false;
    result.soPath = out.empty() ? UniquePath(".so") : out;
    result.objectPath = UniquePath(".o");
    result.sourcePath = UniquePath(".S");

    if (result.soPath.empty()) {
        result.errorMessage = "cant generate so path";
        return result;
    }

    if (result.sourcePath.empty()) {
        result.errorMessage = "cant generate assembly path";
        return result;
    }

    if (result.objectPath.empty()) {
        result.errorMessage = "cant generate object path";
        return result;
    }

    auto compilationResult = CompileToAssembly(program);
    if (!compilationResult.success) {
        result.errorMessage = compilationResult.errorMessage;
        return result;
    }

    if (!WriteAllToFile(result.sourcePath, compilationResult.assemblyCode)) {
        result.errorMessage = "cant write assembly to " + result.sourcePath.string();
        return result;
    }

    std::string asOutput;
    ExecuteAndGetStdout(Format("as %s -o %s", result.sourcePath.c_str(), result.objectPath.c_str()), asOutput);
    if (!asOutput.empty()) {
        result.errorMessage = Format("compilation failed: '%s'", asOutput.c_str());
        return result;
    }

    asOutput.clear();
    ExecuteAndGetStdout(Format("ld %s -shared -o %s", result.objectPath.c_str(), result.soPath.c_str()), asOutput);
    if (!asOutput.empty()) {
        result.errorMessage = Format("link failed: '%s'", asOutput.c_str());
        return result;
    }

    result.success = true;
    return result;
}
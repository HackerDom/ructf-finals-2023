#include <iostream>
#include <iomanip>
#include <random>
#include <stdexcept>

#include <thirdparty/nlohmann/json.hpp>

#include <utils/strings/strings.h>
#include <parser/parser.h>
#include <compiler/compiler.h>
#include <translator/translator.h>

#include <playground/executor.h>

std::random_device dev;
std::mt19937 rng(dev());
std::uniform_int_distribution<std::mt19937::result_type> distByte(0, 0xFF);

double generateRandomDouble() {
    union {
        double v;
        uint8_t bytes[sizeof(double)];
    } gen{};

    for (auto &b : gen.bytes) {
        b = distByte(rng);
    }

    return gen.v;
}

std::string generateRandomDoubleStr() {
    std::stringstream ss;
    ss << std::setprecision(30) << generateRandomDouble();

    return ss.str();
}

std::string executeText(std::string_view text) {
    auto tokens = TokenizeString(text);
    if (!tokens.Success) {
        return Format("compilation error: %s", tokens.ErrorMessage.c_str());
    }
    auto ast = ParseTokens(tokens.Tokens);
    if (!ast.Success) {
        return Format("compilation error: %s", ast.ErrorMessage.c_str());
    }
    auto compiled = CompileToAssembly(ast.ProgramNode);
    if (!compiled.Success) {
        return Format("compilation error: %s", compiled.ErrorMessage.c_str());
    }
    auto translated = TranslateAssembly(compiled.AssemblyCode);
    if (!translated.Success) {
        return Format("compilation error: %s", translated.ErrorMessage.c_str());
    }

    Executor executor(1);

    auto result = executor.Execute(*translated.Translated);

    if (result->Status == Executor::ExecuteResult::OK) {
        std::stringstream ss;
        ss << std::setprecision(30) << result->Value;
        return ss.str();
    } else if (result->Status == Executor::ExecuteResult::CRASH || result->Status == Executor::ExecuteResult::TIMEOUT) {
        return "timeout or crash";
    }

    throw std::runtime_error(Format("invalid status: %d", result->Status));
}


enum TestType {
    TestFibonacci,
    TestFactorial,
    TestStackoverflow,
    TestGlobalConstants,
    TestLocalVariables,
    TestFunctions,
};

std::string generateTestFibonacci() {
    return Format(R"(
fun fib(a, b, n) {
    c = a + b;
    a = b;
    b = c;
    if (n <= 0) {
        return b;
    } else {
        return fib(a, b, n - 1);
    }
}

fun main() {
    return fib(%f, %f, %d);
}
)", generateRandomDouble(), generateRandomDouble(), distByte(rng));
}

int main() {
    auto text = generateTestFibonacci();
    std::cout << text << std::endl << executeText(text) << std::endl;

    return 0;
}

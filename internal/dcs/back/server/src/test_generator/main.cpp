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
std::uniform_int_distribution<std::mt19937::result_type> distInt(0, 10000000);

double generateRandomDouble() {
    int attempt = 0;
    while (attempt < 3) {
        union {
            double v;
            uint8_t bytes[sizeof(double)];
        } gen{};

        for (unsigned char &byte: gen.bytes) {
            byte = distByte(rng);
        }

        if (!std::isnan(gen.v)) {
            return gen.v;
        }
        attempt++;
    }

    return 42.;
}

std::uniform_real_distribution<double> unif(-1e-3,1e3);
std::default_random_engine re;

double generateSmallDouble() {
    return unif(re);
}

std::string generateRandomDoubleSmallStr() {
    std::stringstream ss;
    ss << std::setprecision(5 + distByte(rng) % 3) << generateSmallDouble();

    return ss.str();
}

std::string generateRandomDoubleStr() {
    std::stringstream ss;
    ss << std::setprecision(5 + distByte(rng) % 25) << generateRandomDouble();

    return ss.str();
}

char generateAlpha() {
    const char *alpha = "_qwertyuiopasdfghjklzxcvbnm";
    return alpha[distByte(rng) % 27];
}

char generateAlphaDigit() {
    const char *alpha = "_qwertyuiopasdfghjklzxcvbnm0987654321";
    return alpha[distByte(rng) % 37];
}

std::string generateName() {
    std::string name;

    auto len = 2 + distByte(rng) % 7;

    name += generateAlpha();

    for (std::size_t i = 0; i < len; ++i) {
        name += generateAlphaDigit();
    }

    return name;
}

std::pair<std::string, std::string> executeText(std::string_view text) {
    auto tokens = TokenizeString(text);
    if (!tokens.Success) {
        return std::make_pair(Format("compilation error: %s", tokens.ErrorMessage.c_str()), "");
    }
    auto ast = ParseTokens(tokens.Tokens);
    if (!ast.Success) {
        return std::make_pair(Format("compilation error: %s", ast.ErrorMessage.c_str()), "");
    }
    auto compiled = CompileToAssembly(ast.ProgramNode);
    if (!compiled.Success) {
        return std::make_pair(Format("compilation error: %s", compiled.ErrorMessage.c_str()), "");
    }
    auto translated = TranslateAssembly(compiled.AssemblyCode);
    if (!translated.Success) {
        return std::make_pair(Format("compilation error: %s", translated.ErrorMessage.c_str()), "");
    }

    Executor executor(1);

    auto result = executor.Execute(*translated.Translated);

    if (result->Status == Executor::ExecuteResult::OK) {
        std::stringstream ss;
        ss << std::setprecision(30) << result->Value;
        return std::make_pair("", ss.str());
    } else if (result->Status == Executor::ExecuteResult::CRASH || result->Status == Executor::ExecuteResult::TIMEOUT) {
        return std::make_pair("", "program timeout or crash");
    }

    throw std::runtime_error(Format("invalid status: %d", result->Status));
}

std::string generateTestFibonacci() {
    auto name = generateName();
    return Format(R"(
fun %s(a, b, n) {
    c = a + b;
    a = b;
    b = c;
    if (n <= 0) {
        return b;
    } else {
        return %s(a, b, n - 1);
    }
}

fun main() {
    return %s(%f, %f, %d);
}
)", name.c_str(), name.c_str(), name.c_str(), generateRandomDouble(), generateRandomDouble(), distByte(rng));
}

std::string generateTestFactorial() {
    auto name = generateName();
    return Format(R"(
fun %s(n) {
    if (n <= 1) {
        return 1;
    }

    return n * %s(n - 1) * -1;
}

fun main() {
    return %s(%d);
}
)", name.c_str(), name.c_str(), name.c_str(), distByte(rng) % 20);
}

std::string generateTestStackOverflow() {
    auto name = generateName();
    return Format(R"(
fun %s(x) {
    return %s(x - 1) + x;
}

fun main() {
    return %s(%s);
}
)", name.c_str(), name.c_str(), name.c_str(), generateRandomDoubleStr().c_str());
}

std::string generateTestSimilarToSploit() {
    std::vector<std::string> names;
    for (int i = 0; i < 30; ++i) {
        names.push_back(generateName());
    }

    std::stringstream text;

    for (auto &n: names) {
        text << n << " = " << generateRandomDoubleStr() << ";" << std::endl;
    }

    text << Format("fun main() { return %s; }", names[distByte(rng) % names.size()].c_str());

    return text.str();
}

std::string generateOperand(const std::vector<std::string> &vars) {
    if ((distByte(rng) % 10) <= 3) {
        return generateRandomDoubleSmallStr();
    }

    return vars[distByte(rng) % vars.size()];
}

std::string generateExpression(const std::vector<std::string> &vars, int depth) {
    if (depth >= 10) {
        return generateOperand(vars);
    }

    auto n = distByte(rng) % 40;

    if (n <= 10) {
        return generateOperand(vars);
    } else if (n <= 20) {
        return Format("-(%s)", generateExpression(vars, depth + 1).c_str());
    } else if (n <= 25) {
        return Format("%s + %s", generateExpression(vars, depth + 1).c_str(), generateExpression(vars, depth + 1).c_str());
    } else if (n <= 30) {
        return Format("%s - %s", generateExpression(vars, depth + 1).c_str(), generateExpression(vars, depth + 1).c_str());
    } else if (n <= 35) {
        return Format("%s * %s", generateExpression(vars, depth + 1).c_str(), generateExpression(vars, depth + 1).c_str());
    } else {
        return Format("%s / %s", generateExpression(vars, depth + 1).c_str(), generateExpression(vars, depth + 1).c_str());
    }
}

std::string generateTestVariableExpression() {
    std::vector<std::string> vars;
    vars.resize(distByte(rng) % 10 + 2);
    for (auto &var: vars) {
        var = generateName();
    }

    std::stringstream text;

    text << "fun main() {\n";
    for (auto &v: vars) {
        text << "\t" << v << " = " << generateRandomDoubleSmallStr() << ";\n";
    }

    text << Format("\treturn %s;\n}", generateExpression(vars, 0).c_str());

    return text.str();
}

std::string generateTestWithCompilationError() {
    auto num = distByte(rng) % 7;

    if (num == 0) {
        return "fun main() { return f(-124345.4523452345); }";
    } else if (num == 1) {
        return "fun main() { x = 123123.4564564; return x + y; }";
    } else if (num == 2) {
        return R"(
fun f(x) {
    if (x < 0) {
        return -x;
    }

    return x;
}

fun main() {
    return f(-1.234234e123, 431342.234234);
}
)";
    } else if (num == 3) {
        return "fun main() { wessf_1231 = 123134245341.312312312; }";
    } else if (num == 4) {
        auto text = generateTestVariableExpression();
        auto start = distInt(rng) % (text.size() * 3 / 2);
        auto len = 2 + distByte(rng) % 5;
        if (start + len >= text.size()) {
            return text.substr(0, start);
        }

        return text.substr(0, start) + text.substr(start + len);
    } else if (num == 5) {
        return "fun f() { return 4.32; }";
    } else if (num == 6) {
        return "fun main() { if () { return 42; } }";
    }

    throw std::runtime_error("generateTestWithCompilationError");
}

std::string generateTestText(bool isGet) {
    auto num = distByte(rng) % 7;

    if (num == 0) {
        return generateTestFibonacci();
    } else if (num == 1) {
        return generateTestFactorial();
    } else if (num == 2) {
        return generateTestStackOverflow();
    } else if (num == 3) {
        return generateTestSimilarToSploit();
    } else if (num == 4 || num == 5) {
        return generateTestVariableExpression();
    } else if (num == 6) {
        if (isGet) {
            return generateTestVariableExpression();
        }
        return generateTestWithCompilationError();
    }

    throw std::runtime_error("invalid test case");
}

int main(int argc, char **argv) {
    if (argc != 2) {
        std::cout << "expected argument for mode: get|check" << std::endl;
        return 1;
    }

    bool isGet;

    if (std::strcmp(argv[1], "get") == 0) {
        isGet = true;
    } else if (std::strcmp(argv[1], "check") == 0) {
        isGet = false;
    } else {
        std::cout << "unknown mode: " << argv[1] << std::endl;
        return 1;
    }

    auto text = generateTestText(isGet);
    auto [error, result] = executeText(text);
    nlohmann::json j({
                             {"code",   text},
                             {"result", result},
                             {"error", error},
                     });

    std::cout << j.dump() << std::endl;

    return 0;
}

#include <gtest/gtest.h>

#include "lexer.h"
#include "parser.h"
#include "compiler.h"
#include "utils.h"

void assertProgramResult(const std::string &program, double result, const std::string &errorMessage) {
    auto tokens = TokenizeString(program);
    ASSERT_TRUE(tokens.success);
    auto parsed = ParseTokens(tokens.tokens);
    ASSERT_TRUE(parsed.success);
    auto compilationResult = CompileToSharedLibrary(parsed.programNode);

    if (!errorMessage.empty()) {
        EXPECT_FALSE(compilationResult.success);
        EXPECT_EQ(errorMessage, compilationResult.errorMessage);
        return;
    }

    EXPECT_TRUE(compilationResult.success);
    ASSERT_EQ(compilationResult.errorMessage, "");

    DynamicLibrary lb(compilationResult.soPath);
    EXPECT_TRUE(lb.IsOpen());
    ASSERT_EQ(lb.GetError(), "");

    auto k = lb.LoadSymbol("main");
    ASSERT_EQ(k.Error, "");

    typedef double (*MainPtrT)();

    auto f = reinterpret_cast<MainPtrT>(k.SymPtr);

    auto r = f();

    if (std::abs(r - result) > 1e-6) {
        ASSERT_DOUBLE_EQ(r, result);
    } else {
        std::filesystem::remove_all(compilationResult.soPath);
        std::filesystem::remove_all(compilationResult.sourcePath);
        std::filesystem::remove_all(compilationResult.objectPath);
    }
}

TEST(Functional, JustReturnValue) {
    assertProgramResult(R"(
fun main() {
    return 3.141592;
}
)", 3.141592, "");
}

TEST(Functional, JustReturnSumOfTwoConstants) {
    assertProgramResult(R"(
fun main() {
    return 3.141592 + 2.718281828;
}
)", 5.8598738, "");
}

TEST(Functional, JustReturnSumOfThreeConstants) {
    assertProgramResult(R"(
fun main() {
    return 3.141592 + 2.718281828 + 1488;
}
)", 1493.859873828, "");
}

TEST(Functional, JustReturnSumOfTwoMinusAnother) {
    assertProgramResult(R"(
fun main() {
    return 3.141592 + 2.718281828 - 1488;
}
)", -1482.140126172, "");
}

TEST(Functional, JustReturnMulOfTwoNumers) {
    assertProgramResult(R"(
fun main() {
    return 3.141592 * 2.718281828;
}
)", 8.5397324445901752, "");
}

TEST(Functional, JustReturnMulOfThreeNumers) {
    assertProgramResult(R"(
fun main() {
    return 3.141592 * 2.718281828 * 1337;
}
)", 11417.622278417064, "");
}

TEST(Functional, JustReturnMulOfTwoNumersAndDivideByAnother) {
    assertProgramResult(R"(
fun main() {
    return 3.141592 * 2.718281828 / -1337;
}
)", -0.0063872344387361068, "");
}

TEST(Functional, ReturnOfMulAndSum) {
    assertProgramResult(R"(
fun main() {
    return (3.141592 - 2.718281828) * -1337 + 12.432 / 87;
}
)", -565.82280341227636, "");
}

TEST(Functional, ReturnGlobalConstant) {
    assertProgramResult(R"(
pi = 3.1415927;
fun main() {
    return pi;
}
)", 3.1415927, "");
}

TEST(Functional, ReturnExpressionWithGlobalConstants) {
    assertProgramResult(R"(
pi = 3.1415927;
e = 2.7;
fun main() {
    return (pi * pi * e / pi / e - pi + e) * pi - e;
}
)", 5.782300290000, "");
}
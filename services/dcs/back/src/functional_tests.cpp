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

TEST(Functional, ReturnVariable) {
    assertProgramResult(R"(
fun main() {
    pi = 3.1415927;
    return pi;
}
)", 3.1415927, "");
}

TEST(Functional, ReturnVariablesExpression) {
    assertProgramResult(R"(
fun main() {
    pi = 3.1415927;
    e = 2.7;
    k = 1337;
    return pi * e / k;
}
)", 0.0063442784517576661, "");
}

TEST(Functional, ReturnRedefinedVariable) {
    assertProgramResult(R"(
fun main() {
    pi = 2.7;
    pi = 3.1415927;
    return pi;
}
)", 3.1415927, "");
}

TEST(Functional, TryAssignValueToConstant) {
    assertProgramResult(R"(
pi = 2.7;
fun main() {
    pi = 3.1415927;
    return pi;
}
)", 0.0, "cant create local variable with name 'pi': there is constant with that name");
}

TEST(Functional, StatementsAfteReturn) {
    assertProgramResult(R"(
fun main() {
    pi = 3.1415927;
    return pi;
    pi = 2.7;
    return pi;
}
)", 3.1415927, "");
}

TEST(Functional, FunctionCallNoArguments) {
    assertProgramResult(R"(
fun f() {
    return 1234567;
}

fun main() {
    return f();
}
)", 1234567, "");
}

TEST(Functional, FunctionCallOneArgument) {
    assertProgramResult(R"(
fun f(x) {
    return x;
}

fun main() {
    return f(42);
}
)", 42.0, "");
}

TEST(Functional, FunctionCallTwoArguments) {
    assertProgramResult(R"(
fun f(x, y) {
    return x + y;
}

fun main() {
    return f(3.1415927, 3.1415927);
}
)", 6.2831853, "");
}

TEST(Functional, FunctionCallThreeArguments) {
    assertProgramResult(R"(
fun f(x, y, z) {
    return (x + y) * z;
}

fun main() {
    return f(3.1415927, 2.7, 1337);
}
)", 7810.20943989, "");
}

TEST(Functional, FunctionCallWithFunctionCallAsArgument) {
    assertProgramResult(R"(
fun a(x, y, z) {
    return x + y + z;
}

fun b(x, y, z) {
    return a(x, y, z) * a(x, y, z);
}

fun main() {
    return a(1, 2, 3) + b(1, 2, 3) * b(a(1, 2, 3), a(1, 2, 3), a(1, 2, 3));
}
)", 11670, "");
}

TEST(Functional, ConditionWithoutElseFalseGreat) {
    assertProgramResult(R"(
fun main() {
    if (1 > 2) {
        return 1;
    }
    return 2;
}
)", 2, "");
}

TEST(Functional, ConditionWithoutElseTrueGreat) {
    assertProgramResult(R"(
fun main() {
    if (3.14 > 2) {
        return 1;
    }
    return 2;
}
)", 1, "");
}

TEST(Functional, ConditionWithoutElseTrueLess) {
    assertProgramResult(R"(
fun main() {
    if (1 < 2) {
        return 1;
    }
    return 2;
}
)", 1, "");
}

TEST(Functional, ConditionWithoutElseFalseLess) {
    assertProgramResult(R"(
fun main() {
    if (3.14 < 2) {
        return 1;
    }
    return 2;
}
)", 2, "");
}

TEST(Functional, ConditionWithoutElseFalseEq) {
    assertProgramResult(R"(
fun main() {
    if (1 == 2) {
        return 1;
    }
    return 2;
}
)", 2, "");
}

TEST(Functional, ConditionWithoutElseTrueEq) {
    assertProgramResult(R"(
fun main() {
    if (1 == 1) {
        return 1;
    }
    return 2;
}
)", 1, "");
}

TEST(Functional, ConditionWithoutElseFalseNeq) {
    assertProgramResult(R"(
fun main() {
    if (1 != 1) {
        return 1;
    }
    return 2;
}
)", 2, "");
}

TEST(Functional, ConditionWithoutElseTrueNeq) {
    assertProgramResult(R"(
fun main() {
    if (1 != 1.00001) {
        return 1;
    }
    return 2;
}
)", 1, "");
}

TEST(Functional, ConditionWithoutElseFalseGe) {
    assertProgramResult(R"(
fun main() {
    if (1 >= 2) {
        return 1;
    }
    return 2;
}
)", 2, "");
}

TEST(Functional, ConditionWithoutElseTrueGeBecauseOfEq) {
    assertProgramResult(R"(
fun main() {
    if (1 >= 1) {
        return 1;
    }
    return 2;
}
)", 1, "");
}

TEST(Functional, ConditionWithoutElseTrueGeBecauseOfGreat) {
    assertProgramResult(R"(
fun main() {
    if (2 >= 1) {
        return 1;
    }
    return 2;
}
)", 1, "");
}

TEST(Functional, ConditionWithoutElseFlaseGe) {
    assertProgramResult(R"(
fun main() {
    if (0.99999 >= 1) {
        return 1;
    }
    return 2;
}
)", 2, "");
}

TEST(Functional, ConditionWithoutElseTrueLeBecauseOfEq) {
    assertProgramResult(R"(
fun main() {
    if (1 <= 1) {
        return 1;
    }
    return 2;
}
)", 1, "");
}

TEST(Functional, ConditionWithoutElseTrueLeBecauseOfLess) {
    assertProgramResult(R"(
fun main() {
    if (0.99999 <= 1) {
        return 1;
    }
    return 2;
}
)", 1, "");
}

TEST(Functional, ConditionWithoutElseFalseLe) {
    assertProgramResult(R"(
fun main() {
    if (0.99999 <= (0.99999 - 0.0000001)) {
        return 1;
    }
    return 2;
}
)", 2, "");
}

TEST(Functional, ConditionWithElseTrue) {
    assertProgramResult(R"(
fun main() {
    pi = 3.14;
    if (pi > 3) {
        ans = 1;
    } else {
        ans = 2;
    }
    return ans;
}
)", 1, "");
}

TEST(Functional, ConditionWithElseFalse) {
    assertProgramResult(R"(
fun main() {
    pi = 3.14;
    if (pi > 4) {
        ans = 1;
    } else {
        ans = 2;
    }
    return ans;
}
)", 2, "");
}

TEST(Functional, Factorial) {
    assertProgramResult(R"(
fun factorial(n) {
    if (n <= 1) {
        return 1;
    }

    return n * factorial(n - 1);
}

fun main() {
    return factorial(10);
}
)", 3628800, "");
}

TEST(Functional, Fibonacci1) {
    assertProgramResult(R"(
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
    return fib(0, 1, 5);
}
)", 13, "");
}

TEST(Functional, Fibonacci2) {
    assertProgramResult(R"(
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
    return fib(0, 1, 6);
}
)", 21, "");
}

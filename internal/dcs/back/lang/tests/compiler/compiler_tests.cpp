#include <gtest/gtest.h>

#include <utils/strings/strings.h>

#include <lexer/lexer.h>
#include <parser/parser.h>
#include <compiler/compiler.h>

void assertCompilationResult(const std::string &programText, const std::string &expectedAssembly, const std::string &expectedError) {
    auto tokens = TokenizeString(programText);
    ASSERT_TRUE(tokens.Success);
    auto parsed = ParseTokens(tokens.Tokens);
    ASSERT_TRUE(parsed.Success);
    auto compiled = CompileToAssembly(parsed.ProgramNode);

    if (!expectedError.empty()) {
        EXPECT_FALSE(compiled.Success);
        ASSERT_EQ(compiled.ErrorMessage, expectedError);
        return;
    }

    Trim(compiled.AssemblyCode);
    auto ea = TrimCopy(expectedAssembly);

    EXPECT_TRUE(compiled.Success);
    EXPECT_EQ(compiled.ErrorMessage, "");
    ASSERT_EQ(compiled.AssemblyCode, ea);
}

TEST(Compiler, OnlyConstants) {
    assertCompilationResult(R"(
pi = 3.1415927;
x2 = -234234.123123;
e = 2.7;
x1 = 1.23123123;

fun main() { return 0; }
)", R"(
.section .text
.globl main

main:
    push    %rbp
    mov     %rsp,%rbp
    movsd   _c_const_main_0(%rip),%xmm0
    leaveq
    retq


_c_const_main_0: .quad 0x0
e: .quad 0x400599999999999a
pi: .quad 0x400921fb5a7ed197
x1: .quad 0x3ff3b31f84aa9f25
x2: .quad 0xc10c97d0fc27e953
.sign_bit: .quad 0x8000000000000000
)", "");
}

TEST(Compiler, RedifinitionsOfConstant) {
    assertCompilationResult(R"(
pi = 3.1415927;
_x = 42;
x2 = -234234.123123;
e = 2.7;
x1 = 1.23123123;
_x = 43;
)", "", "constant '_x' is defined twice");
}

TEST(Compiler, RedifinitionOfFunction) {
    assertCompilationResult(R"(
fun f() {}
fun main() {}
fun f() {}
)", "", "function 'f' is defined twice");
}

TEST(Compiler, FunctionCallWithInvalidArgumentsAmountLess) {
    assertCompilationResult(R"(
fun f() {}

fun main() { return f(1.0); }
)", "", "invalid arguments count for function call 'f': expected 0, but got 1 (in function 'main')");
}

TEST(Compiler, FunctionCallWithInvalidArgumentsAmountMore) {
    assertCompilationResult(R"(
fun f(x, y) { return x + y; }

fun main() { return f(1.0, 2.0, 3.0); }
)", "", "invalid arguments count for function call 'f': expected 2, but got 3 (in function 'main')");
}

TEST(Compiler, ConstantsFromFunctions) {
    assertCompilationResult(R"(
pi = 3.1415927;
x2 = -234234.123123;
e = 2.7;
x1 = 1.23123123;

fun lol(k) {
    l = 43;
    return 1 * 43 + 45 * k;
}

fun main() {
    return 42 / 1244.2234234;
}
)", R"(
.section .text
.globl main

main:
    push    %rbp
    mov     %rsp,%rbp
    movsd   _c_const_main_0(%rip),%xmm0
    sub     $0x10,%rsp
    movsd   %xmm0,(%rsp)
    movsd   _c_const_main_1(%rip),%xmm0
    movaps  %xmm0,%xmm1
    movsd   (%rsp),%xmm0
    add     $0x10,%rsp
    divsd   %xmm1,%xmm0
    leaveq
    retq

lol:
    push    %rbp
    mov     %rsp,%rbp
    sub     $0x10,%rsp
    movsd   %xmm0,-0x8(%rbp)
    movsd   _c_const_lol_0(%rip),%xmm0
    movsd   %xmm0,-0x10(%rbp)
    movsd   _c_const_lol_1(%rip),%xmm0
    sub     $0x10,%rsp
    movsd   %xmm0,(%rsp)
    movsd   _c_const_lol_2(%rip),%xmm0
    movaps  %xmm0,%xmm1
    movsd   (%rsp),%xmm0
    add     $0x10,%rsp
    mulsd   %xmm1,%xmm0
    sub     $0x10,%rsp
    movsd   %xmm0,(%rsp)
    movsd   _c_const_lol_3(%rip),%xmm0
    sub     $0x10,%rsp
    movsd   %xmm0,(%rsp)
    movsd   -0x8(%rbp),%xmm0
    movaps  %xmm0,%xmm1
    movsd   (%rsp),%xmm0
    add     $0x10,%rsp
    mulsd   %xmm1,%xmm0
    movaps  %xmm0,%xmm1
    movsd   (%rsp),%xmm0
    add     $0x10,%rsp
    addsd   %xmm1,%xmm0
    leaveq
    retq


_c_const_lol_0: .quad 0x4045800000000000
_c_const_lol_1: .quad 0x3ff0000000000000
_c_const_lol_2: .quad 0x4045800000000000
_c_const_lol_3: .quad 0x4046800000000000
_c_const_main_0: .quad 0x4045000000000000
_c_const_main_1: .quad 0x409370e4c91a90a5
e: .quad 0x400599999999999a
pi: .quad 0x400921fb5a7ed197
x1: .quad 0x3ff3b31f84aa9f25
x2: .quad 0xc10c97d0fc27e953
.sign_bit: .quad 0x8000000000000000
)", "");
}

TEST(Compiler, RedefinitionOfConstantFromFunction) {
    assertCompilationResult(R"(
pi = 3.1415927;
x2 = -234234.123123;
e = 2.7;
x1 = 1.23123123;
_c_const_lol_1=1;

fun lol() {
    l = 43;
    return 1 * 43 + 45;
}

fun main() {
    return (42);
}
)", "", "cant define constant '_c_const_lol_1' (do not define it manually)");
}

TEST(Compiler, DefineFunctionWithConstantName) {
    assertCompilationResult(R"(
x = 42;
fun x() {}
)", R"()", "cant define function 'x': there is constant with that name");
}

TEST(Compiler, DefineVariableWithConstantName) {
    assertCompilationResult(R"(
x = 42;
fun main() {
    x = 43;
    return x;
}
)", "", "cant create local variable with name 'x': there is constant with that name");
}

TEST(Compiler, DefineVariableWithFunctionName) {
    assertCompilationResult(R"(
fun main() {
    x = 43;
    return x;
}

fun x() { return 42; }

)", "", "cant create local variable with name 'x': there is function with that name");
}

TEST(Compiler, DefineArgumentWithConstantName) {
    assertCompilationResult(R"(
x = 42;

fun f(x) {
    return x * x;
}
)", "", "cant create argument for 'f' with name 'x': there is constant with that name");
}

TEST(Compiler, DefineArgumentWithFunctionName) {
    assertCompilationResult(R"(
fun main(x) {
    return x * x;
}

fun x() { return 52; }
)", "", "cant create argument for 'main' with name 'x': there is function with that name");
}

TEST(Compiler, RedefinitionOfArgument) {
    assertCompilationResult(R"(
fun main(x, y, x) {
    return x * y * x;
}
)", "", "redefinition of argument 'x' in function 'main'");
}

TEST(Compiler, UnknownVariableInUsage) {
    assertCompilationResult(R"(
fun main(x) {
    return x * 1 / (y);
}
)", "", "unknown variable 'y' in function 'main'");
}

TEST(Compiler, UnknownVariableInFuctionCall) {
    assertCompilationResult(R"(
fun f(x) {
    return x;
}

fun main() {
    return f(y);
}
)", "", "unknown variable 'y' in function 'main'");
}

TEST(Compiler, UnknownFunctionCall) {
    assertCompilationResult(R"(
fun c(x, y) {
    return x + y;
}

fun main() {
    return 1 + c(42, l(44));
}
)", "", "unknown function call 'l' in 'main'");
}

TEST(Compiler, MainFunctionCantGetArguments) {
    assertCompilationResult(R"(
fun main(x) {
    return x;
}
)", "", "'main' function cant have any arguments");
}

TEST(Compiler, ReturnGlobalConstant) {
    assertCompilationResult(R"(
pi = 3.1415927;
fun main() {
    return pi;
}
)", R"(
.section .text
.globl main

main:
    push    %rbp
    mov     %rsp,%rbp
    movsd   pi(%rip),%xmm0
    leaveq
    retq


pi: .quad 0x400921fb5a7ed197
.sign_bit: .quad 0x8000000000000000
)", "");
}

TEST(Compiler, ReturnVariable) {
    assertCompilationResult(R"(
fun main() {
    pi = 3.1415927;
    return pi;
}
)", R"(
.section .text
.globl main

main:
    push    %rbp
    mov     %rsp,%rbp
    sub     $0x8,%rsp
    movsd   _c_const_main_0(%rip),%xmm0
    movsd   %xmm0,-0x8(%rbp)
    movsd   -0x8(%rbp),%xmm0
    leaveq
    retq


_c_const_main_0: .quad 0x400921fb5a7ed197
.sign_bit: .quad 0x8000000000000000
)", "");
}

TEST(Compiler, ReturnVariablesExpression) {
    assertCompilationResult(R"(
fun main() {
    pi = 3.1415927;
    e = 2.7;
    k = 1337;
    return pi * e / k;
}
)", R"(
.section .text
.globl main

main:
    push    %rbp
    mov     %rsp,%rbp
    sub     $0x18,%rsp
    movsd   _c_const_main_0(%rip),%xmm0
    movsd   %xmm0,-0x8(%rbp)
    movsd   _c_const_main_1(%rip),%xmm0
    movsd   %xmm0,-0x10(%rbp)
    movsd   _c_const_main_2(%rip),%xmm0
    movsd   %xmm0,-0x18(%rbp)
    movsd   -0x8(%rbp),%xmm0
    sub     $0x10,%rsp
    movsd   %xmm0,(%rsp)
    movsd   -0x10(%rbp),%xmm0
    movaps  %xmm0,%xmm1
    movsd   (%rsp),%xmm0
    add     $0x10,%rsp
    mulsd   %xmm1,%xmm0
    sub     $0x10,%rsp
    movsd   %xmm0,(%rsp)
    movsd   -0x18(%rbp),%xmm0
    movaps  %xmm0,%xmm1
    movsd   (%rsp),%xmm0
    add     $0x10,%rsp
    divsd   %xmm1,%xmm0
    leaveq
    retq


_c_const_main_0: .quad 0x400921fb5a7ed197
_c_const_main_1: .quad 0x400599999999999a
_c_const_main_2: .quad 0x4094e40000000000
.sign_bit: .quad 0x8000000000000000
)", "");
}

TEST(Compiler, TryAddStatementsAfterReturn) {
    assertCompilationResult(R"(
fun main() {
    pi = 3.1415927;
    return pi;
    pi = 2.7;
    return pi;
}
)", R"(
.section .text
.globl main

main:
    push    %rbp
    mov     %rsp,%rbp
    sub     $0x8,%rsp
    movsd   _c_const_main_0(%rip),%xmm0
    movsd   %xmm0,-0x8(%rbp)
    movsd   -0x8(%rbp),%xmm0
    leaveq
    retq


_c_const_main_0: .quad 0x400921fb5a7ed197
_c_const_main_1: .quad 0x400599999999999a
.sign_bit: .quad 0x8000000000000000
)", "");
}

TEST(Compiler, FunctionCallNoArgument) {
    assertCompilationResult(R"(
fun f() {
    return 1234567;
}

fun main() {
    return f();
}
)", R"(
.section .text
.globl main

main:
    push    %rbp
    mov     %rsp,%rbp
    call    f
    leaveq
    retq

f:
    push    %rbp
    mov     %rsp,%rbp
    movsd   _c_const_f_0(%rip),%xmm0
    leaveq
    retq


_c_const_f_0: .quad 0x4132d68700000000
.sign_bit: .quad 0x8000000000000000
)", "");
}

TEST(Compiler, FunctionCallOneArgument) {
    assertCompilationResult(R"(
fun f(x) {
    return x;
}

fun main() {
    return f(42);
}
)", R"(
.section .text
.globl main

main:
    push    %rbp
    mov     %rsp,%rbp
    movsd   _c_const_main_0(%rip),%xmm0
    call    f
    leaveq
    retq

f:
    push    %rbp
    mov     %rsp,%rbp
    sub     $0x8,%rsp
    movsd   %xmm0,-0x8(%rbp)
    movsd   -0x8(%rbp),%xmm0
    leaveq
    retq


_c_const_main_0: .quad 0x4045000000000000
.sign_bit: .quad 0x8000000000000000
)", "");
}

TEST(Compiler, FunctionCallTwoArguments) {
    assertCompilationResult(R"(
fun f(x, y) {
    return x + y;
}

fun main() {
    return f(3.1415927, 3.1415927);
}
)", R"(
.section .text
.globl main

main:
    push    %rbp
    mov     %rsp,%rbp
    movsd   _c_const_main_1(%rip),%xmm0
    sub     $0x10,%rsp
    movsd   %xmm0,(%rsp)
    movsd   _c_const_main_0(%rip),%xmm0
    movsd   (%rsp),%xmm1
    add     $0x10,%rsp
    call    f
    leaveq
    retq

f:
    push    %rbp
    mov     %rsp,%rbp
    sub     $0x10,%rsp
    movsd   %xmm0,-0x8(%rbp)
    movsd   %xmm1,-0x10(%rbp)
    movsd   -0x8(%rbp),%xmm0
    sub     $0x10,%rsp
    movsd   %xmm0,(%rsp)
    movsd   -0x10(%rbp),%xmm0
    movaps  %xmm0,%xmm1
    movsd   (%rsp),%xmm0
    add     $0x10,%rsp
    addsd   %xmm1,%xmm0
    leaveq
    retq


_c_const_main_0: .quad 0x400921fb5a7ed197
_c_const_main_1: .quad 0x400921fb5a7ed197
.sign_bit: .quad 0x8000000000000000
)", "");
}

TEST(Compiler, FunctionCallThreeArguments) {
    assertCompilationResult(R"(
fun f(x, y, z) {
    return (x + y) * z;
}

fun main() {
    return f(3.1415927, 2.7, 1337);
}
)", R"(
.section .text
.globl main

main:
    push    %rbp
    mov     %rsp,%rbp
    movsd   _c_const_main_2(%rip),%xmm0
    sub     $0x10,%rsp
    movsd   %xmm0,(%rsp)
    movsd   _c_const_main_1(%rip),%xmm0
    sub     $0x10,%rsp
    movsd   %xmm0,(%rsp)
    movsd   _c_const_main_0(%rip),%xmm0
    movsd   (%rsp),%xmm1
    add     $0x10,%rsp
    movsd   (%rsp),%xmm2
    add     $0x10,%rsp
    call    f
    leaveq
    retq

f:
    push    %rbp
    mov     %rsp,%rbp
    sub     $0x18,%rsp
    movsd   %xmm0,-0x8(%rbp)
    movsd   %xmm1,-0x10(%rbp)
    movsd   %xmm2,-0x18(%rbp)
    movsd   -0x8(%rbp),%xmm0
    sub     $0x10,%rsp
    movsd   %xmm0,(%rsp)
    movsd   -0x10(%rbp),%xmm0
    movaps  %xmm0,%xmm1
    movsd   (%rsp),%xmm0
    add     $0x10,%rsp
    addsd   %xmm1,%xmm0
    sub     $0x10,%rsp
    movsd   %xmm0,(%rsp)
    movsd   -0x18(%rbp),%xmm0
    movaps  %xmm0,%xmm1
    movsd   (%rsp),%xmm0
    add     $0x10,%rsp
    mulsd   %xmm1,%xmm0
    leaveq
    retq


_c_const_main_0: .quad 0x400921fb5a7ed197
_c_const_main_1: .quad 0x400599999999999a
_c_const_main_2: .quad 0x4094e40000000000
.sign_bit: .quad 0x8000000000000000
)", "");
}

TEST(Compiler, ConditionalStatementWithoutElse) {
    assertCompilationResult(R"(
fun main() {
    if (1 > 2) {
        return 1;
    }
    return 2;
}
)", R"(
.section .text
.globl main

main:
    push    %rbp
    mov     %rsp,%rbp
    movsd   _c_const_main_0(%rip),%xmm0
    sub     $0x10,%rsp
    movsd   %xmm0,(%rsp)
    movsd   _c_const_main_1(%rip),%xmm0
    movaps  %xmm0,%xmm1
    movsd   (%rsp),%xmm0
    add     $0x10,%rsp
    comisd  %xmm1,%xmm0
    jbe ._0
    movsd   _c_const_main_3(%rip),%xmm0
    leaveq
    retq
._0:
    movsd   _c_const_main_2(%rip),%xmm0
    leaveq
    retq


_c_const_main_0: .quad 0x3ff0000000000000
_c_const_main_1: .quad 0x4000000000000000
_c_const_main_2: .quad 0x4000000000000000
_c_const_main_3: .quad 0x3ff0000000000000
.sign_bit: .quad 0x8000000000000000
)", "");
}

TEST(Compiler, ConditionalStatementWithElse) {
    assertCompilationResult(R"(
fun main() {
    pi = 3.14;
    if (pi > 3) {
        ans = 1;
    } else {
        and = 2;
    }
    return ans;
}
)", R"(
.section .text
.globl main

main:
    push    %rbp
    mov     %rsp,%rbp
    sub     $0x18,%rsp
    movsd   _c_const_main_0(%rip),%xmm0
    movsd   %xmm0,-0x8(%rbp)
    movsd   -0x8(%rbp),%xmm0
    sub     $0x10,%rsp
    movsd   %xmm0,(%rsp)
    movsd   _c_const_main_1(%rip),%xmm0
    movaps  %xmm0,%xmm1
    movsd   (%rsp),%xmm0
    add     $0x10,%rsp
    comisd  %xmm1,%xmm0
    ja ._1
    movsd   _c_const_main_3(%rip),%xmm0
    movsd   %xmm0,-0x18(%rbp)
    jmp ._0
._1:
    movsd   _c_const_main_2(%rip),%xmm0
    movsd   %xmm0,-0x10(%rbp)
._0:
    movsd   -0x10(%rbp),%xmm0
    leaveq
    retq


_c_const_main_0: .quad 0x40091eb851eb851f
_c_const_main_1: .quad 0x4008000000000000
_c_const_main_2: .quad 0x3ff0000000000000
_c_const_main_3: .quad 0x4000000000000000
.sign_bit: .quad 0x8000000000000000
)", "");
}

TEST(Compiler, AssignValueToConstant) {
    assertCompilationResult(R"(
pi = 2;
fun main() {
    pi = 3.14;
    return pi;
}
)", "", "cant create local variable with name 'pi': there is constant with that name");
}

TEST(Compiler, NegativeNumber) {
    assertCompilationResult(R"(
fun main() {
    return -3.14;
}
)", R"(
.section .text
.globl main

main:
    push    %rbp
    mov     %rsp,%rbp
    movsd   _c_const_main_0(%rip),%xmm0
    leaveq
    retq


_c_const_main_0: .quad 0xc0091eb851eb851f
.sign_bit: .quad 0x8000000000000000
)", "");
}

TEST(Compiler, NegNegNeg) {
    assertCompilationResult(R"(
fun main() {
    return ---5;
}
)", R"(
.section .text
.globl main

main:
    push    %rbp
    mov     %rsp,%rbp
    movsd   _c_const_main_0(%rip),%xmm0
    movsd   .sign_bit(%rip),%xmm1
    xorpd   %xmm1,%xmm0
    movsd   .sign_bit(%rip),%xmm1
    xorpd   %xmm1,%xmm0
    leaveq
    retq


_c_const_main_0: .quad 0xc014000000000000
.sign_bit: .quad 0x8000000000000000
)", "");
}

TEST(Compiler, NegExpr) {
    assertCompilationResult(R"(
fun main() {
    return -(5 +6);
}
)", R"(
.section .text
.globl main

main:
    push    %rbp
    mov     %rsp,%rbp
    movsd   _c_const_main_0(%rip),%xmm0
    sub     $0x10,%rsp
    movsd   %xmm0,(%rsp)
    movsd   _c_const_main_1(%rip),%xmm0
    movaps  %xmm0,%xmm1
    movsd   (%rsp),%xmm0
    add     $0x10,%rsp
    addsd   %xmm1,%xmm0
    movsd   .sign_bit(%rip),%xmm1
    xorpd   %xmm1,%xmm0
    leaveq
    retq


_c_const_main_0: .quad 0x4014000000000000
_c_const_main_1: .quad 0x4018000000000000
.sign_bit: .quad 0x8000000000000000
)", "");
}

TEST(Compiler, ErrorWithNoReturn) {
    assertCompilationResult(R"(
fun main() {
    a = 3.14;
}
)", "", "not all code paths of 'main' returns value");
}

TEST(Compiler, IncorrectCodePathsReturnsCheck) {
    assertCompilationResult(R"(
fun main() {
    if (1 < 2) {
        return 3.14;
    }
}
)",
R"(
.section .text
.globl main

main:
    push    %rbp
    mov     %rsp,%rbp
    movsd   _c_const_main_0(%rip),%xmm0
    sub     $0x10,%rsp
    movsd   %xmm0,(%rsp)
    movsd   _c_const_main_1(%rip),%xmm0
    movaps  %xmm0,%xmm1
    movsd   (%rsp),%xmm0
    add     $0x10,%rsp
    comisd  %xmm1,%xmm0
    jae ._0
    movsd   _c_const_main_2(%rip),%xmm0
    leaveq
    retq
._0:


_c_const_main_0: .quad 0x3ff0000000000000
_c_const_main_1: .quad 0x4000000000000000
_c_const_main_2: .quad 0x40091eb851eb851f
.sign_bit: .quad 0x8000000000000000
)", "");
}
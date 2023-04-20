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

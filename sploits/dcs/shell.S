.section .text
.globl _start

_start:
    push    %rbp
    mov     %rsp, %rbp

    # open directory with sys_open
    mov     $2, %rax
    lea     dir_path(%rip), %rdi
    mov     $0, %rsi # O_RDONLY
    syscall

    # -8(%rbp) is directory fd
    mov     %rax, -8(%rbp)

    # rax - file descriptor
    test    %rax, %rax
    js      .fail

    

# change exit to set %xmm0 according to stealed flags
.success:
    # close directory fd
    mov     $3, %rax
    mov     -8(%rbp), %rdi
    syscall

    # leaveq
    # ret

    mov     $60, %rax
    xor     %rdi, %rdi
    syscall

.fail:
    # check errno in gdb
    neg     %rax
    # mov     %rax,%xmm0
    # ret

    mov     $60, %rax
    mov     $1, %rdi
    syscall

dir_path: .ascii "./dataexample"


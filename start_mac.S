.intel_syntax noprefix
.text
    .globl _start, _syscall5, _syscall6

    _start:
        mov rbp, 0
        pop rdi
        mov rsi, rsp
        and rsp, -16
        call _main

        mov rdi, rax
        mov rax, 1
        syscall

        ret /* should never be reached, but if the OS somehow fails
               to kill us, it will cause a segmentation fault */

    // this can be used for syscalls with 1-5 arguments
    _syscall5:
        push rbp
        mov rbp, rsp
        mov rax, r9
        syscall
        pop rbp
        ret

    _syscall6:
        push rbp
        mov rbp, rsp
        mov r10, rcx
        mov rax, [rbp + 16]
        syscall
        pop rbp
        ret

SECTION .bss

ALIGN 16
stackTop: resb 8192
stackBottom:

SECTION .text

GLOBAL kstart
kstart:
    ; Prepare the environment for the kernel

    ; Prepare paging

    ; Setup stack
    mov rsp, stackBottom

    ; Jump to kernel
    jmp rcx
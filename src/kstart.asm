SECTION .bss

ALIGN 16
stackTop: resb 8192
stackBottom:

ALIGN 4096
pml4: resq 512

SECTION .text

GLOBAL kstart
kstart:
    ; Prepare the environment for the kernel

    ; Setup stack
    mov rsp, stackBottom
    
    ; Save args
    push rdi
    push rsi
    push rdx
    push rcx

    ; Prepare paging
    mov rbx, cr3
    mov rsi, pml4
    mov rcx, 256

    .copyLoop1:
        mov rax, [rbx]
        mov [rsi], rax
        add rbx, 8
        add rsi, 8
        loop .copyLoop1

    mov rbx, cr3
    mov rcx, 256
    
    .copyLoop2:
        mov rax, [rbx]
        mov [rsi], rax
        add rbx, 8
        add rsi, 8
        loop .copyLoop2

    mov rax, pml4
    mov cr3, rax

    ; Jump to kernel
    pop rcx
    pop rdx
    pop rsi
    pop rdi

    jmp rcx
; Pristine
; stage2_paging: paging related subroutines
; SPDX-License-Identifier: MIT

[bits 32]

; void kernel_entry(uint32_t pml4_address, uint64_t kernel_entry_address)
global kernel_entry
kernel_entry:
    mov edi, [esp+8]    ; Low 32 bits of kernel load address
    mov esi, [esp+12]   ; High 32 bits of kernel load address
    xor ebx, ebx
    mov ebx, [esp+16]   ; 32 bits BootInfo pointer

    ; PAE
    mov eax, cr4
    or eax, 0x20
    mov cr4, eax

    ; PML4
    mov eax, [esp+4]
    mov cr3, eax

    ; EFER (LME, NXE, SCE)
    mov ecx, 0xC0000080
    rdmsr
    or eax, 0x901
    wrmsr

    ; Paging
    mov eax, cr0
    or eax, (1 << 31)
    mov cr0, eax

    ; We are going to just assume SSE is present on the CPU
    ; Enable SSE
    mov eax, cr0
    and eax, ~(1 << 2)
    or eax, (1 << 1)
    mov cr0, eax

    mov eax, cr4
    or eax, (1 << 9) | (1 << 10)
    mov cr4, eax

    jmp 0x08:.stage2_farjump

[bits 64]
.stage2_farjump:
    mov ax, 0x10
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax
    mov ss, ax
    shl rsi, 32
    or rdi, rsi
    mov rax, rdi

    mov rdi, rbx

    jmp rax
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

    ; 64 bit GDT
    lgdt [stage2_gdt_ptr]

    ; PAE
    mov eax, cr4
    or eax, 0x20
    mov cr4, eax

    ; PML4
    mov eax, [esp+4]
    mov cr3, eax

    ; EFER.LME
    mov ecx, 0xC0000080
    rdmsr
    or eax, 0x100
    wrmsr

    ; Paging
    mov eax, cr0
    or eax, (1 << 31)
    mov cr0, eax

    jmp 0x08:.stage2_farjump

[bits 64]
.stage2_farjump:
    mov ax, 0x10
    mov ds, rax
    mov es, rax
    mov fs, rax
    mov gs, rax
    mov ss, rax
    shl rsi, 32
    or rdi, rsi
    mov rax, rdi

    mov rsp, (0x200000 - 8)
    mov edi, ebx

    jmp rax

%macro GDT_ENTRY 4
    dw (%2 & 0xFFFF)
    dw (%1 & 0xFFFF)
    db ((%1 >> 16) & 0xFF)
    db (%3)
    db ((%4 << 4) | ((%2 >> 16) & 0x0F))
    db ((%1 >> 24) & 0xFF)
%endmacro

stage2_gdt_start:
    GDT_ENTRY 0, 0, 0, 0
    GDT_ENTRY 0, 0xFFFFF, 0x9A, 0xA
    GDT_ENTRY 0, 0xFFFFF, 0x92, 0xC
stage2_gdt_end:

stage2_gdt_ptr:
    dw stage2_gdt_end - stage2_gdt_start - 1
    dd stage2_gdt_start
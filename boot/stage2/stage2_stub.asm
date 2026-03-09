; Pristine
; stage2_stub - real entry point of stage 2 bootloader
; SPDX-License-Identifier: MIT

; To ensure we are loaded at the adress we promised we can't really confirm linker will put the entry point at top
; Also having stage2_boot at the top of the file at all times isn't really an option
; Instead we have a small loader stub at the promised address, which is properly linked to stage 2 entry point

[bits 32]
global stage2_entry
extern stage2_boot
extern __bss_start
extern __bss_end

section .text.entry
stage2_entry:
    mov eax, 0x10
    mov ds, eax
    mov es, eax
    mov fs, eax
    mov gs, eax
    mov ss, eax
    mov esp, 0x00F00000

    mov edi, __bss_start
    mov ecx, __bss_end
    sub ecx, edi
    xor eax, eax
    rep stosb

    call stage2_boot

    cli
    hlt
    jmp $
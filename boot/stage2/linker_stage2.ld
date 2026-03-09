ENTRY(stage2_entry)
SECTIONS
{
    . = 0x7E00;
    .text : {
        bin/boot/stage2/stage2_entry.o(.text.entry)
        bin/boot/stage2/stage2.o(.text.stage2)
        *(.text*)
    }
    .rodata : { *(.rodata*) }
    .data   : { *(.data*)   }
    .bss    : { 
        __bss_start = ADDR(.bss);
        *(.bss*)
        *(COMMON)
        __bss_end = ADDR(.bss) + SIZEOF(.bss);
    }
}
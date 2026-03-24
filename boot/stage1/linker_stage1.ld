ENTRY(stage1_boot)
SECTIONS
{
    . = 0x7C00;
    .text : {
        bin/boot/stage1/stage1.o(.text)
        *(.text*)
        *(.data*)
        *(.rodata*)
    }

    /DISCARD/ : {
        *(.debug*)
        *(.stabs*)
        *(.comment*)
        *(.symtab)
        *(.strtab)
    }

    . = 0x7C00 + 510;
    .sig : {
        SHORT(0xAA55);
    }
}
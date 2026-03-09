ENTRY(stage1_boot)
SECTIONS
{
    . = 0x7C00;
    .text : {
        bin/boot/stage1/stage1.o(.text)
        *(.text*)
        *(.data*)
        . = 0x7C00 + 510;
        SHORT(0xAA55);
    }
}
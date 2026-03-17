ENTRY(_start)

phys_base = 0x200000;
virt_base = 0xFFFFFFFF80000000;
offset = virt_base - phys_base;

SECTIONS
{
    . = virt_base + phys_base;

    .text : AT(ADDR(.text) - offset)
    { 
        *(.text*) 
    }

    .data : AT(ADDR(.data) - offset)
    { 
        *(.data*) 
    }

    .rodata : AT(ADDR(.rodata) - offset)
    { 
        *(.rodata*) 
    }

    .bss : AT(ADDR(.bss) - offset)
    { 
        *(.bss*) 
    }
}
ENTRY(_start)

phys_base = 0x400000;
virt_base = 0xFFFFFFFF80000000;
offset = virt_base;

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
        . = ALIGN(4096);
        __kernel_ist1_guard = .;
        . += 4096;
        __kernel_ist1_start = . + 0x1000;
        . += 0x1000;

        . = ALIGN(4096);
        __kernel_rsp0_guard = .;
        . += 4096;
        __kernel_rsp0_start = . + 0x1000;
        . += 0x1000;

        . = ALIGN(4096);
        __kernel_stack_guard = .;
        . += 4096;
        __kernel_stack_start = . + 0x4000;
        . += 0x4000;
        *(.bss*) 
    }
}
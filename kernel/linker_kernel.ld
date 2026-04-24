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
        __kernel_ist1_base = .;
        . += 0x1000;
        __kernel_ist1_top = .;

        . = ALIGN(4096);
        __kernel_ist2_guard = .;
        . += 4096;
        __kernel_ist2_base = .;
        . += 0x1000;
        __kernel_ist2_top = .;

        . = ALIGN(4096);
        __kernel_ist3_guard = .;
        . += 4096;
        __kernel_ist3_base = .;
        . += 0x1000;
        __kernel_ist3_top = .;

        . = ALIGN(4096);
        __kernel_rsp0_guard = .;
        . += 4096;
        __kernel_rsp0_base = .;
        . += 0x1000;
        __kernel_rsp0_top = .;

        . = ALIGN(4096);
        __kernel_stack_guard = .;
        . += 4096;
        __kernel_stack_base = .;
        . += 0x4000;
        __kernel_stack_top = .;

        __kernel_bss_guard_base = .;
        . += 4096;
        __kernel_bss_guard_top = .;

        *(.bss*) 
    }
}
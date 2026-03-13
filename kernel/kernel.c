/*
 * Pristine
 * entry - kernel entry point
 * SPDX-License-Identifier: MIT
 */

#include "common/bootinfo.h"
#include "common/memmap.h"
#include <stdint.h>

void _putchar(char ch) {}

void kmain(uint32_t bootinfo_addr) {
    BootInfo *bootinfo = (BootInfo*)(uintptr_t)bootinfo_addr;
    MemmapEntry *entries = (MemmapEntry*)(uintptr_t)bootinfo->memory_map_addr;
    while(1);
}
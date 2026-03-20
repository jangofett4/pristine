/*
 * Pristine
 * SPDX-License-Identifier: MIT
 */

#pragma once

#include <stdint.h>
#include "lib32/printf/printf.h"

#define PANIC(fmt, ...) do { \
    printf("STAGE 2 PANIC at %s:%d in %s(): " fmt "\n", __FILE__, __LINE__, __func__, ##__VA_ARGS__); \
    __asm__ volatile ("cli; hlt"); \
} while(0)

#define STAGE2_MEMORY_BITMAP_START 0x100000
#define STAGE2_MEMORY_BITMAP_SIZE  0x0C0000
#define STAGE2_MEMORY_BITMAP_END   (STAGE2_MEMORY_BITMAP_START + STAGE2_MEMORY_BITMAP_SIZE)

#define STAGE2_PML4_ADDRESS      STAGE2_MEMORY_BITMAP_END
#define STAGE2_PML4_TABLE_COUNT  1

// we want 3 PDPTs
// one for the higher half direct mapping, one for kernel and one for the first 2 MiB direct mapping

#define STAGE2_PDPT_ADDRESS      (STAGE2_PML4_ADDRESS + (0x1000 * STAGE2_PML4_TABLE_COUNT))
#define STAGE2_PDPT_TABLE_COUNT  3

// first PD is kernel direct mapping
// second one is the bootloader direct mapping

#define STAGE2_PD_ADDRESS        (STAGE2_PDPT_ADDRESS + (0x1000 * STAGE2_PDPT_TABLE_COUNT))
#define STAGE2_PD_TABLE_COUNT    2

#define STAGE2_PT_ADDRESS        (STAGE2_PD_ADDRESS + (0x1000 * STAGE2_PD_TABLE_COUNT))
#define STAGE2_PT_TABLE_COUNT    2

_Static_assert(
    STAGE2_PML4_TABLE_COUNT >= 1 &&
    STAGE2_PDPT_TABLE_COUNT >= 3 &&
    STAGE2_PD_TABLE_COUNT >= 2 &&
    STAGE2_PT_TABLE_COUNT >= 2,
    "At least 1 PML4, 3 PDPT, 2 PD and 2 PT is needed for 64 bit paging"
);

#define STAGE2_KERNEL_PHYS_LOAD_ADDR 0x400000

#define STAGE2_GDT_ADDRESS     (0x7300)
#define STAGE2_GDT_MAX_ENTRIES (0x0100 / sizeof(uint64_t))

#define STAGE2_MEMMAP_ADDR       (0x6000 + 2)
#define STAGE2_MEMMAP_COUNT_ADDR (STAGE2_MEMMAP_ADDR - 2)

#define STAGE2_VESA_INFO_ADDR      0x7000
#define STAGE2_VESA_MODE_INFO_ADDR 0x7100
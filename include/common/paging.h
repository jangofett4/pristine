/*
 * Pristine
 * paging: paging related definitions
 * SPDX-License-Identifier: MIT
 */

#pragma once

#include <kernel/memory.h>
#include <common/bootinfo.h>
#include <stdint.h>

#define PAGE_DEFAULT_SIZE 4096
#define PAGE_LARGE_SIZE   0x200000
#define PAGE_HUGE_SIZE    0x40000000

#define PG_PML4_ADDRESS     MEMORY_BITMAP_END
#define PG_PML4_TABLE_COUNT 1

// we want 3 PDPTs
// one for the higher half direct mapping, one for kernel and one for the first 2 MiB direct mapping

#define PG_PDPT_ADDRESS     (PG_PML4_ADDRESS + (0x1000 * PG_PML4_TABLE_COUNT))
#define PG_PDPT_TABLE_COUNT 3

// first PD is kernel direct mapping
// second one is the bootloader direct mapping

#define PG_PD_ADDRESS       (PG_PDPT_ADDRESS + (0x1000 * PG_PDPT_TABLE_COUNT))
#define PG_PD_TABLE_COUNT   2

#define PG_PT_ADDRESS       (PG_PD_ADDRESS + (0x1000 * PG_PD_TABLE_COUNT))
#define PG_PT_TABLE_COUNT   2

_Static_assert(
    PG_PML4_TABLE_COUNT >= 1 &&
    PG_PDPT_TABLE_COUNT >= 3 &&
    PG_PD_TABLE_COUNT >= 2 &&
    PG_PT_TABLE_COUNT >= 2,
    "At least 1 PML4, 2 PDPT, 2 PD and 2 PT is needed for 64 bit paging"
);

#define PG_PML4_IDX(addr) (((addr) >> 39) & 0x1FF)
#define PG_PDPT_IDX(addr) (((addr) >> 30) & 0x1FF)
#define PG_PD_IDX(addr)   (((addr) >> 21) & 0x1FF)
#define PG_PT_IDX(addr)   (((addr) >> 12) & 0x1FF)

#define PAGING_PT_DEFAULT_FLAGS   0x03
#define PAGING_PT_GUARD_FLAGS     0x00

#define PAGING_PD_DEFAULT_FLAGS   0x03
#define PAGING_PD_LARGE_FLAGS     0x83

#define PAGING_PDPT_DEFAULT_FLAGS 0x03
#define PAGING_PDPT_HUGE_FLAGS   0x83

#define PAGING_PML4_DEFAULT_FLAGS 0x03

extern uint64_t volatile *__pg_pml4;
extern uint64_t volatile *__pg_pdpt_kernel;
extern uint64_t volatile *__pg_pdpt_higher_half;
extern uint64_t volatile *__pg_pdpt_ident;
extern uint64_t volatile *__pg_pd_kernel;
extern uint64_t volatile *__pg_pd_ident;
extern uint64_t volatile *__pg_pt_ident0;
extern uint64_t volatile *__pg_pt_ident1;

static inline void page_invlpg(void* address) {
    __asm__ volatile("invlpg (%0)" : : "r"(address));
}
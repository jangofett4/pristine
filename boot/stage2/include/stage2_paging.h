/*
 * Pristine
 * paging: paging related definitions
 * SPDX-License-Identifier: MIT
 */

#pragma once

#include "stage2_common.h"
#include <common/bootinfo.h>
#include <stdint.h>

#define PAGE_DEFAULT_SIZE 4096
#define PAGE_LARGE_SIZE   0x200000
#define PAGE_HUGE_SIZE    0x40000000

#define PG_PML4_IDX(addr) (((addr) >> 39) & 0x1FF)
#define PG_PDPT_IDX(addr) (((addr) >> 30) & 0x1FF)
#define PG_PD_IDX(addr)   (((addr) >> 21) & 0x1FF)
#define PG_PT_IDX(addr)   (((addr) >> 12) & 0x1FF)

#define PAGING_PT_DEFAULT_FLAGS   0x03
#define PAGING_PT_GUARD_FLAGS     0x00

#define PAGING_PD_DEFAULT_FLAGS   0x03
#define PAGING_PD_LARGE_FLAGS     0x83

#define PAGING_PDPT_DEFAULT_FLAGS 0x03
#define PAGING_PDPT_HUGE_FLAGS    0x83

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
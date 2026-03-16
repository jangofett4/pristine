/*
 * Pristine
 * paging: paging related definitions
 * SPDX-License-Identifier: MIT
 */

#pragma once

#include <common/bootinfo.h>
#include <stdint.h>

#define PG_PML4_ADDRESS 0x1000
#define PG_PDPT_ADDRESS 0x2000
#define PG_PD_ADDRESS   0x3000
#define PG_PT0_ADDRESS  0x4000
#define PG_PT1_ADDRESS  0x5000

#define PG_PML4_IDX(addr) (((addr) >> 39) & 0x1FF)
#define PG_PDPT_IDX(addr) (((addr) >> 30) & 0x1FF)
#define PG_PD_IDX(addr)   (((addr) >> 21) & 0x1FF)
#define PG_PT_IDX(addr)   (((addr) >> 12) & 0x1FF)

#define PAGING_PT_DEFAULT_FLAGS   0x03
#define PAGING_PT_GUARD_FLAGS     0x00

#define PAGING_PD_DEFAULT_FLAGS   0x03
#define PAGING_PD_HUGE_FLAGS      0x83

#define PAGING_PDPT_DEFAULT_FLAGS 0x03
#define PAGING_PML4_DEFAULT_FLAGS 0x03

extern uint64_t *volatile __pg_pml4;
extern uint64_t *volatile __pg_pdpt;
extern uint64_t *volatile __pg_pd;
extern uint64_t *volatile __pg_pt0;
extern uint64_t *volatile __pg_pt1;

static inline void pg_invlpg(void* address) {
    __asm__ volatile("invlpg (%0)" : : "r"(address));
}
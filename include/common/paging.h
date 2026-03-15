/*
 * Pristine
 * paging: paging related definitions
 * SPDX-License-Identifier: MIT
 */

#pragma once

#include <common/bootinfo.h>
#include <stdint.h>

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
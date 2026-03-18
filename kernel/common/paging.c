/*
 * Pristine
 * paging: paging related subroutines
 * SPDX-License-Identifier: MIT
 */

#include <kernel/panic.h>
#include <kernel/memory.h>
#include <common/paging.h>
#include <common/string.h>
#include <stdint.h>

uint64_t volatile *__pg_pml4             = (uint64_t*)((PG_PML4_ADDRESS + 0 * PAGE_DEFAULT_SIZE) + MEMORY_HHDM_START);
uint64_t volatile *__pg_pdpt_kernel      = (uint64_t*)((PG_PDPT_ADDRESS + 0 * PAGE_DEFAULT_SIZE) + MEMORY_HHDM_START);
uint64_t volatile *__pg_pdpt_higher_half = (uint64_t*)((PG_PDPT_ADDRESS + 1 * PAGE_DEFAULT_SIZE) + MEMORY_HHDM_START);
uint64_t volatile *__pg_pdpt_ident       = (uint64_t*)((PG_PDPT_ADDRESS + 2 * PAGE_DEFAULT_SIZE) + MEMORY_HHDM_START);
uint64_t volatile *__pg_pd_kernel        = (uint64_t*)((PG_PD_ADDRESS + 0 * PAGE_DEFAULT_SIZE)   + MEMORY_HHDM_START);
uint64_t volatile *__pg_pd_ident         = (uint64_t*)((PG_PD_ADDRESS + 1 * PAGE_DEFAULT_SIZE)   + MEMORY_HHDM_START);
uint64_t volatile *__pg_pt_ident0        = (uint64_t*)((PG_PT_ADDRESS + 0 * PAGE_DEFAULT_SIZE)   + MEMORY_HHDM_START);
uint64_t volatile *__pg_pt_ident1        = (uint64_t*)((PG_PT_ADDRESS + 1 * PAGE_DEFAULT_SIZE)   + MEMORY_HHDM_START);

uint64_t __global_pml4[512];
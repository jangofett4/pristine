/*
 * Pristine
 * stage2_paging: paging related subroutines
 * SPDX-License-Identifier: MIT
 */

#include "stage2_paging.h"
#include "stage2_memory.h"

uint64_t *volatile _pml4;
uint64_t *volatile _pdpt;
uint64_t *volatile _pd;
uint64_t *volatile _pt;

void paging_set_pml4_address(uint64_t address) {
    _pml4 = (uint64_t*)address;
    memset((void*)_pml4, 0, 0x1000);
}

void paging_set_pdpt_address(uint64_t address) {
    _pdpt = (uint64_t*)address;
    memset((void*)_pdpt, 0, 0x1000);
}

void paging_set_pd_address(uint64_t address) {
    _pd = (uint64_t*)address;
    memset((void*)_pd, 0, 0x1000);
}

void paging_set_pt_address(uint64_t address) {
    _pt = (uint64_t*)address;
    memset((void*)_pt, 0, 0x1000);
}

uint64_t volatile* paging_get_pd(void) {
    return _pd;
}

#define PAGING_PD_DEFAULT_FLAGS   0x83
#define PAGING_PDPT_DEFAULT_FLAGS 0x03
#define PAGING_PML4_DEFAULT_FLAGS 0x03

void paging_set_level_3_map(uint16_t pml4idx, uint16_t pdptidx, uint16_t pdidx, uint64_t address) {
    // TODO: would be proper to panic if address is not 2 MiB aligned
    _pd[pdidx]     = PAGING_PD_DEFAULT_FLAGS | address;
    _pdpt[pdptidx] = PAGING_PDPT_DEFAULT_FLAGS | (uint64_t)_pd;
    _pml4[pml4idx] = PAGING_PML4_DEFAULT_FLAGS | (uint64_t)_pdpt;
}
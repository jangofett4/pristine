/*
 * Pristine
 * stage2_paging: paging related subroutines
 * SPDX-License-Identifier: MIT
 */

#pragma once

#include <stdint.h>

void paging_set_pml4_address(uint64_t address);
void paging_set_pdpt_address(uint64_t address);
void paging_set_pd_address(uint64_t address);
void paging_set_pt_address(uint64_t address);

uint64_t volatile* paging_get_pd(void);

void paging_set_level_3_map(uint16_t pml4idx, uint16_t pdptidx, uint16_t pdidx, uint64_t address);

extern void kernel_entry(uint32_t pml4_address, uint64_t kernel_entry_address);
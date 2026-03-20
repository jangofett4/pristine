/*
 * Pristine
 * vmm: virtual memory management related definitions
 * SPDX-License-Identifier: MIT
 */

#pragma once

#include <kernel/pmm.h>
#include <stdint.h>

#define VMM_DEFAULT_PAGE_SIZE 4096
#define VMM_LARGE_PAGE_SIZE   0x200000
#define VMM_HUGE_PAGE_SIZE    0x40000000

#define VMM_PML4_IDX(addr) (((addr) >> 39) & 0x1FF)
#define VMM_PDPT_IDX(addr) (((addr) >> 30) & 0x1FF)
#define VMM_PD_IDX(addr)   (((addr) >> 21) & 0x1FF)
#define VMM_PT_IDX(addr)   (((addr) >> 12) & 0x1FF)

#define VMM_PT_DEFAULT_FLAGS   0x03
#define VMM_PT_GUARD_FLAGS     0x00

#define VMM_PD_DEFAULT_FLAGS   0x03
#define VMM_PD_LARGE_FLAGS     0x83

#define VMM_PDPT_DEFAULT_FLAGS 0x03
#define VMM_PDPT_HUGE_FLAGS    0x83

#define VMM_PML4_DEFAULT_FLAGS 0x03

extern uint64_t volatile *__vmm_pml4;

static inline void vmm_invlpg(void* address) {
    __asm__ volatile("invlpg (%0)" : : "r"(address));
}
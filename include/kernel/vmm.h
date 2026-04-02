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
#define VMM_PTE_ADDR_MASK     0x000FFFFFFFFFF000ULL

#define VMM_PML4_IDX(addr) (((addr) >> 39) & 0x1FF)
#define VMM_PDPT_IDX(addr) (((addr) >> 30) & 0x1FF)
#define VMM_PD_IDX(addr)   (((addr) >> 21) & 0x1FF)
#define VMM_PT_IDX(addr)   (((addr) >> 12) & 0x1FF)

#define VMM_FLAGS_KERNEL_CODE  0x03                   // P + R/W, no NX
#define VMM_FLAGS_KERNEL_DATA  (0x03 | (1ULL << 63))  // P + R/W + NX

// userspace pages
#define VMM_FLAGS_USER_CODE    0x07                   // P + R/W + User
#define VMM_FLAGS_USER_DATA    (0x07 | (1ULL << 63))  // P + R/W + User + NX

// intermediate levels (PML4, PDPT, PD)
#define VMM_FLAGS_TABLE_KERNEL 0x03                   // P + R/W
#define VMM_FLAGS_TABLE_USER   0x07 

#define VMM_FLAG_USER          0x04

#define VMM_PDPT_HUGE_FLAGS    0x80
#define VMM_PD_LARGE_FLAGS     0x80

extern uint64_t *vmm_pml4;

void  vmm_init(void);

void  vmm_map(uint64_t *pml4, uint64_t phys, uint64_t virt, uint64_t flags);
void  vmm_unmap(uint64_t *pml4, uint64_t virt);

void  vmm_map_large(uint64_t *pml4, uint64_t phys, uint64_t virt, uint64_t flags);
void  vmm_unmap_large(uint64_t *pml4, uint64_t virt);

void  vmm_map_huge(uint64_t *pml4, uint64_t phys, uint64_t virt, uint64_t flags);
void  vmm_unmap_huge(uint64_t *pml4, uint64_t virt);

// Deallocates all sub entries of given PML4, doesn't free the PML4 itself. Only destroys up to PML4[256], upper half is untouched.
void  vmm_destroy(uint64_t *pml4);

// void *vmm_alloc(void);
// void  vmm_free(void* memory);
void  vmm_switch(uint64_t *pml4);

static inline void vmm_invlpg(void* address) {
    __asm__ volatile("invlpg (%0)" : : "r"(address));
}
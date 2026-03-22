/*
 * Pristine
 * vmm: virtual memory management related definitions
 * SPDX-License-Identifier MIT
 */

#include <kernel/panic.h>
#include <kernel/vmm.h>
#include <kernel/pmm.h>
#include <common/common.h>
#include <common/string.h>

uint64_t *__vmm_pml4;

void vmm_init(void) {
    __vmm_pml4 = phys_to_virt(pmm_alloc());
    memset(__vmm_pml4, 0, VMM_DEFAULT_PAGE_SIZE);
}

void vmm_map(uint64_t *pml4, uint64_t phys, uint64_t virt, uint64_t flags) {
    const uint16_t pml4idx = VMM_PML4_IDX(virt);
    const uint16_t pdptidx = VMM_PDPT_IDX(virt);
    const uint16_t pdidx   = VMM_PD_IDX(virt);
    const uint16_t ptidx   = VMM_PT_IDX(virt);
    uint64_t  table_phys_addr;
    uint64_t *table_ptr;

    #ifdef PRISTINE_DEBUG
    if (!is_aligned(phys, VMM_DEFAULT_PAGE_SIZE)) {
        KPANIC("vmm_map: unaligned physical address 0x%lx", phys);
    }
    if (!is_aligned(virt, VMM_DEFAULT_PAGE_SIZE)) {
        KPANIC("vmm_map: unaligned virtual address 0x%lx", virt);
    }
    #endif
    
    uint64_t iflags = VMM_FLAGS_TABLE_KERNEL;
    if (flags & VMM_FLAG_USER)
        iflags = VMM_FLAGS_TABLE_USER;

    if (pml4[pml4idx] == 0) {
        table_phys_addr = pmm_alloc();
        pml4[pml4idx] = table_phys_addr | iflags;
        table_ptr = phys_to_virt(table_phys_addr);
        memset(table_ptr, 0, VMM_DEFAULT_PAGE_SIZE);
    } else {
        table_ptr = phys_to_virt(pml4[pml4idx] & VMM_PTE_ADDR_MASK);
    }

    if (table_ptr[pdptidx] == 0) {
        table_phys_addr = pmm_alloc();
        table_ptr[pdptidx] = table_phys_addr | iflags;
        table_ptr = phys_to_virt(table_phys_addr);
        memset(table_ptr, 0, VMM_DEFAULT_PAGE_SIZE);
    } else {
        table_ptr = phys_to_virt(table_ptr[pdptidx] & VMM_PTE_ADDR_MASK);
    }

    if (table_ptr[pdidx] == 0) {
        table_phys_addr = pmm_alloc();
        table_ptr[pdidx] = table_phys_addr | iflags;
        table_ptr = phys_to_virt(table_phys_addr);
        memset(table_ptr, 0, VMM_DEFAULT_PAGE_SIZE);
    } else {
        table_ptr = phys_to_virt(table_ptr[pdidx] & VMM_PTE_ADDR_MASK);
    }

    #ifdef PRISTINE_DEBUG
    if (table_ptr[ptidx] != 0)
        KPANIC("vmm_map: mapping already exists at virt 0x%lx", virt);
    #endif

    table_ptr[ptidx] = phys | flags;
}

void vmm_map_large(uint64_t *pml4, uint64_t phys, uint64_t virt, uint64_t flags) {
    const uint16_t pml4idx = VMM_PML4_IDX(virt);
    const uint16_t pdptidx = VMM_PDPT_IDX(virt);
    const uint16_t pdidx   = VMM_PD_IDX(virt);
    uint64_t  table_phys_addr;
    uint64_t *table_ptr;

    #ifdef PRISTINE_DEBUG
    if (!is_aligned(phys, VMM_LARGE_PAGE_SIZE)) {
        KPANIC("vmm_map_large: unaligned physical address 0x%lx", phys);
    }
    if (!is_aligned(virt, VMM_LARGE_PAGE_SIZE)) {
        KPANIC("vmm_map_large: unaligned virtual address 0x%lx", virt);
    }
    #endif
    
    uint64_t iflags = VMM_FLAGS_TABLE_KERNEL;
    if (flags & VMM_FLAG_USER)
        iflags = VMM_FLAGS_TABLE_USER;

    if (pml4[pml4idx] == 0) {
        table_phys_addr = pmm_alloc();
        pml4[pml4idx] = table_phys_addr | iflags;
        table_ptr = phys_to_virt(table_phys_addr);
        memset(table_ptr, 0, VMM_DEFAULT_PAGE_SIZE);
    } else {
        table_ptr = phys_to_virt(pml4[pml4idx] & VMM_PTE_ADDR_MASK);
    }

    if (table_ptr[pdptidx] == 0) {
        table_phys_addr = pmm_alloc();
        table_ptr[pdptidx] = table_phys_addr | iflags;
        table_ptr = phys_to_virt(table_phys_addr);
        memset(table_ptr, 0, VMM_DEFAULT_PAGE_SIZE);
    } else {
        #ifdef PRISTINE_DEBUG
        if (table_ptr[pdptidx] & VMM_PD_LARGE_FLAGS)  // PS bit set = huge/large page, not a table
            KPANIC("vmm_map: tried to map through existing huge page at virt 0x%lx", virt);
        #endif
        table_ptr = phys_to_virt(table_ptr[pdptidx] & VMM_PTE_ADDR_MASK);
    }

    #ifdef PRISTINE_DEBUG
    if (table_ptr[pdidx] != 0)
        KPANIC("vmm_map_large: mapping already exists at virt 0x%lx", virt);
    #endif

    table_ptr[pdidx] = phys | (flags | VMM_PD_LARGE_FLAGS);
}

void vmm_map_huge(uint64_t *pml4, uint64_t phys, uint64_t virt, uint64_t flags) {
    const uint16_t pml4idx = VMM_PML4_IDX(virt);
    const uint16_t pdptidx = VMM_PDPT_IDX(virt);
    uint64_t  table_phys_addr;
    uint64_t *table_ptr;

    #ifdef PRISTINE_DEBUG
    if (!is_aligned(phys, VMM_HUGE_PAGE_SIZE)) {
        KPANIC("vmm_map_huge: unaligned physical address 0x%lx", phys);
    }
    if (!is_aligned(virt, VMM_HUGE_PAGE_SIZE)) {
        KPANIC("vmm_map_huge: unaligned virtual address 0x%lx", virt);
    }
    #endif
    
    uint64_t iflags = VMM_FLAGS_TABLE_KERNEL;
    if (flags & VMM_FLAG_USER)
        iflags = VMM_FLAGS_TABLE_USER;

    if (pml4[pml4idx] == 0) {
        table_phys_addr = pmm_alloc();
        pml4[pml4idx] = table_phys_addr | iflags;
        table_ptr = phys_to_virt(table_phys_addr);
        memset(table_ptr, 0, VMM_DEFAULT_PAGE_SIZE);
    } else {
        table_ptr = phys_to_virt(pml4[pml4idx] & VMM_PTE_ADDR_MASK);
    }

    #ifdef PRISTINE_DEBUG
    if (table_ptr[pdptidx] != 0)
        KPANIC("vmm_map_huge: mapping already exists at virt 0x%lx", virt);
    #endif

    table_ptr[pdptidx] = phys | (flags | VMM_PDPT_HUGE_FLAGS);
}

void *vmm_alloc(void) {
    uint64_t addr = pmm_alloc();
    const uint16_t pml4idx = VMM_PML4_IDX(addr);
    const uint16_t pdptidx = VMM_PDPT_IDX(addr);
    const uint16_t pdidx   = VMM_PD_IDX(addr);
    const uint16_t ptidx   = VMM_PT_IDX(addr);
    uint64_t  table_phys_addr;
    uint64_t *table_ptr;

    // create PDPT in PML4 if doesn't exists
    if (__vmm_pml4[pml4idx] == 0) {
        table_phys_addr = pmm_alloc();
        __vmm_pml4[pml4idx] = table_phys_addr | VMM_FLAGS_TABLE_KERNEL;
        table_ptr = phys_to_virt(table_phys_addr);
        memset(table_ptr, 0, VMM_DEFAULT_PAGE_SIZE);
    } else {
        table_ptr = phys_to_virt(__vmm_pml4[pml4idx] & VMM_PTE_ADDR_MASK);
    }

    // create PD in PDPT if doesn't exists
    if (table_ptr[pdptidx] == 0) {
        table_phys_addr = pmm_alloc();
        table_ptr[pdptidx] = table_phys_addr | VMM_FLAGS_TABLE_KERNEL;
        table_ptr = phys_to_virt(table_phys_addr);
        memset(table_ptr, 0, VMM_DEFAULT_PAGE_SIZE);
    } else {
        table_ptr = phys_to_virt(table_ptr[pdptidx] & VMM_PTE_ADDR_MASK);
    }

    // create PT in PD if doesn't exists
    if (table_ptr[pdidx] == 0) {
        table_phys_addr = pmm_alloc();
        table_ptr[pdidx] = table_phys_addr | VMM_FLAGS_TABLE_KERNEL;
        table_ptr = phys_to_virt(table_phys_addr);
        memset(table_ptr, 0, VMM_DEFAULT_PAGE_SIZE);
    } else {
        table_ptr = phys_to_virt(table_ptr[pdidx] & VMM_PTE_ADDR_MASK);
    }

    table_ptr[ptidx] = addr | VMM_FLAGS_KERNEL_DATA;
    return table_ptr;
}

void vmm_switch(uint64_t *pml4) {
    uint64_t phys = virt_to_phys(pml4);
    __asm__ volatile("mov %0, %%cr3" : : "r"(phys) : "memory");
}

void vmm_free(void *memory) {

}
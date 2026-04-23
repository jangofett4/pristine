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

uint64_t *vmm_init(void) {
    uint64_t *pml4 = phys_to_virt(pmm_alloc());
    memset(pml4, 0, VMM_DEFAULT_PAGE_SIZE);
    return pml4;
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

void vmm_unmap(uint64_t *pml4, uint64_t virt) {
    const uint16_t pml4idx = VMM_PML4_IDX(virt);
    const uint16_t pdptidx = VMM_PDPT_IDX(virt);
    const uint16_t pdidx   = VMM_PD_IDX(virt);
    const uint16_t ptidx   = VMM_PT_IDX(virt);
    uint64_t *table_ptr;

    #ifdef PRISTINE_DEBUG
    if (!is_aligned(virt, VMM_DEFAULT_PAGE_SIZE)) {
        KPANIC("vmm_unmap: unaligned virtual address 0x%lx", virt);
    }
    #endif

    if (pml4[pml4idx] == 0) {
        KPANIC("vmm_unmap: PDPT table doesn't exist for virt 0x%lx\n", virt);
    } else {
        table_ptr = phys_to_virt(pml4[pml4idx] & VMM_PTE_ADDR_MASK);
    }

    if (table_ptr[pdptidx] == 0) {
        KPANIC("vmm_unmap: PD table doesn't exist for virt 0x%lx\n", virt);
    } else {
        #ifdef PRISTINE_DEBUG
        if (table_ptr[pdptidx] & VMM_PDPT_HUGE_FLAGS)
            KPANIC("vmm_unmap: mapping for 0x%lx is mapped as huge", virt);
        #endif
        table_ptr = phys_to_virt(table_ptr[pdptidx] & VMM_PTE_ADDR_MASK);
    }

    if (table_ptr[pdidx] == 0) {
        KPANIC("vmm_unmap: PT table doesn't exist for virt 0x%lx\n", virt);
    } else {
        #ifdef PRISTINE_DEBUG
        if (table_ptr[pdidx] & VMM_PD_LARGE_FLAGS)
            KPANIC("vmm_unmap: mapping for 0x%lx is mapped as large", virt);
        #endif
        table_ptr = phys_to_virt(table_ptr[pdidx] & VMM_PTE_ADDR_MASK);
    }

    #ifdef PRISTINE_DEBUG
    if (table_ptr[ptidx] == 0)
        KPANIC("vmm_unmap: mapping for 0x%lx doesn't exist", virt);
    #endif

    table_ptr[ptidx] = 0;
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
            KPANIC("vmm_map_large: tried to map through existing huge page at virt 0x%lx", virt);
        #endif
        table_ptr = phys_to_virt(table_ptr[pdptidx] & VMM_PTE_ADDR_MASK);
    }

    #ifdef PRISTINE_DEBUG
    if (table_ptr[pdidx] != 0)
        KPANIC("vmm_map_large: mapping already exists at virt 0x%lx", virt);
    #endif

    table_ptr[pdidx] = phys | (flags | VMM_PD_LARGE_FLAGS);
}

void vmm_unmap_large(uint64_t *pml4, uint64_t virt) {
    const uint16_t pml4idx = VMM_PML4_IDX(virt);
    const uint16_t pdptidx = VMM_PDPT_IDX(virt);
    const uint16_t pdidx   = VMM_PD_IDX(virt);
    uint64_t *table_ptr;

    #ifdef PRISTINE_DEBUG
    if (!is_aligned(virt, VMM_LARGE_PAGE_SIZE)) {
        KPANIC("vmm_unmap_large: unaligned virtual address 0x%lx", virt);
    }
    #endif

    if (pml4[pml4idx] == 0) {
        KPANIC("vmm_unmap_large: PDPT table doesn't exist for virt 0x%lx\n", virt);
    } else {
        table_ptr = phys_to_virt(pml4[pml4idx] & VMM_PTE_ADDR_MASK);
    }

    if (table_ptr[pdptidx] == 0) {
        KPANIC("vmm_unmap_large: PD table doesn't exist for virt 0x%lx\n", virt);
    } else {
        table_ptr = phys_to_virt(table_ptr[pdptidx] & VMM_PTE_ADDR_MASK);
        #ifdef PRISTINE_DEBUG
        if (!(table_ptr[pdidx] & VMM_PD_LARGE_FLAGS))
            KPANIC("vmm_unmap_large: mapping for 0x%lx is not mapped as large", virt);
        #endif
    }

    #ifdef PRISTINE_DEBUG
    if (table_ptr[pdidx] == 0)
        KPANIC("vmm_unmap_large: mapping for 0x%lx doesn't exist", virt);
    #endif

    table_ptr[pdidx] = 0;
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

void vmm_unmap_huge(uint64_t *pml4, uint64_t virt) {
    const uint16_t pml4idx = VMM_PML4_IDX(virt);
    const uint16_t pdptidx = VMM_PDPT_IDX(virt);
    uint64_t *table_ptr;

    #ifdef PRISTINE_DEBUG
    if (!is_aligned(virt, VMM_HUGE_PAGE_SIZE)) {
        KPANIC("vmm_unmap_huge: unaligned virtual address 0x%lx", virt);
    }
    #endif

    if (pml4[pml4idx] == 0) {
        KPANIC("vmm_unmap_huge: PDPT table doesn't exist for virt 0x%lx\n", virt);
    } else {
        table_ptr = phys_to_virt(pml4[pml4idx] & VMM_PTE_ADDR_MASK);
        #ifdef PRISTINE_DEBUG
        if (!(table_ptr[pdptidx] & VMM_PDPT_HUGE_FLAGS))
            KPANIC("vmm_unmap_huge: mapping for 0x%lx is not mapped as huge", virt);
        #endif
    }

    #ifdef PRISTINE_DEBUG
    if (table_ptr[pdptidx] == 0)
        KPANIC("vmm_unmap_huge: mapping for 0x%lx doesn't exist", virt);
    #endif

    table_ptr[pdptidx] = 0;
}

void vmm_free(uint64_t *pml4, void *virt) {
    const uint16_t pml4idx = VMM_PML4_IDX((uintptr_t)virt);
    const uint16_t pdptidx = VMM_PDPT_IDX((uintptr_t)virt);
    const uint16_t pdidx   = VMM_PD_IDX((uintptr_t)virt);
    const uint16_t ptidx   = VMM_PT_IDX((uintptr_t)virt);

    uint64_t *pdpt = phys_to_virt(pml4[pml4idx] & VMM_PTE_ADDR_MASK);
    if (!pdpt || !(pml4[pml4idx] & VMM_FLAGS_PRESENT)) {
        KPANIC("vmm_free: PDPT at %u is not present", pml4idx);
    }

    if (pdpt[pdptidx] & VMM_PDPT_HUGE_FLAGS) {
        const uintptr_t phys_start = pdpt[pdptidx] & VMM_PTE_ADDR_MASK;
        const uintptr_t phys_end   = phys_start + VMM_HUGE_PAGE_SIZE;
        for (uintptr_t p = phys_start; p < phys_end; p += VMM_DEFAULT_PAGE_SIZE) {
            pmm_free(p);
        }
        pdpt[pdptidx] = 0;
        vmm_invlpg(virt);
        return;
    }

    uint64_t *pd = phys_to_virt(pdpt[pdptidx] & VMM_PTE_ADDR_MASK);
    if (!pd || !(pdpt[pdptidx] & VMM_FLAGS_PRESENT)) {
        KPANIC("vmm_free: PD at %u is not present", pdptidx);
    }

    if (pd[pdidx] & VMM_PD_LARGE_FLAGS) {
        const uintptr_t phys_start = pd[pdidx] & VMM_PTE_ADDR_MASK;
        const uintptr_t phys_end   = phys_start + VMM_LARGE_PAGE_SIZE;
        for (uintptr_t p = phys_start; p < phys_end; p += VMM_DEFAULT_PAGE_SIZE) {
            pmm_free(p);
        }
        pd[pdidx] = 0;
        vmm_invlpg(virt);
        return;
    }

    uint64_t *pt = phys_to_virt(pd[pdidx] & VMM_PTE_ADDR_MASK);
    if (!pt || !(pt[ptidx] & VMM_FLAGS_PRESENT)) {
        KPANIC("vmm_free: PT at %u is not present", ptidx);
    }

    pmm_free(pt[ptidx] & VMM_PTE_ADDR_MASK);
    pt[ptidx] = 0;
    vmm_invlpg(virt);
}

void vmm_destroy(uint64_t *pml4) {
    for (size_t pml4idx = 0; pml4idx < 256; pml4idx++) { // skip upper half, kernel is mapped there
        if (pml4[pml4idx] == 0) continue;
        uint64_t *pdpt_table = phys_to_virt(pml4[pml4idx] & VMM_PTE_ADDR_MASK);
        for (size_t pdptidx = 0; pdptidx < 512; pdptidx++) {
            if (pdpt_table[pdptidx] == 0) continue;
            if (pdpt_table[pdptidx] & VMM_PDPT_HUGE_FLAGS) continue;
            uint64_t *pd_table = phys_to_virt(pdpt_table[pdptidx] & VMM_PTE_ADDR_MASK);
            for (size_t pdidx = 0; pdidx < 512; pdidx++) {
                if (pd_table[pdidx] == 0) continue;
                if (pd_table[pdidx] & VMM_PD_LARGE_FLAGS) continue;
                uint64_t *pt_table = phys_to_virt(pd_table[pdidx] & VMM_PTE_ADDR_MASK);
                for (size_t ptidx = 0; ptidx < 512; ptidx++) {
                    if (pt_table[ptidx] == 0) continue;
                    pmm_free(pt_table[ptidx] & VMM_PTE_ADDR_MASK);
                }
                pmm_free(pd_table[pdidx] & VMM_PTE_ADDR_MASK);
            }
            pmm_free(pdpt_table[pdptidx] & VMM_PTE_ADDR_MASK);
        }
        pmm_free(pml4[pml4idx] & VMM_PTE_ADDR_MASK);
    }
}

void vmm_switch(uint64_t *pml4) {
    uint64_t phys = virt_to_phys(pml4);
    __asm__ volatile("mov %0, %%cr3" : : "r"(phys) : "memory");
}
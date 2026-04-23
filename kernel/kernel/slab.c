/*
 * Pristine
 * slab: Binning slab allocator definitions
 * SPDX-License-Identifier: MIT
 */

#include <kernel/cpu.h>
#include <common/common.h>
#include <common/idt64.h>
#include <kernel/panic.h>
#include <kernel/state.h>
#include <kernel/slab.h>
#include <kernel/pmm.h>
#include <kernel/vmm.h>
#include <stdint.h>

static BinHeader bins[SLAB_TOTAL_BINS] = {
    { .object_size = 8   , .free_slabs = NULL, .full_slabs = NULL, .max_objects = (VMM_DEFAULT_PAGE_SIZE - sizeof(SlabHeader)) / 8 },
    { .object_size = 16  , .free_slabs = NULL, .full_slabs = NULL, .max_objects = (VMM_DEFAULT_PAGE_SIZE - sizeof(SlabHeader)) / 16 },
    { .object_size = 24  , .free_slabs = NULL, .full_slabs = NULL, .max_objects = (VMM_DEFAULT_PAGE_SIZE - sizeof(SlabHeader)) / 24 },
    { .object_size = 32  , .free_slabs = NULL, .full_slabs = NULL, .max_objects = (VMM_DEFAULT_PAGE_SIZE - sizeof(SlabHeader)) / 32 },
    { .object_size = 40  , .free_slabs = NULL, .full_slabs = NULL, .max_objects = (VMM_DEFAULT_PAGE_SIZE - sizeof(SlabHeader)) / 40 },
    { .object_size = 48  , .free_slabs = NULL, .full_slabs = NULL, .max_objects = (VMM_DEFAULT_PAGE_SIZE - sizeof(SlabHeader)) / 48 },
    { .object_size = 56  , .free_slabs = NULL, .full_slabs = NULL, .max_objects = (VMM_DEFAULT_PAGE_SIZE - sizeof(SlabHeader)) / 56 },
    { .object_size = 64  , .free_slabs = NULL, .full_slabs = NULL, .max_objects = (VMM_DEFAULT_PAGE_SIZE - sizeof(SlabHeader)) / 64 },
    { .object_size = 80  , .free_slabs = NULL, .full_slabs = NULL, .max_objects = (VMM_DEFAULT_PAGE_SIZE - sizeof(SlabHeader)) / 80 },
    { .object_size = 96  , .free_slabs = NULL, .full_slabs = NULL, .max_objects = (VMM_DEFAULT_PAGE_SIZE - sizeof(SlabHeader)) / 96 },
    { .object_size = 112 , .free_slabs = NULL, .full_slabs = NULL, .max_objects = (VMM_DEFAULT_PAGE_SIZE - sizeof(SlabHeader)) / 112 },
    { .object_size = 128 , .free_slabs = NULL, .full_slabs = NULL, .max_objects = (VMM_DEFAULT_PAGE_SIZE - sizeof(SlabHeader)) / 128 },
    { .object_size = 160 , .free_slabs = NULL, .full_slabs = NULL, .max_objects = (VMM_DEFAULT_PAGE_SIZE - sizeof(SlabHeader)) / 160 },
    { .object_size = 192 , .free_slabs = NULL, .full_slabs = NULL, .max_objects = (VMM_DEFAULT_PAGE_SIZE - sizeof(SlabHeader)) / 192 },
    { .object_size = 224 , .free_slabs = NULL, .full_slabs = NULL, .max_objects = (VMM_DEFAULT_PAGE_SIZE - sizeof(SlabHeader)) / 224 },
    { .object_size = 256 , .free_slabs = NULL, .full_slabs = NULL, .max_objects = (VMM_DEFAULT_PAGE_SIZE - sizeof(SlabHeader)) / 256 },
    { .object_size = 320 , .free_slabs = NULL, .full_slabs = NULL, .max_objects = (VMM_DEFAULT_PAGE_SIZE - sizeof(SlabHeader)) / 320 },
    { .object_size = 384 , .free_slabs = NULL, .full_slabs = NULL, .max_objects = (VMM_DEFAULT_PAGE_SIZE - sizeof(SlabHeader)) / 384 },
    { .object_size = 448 , .free_slabs = NULL, .full_slabs = NULL, .max_objects = (VMM_DEFAULT_PAGE_SIZE - sizeof(SlabHeader)) / 448 },
    { .object_size = 512 , .free_slabs = NULL, .full_slabs = NULL, .max_objects = (VMM_DEFAULT_PAGE_SIZE - sizeof(SlabHeader)) / 512 },
    { .object_size = 640 , .free_slabs = NULL, .full_slabs = NULL, .max_objects = (VMM_DEFAULT_PAGE_SIZE - sizeof(SlabHeader)) / 640 },
    { .object_size = 768 , .free_slabs = NULL, .full_slabs = NULL, .max_objects = (VMM_DEFAULT_PAGE_SIZE - sizeof(SlabHeader)) / 768 },
    { .object_size = 896 , .free_slabs = NULL, .full_slabs = NULL, .max_objects = (VMM_DEFAULT_PAGE_SIZE - sizeof(SlabHeader)) / 896 },
    { .object_size = 1024, .free_slabs = NULL, .full_slabs = NULL, .max_objects = (VMM_DEFAULT_PAGE_SIZE - sizeof(SlabHeader)) / 1024 },
};

static uintptr_t slab_top = 0;

void *bin_alloc(uint16_t bin_idx) {
    #if PRISTINE_DEBUG
    if (bin_idx >= SLAB_TOTAL_BINS) {
        KPANIC("bin_alloc: invalid bin index: %u", bin_idx);
    }
    if (slab_top >= SLAB_VIRT_END - VMM_DEFAULT_PAGE_SIZE) {
        // TODO: At this point we should probably print out who allocated so many slabs
        //       for debug purposes
        KPANIC("bin_alloc: virtal address space exhausted for slab allocation");
    }
    #endif

    BinHeader  *bin = bins + bin_idx;
    SlabHeader *slab;

    if (bin->free_slabs == NULL) {
        const uintptr_t slab_phys     = pmm_alloc();
        const uintptr_t slab_virt     = SLAB_VIRT_START + slab_top;
        const uintptr_t slab_virt_end = slab_virt + VMM_DEFAULT_PAGE_SIZE;
        vmm_map(
            get_global_state()->pml4,
            slab_phys,
            slab_virt,
            VMM_FLAGS_KERNEL_DATA
        );
        slab_top += VMM_DEFAULT_PAGE_SIZE;

        slab                = (SlabHeader*)slab_virt;
        slab->count_objects = 0;
        slab->phys          = slab_phys;
        slab->bin           = bin_idx;
        for (uintptr_t i = slab_virt + sizeof(SlabHeader); i < slab_virt_end; i += bin->object_size) {
            *((uintptr_t*)i) = i + bin->object_size;
        }
        *((uintptr_t*)(slab_virt_end - bin->object_size)) = 0;
        slab->next_free = (void*)slab_virt + sizeof(SlabHeader);
        slab->next_slab = bin->free_slabs;
        slab->prev_slab = NULL;
        if (bin->free_slabs != NULL)
            bin->free_slabs->prev_slab = slab;
        bin->free_slabs = slab;
    }

    slab = bin->free_slabs;
    void* addr = slab->next_free;
    uintptr_t next_free = *(uintptr_t*)slab->next_free;
    slab->next_free = (uintptr_t*)next_free;
    ((SlabHeader*)((uintptr_t)addr & ~0xFFF))->count_objects++;

    if (slab->count_objects == bin->max_objects) {
        bin->free_slabs = slab->next_slab;
        if (bin->free_slabs != NULL)
            bin->free_slabs->prev_slab = NULL;
        if (bin->full_slabs != NULL)
            bin->full_slabs->prev_slab = slab;
        slab->next_slab = bin->full_slabs;
        slab->prev_slab = NULL;
        bin->full_slabs = slab;
    }

    return addr;
}

bool bin_free(void *addr) {
    #if PRISTINE_DEBUG
    if ((uintptr_t)addr < SLAB_VIRT_START || (uintptr_t)addr > SLAB_VIRT_END) {
        KPANIC("bin_free: non-slab allocated address passed: %p", addr);
    }
    #endif

    SlabHeader *slab = (SlabHeader*)((uintptr_t)addr & ~0xFFF);
    
    #if PRISTINE_DEBUG
    if (slab->bin >= SLAB_TOTAL_BINS) {
        KPANIC("bin_free: invalid bin index: %u", slab->bin);
    }
    #endif

    BinHeader *bin = bins + slab->bin;

    // TODO: alignment check here is removed.
    //       this makes the bin_alloc much less convoluted, but it also could be
    //       hard to debug if a moved pointer is passed (bin_free(ptr + 1))
    //       a proper alignment check needs to be implemented

    const bool was_full = slab->count_objects == bin->max_objects;

    *(uintptr_t*)addr = (uintptr_t)slab->next_free;
    slab->next_free = (uintptr_t*)addr;
    slab->count_objects--;

    if (was_full) {
        if (bin->full_slabs == slab)
            bin->full_slabs = slab->next_slab;
        if (slab->next_slab != NULL)
            slab->next_slab->prev_slab = slab->prev_slab; 
        if (slab->prev_slab != NULL)
            slab->prev_slab->next_slab = slab->next_slab;
        if (bin->free_slabs != NULL)
            bin->free_slabs->prev_slab = slab;
        slab->next_slab = bin->free_slabs;
        slab->prev_slab = NULL;
        bin->free_slabs = slab;
    }

    if (slab->count_objects == 0) {
        if (bin->free_slabs == slab)
            bin->free_slabs = slab->next_slab;
        if (slab->next_slab != NULL)
            slab->next_slab->prev_slab = slab->prev_slab; 
        if (slab->prev_slab != NULL)
            slab->prev_slab->next_slab = slab->next_slab;
        pmm_free(slab->phys);
        vmm_unmap(get_global_state()->pml4, (uint64_t)slab);
        vmm_invlpg(slab);
    }
    
    return true;
}
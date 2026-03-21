/*
 * Pristine
 * bootinfo: data expected by kernel upon loading
 * SPDX-License-Identifier MIT
 */

#include <kernel/pmm.h>
#include <stddef.h> 
#include <stdint.h>
 
#include <common/bootinfo.h>
#include <common/memmap.h>
#include <common/vesa.h>

BootInfo bootinfo_copy(const RawBootInfo *ptr) {
    BootInfo bootinfo;

    bootinfo.raw = *ptr;
    
    for (size_t i = 0; i < ptr->memory_map_count; i++) {
        bootinfo.memory_map[i] = ((MemmapEntry*)(uintptr_t)(ptr->memory_map_addr + PMM_HHDM_START))[i];
    }
    bootinfo.memory_map_count = ptr->memory_map_count;
    
    bootinfo.vesa_vbe_info = *(VesaVbeInfo*)(uintptr_t)(ptr->vesa_vbe_info_addr + PMM_HHDM_START);
    bootinfo.vesa_vbe_mode_info = *(VesaVbeModeInfo*)(uintptr_t)(ptr->vesa_vbe_mode_info_addr + PMM_HHDM_START);

    bootinfo.memory_bitmap = (uint8_t*)((uintptr_t)ptr->memory_bitmap_address + PMM_HHDM_START);
    bootinfo.memory_bitmap_size = ptr->memory_bitmap_size;

    bootinfo.pml4 = (uint64_t*)((uintptr_t)ptr->pml4_address + PMM_HHDM_START);
    bootinfo.page_table_count = ptr->page_table_count;

    bootinfo.gdt = (uint64_t*)((uintptr_t)ptr->gdt_address + PMM_HHDM_START);
    bootinfo.gdt_entries = ptr->gdt_entries;

    return bootinfo;
}
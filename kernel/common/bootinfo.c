/*
 * Pristine
 * bootinfo: data expected by kernel upon loading
 * SPDX-License-Identifier MIT
 */

#include "kernel/kernel.h"
#include "kernel/memory.h"
#include <stddef.h> 
#include <stdint.h>
 
#include <common/bootinfo.h>
#include <common/memmap.h>
#include <common/vesa.h>

BootInfo bootinfo_copy(const RawBootInfo *ptr) {
    BootInfo bootinfo;
    for (size_t i = 0; i < ptr->memory_map_count; i++) {
        bootinfo.memory_map[i] = ((MemmapEntry*)(uintptr_t)(ptr->memory_map_addr + MEMORY_HHDM_START))[i];
    }
    bootinfo.memory_map_count = ptr->memory_map_count;
    bootinfo.vesa_vbe_info = *(VesaVbeInfo*)(uintptr_t)(ptr->vesa_vbe_info_addr + MEMORY_HHDM_START);
    bootinfo.vesa_vbe_mode_info = *(VesaVbeModeInfo*)(uintptr_t)(ptr->vesa_vbe_mode_info_addr + MEMORY_HHDM_START);
    return bootinfo;
}
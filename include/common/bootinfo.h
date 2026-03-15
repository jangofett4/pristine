/*
 * Pristine
 * bootinfo: data expected by kernel upon loading
 * SPDX-License-Identifier MIT
 */

#pragma once

#include <stdint.h>

#include <common/memmap.h>
#include <common/vesa.h>

typedef struct {
    uint32_t memory_map_addr;
    uint16_t memory_map_count;
    uint32_t vesa_vbe_info_addr;
    uint32_t vesa_vbe_mode_info_addr;
} RawBootInfo;

typedef struct {
    MemmapEntry memory_map[MEMMAP_MAX_ITEMS];
    uint16_t memory_map_count;
    VesaVbeInfo vesa_vbe_info;
    VesaVbeModeInfo vesa_vbe_mode_info;
} BootInfo;

BootInfo bootinfo_copy(const RawBootInfo *ptr);
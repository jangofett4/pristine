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
    uint32_t memory_bitmap_address;
    uint32_t memory_bitmap_size;
    uint32_t pml4_address;
    uint32_t page_table_count;

    uint32_t gdt_address;
    uint32_t gdt_entries;
} RawBootInfo;

typedef struct {
    RawBootInfo raw;
    
    MemmapEntry memory_map[MEMMAP_MAX_ITEMS];
    uint16_t memory_map_count;

    // memory bitmap resides on non-kernel allocated memory
    uint8_t* memory_bitmap;
    uint32_t memory_bitmap_size;

    // initial PML4 resides on non-kernel allocated memory
    uint64_t *pml4;

    // initial page table count, assumed to be continuous
    uint32_t  page_table_count;

    // initial GDT resides on non-kernel allocated memory
    uint64_t *gdt;
    uint32_t gdt_entries;

    VesaVbeInfo vesa_vbe_info;
    VesaVbeModeInfo vesa_vbe_mode_info;
} BootInfo;

BootInfo bootinfo_copy(const RawBootInfo *ptr);
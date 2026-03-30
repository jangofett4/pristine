/*
 * Pristine
 * SPDX-License-Identifier MIT
 */

#pragma once

#include <common/bootinfo.h>
#include <common/bsfs/bsfs.h>
#include <common/bsfs/bsfs_ops.h>
#include <common/disk.h>
#include <common/gdt.h>
#include <common/serial.h>
#include <drivers/video/video.h>
#include <stdint.h>

#define KERNEL_BASE_PHYS      (0x400000)
#define KERNEL_END_PHYS       (KERNEL_BASE_PHYS + 0x200000)

#define KERNEL_BASE_VIRT      (0xFFFFFFFF80000000ULL + KERNEL_BASE_PHYS)

extern uint8_t __kernel_ist1_start[];
extern uint8_t __kernel_rsp0_start[];
extern uint8_t __kernel_stack_start[];

typedef struct {
    BootInfo       bootinfo;
    
    uint64_t      *gdt;
    TSSEntry      *tss;
    uint8_t       *memory_bitmap;

    Serial        serial;
    Video         video;
    DiskOpsVtable disk_ops;
    BsfsHeader    bsfs_header;
    BsfsContext   bsfs_context;

    uint64_t      system_memory;
} KernelState;
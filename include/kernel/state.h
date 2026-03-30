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
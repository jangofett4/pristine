/*
 * Pristine
 * SPDX-License-Identifier MIT
 */

#pragma once

#include <kernel/process.h>
#include <common/bootinfo.h>
#include <common/bsfs/bsfs.h>
#include <common/bsfs/bsfs_ops.h>
#include <common/disk.h>
#include <common/gdt.h>
#include <common/serial.h>
#include <drivers/video/video.h>

typedef struct {
    BootInfo      bootinfo;

    Serial        serial;
    Video         video;
    DiskOpsVtable disk_ops;
    BsfsHeader    bsfs_header;
    BsfsContext   bsfs_context;

    uint8_t       *memory_bitmap;
    uint64_t      system_memory;
} GlobalState;

typedef struct {
    uint64_t       kernel_stack_top;
    uint64_t       user_stack_rsp;
    Process        *current_process;

    uint64_t       *gdt;
    TSSEntry       *tss;
} CpuState;

_Static_assert(offsetof(CpuState, kernel_stack_top) == 0x00, "kernel_stack_top wrong offset");
_Static_assert(offsetof(CpuState, user_stack_rsp)   == 0x08, "user_stack_rsp wrong offset");
_Static_assert(offsetof(CpuState, current_process)  == 0x10, "current_process wrong offset");
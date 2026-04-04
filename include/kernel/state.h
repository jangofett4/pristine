/*
 * Pristine
 * SPDX-License-Identifier MIT
 */

#pragma once

#include <common/idt64.h>
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
    IDT64Entry    idt_table[IDT64_VECTOR_COUNT];
} GlobalState;

typedef struct {
    uintptr_t      self;
    uint64_t       kernel_stack_top;
    uint64_t       user_stack_rsp;
    Process        *current_process;

    uint64_t       *gdt;
    TSSEntry       *tss;
} CpuState;

_Static_assert(offsetof(CpuState, self)             == 0x00, "CpuState.self at wrong offset");
_Static_assert(offsetof(CpuState, kernel_stack_top) == 0x08, "CpuState.kernel_stack_top at wrong offset");
_Static_assert(offsetof(CpuState, user_stack_rsp)   == 0x10, "CpuState.user_stack_rsp at wrong offset");
_Static_assert(offsetof(CpuState, current_process)  == 0x18, "CpuState.current_process at wrong offset");

static inline CpuState *get_cpu_state() {
    CpuState *state;
    __asm__ volatile(
        "mov %%gs:0, %0"
        : "=r"(state) 
    );
    return state;
}
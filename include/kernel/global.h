/*
 * Pristine
 * SPDX-License-Identifier MIT
 */

#pragma once

#include <kernel/kernel.h>
#include <kernel/pmm.h>
#include <common/bootinfo.h>
#include <common/bsfs/bsfs_ops.h>
#include <common/disk.h>
#include <common/serial.h>
#include <drivers/storage/ata/atapio.h>
#include <drivers/video/video.h>
#include <common/idt64.h>
#include <common/gdt.h>

#include <stdint.h>

static BootInfo           __global_bootinfo;
static uint64_t          *__global_gdt;
static TSSEntry          *__global_tss;
static uint8_t           *__global_memory_bitmap;
static IDT64Entry         __global_idt[IDT64_SIZE];
static IDT64ISRHandler    __global_isr_table[IDT64_SIZE];

static Serial        __global_serial;
static Video         __global_video;
static DiskOpsVtable __global_diskops;
static BsfsHeader    __global_bsfsheader;
static BsfsContext   __global_bsfscontext;

static uint8_t __global_tmp_diskbuf[ATA_PIO_SECTOR_SIZE];
static uint8_t __global_disk_scratchbuf[ATA_PIO_SECTOR_SIZE * DISK_READ_MAX_BLOCKS];
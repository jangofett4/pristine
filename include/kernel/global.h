/*
 * Pristine
 * SPDX-License-Identifier MIT
 */

#pragma once

#include <kernel/kernel.h>
#include <kernel/memory.h>
#include <common/bsfs/bsfs_ops.h>
#include <common/disk.h>
#include <common/serial.h>
#include <drivers/storage/ata/atapio.h>
#include <drivers/video/video.h>
#include <common/idt64.h>
#include <common/gdt.h>

#include <stdint.h>

static uint64_t volatile *__global_gdt           = (uint64_t*)(GDT_ADDRESS + MEMORY_HDDM_START);
static TSSEntry volatile *__global_tss           = (TSSEntry*)(TSS_ADDRESS + MEMORY_HDDM_START);
static uint8_t           *__global_memory_bitmap = (uint8_t*)(MEMORY_BITMAP_PHYS + MEMORY_HDDM_START);;
static IDT64Entry         __global_idt[IDT64_SIZE];

static Serial        __global_serial;
static Video         __global_video;
static DiskOpsVtable __global_diskops;
static BsfsContext   __global_bsfscontext;

static uint8_t __global_tmp_diskbuf[ATA_PIO_SECTOR_SIZE];
static uint8_t __global_disk_scratchbuf[ATA_PIO_SECTOR_SIZE * DISK_READ_MAX_BLOCKS];
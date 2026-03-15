/*
 * Pristine
 * kernel: kernel entry point
 * SPDX-License-Identifier: MIT
 */

#include <stdint.h>

#include <pristine.h>
#include <kernel/kernel.h>
#include <kernel/panic.h>
#include <common/pic.h>
#include <common/disk.h>
#include <common/idt64.h>
#include <common/arena.h>
#include <common/serial.h>
#include <common/string.h>
#include <common/bootinfo.h>

#include <common/bsfs/bsfs.h>
#include <common/bsfs/bsfs_ops.h>
#include <common/bsfs/bsfs_defaults.h>

#include <drivers/storage/ata/atapio.h>
#include <drivers/video/video.h>

#include <lib64/printf/printf.h>
#include <stdio.h>

static IDT64Entry __global_idt[IDT64_SIZE];

static Serial __global_serial;
static Video __global_video;
static DiskOpsVtable __global_diskops;
static BsfsContext __global_bsfscontext;

static uint8_t __global_tmp_diskbuf[ATA_PIO_SECTOR_SIZE];
static uint8_t __global_disk_scratchbuf[ATA_PIO_SECTOR_SIZE * DISK_READ_MAX_BLOCKS];

void overflow(volatile uint8_t t[1024]) {
    volatile uint8_t tmp[1024];
    overflow(tmp);
}

void kmain(uint32_t bootinfo_addr) {
    RawBootInfo *rawbootinfo = (RawBootInfo*)(uintptr_t)bootinfo_addr;
    BootInfo bootinfo = bootinfo_copy(rawbootinfo);
    rawbootinfo = 0;

    idt64_disable_interrupts();

    IDT64ISRHandler isr_table[] = {
        IDT64_ISR_T(0),  IDT64_ISR_T(1),  IDT64_ISR_I(2),  IDT64_ISR_T(3),  IDT64_ISR_T(4),  IDT64_ISR_T(5),
        IDT64_ISR_T(6),  IDT64_ISR_T(7),  IDT64_ISR_T(8),  IDT64_ISR_T(9),  IDT64_ISR_T(10), IDT64_ISR_T(11),
        IDT64_ISR_T(12), IDT64_ISR_T(13), IDT64_ISR_T(14), IDT64_ISR_I(15), IDT64_ISR_T(16), IDT64_ISR_T(17),
        IDT64_ISR_T(18), IDT64_ISR_T(19), IDT64_ISR_T(20), IDT64_ISR_T(21), IDT64_ISR_I(22), IDT64_ISR_I(23),
        IDT64_ISR_I(24), IDT64_ISR_I(25), IDT64_ISR_I(26), IDT64_ISR_I(27), IDT64_ISR_I(28), IDT64_ISR_I(29),
        IDT64_ISR_I(30), IDT64_ISR_I(31), 
        {.type=IDT64_ISR_INTERRUPT, .handler = pic_isr_32 },
        {.type=IDT64_ISR_INTERRUPT, .handler = pic_isr_33 },
        {.type=IDT64_ISR_INTERRUPT, .handler = pic_isr_34 },
        {.type=IDT64_ISR_INTERRUPT, .handler = pic_isr_35 },
        {.type=IDT64_ISR_INTERRUPT, .handler = pic_isr_36 },
        {.type=IDT64_ISR_INTERRUPT, .handler = pic_isr_37 },
        {.type=IDT64_ISR_INTERRUPT, .handler = pic_isr_38 },
        {.type=IDT64_ISR_INTERRUPT, .handler = pic_isr_39 },
        {.type=IDT64_ISR_INTERRUPT, .handler = pic_isr_40 },
        {.type=IDT64_ISR_INTERRUPT, .handler = pic_isr_41 },
        {.type=IDT64_ISR_INTERRUPT, .handler = pic_isr_42 },
        {.type=IDT64_ISR_INTERRUPT, .handler = pic_isr_43 },
        {.type=IDT64_ISR_INTERRUPT, .handler = pic_isr_44 },
        {.type=IDT64_ISR_INTERRUPT, .handler = pic_isr_45 },
        {.type=IDT64_ISR_INTERRUPT, .handler = pic_isr_46 },
        {.type=IDT64_ISR_INTERRUPT, .handler = pic_isr_47 },
    };

    idt64_set_entries(__global_idt, isr_table, IDT64_SIZE);
    
    IDT64Ptr idt_ptr;
    idt_ptr.limit = (IDT64_SIZE * sizeof(IDT64Entry)) - 1;
    idt_ptr.base = (uint64_t)(uintptr_t)&__global_idt;

    idt64_load_idtr(&idt_ptr);
    pic_init();

    idt64_enable_interrupts();

    pic_unmask_irq(1);

    serial_init(&__global_serial, 0x3F8);
    serial_set_default(&__global_serial);

    video_init(&__global_video, &bootinfo.vesa_vbe_mode_info);
    video_set_default(&__global_video);

    printf("Pristine\n");
    printf("Kernel Version %s\n", PRISTINE_VERSION_STR);

    printf("KERNEL_BASE_ADDRESS   : %08x\n", KERNEL_BASE_ADDRESS);
    printf("KERNEL_STACK_START    : %08x\n", KERNEL_STACK_START);
    printf("KERNEL_STACK_END      : %08x\n", KERNEL_STACK_END);
    printf("KERNEL_STACK_GUARD    : %08x\n", KERNEL_STACK_GUARD);
    printf("KERNEL_TSS_RSP0_START : %08x\n", KERNEL_TSS_RSP0_START);
    printf("KERNEL_TSS_RSP0_END   : %08x\n", KERNEL_TSS_RSP0_END);
    printf("KERNEL_TSS_RSP0_GUARD : %08x\n", KERNEL_TSS_RSP0_GUARD);
    printf("KERNEL_TSS_IST1_START : %08x\n", KERNEL_TSS_IST1_START);
    printf("KERNEL_TSS_IST1_END   : %08x\n", KERNEL_TSS_IST1_END);
    printf("KERNEL_TSS_IST1_GUARD : %08x\n", KERNEL_TSS_IST1_GUARD);

    // uint8_t *ptr = (uint8_t*)__global_video.address;
    // for (size_t y = 50; y < 150; y++) {
    //     for (size_t x = 50; x < 150; x++) {
    //         uint32_t *pixel = (uint32_t*)(ptr + y * __global_video.bytes_per_scanline + x * 4);
    //         *pixel = 0x00FF0000;
    //     }
    // }

    __global_diskops = ata_pio_get_disk_ops();

    const uint32_t bsfs_offset = PRISTINE_BSFS_OFFSET * 4096;

    if (!__global_diskops.read(bsfs_offset / ATA_PIO_SECTOR_SIZE, 1, __global_tmp_diskbuf)) {
        KPANIC("unable to read disk");
    }

    BsfsHeader bsfs_header;
    memcpy(&bsfs_header, __global_tmp_diskbuf, sizeof(BsfsHeader));

    if (bsfs_header.block_size != PRISTINE_BSFS_BLOCKSIZE) {
        KPANIC("invalid filesystem block size, expected %i, got %u", PRISTINE_BSFS_BLOCKSIZE, bsfs_header.block_size);
    }

    __global_bsfscontext = (BsfsContext){
        .disk_ops = &__global_diskops,
        .header = &bsfs_header,
        .phys_sector_size = ATA_PIO_SECTOR_SIZE,
        .scratch_buf = __global_disk_scratchbuf,
        .scratch_buf_size = ATA_PIO_SECTOR_SIZE * DISK_READ_MAX_BLOCKS
    };

    arena_reset();
    BsfsFile logofile = {
        .buf = arena_alloc(bsfs_header.block_size, 1),
        .bufsize = bsfs_header.block_size,
    };
    if (logofile.inode->size >= (2 * sizeof(uint32_t) + 3 * sizeof(uint8_t))) { // literally the minimum size it can go, 1x1 image
        if (bsfs_fopen(&__global_bsfscontext, "/boot/logo.bin", &logofile) < 0) {
            KPANIC();
        }
        uint32_t w = 0, h = 0;
        bsfs_fread(&__global_bsfscontext, &w, sizeof(uint32_t), 1, &logofile);
        bsfs_fread(&__global_bsfscontext, &h, sizeof(uint32_t), 1, &logofile);

        printf("Logo: Width: %u, Height: %u\n");
        printf("Position: %lu\n", logofile.position);
    }
    arena_reset();

    while(1);
}
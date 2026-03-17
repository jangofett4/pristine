/*
 * Pristine
 * kernel: kernel entry point
 * SPDX-License-Identifier: MIT
 */

#include <stdint.h>

#include <pristine.h>
#include <kernel/kernel.h>
#include <kernel/global.h>
#include <kernel/memory.h>
#include <kernel/panic.h>
#include <common/pic.h>
#include <common/gdt.h>
#include <common/disk.h>
#include <common/idt64.h>
#include <common/arena.h>
#include <common/serial.h>
#include <common/string.h>
#include <common/paging.h>
#include <common/bootinfo.h>

#include <common/bsfs/bsfs.h>
#include <common/bsfs/bsfs_ops.h>
#include <common/bsfs/bsfs_defaults.h>

#include <drivers/storage/ata/atapio.h>
#include <drivers/video/video.h>

#include <lib64/printf/printf.h>
#include <stdio.h>

void kmain(uint64_t bootinfo_addr) {
    RawBootInfo *rawbootinfo = (RawBootInfo*)(bootinfo_addr + MEMORY_HDDM_START);
    BootInfo bootinfo = bootinfo_copy(rawbootinfo);
    rawbootinfo = 0;

    __pg_pml4[0] = 0;
    page_invlpg((void*)__pg_pml4);

    // following statements only work because we are at 2 MiB mark
    // honestly one of the ugliest assumptions I did in this project
    // this won't work if kernel is anywhere else beside the 2 MiB
    // __pg_pt_ident0[KERNEL_STACK_GUARD / 4096] = PAGING_PT_GUARD_FLAGS;
    // __pg_pt_ident0[KERNEL_TSS_RSP0_GUARD / 4096] = PAGING_PT_GUARD_FLAGS;
    // __pg_pt_ident0[KERNEL_TSS_IST1_GUARD / 4096] = PAGING_PT_GUARD_FLAGS;
    // page_invlpg((void*)KERNEL_STACK_GUARD);
    // page_invlpg((void*)KERNEL_TSS_RSP0_GUARD);
    // page_invlpg((void*)KERNEL_TSS_IST1_GUARD);

    __global_tss[0].rsp0 = KERNEL_TSS_RSP0_START;
    __global_tss[0].ist1 = KERNEL_TSS_IST1_START;

    gdt_set_tss_entry(__global_gdt, 3, __global_tss, 0x89, 0, sizeof(TSSEntry) - 1);
    gdt_load_gdtr(__global_gdt,  5);
    gdt_load_tss(0x18);

    idt64_disable_interrupts();

    IDT64ISRHandler isr_table[] = {
        IDT64_ISR_T(0, 1),  IDT64_ISR_T(1, 1),  IDT64_ISR_I(2, 0),  IDT64_ISR_T(3, 1),  IDT64_ISR_T(4, 1),  IDT64_ISR_T(5, 1),
        IDT64_ISR_T(6, 1),  IDT64_ISR_T(7, 1),  IDT64_ISR_T(8, 1),  IDT64_ISR_T(9, 1),  IDT64_ISR_T(10, 1), IDT64_ISR_T(11, 1),
        IDT64_ISR_T(12, 1), IDT64_ISR_T(13, 1), IDT64_ISR_T(14, 1), IDT64_ISR_I(15, 0), IDT64_ISR_T(16, 1), IDT64_ISR_T(17, 1),
        IDT64_ISR_T(18, 1), IDT64_ISR_T(19, 1), IDT64_ISR_T(20, 1), IDT64_ISR_T(21, 1), IDT64_ISR_I(22, 0), IDT64_ISR_I(23, 0),
        IDT64_ISR_I(24, 0), IDT64_ISR_I(25, 0), IDT64_ISR_I(26, 0), IDT64_ISR_I(27, 0), IDT64_ISR_I(28, 0), IDT64_ISR_I(29, 0),
        IDT64_ISR_I(30, 0), IDT64_ISR_I(31, 0), 
        {.type=IDT64_ISR_INTERRUPT, .handler = pic_isr_32, .ist = 0 },
        {.type=IDT64_ISR_INTERRUPT, .handler = pic_isr_33, .ist = 0 },
        {.type=IDT64_ISR_INTERRUPT, .handler = pic_isr_34, .ist = 0 },
        {.type=IDT64_ISR_INTERRUPT, .handler = pic_isr_35, .ist = 0 },
        {.type=IDT64_ISR_INTERRUPT, .handler = pic_isr_36, .ist = 0 },
        {.type=IDT64_ISR_INTERRUPT, .handler = pic_isr_37, .ist = 0 },
        {.type=IDT64_ISR_INTERRUPT, .handler = pic_isr_38, .ist = 0 },
        {.type=IDT64_ISR_INTERRUPT, .handler = pic_isr_39, .ist = 0 },
        {.type=IDT64_ISR_INTERRUPT, .handler = pic_isr_40, .ist = 0 },
        {.type=IDT64_ISR_INTERRUPT, .handler = pic_isr_41, .ist = 0 },
        {.type=IDT64_ISR_INTERRUPT, .handler = pic_isr_42, .ist = 0 },
        {.type=IDT64_ISR_INTERRUPT, .handler = pic_isr_43, .ist = 0 },
        {.type=IDT64_ISR_INTERRUPT, .handler = pic_isr_44, .ist = 0 },
        {.type=IDT64_ISR_INTERRUPT, .handler = pic_isr_45, .ist = 0 },
        {.type=IDT64_ISR_INTERRUPT, .handler = pic_isr_46, .ist = 0 },
        {.type=IDT64_ISR_INTERRUPT, .handler = pic_isr_47, .ist = 0 },
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

    // Mark the regions we got from memory map
    if (bootinfo.memory_map_count > MEMMAP_MAX_ITEMS) {
        KPANIC("kernel: memory map exceeds maximum items of %i, possibly malformed memory", MEMMAP_MAX_ITEMS);
    }

    for (size_t i = 0; i < bootinfo.memory_map_count; i++) {
        MemmapEntry *entry = bootinfo.memory_map + i;
        uint64_t entry_base = (uint64_t)entry->BaseAddrHigh << 32 | (uint64_t)entry->BaseAddrLow;
        uint64_t entry_size = (uint64_t)entry->LengthHigh << 32 | (uint64_t)entry->LengthLow;
        printf(" 0x%016llx:0x%016llx (%llu KiB), Type: ", entry_base, entry_base + entry_size, entry_size / 1024);
        if (entry->Type == 1) {
            for (uint64_t m = entry_base; m < entry_base + entry_size; m += 4096) {
                kmem_bitmap_clear(__global_memory_bitmap, m / 4096);
            }
        } else {
            for (uint64_t m = entry_base; m < entry_base + entry_size; m += 4096) {
                kmem_bitmap_set(__global_memory_bitmap, m / 4096);
            }
        }
        switch (entry->Type) {
            case 1:
                printf("Usable");
                break;
            case 2:
                printf("Reserved");
                break;
            case 3:
                printf("ACPI");
                break;
            case 4:
                printf("NVS");
                break;
            case 5:
                printf("Unusable");
                break;
            case 6:
                printf("Disabled");
                break;
            default:
                printf("Unknown");
                break;
        }
        printf("\n");
    }
    
    // mark the regions we are currently using (4 MiB)
    for (size_t i = 0; i < 0x400000; i += 0x1000) {
        kmem_bitmap_set(__global_memory_bitmap, i / 0x1000);
    }

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

    pmm_set_bitmap(__global_memory_bitmap);
    
    // void* file_buf = pmm_alloc() + MEMORY_HDDM_START;
    // BsfsInode logoinode;
    // BsfsFile logofile = {
    //     .buf = file_buf,
    //     .bufsize = bsfs_header.block_size,
    //     .inode = &logoinode
    // };
    // if (bsfs_fopen(&__global_bsfscontext, "/boot/logo.bin", &logofile) < 0) {
    //     KPANIC();
    // }
    // if (logofile.inode->size >= (2 * sizeof(uint32_t) + 3 * sizeof(uint8_t))) { // literally the minimum size it can go, 1x1 image
    //     uint32_t w = 0, h = 0;
    //     bsfs_fread(&__global_bsfscontext, &w, sizeof(uint32_t), 1, &logofile);
    //     bsfs_fread(&__global_bsfscontext, &h, sizeof(uint32_t), 1, &logofile);
    // }
    // pmm_free(file_buf - MEMORY_HDDM_START);

    while(1);
}
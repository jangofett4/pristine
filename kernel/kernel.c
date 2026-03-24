/*
 * Pristine
 * kernel: kernel entry point
 * SPDX-License-Identifier: MIT
 */

#include "common/memmap.h"
#include <stdint.h>

#include <pristine.h>
#include <kernel/panic.h>
#include <kernel/kernel.h>
#include <kernel/global.h>
#include <kernel/pmm.h>
#include <kernel/vmm.h>
#include <common/common.h>
#include <common/pic.h>
#include <common/gdt.h>
#include <common/disk.h>
#include <common/idt64.h>
#include <common/arena.h>
#include <common/serial.h>
#include <common/string.h>
#include <common/bootinfo.h>

#include <common/bsfs/bsfs.h>
#include <common/bsfs/bsfs_ops.h>
#include <common/bsfs/bsfs_defaults.h>

#include <bitmap.h>

#include <drivers/storage/ata/atapio.h>
#include <drivers/video/video.h>

#include <lib64/printf/printf.h>

void kmain(uint64_t bootinfo_addr) {
    // verify we're running where we think we are    
    uint64_t rip;
    __asm__ volatile("lea (%%rip), %0" : "=r"(rip));
    if (rip < KERNEL_BASE_VIRT || rip > KERNEL_BASE_VIRT + 0x200000) {
        KPANIC_SILENT();
    }

    bootinfo_init(&__global_bootinfo, bootinfo_addr);

    // ======== IDT64 ========
    idt64_disable_interrupts();

    idt64_init();
    
    IDT64Ptr idt_ptr;
    idt_ptr.limit = (IDT64_SIZE * sizeof(IDT64Entry)) - 1;
    idt_ptr.base = (uint64_t)(uintptr_t)&__global_idt;

    idt64_load_idtr(&idt_ptr);

    // ======== PIC ========

    pic_init();

    idt64_enable_interrupts();

    // pic_unmask_irq(1);

    // ======== Globals ========

    serial_init(&__global_serial, 0x3F8);
    serial_set_default(&__global_serial);

    video_init(&__global_video, &__global_bootinfo.vesa_vbe_mode_info);
    video_set_default(&__global_video);

    __global_memory_bitmap = __global_bootinfo.memory_bitmap;
    pmm_init(__global_memory_bitmap, __global_bootinfo.memory_bitmap_size);
    bitmap_clear_all(__global_memory_bitmap, __global_bootinfo.memory_bitmap_size);

    printf_("Pristine\n");
    printf_("Kernel Version " PRISTINE_VERSION_STR "\n");

    printf_("KERNEL_BASE_VIRT      : %llx\n", KERNEL_BASE_VIRT);
    printf_("KERNEL_STACK_START    : %lx\n", (uint64_t)__kernel_stack_start);
    printf_("KERNEL_TSS_RSP0_START : %lx\n", (uint64_t)__kernel_rsp0_start);
    printf_("KERNEL_TSS_IST1_START : %lx\n", (uint64_t)__kernel_ist1_start);

    // ======== Memory Map ========

    if (__global_bootinfo.memory_map_count > MEMMAP_MAX_ITEMS) {
        KPANIC("kernel: memory map exceeds maximum items of %i, possibly malformed memory", MEMMAP_MAX_ITEMS);
    }

    __kernel_system_memory = memmap_bitmap_init(__global_bootinfo.memory_map, __global_bootinfo.memory_map_count, __global_memory_bitmap);

    uint64_t memory_bitmap_start_phys = ((uint64_t)__global_bootinfo.memory_bitmap) - PMM_HHDM_START;
    uint64_t memory_bitmap_end_phys   = memory_bitmap_start_phys + __global_bootinfo.memory_bitmap_size;

    uint64_t memory_bitmap_start_phys_aligned = ALIGN_UP(memory_bitmap_start_phys, VMM_DEFAULT_PAGE_SIZE);
    uint64_t memory_bitmap_end_phys_aligned   = ALIGN_DOWN(memory_bitmap_end_phys, VMM_DEFAULT_PAGE_SIZE);

    printf_("Memory Bitmap Start: %lu, End: %lu (Size: %lu, Covers %lu MiB)\n", 
        memory_bitmap_start_phys_aligned, 
        memory_bitmap_end_phys_aligned, 
        memory_bitmap_end_phys - memory_bitmap_start_phys, 
        (memory_bitmap_end_phys - memory_bitmap_start_phys) * 8 * VMM_DEFAULT_PAGE_SIZE / 1024 / 1024
    );

    // mark memory bitmap as used
    for (size_t i = memory_bitmap_start_phys_aligned; i < memory_bitmap_end_phys_aligned; i += VMM_DEFAULT_PAGE_SIZE) {
        bitmap_set(__global_memory_bitmap, i / VMM_DEFAULT_PAGE_SIZE);
    }

    // mark kernel memory area as used
    for (size_t i = KERNEL_BASE_PHYS; i < KERNEL_END_PHYS; i += VMM_DEFAULT_PAGE_SIZE) {
        bitmap_set(__global_memory_bitmap, i / VMM_DEFAULT_PAGE_SIZE);
    }

    // mark dangerous regions as used, no need to allocate 1 MiB of space here, but just being careful here
    for (size_t i = 0; i < 0x100000; i += VMM_DEFAULT_PAGE_SIZE) {
        bitmap_set(__global_memory_bitmap, i / VMM_DEFAULT_PAGE_SIZE);
    }

    printf_("Total Usable Memory: %zu MiB\n", __kernel_system_memory / 1024 / 1024);

    // ======== GDT, TSS ========

    __global_gdt = phys_to_virt(pmm_alloc());
    __global_tss = phys_to_virt(pmm_alloc());

    printf_("GDT %p\n", __global_gdt);
    printf_("TSS %p\n", __global_gdt);

    memcpy(__global_gdt, __global_bootinfo.gdt, __global_bootinfo.gdt_entries * sizeof(uint64_t));
    
    __global_tss[0].rsp0 = (uint64_t)__kernel_rsp0_start;
    __global_tss[0].ist1 = (uint64_t)__kernel_ist1_start;

    printf_("TSS.RSP0 %p\n", __kernel_rsp0_start);
    printf_("TSS.IST1 %p\n", __kernel_ist1_start);

    gdt_set_tss_entry(__global_gdt, 3, (void*)__global_tss, 0x89, 0, sizeof(TSSEntry) - 1);
    gdt_load_gdtr(__global_gdt, 5);
    gdt_reload_cs(0x08);
    gdt_reload_segments(0x10);
    gdt_load_tss(0x18);

    // ======== Paging ========

    for (uint64_t i = __global_bootinfo.raw.pml4_address; i < (__global_bootinfo.raw.pml4_address + __global_bootinfo.raw.page_table_count * VMM_DEFAULT_PAGE_SIZE); i += VMM_DEFAULT_PAGE_SIZE) {
        bitmap_set(__global_memory_bitmap, i / VMM_DEFAULT_PAGE_SIZE);
    }

    vmm_init();

    // HHDM
    for (size_t i = 0; i < 512; i++) {
        vmm_map_huge(
            __vmm_pml4, 
            i * VMM_HUGE_PAGE_SIZE, 
            PMM_HHDM_START + (i * VMM_HUGE_PAGE_SIZE), 
            VMM_FLAGS_KERNEL_CODE
        );
    }

    // Kernel
    for (uint64_t off = 0; off < 0x200000; off += VMM_DEFAULT_PAGE_SIZE) {
        vmm_map(
            __vmm_pml4,
            KERNEL_BASE_PHYS + off,
            KERNEL_BASE_VIRT + off,
            VMM_FLAGS_KERNEL_CODE
        );
    }

    // GDT and TSS
    // vmm_map(__vmm_pml4, virt_to_phys(__global_gdt), (uint64_t)__global_gdt, VMM_FLAGS_KERNEL_DATA);
    // vmm_map(__vmm_pml4, virt_to_phys(__global_tss), (uint64_t)__global_tss, VMM_FLAGS_KERNEL_DATA);

    // Stack, RSP0, IST1 
    // vmm_map(__vmm_pml4, virt_to_phys(__kernel_stack_start), (uint64_t)__kernel_stack_start, VMM_FLAGS_KERNEL_DATA);
    // vmm_map(__vmm_pml4, virt_to_phys(__kernel_rsp0_start), (uint64_t)__kernel_rsp0_start, VMM_FLAGS_KERNEL_DATA);
    // vmm_map(__vmm_pml4, virt_to_phys(__kernel_ist1_start), (uint64_t)__kernel_ist1_start, VMM_FLAGS_KERNEL_DATA);

    idt64_disable_interrupts();
    vmm_switch(__vmm_pml4);
    idt64_enable_interrupts();

    // ======== BSFS ========

    __global_diskops = ata_pio_get_disk_ops();

    const uint32_t bsfs_offset = PRISTINE_BSFS_OFFSET * 4096;

    if (!__global_diskops.read(bsfs_offset / ATA_PIO_SECTOR_SIZE, 1, __global_tmp_diskbuf)) {
        KPANIC("unable to read disk");
    }

    memcpy(&__global_bsfsheader, __global_tmp_diskbuf, sizeof(BsfsHeader));

    if (__global_bsfsheader.block_size != PRISTINE_BSFS_BLOCKSIZE) {
        KPANIC("invalid filesystem block size, expected %i, got %u", PRISTINE_BSFS_BLOCKSIZE, __global_bsfsheader.block_size);
    }

    __global_bsfscontext = (BsfsContext){
        .disk_ops = &__global_diskops,
        .header = &__global_bsfsheader,
        .phys_sector_size = ATA_PIO_SECTOR_SIZE,
        .scratch_buf = __global_disk_scratchbuf,
        .scratch_buf_size = ATA_PIO_SECTOR_SIZE * DISK_READ_MAX_BLOCKS
    };

    printf_("BSFS Version %x\n", __global_bsfsheader.version);

    while(1);
}
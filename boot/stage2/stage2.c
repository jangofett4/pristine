/*
 * Pristine
 * stage2 - stage 2 bootloader entry point
 * SPDX-License-Identifier: MIT
 */

#include <stdint.h>

#include "include/stage2_common.h"

#include <common/bootinfo.h>
#include <common/io.h>
#include <common/gdt.h>
#include <common/pic.h>
#include <common/elf.h>
#include <common/disk.h>
#include <common/idt32.h>
#include <common/paging.h>
#include <common/string.h>
#include <common/serial.h>
#include <common/memmap.h>
#include <common/arena.h>
#include <common/vesa.h>
#include <common/bsfs/bsfs.h>
#include <common/bsfs/bsfs_ops.h>
#include <common/bsfs/bsfs_defaults.h>
#include <drivers/storage/ata/atapio.h>

#include <lib32/printf/printf.h>

#define KERNEL_PHYS_ADDR 0x200000

// information to be passed to kernel
static RawBootInfo bootinfo = {0};
static VesaVbeInfo vesa_vbe_info;
static VesaVbeModeInfo vesa_vbe_mode_info;

static IDT32Entry idt[IDT32_SIZE];

static uint8_t disk_buf[DISK_READ_MAX_BLOCKS * ATA_PIO_SECTOR_SIZE];

static Serial serial;

uint64_t volatile *__gdt = (uint64_t*)GDT_ADDRESS;

#define PIC_ISR_I(i, w) {.type=IDT##w_ISR_INTERRUPT,.handler=idt##w_isr_##i}
#define PIC_ISR_T(i, w) {.type=IDT##w_ISR_TRAP,.handler=idt##w_isr_##i}

extern void kernel_entry(uint32_t pml4_address, uint64_t kernel_entry_address, RawBootInfo *bootinfo);

__attribute__((section(".text.stage2"))) void stage2_boot(void) {
    serial_init(&serial, 0x3F8);
    serial_set_default(&serial);

    printf("Stage 2 bootloader started\n");

    printf("Getting memory map...\n");
    uint16_t memmap_count = *(uint16_t*)MEMMAP_COUNT_ADDR;
    printf("Number of memory map entries: %u\n", memmap_count);
    
    printf("Setting up interrupts...\n");

    IDT32ISRHandler isr_table[] = {
        IDT32_ISR_T(0),  IDT32_ISR_T(1),  IDT32_ISR_I(2),  IDT32_ISR_T(3),  IDT32_ISR_T(4),  IDT32_ISR_T(5),
        IDT32_ISR_T(6),  IDT32_ISR_T(7),  IDT32_ISR_T(8),  IDT32_ISR_T(9),  IDT32_ISR_T(10), IDT32_ISR_T(11),
        IDT32_ISR_T(12), IDT32_ISR_T(13), IDT32_ISR_T(14), IDT32_ISR_I(15), IDT32_ISR_T(16), IDT32_ISR_T(17),
        IDT32_ISR_T(18), IDT32_ISR_T(19), IDT32_ISR_T(20), IDT32_ISR_T(21), IDT32_ISR_I(22), IDT32_ISR_I(23),
        IDT32_ISR_I(24), IDT32_ISR_I(25), IDT32_ISR_I(26), IDT32_ISR_I(27), IDT32_ISR_I(28), IDT32_ISR_I(29),
        IDT32_ISR_I(30), IDT32_ISR_I(31), 
        {.type=IDT32_ISR_INTERRUPT, .handler = pic_isr_32 },
        {.type=IDT32_ISR_INTERRUPT, .handler = pic_isr_33 },
        {.type=IDT32_ISR_INTERRUPT, .handler = pic_isr_34 },
        {.type=IDT32_ISR_INTERRUPT, .handler = pic_isr_35 },
        {.type=IDT32_ISR_INTERRUPT, .handler = pic_isr_36 },
        {.type=IDT32_ISR_INTERRUPT, .handler = pic_isr_37 },
        {.type=IDT32_ISR_INTERRUPT, .handler = pic_isr_38 },
        {.type=IDT32_ISR_INTERRUPT, .handler = pic_isr_39 },
        {.type=IDT32_ISR_INTERRUPT, .handler = pic_isr_40 },
        {.type=IDT32_ISR_INTERRUPT, .handler = pic_isr_41 },
        {.type=IDT32_ISR_INTERRUPT, .handler = pic_isr_42 },
        {.type=IDT32_ISR_INTERRUPT, .handler = pic_isr_43 },
        {.type=IDT32_ISR_INTERRUPT, .handler = pic_isr_44 },
        {.type=IDT32_ISR_INTERRUPT, .handler = pic_isr_45 },
        {.type=IDT32_ISR_INTERRUPT, .handler = pic_isr_46 },
        {.type=IDT32_ISR_INTERRUPT, .handler = pic_isr_47 },
    };

    idt32_set_entries(idt, isr_table, 48);

    IDT32Ptr idt_ptr;
    idt_ptr.limit = (IDT32_SIZE * sizeof(IDT32Entry)) - 1;
    idt_ptr.base = (uint32_t)(uintptr_t)&idt;

    idt32_load_idtr(&idt_ptr);
    pic_init();

    idt32_enable_interrupts();

    AtaPioStatus atapio_status;

    ata_pio_set_disk(ATA_PIO_DISK_MASTER);
    atapio_status = ata_pio_get_status();
    if (atapio_status.raw == 0xFF) {
      PANIC("No drive detected!\n");
    } else {
      printf("Master disk is present\n");
    }
    ata_pio_zero();
    atapio_status = ata_pio_identify();
    if (atapio_status.raw == 0 || atapio_status.flags.err) {
      PANIC("Disk identify error!\n");
    }

    char ata_serial_number_buf[21];
    ata_pio_get_serial_number(ata_serial_number_buf);
    printf("Drive serial Number: %s\n", ata_serial_number_buf);

    char ata_model_number_buf[41];
    ata_pio_get_model_number(ata_model_number_buf);
    printf("Drive model Number: %s\n", ata_model_number_buf);

    uint32_t ata_max_sectors = ata_pio_get_max_sectors();
    printf("Drive size: %i MiB\n", ata_max_sectors * 512 / 1024 / 1024);

    while (!ata_pio_drive_ready());

    DiskOpsVtable disk_ops = ata_pio_get_disk_ops();

    const uint32_t bsfs_offset = PRISTINE_BSFS_OFFSET * PRISTINE_BSFS_BLOCKSIZE;

    disk_ops.read(bsfs_offset / ATA_PIO_SECTOR_SIZE, 1, disk_buf);
    BsfsHeader header;
    memcpy(&header, disk_buf, sizeof(BsfsHeader));

    printf("BSFS:\n");
    printf(" Label:             %s\n", header.label);
    printf(" Version:           %i.%i.%i.%i\n", 
        (header.version >> 24) & 0xff,
        (header.version >> 16) & 0xff,
        (header.version >> 8) & 0xff,
        (header.version & 0xff)
    );
    printf(" Block Size:        %u\n", header.block_size);
    printf(" Total Blocks:      %u\n", header.total_blocks);
    printf(" Total Inodes:      %u\n", header.inode_count);
    printf(" Free Blocks:       %u\n", header.free_blocks);
    printf(" Root Inode:        0x%04x\n", header.root_inode);
    printf(" Inode Table Start: 0x%04x\n", header.inode_table_start);

    if (header.block_size != PRISTINE_BSFS_BLOCKSIZE) {
        PANIC("bsfs: image has %u byte blocks, cannot process", header.block_size);
    }

    // arena_reset();
    // BsfsFile font_file = {
    //     .buf = arena_alloc(header.block_size, 1), 
    //     .bufsize = header.block_size
    // };
    // if (bsfs_fopen(&bsfs_context, "/fonts/tamzen/Tamzen8x15.psf", &font_file) < 0) {
    //     // TODO: instead of straight up panic, we can iterate over what we have,
    //     // for now this will have to do
    //     PANIC("stage2: /fonts/tamzen/Tamzen8x16.psf not found!");
    // }
    // Psf1Header psf1header;
    // if ((fread_count = bsfs_fread(
    //     &bsfs_context, 
    //     &psf1header, 
    //     sizeof(Psf1Header), 
    //     1,
    //     &font_file)) < 0
    // ) {
    //     PANIC("stage2: bsfs_fread returned %i", fread_count);
    // }

    // if (psf1header.magic[0] != 0x36 && psf1header.magic[1] != 0x04) {
    //     PANIC("stage2: font file is not PSF1");
    // }

    // video_psf_set_font(psf1header);
    // for (size_t i = 0; i < 256; i++) {
    //     uint8_t *glyph = arena_alloc(psf1header.size, 1);
    //     if ((fread_count = bsfs_fread(&bsfs_context, glyph, 1, psf1header.size, &font_file)) < 0) {
    //         PANIC("stage2: malformed PSF1 font file");
    //     }
    //     int setglyph_ret = 0;
    //     if ((setglyph_ret = video_psf_set_glyph(i, glyph, psf1header.size)) < 0) {
    //         if (setglyph_ret == PSF_SET_GLYPH_SIZE_TOO_BIG) {
    //             PANIC("psf_set_glyph: PSF1 glyph size too big");
    //         } else {
    //             PANIC("psf_set_glyph: unknown error");
    //         }
    //     }
    // }
    // bsfs_fclose(&font_file);
    // arena_reset();

    vesa_vbe_info = *(VesaVbeInfo*)VESA_INFO_ADDR;
    vesa_vbe_mode_info = *(VesaVbeModeInfo*)VESA_MODE_INFO_ADDR;

    printf("Loading kernel...\n");

    BsfsContext bsfs_context = {
        .header = &header, 
        .disk_ops = &disk_ops,
        .phys_sector_size = ATA_PIO_SECTOR_SIZE
    };
    int fread_count = -1;

    arena_reset();
    BsfsFile kernel_file = {
        .buf = (uint8_t*)arena_alloc(header.block_size, 1),
        .bufsize = header.block_size
    };
    if (bsfs_fopen(&bsfs_context, "/kernel.elf", &kernel_file) < 0) {
        PANIC("stage2: kernel.elf not found!");
    }

    Elf64Ehdr kernel_elf_hdr;
    if ((fread_count = bsfs_fread(&bsfs_context, &kernel_elf_hdr, sizeof(Elf64Ehdr), 1, &kernel_file)) < sizeof(Elf64Ehdr)) {
        PANIC("stage2: unable to read kernel.elf, malformed file or filesystem");
    }

    if (
        kernel_elf_hdr.e_ident[0] != 0x7F ||
        kernel_elf_hdr.e_ident[1] != 0x45 ||
        kernel_elf_hdr.e_ident[2] != 0x4C ||
        kernel_elf_hdr.e_ident[3] != 0x46
    ) {
        PANIC("stage2: unable to load kernel.elf, malformed ELF header");
    }

    if (kernel_elf_hdr.e_ident[4] != 0x02) {
        PANIC("stage2: kernel.elf is not ELF64 compiled");
    }

    if (kernel_elf_hdr.e_ident[5] != 0x01) {
        PANIC("stage2: kernel.elf is not little endian");
    }

    if (kernel_elf_hdr.e_type != 0x02) {
        PANIC("stage2: kernel.elf not executable");
    }

    if (bsfs_fseeko(&bsfs_context, &kernel_file, kernel_elf_hdr.e_phoff, BSFS_FSEEKO_SET) < 0) {
        PANIC("stage2: unable to load kernel.elf, malformed ELF binary");
    }

    printf(" e_phentsize: %u\n", kernel_elf_hdr.e_phentsize);
    printf(" Elf64Phdr:   %u\n", sizeof(Elf64Phdr));

    if (kernel_elf_hdr.e_phentsize != sizeof(Elf64Phdr)) {
        PANIC("stage2: e_phentsize != sizeof(Elf64Phdr)");
    }

    for (size_t ph = 0; ph < kernel_elf_hdr.e_phnum; ph++) {
        Elf64Phdr phdr;
        uint64_t ph_offset = kernel_elf_hdr.e_phoff + (ph * kernel_elf_hdr.e_phentsize);

        if (bsfs_fseeko(&bsfs_context, &kernel_file, ph_offset, BSFS_FSEEKO_SET) < 0) {
            PANIC("stage2: unable to seek to program header at index %u", ph);
        }

        if ((fread_count = bsfs_fread(&bsfs_context, &phdr, kernel_elf_hdr.e_phentsize, 1, &kernel_file)) < 0) {
            PANIC("stage2: unable to read program header at index %u", ph);
        }
        if (phdr.p_filesz > phdr.p_memsz) {
            PANIC("stage2: unable to load kernel.elf, p_filesz > p_memsz (%llu, %llu)", phdr.p_filesz, phdr.p_memsz);
        }

        if (phdr.p_type == PT_LOAD) {
            if (bsfs_fseeko(&bsfs_context, &kernel_file, phdr.p_offset, BSFS_FSEEKO_SET) < 0) {
                PANIC("stage2: unable to load kernel.elf, malformed ELF binary");
            }
            printf("Program header address: 0x%lx\n", phdr.p_paddr);
            printf("Size: %lu\n", phdr.p_filesz);
            uint8_t *phaddr = (uint8_t*)phdr.p_paddr;
            int64_t remaining = phdr.p_filesz;
            while (remaining > 0) {
                if (remaining < header.block_size) {
                    if ((fread_count = bsfs_fread(&bsfs_context, phaddr, 1, remaining, &kernel_file)) < 0) {
                        PANIC("stage2: unable to load kernel.elf, malformed ELF binary");
                    }
                } else {
                    if ((fread_count = bsfs_fread(&bsfs_context, phaddr, 1, header.block_size, &kernel_file)) < 0) {
                        PANIC("stage2: unable to load kernel.elf, malformed ELF binary");
                    }
                }
                remaining -= fread_count;
                phaddr += fread_count;
                printf("Read %i bytes, remaining: %lu\n", fread_count, remaining);
            }
            memset(phaddr, 0, phdr.p_memsz - phdr.p_filesz);
        }
    }

    printf("Kernel Entry: 0x%016x\n", kernel_elf_hdr.e_entry);

    bsfs_fclose(&kernel_file);
    arena_reset();
    
    bootinfo.memory_map_addr = (uint32_t)(uintptr_t)MEMMAP_ADDR;
    bootinfo.memory_map_count = memmap_count;
    bootinfo.vesa_vbe_info_addr = (uint32_t)(uintptr_t)&vesa_vbe_info;
    bootinfo.vesa_vbe_mode_info_addr = (uint32_t)(uintptr_t)&vesa_vbe_mode_info;

    printf("Setting up higher half direct mapping...\n");
    memset((void*)__pg_pml4, 0, PAGE_DEFAULT_SIZE);
    memset((void*)__pg_pdpt_kernel, 0, PAGE_DEFAULT_SIZE);
    memset((void*)__pg_pdpt_higher_half, 0, PAGE_DEFAULT_SIZE);
    memset((void*)__pg_pd_kernel, 0, PAGE_DEFAULT_SIZE);

    __pg_pml4[0]   = (uint64_t)__pg_pdpt_ident       | PAGING_PML4_DEFAULT_FLAGS;
    __pg_pml4[256] = (uint64_t)__pg_pdpt_higher_half | PAGING_PML4_DEFAULT_FLAGS;
    __pg_pml4[511] = (uint64_t)__pg_pdpt_kernel      | PAGING_PML4_DEFAULT_FLAGS;

    __pg_pdpt_kernel[510] = (uint64_t)__pg_pd_kernel | PAGING_PDPT_DEFAULT_FLAGS;
    __pg_pdpt_ident[0]    = (uint64_t)__pg_pd_ident  | PAGING_PDPT_DEFAULT_FLAGS;

    __pg_pd_ident[0]      = (uint64_t)__pg_pt_ident0 | PAGING_PD_DEFAULT_FLAGS;
    __pg_pd_ident[1]      = (uint64_t)__pg_pt_ident1 | PAGING_PD_DEFAULT_FLAGS;
    
    for (size_t i = 0; i < 4; i++) {
        __pg_pd_kernel[i] = ((i * PAGE_LARGE_SIZE) + KERNEL_PHYS_ADDR) | PAGING_PD_LARGE_FLAGS;
    }

    for (size_t i = 0; i < 512; i++) {
        __pg_pdpt_higher_half[i] = (i * PAGE_HUGE_SIZE) | PAGING_PDPT_HUGE_FLAGS;
    }

    for (size_t i = 0; i < 512; i++) {
        __pg_pt_ident0[i] = (i * PAGE_DEFAULT_SIZE)            | PAGING_PT_DEFAULT_FLAGS;
        __pg_pt_ident1[i] = (i * PAGE_DEFAULT_SIZE) + 0x200000 | PAGING_PT_DEFAULT_FLAGS;
    }

    gdt_set_entry(__gdt, 0, 0, 0, 0, 0);                  // null segment descriptor
    gdt_set_entry(__gdt, 1, 0, 0xFFFFF, 0b10011010, 0xA); // code segment descriptor (present, ring 0, executable)
    gdt_set_entry(__gdt, 2, 0, 0xFFFFF, 0b10010010, 0xC); // data segment descriptor (present, ring 0, not executable, grows upward, R/W)
    gdt_load_gdtr(__gdt, 3);

    kernel_entry(PG_PML4_ADDRESS, kernel_elf_hdr.e_entry, &bootinfo);

    while (1);
}

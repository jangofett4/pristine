/*
 * Pristine
 * stage2 - stage 2 bootloader entry point
 * SPDX-License-Identifier: MIT
 */

#include <stdint.h>
#define KERNEL_LOAD_ADDR 0x200000

#include "stage2_ata_pio.h"
#include "stage2_bsfs.h"
#include "stage2_common.h"
#include "stage2_disk.h"
#include "stage2_idt.h"
#include "stage2_io.h"
#include "stage2_memory.h"
#include "stage2_paging.h"
#include "stage2_pic.h"
#include "stage2_serial.h"
#include "stage2_vesa.h"
#include "stage2_video.h"

#include "elf.h"

#include "lib32/printf/printf.h"

static IDT32Entry idt[IDT32_SIZE];

static uint8_t disk_buf[DISK_READ_MAX_SECTORS * 512];
// static uint8_t block_buf[BSFS_BLOCKSIZE];

static Video video;
static Serial serial;

void keyboard_handler(IDT32ISRFrame *frame) {
    uint8_t scancode = io_inb(0x60);
    printf("Scancode 0x%x\n", scancode);
}

__attribute__((section(".text.stage2"))) void stage2_boot(void) {
    serial_init(&serial, 0x3F8);
    serial_set_default(&serial);

    // VesaVbeInfo vesa_info = vesa_vbe_get_info();
    VesaVbeModeInfo vesa_mode_info = vesa_vbe_get_mode_info();

    printf("Stage 2 bootloader started\n");
    printf("Setting up interrupts...\n");

    IDT32ISRHandler isr_table[] = {
        ISR_T(0),  ISR_T(1),  ISR_I(2),  ISR_T(3),  ISR_T(4),  ISR_T(5),
        ISR_T(6),  ISR_T(7),  ISR_T(8),  ISR_T(9),  ISR_T(10), ISR_T(11),
        ISR_T(12), ISR_T(13), ISR_T(14), ISR_I(15), ISR_T(16), ISR_T(17),
        ISR_T(18), ISR_T(19), ISR_T(20), ISR_T(21), ISR_I(22), ISR_I(23),
        ISR_I(24), ISR_I(25), ISR_I(26), ISR_I(27), ISR_I(28), ISR_I(29),
        ISR_I(30), ISR_I(31), ISR_I(32), ISR_I(33), ISR_I(34), ISR_I(35),
        ISR_I(36), ISR_I(37), ISR_I(38), ISR_I(39), ISR_I(40), ISR_I(41),
        ISR_I(42), ISR_I(43), ISR_I(44), ISR_I(45), ISR_I(46), ISR_I(47),
    };

    idt32_set_entries(idt, isr_table, 48);

    IDT32Ptr idt_ptr;
    idt_ptr.limit = (IDT32_SIZE * sizeof(IDT32Entry)) - 1;
    idt_ptr.base = (uint32_t)&idt;

    idt32_load_idtr(&idt_ptr);
    pic_init();

    idt32_enable_interrupts();

    idt32_set_dispatch(33, keyboard_handler);
    // pic_unmask_irq(1); // Keyboard

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

    const uint32_t bsfs_offset = 16 * 4096;

    disk_ops.read(bsfs_offset / 512, 1, disk_buf);
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

    if (header.block_size != BSFS_BLOCKSIZE) {
        PANIC("bsfs: image has %u byte blocks, cannot process", header.block_size);
    }

    BsfsContext bsfs_context = {.header = &header, .disk_ops = &disk_ops};
    int fread_count = -1;

    arena_reset();
    BsfsFile font_file = {
        .buf = arena_alloc(header.block_size, 1), 
        .bufsize = header.block_size
    };
    if (bsfs_fopen(&bsfs_context, "/fonts/tamzen/Tamzen8x15.psf", &font_file) < 0) {
        // TODO: instead of straight up panic, we can iterate over what we have,
        // for now this will have to do
        PANIC("stage2: /fonts/tamzen/Tamzen8x16.psf not found!");
    }
    Psf1Header psf1header;
    if ((fread_count = bsfs_fread(
        &bsfs_context, 
        &psf1header, 
        sizeof(Psf1Header), 
        1,
        &font_file)) < 0
    ) {
        PANIC("stage2: bsfs_fread returned %i", fread_count);
    }

    if (psf1header.magic[0] != 0x36 && psf1header.magic[1] != 0x04) {
        PANIC("stage2: font file is not PSF1");
    }

    video_psf_set_font(psf1header);
    for (size_t i = 0; i < 256; i++) {
        uint8_t *glyph = arena_alloc(psf1header.size, 1);
        if ((fread_count = bsfs_fread(&bsfs_context, glyph, 1, psf1header.size, &font_file)) < 0) {
            PANIC("stage2: malformed PSF1 font file");
        }
        int setglyph_ret = 0;
        if ((setglyph_ret = video_psf_set_glyph(i, glyph, psf1header.size)) < 0) {
            if (setglyph_ret == PSF_SET_GLYPH_SIZE_TOO_BIG) {
                PANIC("psf_set_glyph: PSF1 glyph size too big");
            } else {
                PANIC("psf_set_glyph: unknown error");
            }
        }
    }
    bsfs_fclose(&font_file);
    arena_reset();

    video_init(&video, &vesa_mode_info);
    video_set_default(&video);

    printf("Loading kernel...\n");

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

    for (size_t ph = 0; ph < kernel_elf_hdr.e_phnum; ph++) {
        Elf64Phdr phdr;
        if ((fread_count = bsfs_fread(&bsfs_context, &phdr, kernel_elf_hdr.e_phentsize, 1, &kernel_file)) < 0) {
            PANIC("stage2: unable to read program header at index %u", ph);
        }
        if (phdr.p_memsz < phdr.p_filesz) {
            PANIC("stage2: unable to load kernel.elf, p_filesz > p_memsz");
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

    printf("Entry: 0x%lx\n", kernel_elf_hdr.e_entry);

    bsfs_fclose(&kernel_file);
    arena_reset();

    printf("Setting up 16 MiB identity mapped paging...\n");
    paging_set_pml4_address(0x1000);
    paging_set_pdpt_address(0x2000);
    paging_set_pd_address(0x3000);
    for (size_t i = 0; i < 8; i++) {
      paging_set_level_3_map(0, 0, i, i * 0x200000);
    }

    kernel_entry(0x1000, kernel_elf_hdr.e_entry);

    while (1);
}

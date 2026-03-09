/*
 * Pristine
 * stage2 - stage 2 bootloader entry point
 * SPDX-License-Identifier: MIT
 */

#include "stage2_common.h"
#include "stage2_string.h"
#include "stage2_memory.h"
#include "stage2_vesa.h"
#include "stage2_video.h"
#include "stage2_serial.h"
#include "stage2_idt.h"
#include "stage2_pic.h"
#include "stage2_io.h"
#include "stage2_disk.h"
#include "stage2_ata_pio.h"
#include "stage2_bsfs.h"

#include "printf.h"

static IDT32Entry idt[IDT32_SIZE];

static uint8_t disk_buf[DISK_READ_MAX_SECTORS * 512];
static uint8_t block_buf[BSFS_BLOCKSIZE];

static Video video;
static Serial serial;

void keyboard_handler(IDT32ISRFrame *frame) {
    uint8_t scancode = io_inb(0x60);
    printf("Scancode 0x%x\n", scancode);
}

__attribute__((section(".text.stage2")))
void stage2_boot(void) {
    // video_init(&video);
    // video_set_default(&video);

    serial_init(&serial, 0x3F8);
    serial_set_default(&serial);

    VesaVbeInfo vesa_info = vesa_vbe_get_info();
    VesaVbeModeInfo vesa_mode_info = vesa_vbe_get_mode_info();

    uint8_t *ptr = (uint8_t*)vesa_mode_info.PhysBasePtr;
    for (size_t y = 50; y < 150; y++) {
        for (size_t x = 50; x < 150; x++) {
            uint32_t *pixel = (uint32_t*)(ptr + y * vesa_mode_info.BytesPerScanLine + x * 4);
            *pixel = 0x00FF0000;
        }
    }

    printf("Stage 2 bootloader started\n");
    printf("Setting up interrupts...\n");

    IDT32ISRHandler isr_table[] = {
        ISR_T( 0), ISR_T( 1), ISR_I( 2), ISR_T( 3),
        ISR_T( 4), ISR_T( 5), ISR_T( 6), ISR_T( 7),
        ISR_T( 8), ISR_T( 9), ISR_T(10), ISR_T(11),
        ISR_T(12), ISR_T(13), ISR_T(14), ISR_I(15),
        ISR_T(16), ISR_T(17), ISR_T(18), ISR_T(19),
        ISR_T(20), ISR_T(21), ISR_I(22), ISR_I(23),
        ISR_I(24), ISR_I(25), ISR_I(26), ISR_I(27),
        ISR_I(28), ISR_I(29), ISR_I(30), ISR_I(31),
        ISR_I(32), ISR_I(33), ISR_I(34), ISR_I(35),
        ISR_I(36), ISR_I(37), ISR_I(38), ISR_I(39),
        ISR_I(40), ISR_I(41), ISR_I(42), ISR_I(43),
        ISR_I(44), ISR_I(45), ISR_I(46), ISR_I(47),
    };

    idt32_set_entries(idt, isr_table, 48);

    IDT32Ptr idt_ptr;
    idt_ptr.limit = (IDT32_SIZE * sizeof(IDT32Entry)) - 1;
    idt_ptr.base = (uint32_t)&idt;

    idt32_load_idtr(&idt_ptr);
    pic_init();

    idt32_enable_interrupts();

    idt32_set_dispatch(33, keyboard_handler);
    pic_unmask_irq(1); // Keyboard

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
    printf(" Version:           %i.%i.%i.%i\n", (header.version >> 24) & 0xff, (header.version >> 16) & 0xff, (header.version >> 8) & 0xff, header.version & 0xff);
    printf(" Block Size:        %u\n", header.block_size);
    printf(" Total Blocks:      %u\n", header.total_blocks);
    printf(" Total Inodes:      %u\n", header.inode_count);
    printf(" Free Blocks:       %u\n", header.free_blocks);
    printf(" Root Inode:        0x%04x\n", header.root_inode);
    printf(" Inode Table Start: 0x%04x\n", header.inode_table_start);

    if (header.block_size != BSFS_BLOCKSIZE) {
        PANIC("bsfs: image has %u byte blocks, cannot process", header.block_size);
    }

    BsfsContext bsfs_context = {
        .header = &header,
        .disk_ops = &disk_ops
    };
    
    arena_reset();
    BsfsFile kernel_file = {
        .buf = (uint8_t*)arena_alloc(header.block_size, 1),
        .bufsize = header.block_size
    };
    if (bsfs_fopen(&bsfs_context, "/kernel.elf", &kernel_file) < 0) {
        PANIC("stage2: kernel.elf not found!");
    }
    while (!kernel_file.eof) {
        uint8_t data[16];
        memset(data, 0, 16);
        int ret;
        if ((ret = bsfs_fread(&bsfs_context, data, 1, 16, &kernel_file)) < 0) {
            PANIC("stage2: bsfs_fread returned %i\n", ret);
        }
        for (size_t i = 0; i < ret; i += 2) {
            if (i == ret - 2)
                printf("%02x%02x", data[i], data[i + 1]);
            else
                printf("%02x%02x ", data[i], data[i + 1]);
        }
        
        printf("\n");
    }
    bsfs_fclose(&kernel_file);
    arena_reset();

    while(1);
}

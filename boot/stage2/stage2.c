/*
 * Pristine
 * stage2 - stage 2 bootloader entry point
 * SPDX-License-Identifier: MIT
 */

#include "stage2_common.h"
#include "stage2_string.h"
#include "stage2_memory.h"
#include "stage2_video.h"
#include "stage2_serial.h"
#include "stage2_idt.h"
#include "stage2_pic.h"
#include "stage2_io.h"
#include "stage2_disk.h"
#include "stage2_ata_pio.h"
#include "stage2_fat16.h"
#include "bsfs.h"

#include "printf.h"

static idt32_entry_t idt[IDT32_SIZE];

static uint8_t disk_buf[DISK_READ_MAX_SECTORS * 512];
static uint8_t block_buf[BSFS_BLOCKSIZE];

static video_t video;
static serial_t serial;

void keyboard_handler(idt32_isr_frame_t *frame) {
    uint8_t scancode = io_inb(0x60);
    printf("Scancode 0x%x\n", scancode);
}

__attribute__((section(".text.stage2")))
void stage2_boot(void) {
    video_init(&video);
    video_set_default(&video);
    
    serial_init(&serial, 0x3F8);
    serial_set_default(&serial);

    printf("Stage 2 bootloader started\n");
    printf("Setting up interrupts...\n");

    idt32_isr_handler_t isr_table[] = {
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

    idt32_ptr_t idt_ptr;
    idt_ptr.limit = (IDT32_SIZE * sizeof(idt32_entry_t)) - 1;
    idt_ptr.base = (uint32_t)&idt;

    idt32_load_idtr(&idt_ptr);
    pic_init();

    idt32_enable_interrupts();

    idt32_set_dispatch(33, keyboard_handler);
    pic_unmask_irq(1); // Keyboard

    ata_pio_status_t atapio_status;

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

    disk_ops_vtable_t disk_ops = ata_pio_get_disk_ops();

    const uint32_t bsfs_offset = 8 * 4096;

    disk_ops.read(bsfs_offset / 512, 1, disk_buf);
    bsfs_header_t header;
    memcpy(&header, disk_buf, sizeof(bsfs_header_t));

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

    uint32_t inode_table_start = header.inode_table_start;
    uint64_t root_inode_byte_offset = (header.block_size * inode_table_start) + (header.root_inode * sizeof(bsfs_inode_t));
    uint32_t root_inode_sector = root_inode_byte_offset / 512;

    disk_ops.read(root_inode_sector, 1, disk_buf);
    bsfs_inode_t root_inode;
    memcpy(&root_inode, disk_buf + sizeof(bsfs_inode_t), sizeof(bsfs_inode_t));

    if (root_inode.type != BSFS_INODE_TYPE_DIRECTORY) {
        PANIC("bsfs: root inode is not a directory");
    }

    printf("Root Inode:\n");
    printf(" Size:              %u dirents\n", root_inode.size / sizeof(bsfs_dirent_t));
    printf(" Blocks:            %u\n", root_inode.blocks);

    if (root_inode.size == 0) {
        PANIC("bsfs: root inode has a size of 0");
    }

    uint32_t kernel_inode_idx = 0;
    for (size_t i = 0; i < sizeof(root_inode.blocks_direct) / sizeof(root_inode.blocks_direct[0]); i++) {
        uint32_t current_block = root_inode.blocks_direct[i];
        if (current_block == 0) {
            printf("\n");
            continue;
        }
        printf(" Block %u: %u, navigating...\n", i, current_block);
        disk_ops.read(header.block_size * current_block / 512, 8, block_buf);
        bsfs_dirent_t *dirents = &block_buf;
        for (size_t i = 0; i < header.block_size / sizeof(bsfs_dirent_t); i++) {
            bsfs_dirent_t *dirent = &dirents[i];
            if (dirent->inode == 0) continue;
            printf(" > Dirent: %s [%u]\n", dirent->name, dirent->inode);
            if (strcmp("kernel.elf", dirent->name) == 0) {
                kernel_inode_idx = dirent->inode;
                break;
            }
        }
        if (kernel_inode_idx)
            break;
    }

    if (!kernel_inode_idx) {
        PANIC("pristine: kernel.elf not found");
    }

    printf("kernel.elf found, Inode %u\n", kernel_inode_idx);
    uint64_t kernel_inode_byte_offset = (header.block_size * inode_table_start) + (kernel_inode_idx * sizeof(bsfs_inode_t));
    uint32_t kernel_inode_sector = kernel_inode_byte_offset / 512;
    uint32_t kernel_inode_buf_offset = kernel_inode_byte_offset % 512;
    disk_ops.read(kernel_inode_sector, 1, disk_buf);
    bsfs_inode_t kernel_inode;
    memcpy(&kernel_inode, disk_buf + kernel_inode_buf_offset, sizeof(bsfs_inode_t));

    if (kernel_inode.type != BSFS_INODE_TYPE_FILE) {
        PANIC("pristine: kernel.elf is not a file");
    }

    printf("kernel.elf:\n");
    printf(" Size:   %lu bytes\n", kernel_inode.size);
    printf(" Blocks: %u\n", kernel_inode.blocks);

    for (size_t i = 0; i < sizeof(kernel_inode.blocks_direct) / sizeof(kernel_inode.blocks_direct[0]); i++) {
        uint32_t current_block = kernel_inode.blocks_direct[i];
        if (current_block == 0) continue;
        disk_ops.read((current_block * header.block_size / 512), 8, disk_buf);
        printf("Block %u, [0x%02x 0x%02x 0x%02x 0x%02x]\n", current_block, disk_buf[0], disk_buf[1], disk_buf[2], disk_buf[3]);
    }

    while(1);
}
/*
 * BSFS
 * mkfs.bsfs - filesystem image creation tool
 * SPDX-License-Identifier: MIT
 */

#define _POSIX_C_SOURCE 200809L

#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include <stddef.h>

#include "bsfs.h"

#define DIV_CEIL(a, b) (((a) + (b) - 1) / (b))

int main(int argc, char **argv)
{
    if (argc < 2) {
        printf("Usage: %s image-file [options]\n", argv[0]);
        printf("Valid options are:\n");
        printf(" --offset offset\n");
        printf("   Create BSFS with offset (in blocks)\n\n");
        printf(" --block-size size\n");
        printf("   Set custom block size, default is 4096 (1024..16384)\n\n");
        printf(" --label label\n");
        printf("   Label of the created filesystem\n\n");
        return 1;
    }

    char *image_file  = argv[1];

    uint64_t final_blocksize = 4096;
    uint64_t final_offset = 0;
    char *final_label = "mkfs.bsfs";

    for (int i = 2; i < argc; i++) {

        if (strcmp(argv[i], "--offset") == 0) {
            if (i + 1 >= argc) {
                printf("Error: expected number after --offset.\n");
                return 1;
            }

            char *end; errno = 0;
            long value = strtol(argv[i + 1], &end, 10);

            if (end == argv[i + 1]) {
                printf("Error: unable to convert '%s' to offset.\n", argv[i + 1]);
                return 1;
            } else if (*end != '\0') {
                printf("Error: invalid characters in offset '%s'.\n", argv[i + 1]);
                return 1;
            } else if (errno == ERANGE || value < 0 || value > 1024) {
                printf("Error: offset out of range (0..1024): %ld\n", value);
                return 1;
            }

            final_offset = (uint64_t)value;
            i++;
        } else if (strcmp(argv[i], "--block-size") == 0) {
            if (i + 1 >= argc) {
                printf("Error: expected number after --block-size.\n");
                return 1;
            }

            char *end; errno = 0;
            long value = strtol(argv[i + 1], &end, 10);

            if (end == argv[i + 1]) {
                printf("Error: unable to convert '%s' to block size.\n", argv[i + 1]);
                return 1;
            } else if (*end != '\0') {
                printf("Error: invalid characters in block size '%s'.\n", argv[i + 1]);
                return 1;
            } else if (errno == ERANGE || value < 1024 || value > 16384) {
                printf("Error: block size out of range (1024..16384): %ld\n", value);
                return 1;
            }

            final_blocksize = (uint64_t)value;
            i++;
        } else if (strcmp(argv[i], "--label") == 0) {
            if (i + 1 >= argc) {
                printf("Error: expected string after --label.\n");
                return 1;
            }

            char *label = argv[i + 1];
            if (strlen(label) >= 32) {
                printf("Error: maximum label length is 32 characters (null terminator included)\n");
                return 1;
            }

            final_label = label;
            i++;
        } else {
            printf("Error: unknown option '%s'\n", argv[i]);
            return 1;
        }
    }

    uint64_t bsfs_base = final_blocksize * final_offset;

    FILE *image = fopen(image_file, "rb+");
    if (!image) {
        printf("Error: '%s', ", image_file);
        fflush(stdout);
        perror("fopen");
        return 1;
    }
    fseeko(image, 0, SEEK_END);
    uint64_t imagesize = ftello(image);
    fseeko(image, 0, SEEK_SET);

    if (imagesize % final_blocksize != 0) {
        printf("Error: Image size %lu is not a multiple of block size %lu.\n", imagesize, final_blocksize);
        return 1;
    }

    uint32_t total_blocks            = imagesize / final_blocksize;
    uint32_t total_megablocks        = DIV_CEIL(total_blocks, final_blocksize * 8);
    uint32_t inode_count             = total_blocks / 4;

    uint32_t block_bitmap_bytes      = DIV_CEIL(total_blocks, 8);
    uint32_t block_bitmap_blocks     = DIV_CEIL(block_bitmap_bytes, final_blocksize);

    uint32_t megablock_bitmap_bytes  = DIV_CEIL(total_megablocks, 8);
    uint32_t megablock_bitmap_blocks = DIV_CEIL(megablock_bitmap_bytes, final_blocksize);

    uint32_t inode_bitmap_bytes      = DIV_CEIL(inode_count, 8);
    uint32_t inode_bitmap_blocks     = DIV_CEIL(inode_bitmap_bytes, final_blocksize);

    uint32_t inode_table_bytes       = inode_count * sizeof(BsfsInode);
    uint32_t inode_table_blocks      = DIV_CEIL(inode_table_bytes, final_blocksize);

    uint32_t reserved_blocks        = final_offset + 1;
    uint32_t block_bitmap_start     = reserved_blocks;
    uint32_t megablock_bitmap_start = block_bitmap_start + block_bitmap_blocks;
    uint32_t inode_bitmap_start     = megablock_bitmap_start + megablock_bitmap_blocks;
    uint32_t inode_table_start      = inode_bitmap_start + inode_bitmap_blocks;
    uint32_t data_start             = inode_table_start  + inode_table_blocks;
    uint32_t metadata_blocks        = reserved_blocks + block_bitmap_blocks + megablock_bitmap_blocks + inode_bitmap_blocks + inode_table_blocks;

    printf("%s will generate a filesystem image with:\n", argv[0]);
    printf(" Size:             %.2f KiB (%.2f MiB, %.2f GiB)\n", imagesize / 1024.f, imagesize / 1024.f / 1024, imagesize / 1024.f / 1024 / 1024);
    printf(" Label:            %s\n", final_label);
    printf(" Output image:     %s\n", image_file);
    printf(" Block bitmap:     %u blocks\n", block_bitmap_blocks);
    printf(" Megablock bitmap: %u blocks\n", megablock_bitmap_blocks);
    printf(" Megablock bitmap: %u bits\n", total_megablocks);
    printf(" Inode bitmap:     %u blocks\n", inode_bitmap_blocks);
    printf(" Inode table:      %u blocks\n", inode_table_blocks);
    printf(" Block size:       %lu\n", final_blocksize);
    printf(" Image offset:     0x%04lx\n", bsfs_base);
    printf(" Total blocks:     %u\n", total_blocks);
    printf(" Total inodes:     %u\n", inode_count);

    if (sizeof(BsfsHeader) != 512) {
        BSFS_PANIC("mkfs.bsfs: sizeof(BsfsHeader), expected 512 got %lu", sizeof(BsfsHeader));
    }

    BsfsHeader header = {
        .bootjmp                 = { 0xEB, offsetof(BsfsHeader, bootcode) - 2, 0x90 },
        .magic                   = BSFS_MAGIC,
        .version                 = BSFS_VERSION,
        .block_size              = final_blocksize,
        .total_blocks            = total_blocks,
        .inode_count             = inode_count,
        .free_blocks             = total_blocks - metadata_blocks,
        .free_inodes             = inode_count,
        .block_bitmap_start      = block_bitmap_start,
        .block_bitmap_blocks     = block_bitmap_blocks,
        .megablock_bitmap_start  = megablock_bitmap_start,
        .megablock_bitmap_blocks = megablock_bitmap_blocks,
        .megablock_bitmap_size   = megablock_bitmap_bytes,
        .inode_bitmap_start      = inode_bitmap_start,
        .inode_bitmap_blocks     = inode_bitmap_blocks,
        .inode_table_start       = inode_table_start,
        .inode_table_blocks      = inode_table_blocks,
        .data_start              = data_start,
        .root_inode              = 0,
        .partition_lba           = final_offset,
        .label                   = {0},
        .bootcode                = {0},
        .bootcode_magic          = { 0x55, 0xAA }
    };

    uint8_t *zeros;

    memcpy(header.label, (uint8_t*)final_label, strlen(final_label));
    fseeko(image, (uint64_t)bsfs_base, SEEK_SET);
    fwrite(&header, sizeof(BsfsHeader), 1, image);

    uint64_t padding_size = final_blocksize - sizeof(BsfsHeader);
    zeros = calloc(1, padding_size);
    fwrite(zeros, 1, padding_size, image);
    free(zeros);

    zeros = calloc(1, final_blocksize);
    fseeko(image, final_blocksize * header.block_bitmap_start, SEEK_SET);
    for (size_t i = 0; i < header.block_bitmap_blocks; i++) {
        fwrite(zeros, 1, final_blocksize, image);
    }

    fseeko(image, final_blocksize * header.megablock_bitmap_start, SEEK_SET);
    for (size_t i = 0; i < header.megablock_bitmap_blocks; i++) {
        fwrite(zeros, 1, final_blocksize, image);
    }

    fseeko(image, final_blocksize * header.inode_bitmap_start, SEEK_SET);
    for (size_t i = 0; i < header.inode_bitmap_blocks; i++) {
        fwrite(zeros, 1, final_blocksize, image);
    }

    fseeko(image, final_blocksize * header.inode_table_start, SEEK_SET);
    for (size_t i = 0; i < header.inode_table_blocks; i++) {
        fwrite(zeros, 1, final_blocksize, image);
    }
    free(zeros);

    fclose(image);
    return 0;
}

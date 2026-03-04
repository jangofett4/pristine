/*
 * Pristine
 * bsfs-populate - filesystem populate tool
 * SPDX-License-Identifier: MIT
 */

#define _DEFAULT_SOURCE

#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <errno.h>
#include <stdlib.h>
#include <stdbool.h>
#include <dirent.h>
#include <time.h>

#if defined (linux) || defined (__linux)
#include <sys/mman.h>
#endif

#include "bsfs.h"

#define DIV_CEIL(a, b) (((a) + (b) - 1) / (b))

int main(int argc, char **argv) {
    if (argc < 3) {
        printf("Usage %s image-file /path/to/root/folder [options]\n", argv[0]);
        printf(" --offset offset\n");
        printf("   Look for BSFS header with offset\n\n");
        printf(" --block-size size\n");
        printf("   Set custom block size, default is 4096 (1024..16384)\n\n");
        printf(" --force\n");
        printf("   Force populating the image. Warning: this might cause corrupt images.\n\n");
        return 0;
    }

    char *image_file = argv[1];
    char *root_path = argv[2];

    uint64_t final_offset = 0;
    uint64_t final_blocksize = 4096;
    bool final_force = false;

    for (int i = 3; i < argc; i++) {
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
        } else if (strcmp(argv[i], "--force") == 0) {
            if (final_force) {
                printf("Error: duplicate --force flag.");
                return 1;
            }
            final_force = true;
            continue;
        }
    }

    uint32_t bsfs_header_idx = final_offset * final_blocksize;

    printf("%s will populate '%s' with contents of '%s':\n", argv[0], argv[1], argv[2]);
    printf(" Block size: %lu\n", final_blocksize);
    printf(" Offset:     0x%04x\n", bsfs_header_idx);

    FILE *image = fopen(image_file, "rb+");
    if (!image) {
        printf("Error: unable to open image file '%s', ", image_file);
        fflush(stdout);
        perror("fopen");
        return 1;
    }
    setvbuf(image, NULL, _IOFBF, 1024 * 1024 * 64);

    bsfs_header_t header;
    fseeko(image, bsfs_header_idx, SEEK_SET);
    if (!fread(&header, sizeof(bsfs_header_t), 1, image)) {
        printf("Error: unable to read header at offset 0x%04x\n", bsfs_header_idx);
        return 1;
    }

    if (header.magic != BSFS_MAGIC) {
        printf("Error: header at offset 0x%04x doesn't contain a valid BSFS signature.\n", bsfs_header_idx);
        return 1;
    }

    if (header.version != BSFS_VERSION) {
        if (!final_force) {
            printf("Error: Filesystem version mismatch. Image: %u, Current: %u.\n", header.version, BSFS_VERSION);
            printf("Use --force to try force populate the image file.\n");
            return 1;
        } else {
            printf("Warning: Force populating filesystem version mismatched image!\n");
        }
    }

    if (header.block_size != final_blocksize) {
        printf("Error: Filesystem block size mismatch. Image: %u, Current: %lu.\n", header.block_size, final_blocksize);
        return 1;
    }

    fseeko(image, 0, SEEK_END);
    uint64_t image_file_size = ftello(image);
    fseeko(image, 0, SEEK_SET);

    printf("Filesystem information:\n");
    printf(" Image label:        %s\n", header.label);
    printf(" Version:            %u.%u.%u.%u\n",
        (header.version >> 24) & 0xFF,
        (header.version >> 16) & 0xFF,
        (header.version >> 8)  & 0xFF,
        (header.version)       & 0xFF
    );
    printf(" Total Blocks:            %u\n", header.total_blocks);
    printf(" Total Inodes:            %u\n", header.inode_count);
    printf(" Block bitmap start:      0x%04x\n", header.block_bitmap_start);
    printf(" Megablock bitmap start:  0x%04x\n", header.megablock_bitmap_start);
    printf(" Megablock bitmap bits:   %u\n", header.megablock_bitmap_size * 8);
    printf(" Inode bitmap start:      0x%04x\n", header.inode_bitmap_start);
    printf(" Inode table start:       0x%04x\n", header.inode_table_start);
    printf(" Block bitmap blocks:     %u\n", header.block_bitmap_blocks);
    printf(" Megablock bitmap blocks: %u\n", header.megablock_bitmap_blocks);
    printf(" Inode bitmap blocks:     %u\n", header.inode_bitmap_blocks);
    printf(" Inode table blocks:      %u\n", header.inode_table_blocks);
    printf(" Data start:              0x%04x\n", header.data_start);
    printf(" Image Size:              %.2f KiB (calculated)\n", header.total_blocks * final_blocksize / 1024.f);
    printf(" Image File Size:         %.2f KiB (file size)\n", image_file_size / 1024.f);

    if (image_file_size != header.total_blocks * final_blocksize) {
        int64_t diff;
        if (image_file_size > header.total_blocks * final_blocksize) {
            diff = image_file_size - header.total_blocks * final_blocksize;
        } else {
            diff = header.total_blocks * final_blocksize - image_file_size;
        }

        printf("Warning: Calculated image size and file size does not match (%lu bytes, %.2f KiB)\n", diff, diff / 1024.f);
    }

    uint32_t total_megablocks      = DIV_CEIL(header.total_blocks, final_blocksize * 8);

    uint64_t block_bitmap_size     = header.block_bitmap_blocks * final_blocksize;
    uint64_t megablock_bitmap_size = header.megablock_bitmap_size;
    uint64_t inode_bitmap_size     = header.inode_bitmap_blocks * final_blocksize;
    uint64_t inode_table_size      = header.inode_count * sizeof(bsfs_inode_t);

    uint64_t total_block_count       = image_file_size / final_blocksize;
    uint64_t total_inode_count       = total_block_count / 4;
    uint32_t block_bitmap_blocks     = header.block_bitmap_blocks;
    uint32_t megablock_bitmap_blocks = header.megablock_bitmap_blocks;
    uint32_t inode_bitmap_blocks     = header.inode_bitmap_blocks;
    uint32_t inode_table_blocks      = header.inode_table_blocks;
    uint32_t reserved_blocks         = final_offset + 1; // +1 is the superblock / header
    uint32_t metadata_blocks         = reserved_blocks + block_bitmap_blocks + megablock_bitmap_blocks + inode_bitmap_blocks + inode_table_blocks;

    header.free_blocks = total_block_count - metadata_blocks;
    header.free_inodes = total_inode_count;

    uint8_t *block_bitmap     = calloc(block_bitmap_size, 1);
    uint8_t *megablock_bitmap = calloc(megablock_bitmap_size, 1);
    uint8_t *inode_bitmap     = calloc(inode_bitmap_size, 1);
    bsfs_inode_t *inode_table = calloc(header.inode_count, sizeof(bsfs_inode_t));

#if defined (linux) || defined (__linux__)
    madvise(block_bitmap, block_bitmap_size, MADV_WILLNEED);
    madvise(megablock_bitmap, megablock_bitmap_size, MADV_WILLNEED);
    madvise(inode_bitmap, block_bitmap_size, MADV_WILLNEED);
    madvise(inode_table, inode_bitmap_size, MADV_WILLNEED);
#endif

    // Reserve metadata blocks, we VERY MUCH would not like to overwrite them accidentally
    for (size_t i = 0; i < metadata_blocks; i++) {
        bsfs_bitmap_set(block_bitmap, i);
    }

    for (uint32_t mb = 0; mb < total_megablocks; mb++) {
        uint8_t *slice = block_bitmap + mb * final_blocksize;
        bool full = true;
        for (uint32_t b = 0; b < final_blocksize; b++) {
            if (slice[b] != UINT8_MAX) { full = false; break; }
        }
        if (full)
            bsfs_bitmap_set(megablock_bitmap, mb);
    }

    time_t populate_time = time(NULL);

    uint32_t sentinel_inode_idx = bsfs_alloc_inode(&header, inode_bitmap);
    uint32_t root_inode_idx = bsfs_alloc_inode(&header, inode_bitmap);

    uint32_t cnt = header.free_blocks;
    for (size_t i = 0; i < cnt; i++) {
        uint32_t idx = bsfs_alloc_block(&header, block_bitmap, megablock_bitmap);
    }
    return 0;
}

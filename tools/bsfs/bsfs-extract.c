/*
 * Pristine
 * bsfs-extract - filesystem extraction tool
 * SPDX-License-Identifier: MIT
 */

#define _POSIX_C_SOURCE 200809L
#define _DEFAULT_SOURCE

#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <errno.h>
#include <stdlib.h>
#include <stdbool.h>

#if defined(linux) || defined(__linux)
#include <sys/mman.h>
#endif

#include "bsfs.h"

int main(int argc, char **argv) {
    if (argc < 3) {
        printf("Usage %s image-file /output/folder/path [options]\n", argv[0]);
        printf(" --offset offset\n");
        printf("   Look for BSFS header with offset\n\n");
        printf(" --block-size size\n");
        printf("   Set custom block size, default is 4096 (1024..16384)\n\n");
        printf(" --force\n");
        printf("   Force extracting the image. Warning: this might extract invalid data from the image.\n\n");
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
            continue;;
        }
    }

    uint32_t bsfs_header_idx = final_offset * final_blocksize;

    printf("%s will extract '%s' to '%s':\n", argv[0], argv[1], argv[2]);
    printf(" Block size: %lu\n", final_blocksize);
    printf(" Offset:     0x%04x\n", bsfs_header_idx);

    FILE *image = fopen(image_file, "rb");
    if (!image) {
        printf("Error: unable to open image file '%s', ", image_file);
        fflush(stdout);
        perror("fopen");
        return 1;
    }
    setvbuf(image, NULL, _IOFBF, 64 * 1024 * 1024);

    bsfs_header_t header;
    fseek(image, bsfs_header_idx, SEEK_SET);
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
            printf("Use --force to try force extract the image file.\n");
            return 1;
        } else {
            printf("Warning: Force extracting filesystem version mismatched image!\n");
        }
    }

    if (header.block_size != final_blocksize) {
        printf("Error: Filesystem block size mismatch. Image: %u, Current: %lu.\n", header.block_size, final_blocksize);
        return 1;
    }

    printf("Filesystem information:\n");
    printf(" Image label:        %s\n", header.label);
    printf(" Version:            %u.%u.%u.%u\n",
        (header.version >> 24) & 0xFF,
        (header.version >> 16) & 0xFF,
        (header.version >> 8)  & 0xFF,
        (header.version)       & 0xFF
    );
    printf(" Total Blocks:       %u\n", header.total_blocks);
    printf(" Free Blocks:        %u\n", header.free_blocks);
    printf(" Total Inodes:       %u\n", header.inode_count);
    printf(" Free Inodes:        %u\n", header.free_inodes);
    printf(" Block bitmap start: 0x%04x\n", header.block_bitmap_start);
    printf(" Inode bitmap start: 0x%04x\n", header.inode_bitmap_start);
    printf(" Inode table start:  0x%04x\n", header.inode_table_start);
    printf(" Inode table blocks: 0x%04x\n", header.inode_table_blocks);
    printf(" Data start:         0x%04x\n", header.data_start);
    printf(" Root inode:         %u\n", header.root_inode);
    printf(" Image Size:         %.2f KiB (calculated)\n", header.total_blocks * final_blocksize / 1024.f);

    uint32_t block_bitmap_size = header.block_bitmap_blocks * header.block_size;
    uint32_t inode_bitmap_size = header.inode_bitmap_blocks * header.block_size;
    uint32_t inode_table_size  = header.inode_count * sizeof(bsfs_inode_t);

    uint8_t *block_bitmap     = malloc(block_bitmap_size);
    uint8_t *inode_bitmap     = malloc(inode_bitmap_size);
    bsfs_inode_t *inode_table = (bsfs_inode_t*)malloc(inode_table_size);

#if defined(linux) || defined(__linux__)
    // As if we made the rest of the program cross platform...
    // Maybe tiny bit of extra performance
    madvise(block_bitmap, block_bitmap_size, MADV_WILLNEED);
    madvise(inode_bitmap, inode_bitmap_size, MADV_WILLNEED);
    madvise(inode_table, inode_table_size, MADV_WILLNEED);
#endif

    fseeko(image, header.block_bitmap_start * header.block_size, SEEK_SET);
    fread(block_bitmap, 1, block_bitmap_size, image);

    fseeko(image, header.inode_bitmap_start * header.block_size, SEEK_SET);
    fread(inode_bitmap, 1, inode_bitmap_size, image);

    fseeko(image, header.inode_table_start * header.block_size, SEEK_SET);
    fread(inode_table, sizeof(bsfs_inode_t), header.inode_count, image);

    bsfs_inode_t root_inode = inode_table[header.root_inode];
    bsfs_dirent_t *root_dirent = (bsfs_dirent_t*)malloc(root_inode.size);

    fseeko(image, root_inode.blocks_direct[0] * header.block_size, SEEK_SET);
    fread(root_dirent, sizeof(bsfs_dirent_t), root_inode.size / sizeof(bsfs_dirent_t), image);

    printf("root_inode size: %lu\n", root_inode.size);
    for (size_t i = 0; i < root_inode.size / sizeof(bsfs_dirent_t); i++) {
        bsfs_inode_t inode = inode_table[root_dirent[i].inode];
        printf("Dirent: %s, Inode: %u, Size: %lu, (%.2f KiB, %.2f MiB)\n", root_dirent[i].name, root_dirent[i].inode, inode.size, inode.size / 1024.f, inode.size / 1024.f / 1024);
    }

    free(block_bitmap);
    free(inode_bitmap);
    free(inode_table);

    return 0;
}
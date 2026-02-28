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
#include <stdbool.h>
#include <errno.h>
#include <limits.h>
#include <stddef.h>
#include <unistd.h>

#include "bsfs.h"

#define DIV_CEIL(a, b) (((a) + (b) - 1) / (b))

int main(int argc, char **argv)
{
    if (argc < 4) {
        printf("Usage: %s root-folder boot-file image-file [options]\n", argv[0]);
        printf("Valid options are:\n");
        printf(" --size size\n");
        printf("   Image size (in megablocks), default is 16\n\n");
        printf(" --offset offset\n");
        printf("   Create BSFS with offset (in blocks)\n\n");
        printf(" --block-size size\n");
        printf("   Set custom block size (1024..16384), default is 4096\n\n");
        printf(" --label label\n");
        printf("   Label of the created filesystem\n\n");
        printf("\n");
        printf("Glossary:\n");
        printf(" 1 megablock = 1024 blocks\n");
        return 1;
    }

    char *root_folder = argv[1];
    char *boot_file   = argv[2];
    char *image_file  = argv[3];

    uint64_t final_blocksize = 4096;
    uint64_t final_offset = 0;
    uint64_t final_size = 16;
    char *final_label = "mkfs.bsfs";

    for (int i = 4; i < argc; i++) {

        if (strcmp(argv[i], "--offset") == 0) {

            if (i + 1 >= argc) {
                printf("Error: expected number after --offset.\n");
                return 1;
            }

            char *end;
            errno = 0;

            long value = strtol(argv[i + 1], &end, 10);

            if (end == argv[i + 1]) {
                printf("Error: unable to convert '%s' to offset.\n", argv[i + 1]);
                return 1;
            }

            if (*end != '\0') {
                printf("Error: invalid characters in offset '%s'.\n", argv[i + 1]);
                return 1;
            }

            if (errno == ERANGE || value < 0 || value > 1024) {
                printf("Error: offset out of range (0..1024): %ld\n", value);
                return 1;
            }

            final_offset = (uint64_t)value;

            i++;
        }

        else if (strcmp(argv[i], "--block-size") == 0) {

            if (i + 1 >= argc) {
                printf("Error: expected number after --block-size.\n");
                return 1;
            }

            char *end;
            errno = 0;

            long value = strtol(argv[i + 1], &end, 10);

            if (end == argv[i + 1]) {
                printf("Error: unable to convert '%s' to block size.\n", argv[i + 1]);
                return 1;
            }

            if (*end != '\0') {
                printf("Error: invalid characters in block size '%s'.\n", argv[i + 1]);
                return 1;
            }

            if (errno == ERANGE || value < 1024 || value > 16384) {
                printf("Error: block size out of range (1024..16384): %ld\n", value);
                return 1;
            }

            final_blocksize = (uint64_t)value;

            i++;
        }

        else if (strcmp(argv[i], "--size") == 0) {

            if (i + 1 >= argc) {
                printf("Error: expected number after --size.\n");
                return 1;
            }

            char *end;
            errno = 0;

            long value = strtol(argv[i + 1], &end, 10);

            if (end == argv[i + 1]) {
                printf("Error: unable to convert '%s' to size.\n", argv[i + 1]);
                return 1;
            }

            if (*end != '\0') {
                printf("Error: invalid characters in size '%s'.\n", argv[i + 1]);
                return 1;
            }

            if (errno == ERANGE || value < 16) {
                printf("Error: size out of range (16..1048576): %ld\n", value);
                return 1;
            }

            final_size = (uint64_t)value;

            i++;
        }

        else if (strcmp(argv[i], "--label") == 0) {

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
        }

        else {
            printf("Error: unknown option '%s'\n", argv[i]);
            return 1;
        }
    }

    if (final_size > BSFS_MAX_MEGABLOCKS) {
        printf("Error: image size exceeds maximum supported size: '%lu' megablocks.\n", final_size);
        return 1;
    }

    final_size = final_size * final_blocksize * 1024;
    uint32_t bsfs_base = final_blocksize * final_offset;

    printf("%s will generate a filesystem image with:\n", argv[0]);
    printf(" Size:         %.2f KiB (%.2f MiB, %.2f GiB)\n", final_size / 1024.f, final_size / 1024.f / 1024, final_size / 1024.f / 1024 / 1024);
    printf(" Label:        %s\n", final_label);
    printf(" Root folder:  %s\n", root_folder);
    printf(" Boot file:    %s\n", boot_file);
    printf(" Output image: %s\n", image_file);
    printf(" Block size:   %lu\n", final_blocksize);
    printf(" Image offset: 0x%04x\n", bsfs_base);

    FILE *image = fopen(image_file, "wb+");
    if (!image) {
        printf("Error: '%s', ", image_file);
        fflush(stdout);
        perror("fopen");
        return 1;
    }
    ftruncate(fileno(image), final_size);

    FILE *bootbin = fopen(boot_file, "rb");
    if (!bootbin) {
        printf("Error: '%s', ", boot_file);
        fflush(stdout);
        perror("fopen");
        return 1;
    }

    // Read boot binary
    uint8_t *bootdata = malloc(64 * 512); // max 64 sectors
    if (!bootdata) {
        perror("malloc");
        return 1;
    }
    uint64_t bootdata_size = fread(bootdata, 1, 32768, bootbin);
    if (bootdata_size < 512) {
        printf("Error: '%s' boot binary contains less than 512 bytes.\n", boot_file);
        return 1;
    }
    fclose(bootbin);

    // == Calculations ==
    uint32_t total_blocks        = final_size / final_blocksize;
    uint32_t inode_count         = total_blocks / 4;

    uint32_t block_bitmap_bytes  = DIV_CEIL(total_blocks, 8);
    uint32_t block_bitmap_blocks = DIV_CEIL(block_bitmap_bytes, final_blocksize);

    uint32_t inode_bitmap_bytes  = DIV_CEIL(inode_count, 8);
    uint32_t inode_bitmap_blocks = DIV_CEIL(inode_bitmap_bytes, final_blocksize);

    uint32_t inode_table_bytes   = inode_count * sizeof(bsfs_inode_t);
    uint32_t inode_table_blocks  = DIV_CEIL(inode_table_bytes, final_blocksize);

    uint32_t reserved_blocks     = (final_offset == 0) ? 1 : 9;
    uint32_t block_bitmap_start  = reserved_blocks;
    uint32_t inode_bitmap_start  = block_bitmap_start + block_bitmap_blocks;
    uint32_t inode_table_start   = inode_bitmap_start + inode_bitmap_blocks;
    uint32_t data_start          = inode_table_start  + inode_table_blocks;
    uint32_t metadata_blocks     = reserved_blocks + block_bitmap_blocks + inode_bitmap_blocks + inode_table_blocks;

    bsfs_header_t header = {
        .bootjmp             = { 0xEB, offsetof(bsfs_header_t, bootcode) - 2, 0x90 },
        .magic               = BSFS_MAGIC,
        .version             = BSFS_VERSION,
        .block_size          = final_blocksize,
        .total_blocks        = total_blocks,
        .inode_count         = inode_count,
        .free_blocks         = total_blocks - metadata_blocks,
        .free_inodes         = inode_count,
        .block_bitmap_start  = block_bitmap_start,
        .block_bitmap_blocks = block_bitmap_blocks,
        .inode_bitmap_start  = inode_bitmap_start,
        .inode_bitmap_blocks = inode_bitmap_blocks,
        .inode_table_start   = inode_table_start,
        .inode_table_blocks  = inode_table_blocks,
        .data_start          = data_start,
        .root_inode          = 0, // TODO
        .partition_lba       = final_offset,
        .label               = {0},
        .bootcode            = {0},
        .bootcode_magic      = { 0x55, 0xAA }
    };

    memcpy(header.label, (uint8_t*)final_label, strlen(final_label));

    if (final_offset == 0) {
        // Stage 1
        memcpy(&header.bootcode, bootdata, sizeof(header.bootcode));
        fwrite(&header, 1, sizeof(bsfs_header_t), image);

        // Stage 2
        uint8_t stage2_region[32256] = {0};
        uint64_t stage2_size = bootdata_size > 512 ? bootdata_size - 512 : 0;
        memcpy(stage2_region, bootdata + 512, stage2_size < 32256 ? stage2_size : 32256);
        fwrite(stage2_region, 1, sizeof(stage2_region), image);
    } else {
        // Write bootcode, padded with zeros
        uint8_t boot_region[32768] = {0};
        memcpy(boot_region, bootdata, bootdata_size < 32768 ? bootdata_size : 32768);
        fwrite(boot_region, 1, 32768, image);

        // Write header + padding to align to block boundary (512 + 3584 = 4096)
        uint8_t header_region[4096] = {0};
        memcpy(header_region, &header, sizeof(bsfs_header_t));
        fwrite(header_region, 1, 4096, image);
    }

    free(bootdata);

    fclose(image);
    return 0;
}
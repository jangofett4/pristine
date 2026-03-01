/*
 * Pristine
 * mkfs-populate - filesystem populate tool
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
#include <dirent.h>
#include <time.h>

#include "bsfs.h"

#define DIV_CEIL(a, b) (((a) + (b) - 1) / (b))

void populate_dir(const char *path, FILE *image, bsfs_inode_t *inode, bsfs_header_t *header, uint8_t *block_bitmap, uint32_t block_bitmap_size, uint8_t *inode_bitmap, uint32_t inode_bitmap_size, bsfs_inode_t *inode_table);

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
            continue;;
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
            printf("Use --force to force populate the image file.\n");
            return 1;
        } else {
            printf("Warning: Force populating filesystem version mismatched image!\n");
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
    printf(" Total Inodes:       %u\n", header.inode_count);
    printf(" Block bitmap start: 0x%04x\n", header.block_bitmap_start);
    printf(" Inode bitmap start: 0x%04x\n", header.inode_bitmap_start);
    printf(" Inode table start:  0x%04x\n", header.inode_table_start);
    printf(" Inode table blocks: 0x%04x\n", header.inode_table_blocks);
    printf(" Data start:         0x%04x\n", header.data_start);
    printf(" Image Size:         %.2f KiB (calculated)\n", header.total_blocks * final_blocksize / 1024.f);
    
    uint32_t block_bitmap_size = header.block_bitmap_blocks * header.block_size;
    uint32_t inode_bitmap_size = header.inode_bitmap_blocks * header.block_size;

    uint8_t *block_bitmap     = malloc(block_bitmap_size);
    uint8_t *inode_bitmap     = malloc(inode_bitmap_size);
    bsfs_inode_t *inode_table = malloc(header.inode_count * sizeof(bsfs_inode_t));

    memset(block_bitmap, 0, block_bitmap_size);
    memset(inode_bitmap, 0, inode_bitmap_size);
    memset(inode_table, 0, header.inode_count * sizeof(bsfs_inode_t));
    

    // Right off the bat 0..(final_offset + 1) blocks are occupied with metadata & offset
    for (size_t i = 0; i < (final_offset + 1); i++) {
        bsfs_bitmap_set(block_bitmap, i);
    }
    // Block bitmap itself is also not free
    for (size_t i = header.block_bitmap_start; i < (header.block_bitmap_start + header.block_bitmap_blocks); i++) {
        bsfs_bitmap_set(block_bitmap, i);
    }
    // Similarly for inode bitmap
    for (size_t i = header.inode_bitmap_start; i < (header.inode_bitmap_start + header.inode_bitmap_blocks); i++) {
        bsfs_bitmap_set(block_bitmap, i);
    }
    // And also inode table
    for (size_t i = header.inode_table_start; i < (header.inode_table_start + header.inode_table_blocks); i++) {
        bsfs_bitmap_set(block_bitmap, i);
    }

    time_t populate_time = time(NULL);

    // First inode is always root (/)
    uint32_t root_inode_idx = bsfs_alloc_inode(&header, inode_bitmap, inode_bitmap_size);
    bsfs_inode_t root_inode = inode_table[root_inode_idx];
    populate_dir(root_path, image, &root_inode, &header, block_bitmap, block_bitmap_size, inode_bitmap, inode_bitmap_size, inode_table);
    
    // And first dirent is also always root (/)
    uint64_t root_dirent_offset = bsfs_alloc_dirent(&header, &root_inode, block_bitmap, block_bitmap_size);
    bsfs_dirent_t root_dirent = {
        .inode = root_inode_idx,
        .name = "/"
    };

    // Write root inode & dirent
    fseeko(image, BSFS_INODE_OFFSET(&header, root_inode_idx), SEEK_SET);
    fwrite(&root_inode, sizeof(bsfs_inode_t), 1, image);

    fseeko(image, root_dirent_offset, SEEK_SET);
    fwrite(&root_dirent, sizeof(bsfs_dirent_t), 1, image);

    // Write out the bitmaps
    fseeko(image, header.block_bitmap_start * header.block_size, SEEK_SET);
    fwrite(block_bitmap, 1, header.block_bitmap_blocks * header.block_size, image);
    
    fseeko(image, header.inode_bitmap_start * header.block_size, SEEK_SET);
    fwrite(inode_bitmap, 1, header.inode_bitmap_blocks * header.block_size, image);
    
    fseeko(image, header.inode_table_start * header.block_size, SEEK_SET);
    fwrite((uint8_t*)inode_table, 1, header.inode_table_blocks * header.block_size, image);
    
    free(block_bitmap);
    free(inode_bitmap);
    free(inode_table);
    // free(dirents);

    fclose(image);
}

void populate_dir(const char *path, FILE *image, bsfs_inode_t *inode, bsfs_header_t *header, uint8_t *block_bitmap, uint32_t block_bitmap_size, uint8_t *inode_bitmap, uint32_t inode_bitmap_size, bsfs_inode_t *inode_table) {
    DIR *dir = opendir(path);
    if (!dir) {
        BSFS_PANIC("Unable to open path '%s'!", path);
        return;
    }
    
    struct dirent *entry;
    
    uint64_t populate_time = time(NULL);
    
    inode->type = BSFS_INODE_TYPE_DIRECTORY;
    inode->permissions = BSFS_INODE_PERM_READ | BSFS_INODE_PERM_WRITE;
    inode->created = populate_time;
    inode->modified = populate_time;
    inode->accessed = populate_time;
    
    while ((entry = readdir(dir)) != NULL) {
        if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0)
            continue;

        char subpath[PATH_MAX];
        snprintf(subpath, sizeof(subpath), "%s/%s", path, entry->d_name);

        if (entry->d_type == DT_DIR) {
            uint32_t subfolder_inode_idx = bsfs_alloc_inode(header, inode_bitmap, inode_bitmap_size);
            printf("Entering subfolder: '%s'(Inode: %u)\n", subpath, subfolder_inode_idx);

            uint64_t new_dirent_offset = bsfs_alloc_dirent(header, inode, block_bitmap, block_bitmap_size);
            if (new_dirent_offset == UINT64_MAX) {
                BSFS_PANIC("populate_dir: bsfs_alloc_dirent returned sentinel value when populating with path '%s'", subpath);
                return;
            }
            bsfs_dirent_t new_dirent;
            snprintf(new_dirent.name, sizeof(new_dirent.name), "%s", entry->d_name);
            new_dirent.inode = subfolder_inode_idx;
            fseeko(image, new_dirent_offset, SEEK_SET);
            fwrite(&new_dirent, sizeof(bsfs_dirent_t), 1, image);
            
            bsfs_inode_t subfolder_inode = inode_table[subfolder_inode_idx];
            populate_dir(subpath, image, &subfolder_inode, header, block_bitmap, block_bitmap_size, inode_bitmap, inode_bitmap_size, inode_table);

            fseeko(image, BSFS_INODE_OFFSET(header, subfolder_inode_idx), SEEK_SET);
            fwrite(&subfolder_inode, sizeof(bsfs_inode_t), 1, image);

            inode->size += sizeof(bsfs_dirent_t);
            printf("Exited subfolder: '%s' (Inode: %u), %lu dirents\n", subpath, subfolder_inode_idx, subfolder_inode.size / sizeof(bsfs_dirent_t));
        } else if (entry->d_type == DT_REG) {
            uint32_t file_inode_idx = bsfs_alloc_inode(header, inode_bitmap, inode_bitmap_size);
            inode_table[file_inode_idx].type = BSFS_INODE_TYPE_FILE;
            inode_table[file_inode_idx].permissions = BSFS_INODE_PERM_READ | BSFS_INODE_PERM_WRITE;
            inode_table[file_inode_idx].created = populate_time;
            inode_table[file_inode_idx].modified = populate_time;
            inode_table[file_inode_idx].accessed = populate_time;
            
            uint32_t blocks = 0;
            FILE *file = fopen(subpath, "rb");
            fseeko(file, 0, SEEK_END);
            uint64_t file_size = ftello(file);
            fseeko(file, 0, SEEK_SET);
            if (file_size / header->block_size > 10) {
                fclose(file);
                BSFS_PANIC("File size exceeds %.2f KiB, cannot fit into blocks_direct, unimplemented!", header->block_size * 10 / 1024.f);
                return;
            }
            
            printf("Writing file '%s' (Inode %u, %lu blocks): ", subpath, file_inode_idx, DIV_CEIL(file_size, header->block_size));
            uint8_t *bytes = malloc(header->block_size);
            size_t read = 0;
            while ((read = fread(bytes, 1, header->block_size, file))) {
                if (blocks == 10) {
                    // TODO: implement indirect blocks
                    fclose(file);
                    free(bytes);
                    BSFS_PANIC("File size exceeds %.2f KiB, cannot fit into blocks_direct, unimplemented!", header->block_size * 10 / 1024.f);
                    return;
                }
                uint32_t file_block_idx = bsfs_alloc_block(header, block_bitmap, block_bitmap_size);
                fseeko(image, header->block_size * file_block_idx, SEEK_SET);
                fwrite(bytes, 1, read, image);
                inode_table[file_inode_idx].blocks_direct[blocks] = file_block_idx;
                inode_table[file_inode_idx].blocks++;
                inode_table[file_inode_idx].size += read;
                blocks++;
                printf(".");
            }
            printf("\n");

            uint64_t new_dirent_offset = bsfs_alloc_dirent(header, inode, block_bitmap, block_bitmap_size);
            if (new_dirent_offset == UINT64_MAX) {
                BSFS_PANIC("populate_dir: bsfs_alloc_dirent returned sentinel value when populating with path '%s'", subpath);
                return;
            }
            bsfs_dirent_t new_dirent;
            snprintf(new_dirent.name, sizeof(new_dirent.name), "%s", entry->d_name);
            new_dirent.inode = file_inode_idx;
            fseeko(image, new_dirent_offset, SEEK_SET);
            fwrite(&new_dirent, sizeof(bsfs_dirent_t), 1, image);
            inode->size += sizeof(bsfs_dirent_t);

            fseeko(image, BSFS_INODE_OFFSET(header, file_inode_idx), SEEK_SET);
            fwrite(&inode_table[file_inode_idx], sizeof(bsfs_inode_t), 1, image);

            free(bytes);
            fclose(file);
        } else if (entry->d_type == DT_UNKNOWN) {
            BSFS_PANIC("d_type == DT_UNKNOWN\n");
            return;
        } else {
            printf("Info: Skipping '%s', unknown d_type('%02x')\n", entry->d_name, entry->d_type);
        }
    }

    closedir(dir);
}
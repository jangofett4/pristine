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
#include <sys/stat.h>
#include <limits.h>

#if defined(linux) || defined(__linux)
#include <sys/mman.h>
#endif

#include "bsfs.h"

#define ARRAY_LENGTH(x) (sizeof(x) / sizeof(x[0]))

void bsfs_extract_dir(const char *path, FILE *image_file, BsfsHeader *header, BsfsInode *current_guest_inode, uint32_t current_guest_inode_idx, BsfsInode *inode_table);
void bsfs_write_inode_to_file(FILE *image_file, FILE *host_file, BsfsHeader *header, BsfsInode *inode);

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

    BsfsHeader header;
    fseek(image, bsfs_header_idx, SEEK_SET);
    if (!fread(&header, sizeof(BsfsHeader), 1, image)) {
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
    printf(" Total Blocks:           %u\n", header.total_blocks);
    printf(" Free Blocks:            %u\n", header.free_blocks);
    printf(" Total Inodes:           %u\n", header.inode_count);
    printf(" Free Inodes:            %u\n", header.free_inodes);
    printf(" Block bitmap start:     0x%04x\n", header.block_bitmap_start);
    printf(" Megablock bitmap start: 0x%04x\n", header.megablock_bitmap_start);
    printf(" Inode bitmap start:     0x%04x\n", header.inode_bitmap_start);
    printf(" Inode table start:      0x%04x\n", header.inode_table_start);
    printf(" Inode table blocks:     0x%04x\n", header.inode_table_blocks);
    printf(" Data start:             0x%04x\n", header.data_start);
    printf(" Root inode:             %u\n", header.root_inode);
    printf(" Image Size:             %.2f KiB (calculated)\n", header.total_blocks * final_blocksize / 1024.f);

    uint32_t block_bitmap_size     = header.block_bitmap_blocks * header.block_size;
    uint32_t megablock_bitmap_size = header.megablock_bitmap_blocks * header.block_size;
    uint32_t inode_bitmap_size     = header.inode_bitmap_blocks * header.block_size;
    uint32_t inode_table_size      = header.inode_count * sizeof(BsfsInode);

    uint8_t *block_bitmap     = malloc(block_bitmap_size);
    uint8_t *megablock_bitmap = malloc(megablock_bitmap_size);
    uint8_t *inode_bitmap     = malloc(inode_bitmap_size);
    BsfsInode *inode_table = (BsfsInode*)malloc(inode_table_size);

#if defined(linux) || defined(__linux__)
    // As if we made the rest of the program cross platform...
    // Maybe tiny bit of extra performance
    madvise(block_bitmap, block_bitmap_size, MADV_WILLNEED);
    madvise(megablock_bitmap, block_bitmap_size, MADV_WILLNEED);
    madvise(inode_bitmap, inode_bitmap_size, MADV_WILLNEED);
    madvise(inode_table, inode_table_size, MADV_WILLNEED);
#endif

    fseeko(image, header.block_bitmap_start * header.block_size, SEEK_SET);
    fread(block_bitmap, 1, block_bitmap_size, image);

    fseeko(image, header.megablock_bitmap_start * header.block_size, SEEK_SET);
    fread(megablock_bitmap, 1, megablock_bitmap_size, image);

    fseeko(image, header.inode_bitmap_start * header.block_size, SEEK_SET);
    fread(inode_bitmap, 1, inode_bitmap_size, image);

    fseeko(image, header.inode_table_start * header.block_size, SEEK_SET);
    fread(inode_table, sizeof(BsfsInode), header.inode_count, image);

    BsfsInode root_inode = inode_table[header.root_inode];
    BsfsDirent *root_dirent = (BsfsDirent*)malloc(root_inode.size);

    fseeko(image, root_inode.blocks_direct[0] * header.block_size, SEEK_SET);
    fread(root_dirent, sizeof(BsfsDirent), root_inode.size / sizeof(BsfsDirent), image);

    mkdir(root_path, 0755);
    bsfs_extract_dir(root_path, image, &header, &root_inode, header.root_inode, inode_table);

    free(block_bitmap);
    free(inode_bitmap);
    free(inode_table);

    return 0;
}

void bsfs_extract_dir(const char *path, FILE *image_file, BsfsHeader *header, BsfsInode *current_guest_inode, uint32_t current_guest_inode_idx, BsfsInode *inode_table) {
    if (current_guest_inode->type != BSFS_INODE_TYPE_DIRECTORY) {
        BSFS_PANIC("bsfs_extract_dir: inode is not a directory (%u)", current_guest_inode_idx);
    }

    BsfsDirent *current_block_dirents = (BsfsDirent*)malloc(header->block_size);
    // TODO: for now we are only handling blocks_direct
    for (size_t i = 0; i < sizeof(current_guest_inode->blocks_direct) / sizeof(current_guest_inode->blocks_direct[0]); i++) {
        uint32_t current_block = current_guest_inode->blocks_direct[i];
        if (current_block == 0) {
            continue; // No more blocks
        }
        printf("Dirent block: %u\n", current_block);
        fseeko(image_file, header->block_size * current_block, SEEK_SET);
        fread(current_block_dirents, sizeof(BsfsDirent), header->block_size / sizeof(BsfsDirent), image_file);
        for (size_t d = 0; d < header->block_size / sizeof(BsfsDirent); d++) {
            BsfsDirent *current_dirent = &current_block_dirents[d];
            
            char subpath[PATH_MAX];
            snprintf(subpath, PATH_MAX, "%s/%s", path, current_dirent->name);
            printf("> Dirent: %zu, %s [%u]\n", d, current_dirent->name, current_dirent->inode);
            
            if (current_block_dirents[d].inode == BSFS_INODE_TYPE_FREE) {
                continue;
            }

            if (strcmp(current_dirent->name, ".") == 0 || strcmp(current_dirent->name, "..") == 0) {
                continue; // TODO: I don't even know if this is a proper fix
            }

            BsfsInode subinode = inode_table[current_dirent->inode];
            if (subinode.type == BSFS_INODE_TYPE_FREE) {
                continue;
            } else if (subinode.type == BSFS_INODE_TYPE_FILE) {
                printf(" - Inode: File, %.02f KiB\n", subinode.size / 1024.f);
                FILE *subfile = fopen(subpath, "wb+");
                if (!subfile) {
                    BSFS_PANIC("bsfs_extract_dir: unable to create file '%s' in host", subpath);
                }
                bsfs_write_inode_to_file(image_file, subfile, header, &subinode);
                fclose(subfile);
            } else if (subinode.type == BSFS_INODE_TYPE_DIRECTORY) {
                printf(" - Inode: Directory, %lu items\n", subinode.size / sizeof(BsfsDirent));
                mkdir(subpath, 0755);
                bsfs_extract_dir(subpath, image_file, header, &subinode, current_dirent->inode, inode_table);
            } else if (subinode.type == BSFS_INODE_TYPE_SYMLINK) {
                printf(" - Inode: Symlink\n");
            } else {
                BSFS_PANIC("bsfs_extract_dir: filesystem corruption! Unknown inode type '%i'", subinode.type);
            }
        }
    }

    free(current_block_dirents);
}

void bsfs_write_inode_to_file(FILE *image_file, FILE *host_file, BsfsHeader *header, BsfsInode *inode) {
    if (inode->type != BSFS_INODE_TYPE_FILE) {
        BSFS_PANIC("bsfs_write_inode_to_file: called with non-file inode");
    }
    
    fseeko(host_file, 0, SEEK_SET);
    uint8_t *data = malloc(header->block_size);
    int64_t remaining = inode->size;
    bool file_end = false;
    for (size_t i = 0; i < ARRAY_LENGTH(inode->blocks_direct); i++) {
        if (inode->blocks_direct[i] == 0) {
            continue;
        }
        printf("Reading inode block 0x%04x, ", inode->blocks_direct[i]);
        fseeko(image_file, header->block_size * inode->blocks_direct[i], SEEK_SET);
        uint32_t read = fread(data, 1, header->block_size, image_file);
        printf("read %u from image bytes, remaining %lu\n", read, remaining);
        if (read < header->block_size) {
            file_end = true;
            fwrite(data, 1, read, host_file);
            break;
        } else {
            uint32_t writecnt = remaining < header->block_size ? remaining : header->block_size;
            fwrite(data, 1, writecnt, host_file);
            remaining -= header->block_size;
        }
    }
    
    if (file_end) {
        free(data);
        return;
    }

    if (inode->blocks_l1indirect != 0) {
        printf("l1indirect exists\n");
        uint32_t *l1indirect_table_tmp = malloc(header->block_size);
        fseeko(image_file, header->block_size * inode->blocks_l1indirect, SEEK_SET);
        fread(l1indirect_table_tmp, sizeof(uint32_t), header->block_size / sizeof(uint32_t), image_file);
        for (size_t i = 0; i < header->block_size / sizeof(uint32_t) && remaining > 0; i++) {
            uint32_t current_block = l1indirect_table_tmp[i];
            if (!current_block) continue;
            printf("l1 block: %u\n", current_block);
            fseeko(image_file, header->block_size * current_block, SEEK_SET);
            uint32_t read = fread(data, 1, header->block_size, image_file);
            if (read < header->block_size) {
                file_end = true;
                fwrite(data, 1, read, host_file);
                break;
            } else {
                uint32_t writecnt = remaining < header->block_size ? remaining : header->block_size;
                fwrite(data, 1, writecnt, host_file);
                printf("written %u bytes\n", writecnt);
                remaining -= header->block_size;
            }
        }
        free(l1indirect_table_tmp);
    }

    if (file_end) {
        free(data);
        return;
    }
}
/*
 * Pristine
 * bsfs-populate - filesystem populate tool
 * SPDX-License-Identifier: MIT
 */

#include <linux/limits.h>
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

void bsfs_populate_dir(const char *path, FILE *image_fike, bsfs_header_t *header, bsfs_inode_t *current_guest_inode, uint32_t current_guest_inode_idx, uint8_t *block_bitmap, uint8_t *megablock_bitmap, uint8_t *inode_bitmap, bsfs_inode_t *inode_table);

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

    // reserve metadata blocks, we VERY MUCH would not like to overwrite them accidentally
    for (size_t i = 0; i < metadata_blocks; i++) {
        bsfs_bitmap_set(block_bitmap, i);
    }

    // reserve megablocks that contain data currently, later bsfs_alloc_block will handle it automatically
    for (uint32_t mb = 0; mb < total_megablocks; mb++) {
        uint8_t *slice = block_bitmap + mb * final_blocksize;
        bool full = true;
        for (uint32_t b = 0; b < final_blocksize; b++) {
            if (slice[b] != UINT8_MAX) { full = false; break; }
        }
        if (full)
            bsfs_bitmap_set(megablock_bitmap, mb);
    }

    uint32_t sentinel_inode_idx = bsfs_alloc_inode(&header, inode_bitmap);
    uint32_t root_inode_idx = bsfs_alloc_inode(&header, inode_bitmap);
    bsfs_inode_t *root_inode = &inode_table[root_inode_idx];
    header.root_inode = root_inode_idx;

    bsfs_populate_dir(root_path, image, &header, root_inode, root_inode_idx, block_bitmap, megablock_bitmap, inode_bitmap, inode_table);
    // TODO: it would be a good idea to allocate the first block of the root inode before populating
    // Currently it is allocated after bsfs_populate_dir, which means blocks if the first block is sitting deep inside the data blocks
    // It not critical, however it could still be beneficial, since it could be faster to access blocks at the start (spinning drives)

    fseeko(image, header.block_bitmap_start * header.block_size, SEEK_SET);
    fwrite(block_bitmap, 1, header.block_bitmap_blocks * header.block_size, image);

    fseeko(image, header.megablock_bitmap_start * header.block_size, SEEK_SET);
    fwrite(megablock_bitmap, 1, header.megablock_bitmap_blocks * header.block_size, image);

    fseeko(image, header.inode_bitmap_start * header.block_size, SEEK_SET);
    fwrite(inode_bitmap, 1, header.inode_bitmap_blocks * header.block_size, image);

    fseeko(image, header.inode_table_start * header.block_size, SEEK_SET);
    fwrite((uint8_t*)inode_table, 1, header.inode_table_blocks * header.block_size, image);

    fseeko(image, bsfs_header_idx, SEEK_SET);
    fwrite(&header, sizeof(bsfs_header_t), 1, image);

    free(block_bitmap);
    free(megablock_bitmap);
    free(inode_bitmap);
    free(inode_table);

    fclose(image);
    return 0;
}

void bsfs_populate_dir(const char *path, FILE *image_file, bsfs_header_t *header, bsfs_inode_t *current_guest_inode, uint32_t current_guest_inode_idx, uint8_t *block_bitmap, uint8_t *megablock_bitmap, uint8_t *inode_bitmap, bsfs_inode_t *inode_table) {
    DIR *current_dir = opendir(path);
    if (!current_dir) {
        BSFS_PANIC("bsfs_populate_dir: unable to open path '%s'", path);
    }
    struct dirent *current_host_dirent;

    time_t populate_time = time(NULL);

    current_guest_inode->type = BSFS_INODE_TYPE_DIRECTORY;
    current_guest_inode->permissions = BSFS_INODE_PERM_READ | BSFS_INODE_PERM_WRITE;
    current_guest_inode->uid = 0;
    current_guest_inode->gid = 0;
    current_guest_inode->size = 0;
    current_guest_inode->blocks = 0;
    current_guest_inode->created = populate_time;
    current_guest_inode->modified = populate_time;
    current_guest_inode->accessed = populate_time;
    current_guest_inode->link_count = 0;

    while ((current_host_dirent = readdir(current_dir))) {
        if (strcmp(current_host_dirent->d_name, ".") == 0 || strcmp(current_host_dirent->d_name, "..") == 0) {
            continue;
        }

        char subpath[PATH_MAX];
        snprintf(subpath, sizeof(subpath), "%s/%s", path, current_host_dirent->d_name);

        if (current_host_dirent->d_type == DT_DIR) {
            uint64_t guest_directory_dirent_offset = bsfs_alloc_dirent(header, current_guest_inode, block_bitmap, megablock_bitmap);
            uint32_t guest_subdirectory_inode_idx  = bsfs_alloc_inode(header, inode_bitmap);
            bsfs_inode_t *guest_subdirectory_inode = &inode_table[guest_subdirectory_inode_idx];

            current_guest_inode->size += sizeof(bsfs_dirent_t);
            bsfs_dirent_t guest_directory_dirent = {
                .inode = guest_subdirectory_inode_idx
            };
            snprintf(guest_directory_dirent.name, BSFS_MAX_DIRENTNAME, "%s", current_host_dirent->d_name);

            fseeko(image_file, guest_directory_dirent_offset, SEEK_SET);
            fwrite(&guest_directory_dirent, sizeof(bsfs_dirent_t), 1, image_file);

            bsfs_populate_dir(subpath, image_file, header, guest_subdirectory_inode, guest_subdirectory_inode_idx, block_bitmap, megablock_bitmap, inode_bitmap, inode_table);
        } else if (current_host_dirent->d_type == DT_REG) {
            uint64_t guest_file_dirent_offset = bsfs_alloc_dirent(header, current_guest_inode, block_bitmap, megablock_bitmap);
            uint32_t guest_file_inode_idx     = bsfs_alloc_inode(header, inode_bitmap);
            bsfs_inode_t *guest_file_inode    = &inode_table[guest_file_inode_idx];

            current_guest_inode->size += sizeof(bsfs_dirent_t);
            bsfs_dirent_t guest_file_dirent = {
                .inode = guest_file_inode_idx,
            };
            snprintf(guest_file_dirent.name, BSFS_MAX_DIRENTNAME, "%s", current_host_dirent->d_name);

            FILE *host_file = fopen(subpath, "rb");
            if (!host_file) {
                closedir(current_dir);
                BSFS_PANIC("bsfs_populate_dir: unable to open host file '%s'", subpath);
            }
            fseeko(host_file, 0, SEEK_END);
            uint64_t host_file_size = ftello(host_file);
            fseeko(host_file, 0, SEEK_SET);

            uint32_t guest_file_total_blocks_required = DIV_CEIL(host_file_size, header->block_size);

            guest_file_inode->type = BSFS_INODE_TYPE_FILE;
            guest_file_inode->permissions = BSFS_INODE_PERM_READ | BSFS_INODE_PERM_WRITE;
            guest_file_inode->uid = 0;
            guest_file_inode->gid = 0;
            guest_file_inode->size = 0;
            guest_file_inode->created = populate_time;
            guest_file_inode->modified = populate_time;
            guest_file_inode->accessed = populate_time;
            guest_file_inode->link_count = 0;

            uint32_t l1_capacity      = header->block_size / sizeof(uint32_t);                                                // 1024
            uint32_t l1indirect_start = sizeof(guest_file_inode->blocks_direct) / sizeof(guest_file_inode->blocks_direct[0]); // 10
            uint32_t l2indirect_start = l1indirect_start + l1_capacity;                                                       // 10 + 1024 = 1034
            uint32_t l3indirect_start = l2indirect_start + l1_capacity * l1_capacity;                                         // 1034 + (1024 * 1024) = 1049610

            bool needs_l1indirect = guest_file_total_blocks_required >= l1indirect_start;
            bool needs_l2indirect = guest_file_total_blocks_required >= l2indirect_start;
            bool needs_l3indirect = guest_file_total_blocks_required >= l3indirect_start;

            printf("Writing file '%s' (Inode %u, %u blocks, L1: %i, L2: %i, L3: %i)...",
                subpath,
                guest_file_inode_idx,
                guest_file_total_blocks_required,
                needs_l1indirect,
                needs_l2indirect,
                needs_l3indirect
            );

            uint8_t *bytes = malloc(header->block_size);
            size_t read = 0;
            uint32_t guest_file_block_idx;
            uint32_t guest_file_current_block = 0;
            uint32_t *l1indirect_table_tmp = NULL; uint32_t l1indirect_table_idx;
            uint32_t *l2indirect_table_tmp = NULL; uint32_t l2indirect_table_idx;
            uint32_t *l3indirect_table_tmp = NULL; uint32_t l3indirect_table_idx;

            while ((read = fread(bytes, 1, header->block_size, host_file))) {
                // TODO: this could be sped up massively with bsfs_alloc_block_contiguous
                guest_file_block_idx = bsfs_alloc_block(header, block_bitmap, megablock_bitmap);
                fseeko(image_file, header->block_size * guest_file_block_idx, SEEK_SET);

                if (guest_file_current_block < l1indirect_start) {
                    guest_file_inode->blocks_direct[guest_file_current_block] = guest_file_block_idx;
                    fseeko(image_file, header->block_size * guest_file_block_idx, SEEK_SET);
                    fwrite(bytes, 1, read, image_file);
                } else if (guest_file_current_block < l2indirect_start) {
                    uint32_t l1slot = guest_file_current_block - l1indirect_start;
                    if (l1slot == 0) {
                        l1indirect_table_idx = bsfs_alloc_block(header, block_bitmap, megablock_bitmap);
                        guest_file_inode->blocks_l1indirect = l1indirect_table_idx;
                        l1indirect_table_tmp = calloc(1, header->block_size);
                    }
                    l1indirect_table_tmp[l1slot] = guest_file_block_idx;
                    fseeko(image_file, header->block_size * guest_file_block_idx, SEEK_SET);
                    fwrite(bytes, 1, read, image_file);
                } else if (guest_file_current_block < l3indirect_start) {
                    fclose(image_file);
                    fclose(host_file);
                    closedir(current_dir);
                    free(bytes);
                    BSFS_PANIC("populate_dir: file '%s' with size %.2f MiB exceeds blocks_l1indirect, blocks_l2indirect is unimplemented", subpath, host_file_size / 1024.f / 1024);
                } else {
                    fclose(image_file);
                    fclose(host_file);
                    closedir(current_dir);
                    free(bytes);
                    BSFS_PANIC("populate_dir: file '%s' with size %.2f MiB exceeds blocks_l2indirect, blocks_l3indirect is unimplemented", subpath, host_file_size / 1024.f / 1024);
                }

                guest_file_inode->blocks++;
                guest_file_inode->size += read;
                guest_file_current_block++;
            }
            printf("\n");

            if (l1indirect_table_tmp) {
                fseeko(image_file, header->block_size * l1indirect_table_idx, SEEK_SET);
                fwrite(l1indirect_table_tmp, sizeof(uint32_t), l1_capacity, image_file);
                free(l1indirect_table_tmp);
                l1indirect_table_tmp = NULL;
            }

            fseeko(image_file, guest_file_dirent_offset, SEEK_SET);
            fwrite(&guest_file_dirent, sizeof(bsfs_dirent_t), 1, image_file);
        } else if (current_host_dirent->d_type == DT_UNKNOWN) {
            closedir(current_dir);
            BSFS_PANIC("bsfs_populate_dir: host returned DT_UNKNOWN for dirent.d_type, stat() based navigation is unimplemented");
        } else {
            closedir(current_dir);
            BSFS_PANIC("bsfs_populate_dir: unhandled dirent type (%i)", current_host_dirent->d_type);
        }
    }

    closedir(current_dir);
}

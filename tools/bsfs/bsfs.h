/*
 * BSFS
 * SPDX-License-Identifier: MIT
 */

#pragma once

#include <stdint.h>
#include <stdbool.h>

#define BSFS_MAGIC      0x42534653
#define BSFS_VERSION    0x01010100

#define BSFS_MAX_BLOCKS     1073741824
#define BSFS_MAX_MEGABLOCKS 1048576

typedef struct {
    uint8_t  bootjmp[3];
    uint32_t magic;
    uint32_t version;
    uint32_t block_size;
    uint32_t total_blocks;
    uint32_t inode_count;
    uint32_t free_blocks;
    uint32_t free_inodes;
    uint32_t block_bitmap_start;
    uint32_t block_bitmap_blocks;
    uint32_t inode_bitmap_start;
    uint32_t inode_bitmap_blocks;
    uint32_t inode_table_start;
    uint32_t inode_table_blocks;
    uint32_t data_start;
    uint32_t root_inode;
    uint64_t partition_lba;
    uint8_t  label[32];
    uint8_t  bootcode[407];
    uint8_t  bootcode_magic[2];
} __attribute__((packed)) bsfs_header_t;

#define BSFS_INODE_TYPE_FREE        0
#define BSFS_INODE_TYPE_FILE        1
#define BSFS_INODE_TYPE_DIRECTORY   2
#define BSFS_INODE_TYPE_SYMLINK     3

#define BSFS_INODE_PERM_EXEC  0x01
#define BSFS_INODE_PERM_WRITE 0x02
#define BSFS_INODE_PERM_READ  0x04

typedef struct {
    uint8_t  type;
    uint16_t permissions;
    uint32_t uid;
    uint32_t gid;
    uint64_t size;              // Size in bytes
    uint32_t blocks;            // Total allocated blocks
    uint64_t created;           // Creation date, unix timestamp
    uint64_t modified;          // Modification date, unix timestamp
    uint64_t accessed;          // Access date, unix timestamp
    uint32_t link_count;
    uint32_t blocks_direct[10]; // 40k
    uint32_t blocks_l1indirect; // 4MiB
    uint32_t blocks_l2indirect; // 4GiB
    uint32_t blocks_l3indirect; // 4TiB
    uint8_t  reserved[23];
} __attribute__((packed)) bsfs_inode_t;

typedef struct {
    uint32_t inode;
    uint8_t  name[124];
} __attribute__((packed)) bsfs_dirent_t;

bool bsfs_block_is_free(const bsfs_header_t *header, uint32_t block_id);
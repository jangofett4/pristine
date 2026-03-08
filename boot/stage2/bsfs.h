/*
 * BSFS
 * SPDX-License-Identifier: MIT
 */

#pragma once

#include "stage2_common.h"

#define BSFS_MAGIC      0x42534653
#define BSFS_VERSION    0x01020100

#define BSFS_MAX_BLOCKS       1073741824
#define BSFS_MAX_MEGABLOCKS   1048576
#define BSFS_MAX_DIRECTBLOCKS 10
#define BSFS_MAX_DIRENTNAME   124

#define BSFS_BLOCKSIZE        4096

_Static_assert(BSFS_MAX_DIRECTBLOCKS <= 35, "bsfs: too BSFS_MAX_DIRECTBLOCKS is too high, padding bytes count underflow!");
_Static_assert(BSFS_MAX_DIRENTNAME == 124, "bsfs: BSFS_MAX_DIRENTNAME is not 124"); // TODO: this is very limiting

#define BSFS_MAX_PATH       4096

#ifndef BSFS_DIV_CEIL
#define BSFS_DIV_CEIL(a, b) (((a) + (b) - 1) / (b))
#endif

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
    uint32_t megablock_bitmap_start;
    uint32_t megablock_bitmap_blocks;
    uint32_t megablock_bitmap_size;
    uint32_t inode_bitmap_start;
    uint32_t inode_bitmap_blocks;
    uint32_t inode_table_start;
    uint32_t inode_table_blocks;
    uint32_t data_start;
    uint32_t root_inode;
    uint64_t partition_lba;
    uint8_t  label[32];
    uint8_t  bootcode[395];
    uint8_t  bootcode_magic[2];
} __attribute__((packed)) BsfsHeader;

#define BSFS_INODE_TYPE_FREE        0
#define BSFS_INODE_TYPE_FILE        1
#define BSFS_INODE_TYPE_DIRECTORY   2
#define BSFS_INODE_TYPE_SYMLINK     3

#define BSFS_INODE_PERM_EXEC  0x01
#define BSFS_INODE_PERM_WRITE 0x02
#define BSFS_INODE_PERM_READ  0x04

#define BSFS_INODE_NULL 0
#define BSFS_INODE_ROOT 1

typedef struct {
    uint8_t  type;
    uint16_t permissions;
    uint32_t uid;
    uint32_t gid;
    uint64_t size;                                 // Size in bytes
    uint32_t blocks;                               // Total allocated blocks
    uint64_t created;                              // Creation date, unix timestamp
    uint64_t modified;                             // Modification date, unix timestamp
    uint64_t accessed;                             // Access date, unix timestamp
    uint32_t link_count;
    uint32_t blocks_direct[BSFS_MAX_DIRECTBLOCKS]; // 40k with 10 blocks
    uint32_t blocks_l1indirect;                    // 4MiB
    uint32_t blocks_l2indirect;                    // 4GiB
    uint32_t blocks_l3indirect;                    // 4TiB
    uint8_t  reserved[35 - BSFS_MAX_DIRECTBLOCKS];
} __attribute__((packed)) BsfsInode;

typedef struct {
    uint32_t inode;
    char     name[BSFS_MAX_DIRENTNAME];
} __attribute__((packed)) BsfsDirent;

void bsfs_bitmap_set(uint8_t *bitmap, const uint32_t index);
void bsfs_bitmap_clear(uint8_t *bitmap, const uint32_t index);

int bsfs_bitmap_test(uint8_t *bitmap, const uint32_t index);

uint32_t bsfs_bitmap_find(uint8_t *bitmap, const uint32_t bit_count);

// Searches and allocates a block based on header given.
// Requires block and megablock bitmap to be passed.
// Panics when no free block is found.
uint32_t bsfs_alloc_block(BsfsHeader *header, uint8_t *block_bitmap, uint8_t *megablock_bitmap);

uint32_t bsfs_alloc_inode(BsfsHeader *header, uint8_t *inode_bitmap);

// "allocates" a bsfs_dirent_t
uint64_t bsfs_alloc_dirent(BsfsHeader *header, BsfsInode *inode, uint8_t *block_bitmap, uint8_t *megablock_bitmap);
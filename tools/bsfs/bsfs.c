/*
 * BSFS
 * bsfs - reference implementation and toolset support for BSFS filesystem
 * SPDX-License-Identifier: MIT
 */

#include "bsfs.h"

#include <stdio.h>

void bsfs_bitmap_set(uint8_t *bitmap, uint32_t index) {
#ifdef BSFS_DEBUG
    if (bsfs_bitmap_test(bitmap, index)) {
        BSFS_PANIC("bsfs_bitmap_set: index %u is already set", index);
    }
#endif
    bitmap[index / 8] |= 1 << (index % 8);
}

void bsfs_bitmap_clear(uint8_t *bitmap, uint32_t index) {
#ifdef BSFS_DEBUG
    if (!bsfs_bitmap_test(bitmap, index)) {
        BSFS_PANIC("bsfs_bitmap_clear: index %u is already clear", index);
    }
#endif
    bitmap[index / 8] &= ~(1 << (index % 8));
}

bool bsfs_bitmap_test(uint8_t *bitmap, uint32_t index) {
    return bitmap[index / 8] & (1 << (index % 8));
}

uint32_t bsfs_bitmap_find(uint8_t *bitmap, uint32_t size) {
    for (size_t i = 0; i < size; i++) {
        if (!(bitmap[i / 8] & (1 << (i % 8)))) {
            return i;
        }
    }
    return UINT32_MAX;
}

uint32_t bsfs_alloc_block(bsfs_header_t *header, uint8_t *block_bitmap, uint32_t block_bitmap_size) {
    uint32_t free_block = bsfs_bitmap_find(block_bitmap, block_bitmap_size);
    if (free_block == UINT32_MAX) {
        BSFS_PANIC("No free block remain on image!");
        return UINT32_MAX;
    }
    bsfs_bitmap_set(block_bitmap, free_block);
    header->free_blocks--;
    return free_block;
}

uint32_t bsfs_alloc_inode(bsfs_header_t *header, uint8_t *inode_bitmap, uint32_t inode_bitmap_size) {
    uint32_t free_inode = bsfs_bitmap_find(inode_bitmap, inode_bitmap_size);
    if (free_inode == UINT32_MAX) {
        BSFS_PANIC("No free block remain on image!");
        return UINT32_MAX;
    }
    bsfs_bitmap_set(inode_bitmap, free_inode);
    header->free_inodes--;
    return free_inode;
}

// TODO: We can add a new "empty dirent slot" to bsfs_inode_t to keep track or next empty dirent in inode to save some time

uint64_t bsfs_alloc_dirent(bsfs_header_t *header, bsfs_inode_t *inode, uint8_t *block_bitmap, uint32_t block_bitmap_size) {
    #ifdef BSFS_DEBUG
    if (inode->type != BSFS_INODE_TYPE_DIRECTORY) {
        BSFS_PANIC("bsfs_alloc_direct: called with non-directory inode.");
        return UINT64_MAX;
    }
    #endif
    // This function should return an offset on the image so that the caller can't fail
    
    uint64_t used_dirents      = inode->size / sizeof(bsfs_dirent_t);
    uint64_t dirents_per_block = header->block_size / sizeof(bsfs_dirent_t);
    uint64_t current_block     = used_dirents / dirents_per_block;
    uint32_t slot_in_block     = used_dirents % dirents_per_block;

    // scan existing blocks for a free slot
    // TODO: We need to handle deleted inodes.
    /*
    for (uint32_t i = 0; i < inode->blocks; i++) {
        uint64_t block_offset = header->block_size * inode->blocks_direct[i];
        for (uint32_t j = 0; j < dirents_per_block; j++) {
            uint64_t dirent_offset = block_offset + j * sizeof(bsfs_dirent_t);
            // read dirent.inode, if 0 this slot is free
            if (slot_inode == 0) return dirent_offset;
        }
    }
    */

    if (inode->blocks == 0) {
        // first dirent in directory
        uint32_t block_idx = bsfs_alloc_block(header, block_bitmap, block_bitmap_size);
        inode->blocks_direct[0] = block_idx;
        inode->blocks++;
    } else if (slot_in_block == 0) {
        // current block is full, allocate a new one
        if (inode->blocks == (sizeof(inode->blocks_direct) / sizeof(inode->blocks_direct[0]))) {
            BSFS_PANIC("bsfs_alloc_dirent: Inode contains more than 10 blocks, unimplemented.");
            return UINT64_MAX;
        }
        uint32_t block_idx = bsfs_alloc_block(header, block_bitmap, block_bitmap_size);
        inode->blocks_direct[current_block] = block_idx;
        inode->blocks++;
    }

    return (header->block_size * inode->blocks_direct[current_block]) + (slot_in_block * sizeof(bsfs_dirent_t));
}
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

// TODO: These 2 can be improved marginally with just using uint32_t or uint64_t

void bsfs_bitmap_set_contiguous(uint8_t *bitmap, const uint64_t area_bit_count, const uint64_t index) {
    for (size_t i = 0; i < area_bit_count; i++) {
#ifdef BSFS_DEBUG
        if (bsfs_bitmap_test(bitmap, index + i)) {
            BSFS_PANIC("bsfs_bitmap_set_contiguous: index %lu is already set", index + i);
        }
#endif
        bitmap[(index + i) / 8] |= 1 << ((index + i) % 8);
    }
}

void bsfs_bitmap_clear_contiguous(uint8_t *bitmap, const uint64_t area_bit_count, const uint64_t index) {
    for (size_t i = 0; i < area_bit_count; i++) {
#ifdef BSFS_DEBUG
        if (!bsfs_bitmap_test(bitmap, index + i)) {
            BSFS_PANIC("bsfs_bitmap_clear_contiguous: index %lu is already cleared", index + i);
        }
#endif
        bitmap[(index + i) / 8] &= ~(1 << ((index + i) % 8));
    }
}

bool bsfs_bitmap_test(uint8_t *bitmap, uint32_t index) {
    return bitmap[index / 8] & (1 << (index % 8));
}

uint32_t bsfs_bitmap_find(uint8_t *bitmap, uint32_t bit_count) {
    uint64_t *fptr = (uint64_t*)bitmap;
    size_t outer;
    for (outer = 0; outer < bit_count / (sizeof(uint64_t) * 8); outer++) {
        if (fptr[outer] == UINT64_MAX) continue;
        else break;
    }
    for (size_t i = outer * (sizeof(uint64_t) * 8); i < bit_count; i++) {
        if (!(bitmap[i / 8] & (1 << (i % 8)))) {
            return i;
        }
    }
    return UINT32_MAX;
}

uint32_t bsfs_bitmap_find_contiguous(uint8_t *bitmap, const uint32_t area_count, const uint32_t bit_count) {
    for (uint32_t i = 0; i < bit_count; i++) {
        if (!(bitmap[i / 8] & (1 << (i % 8)))) {
            uint32_t j;
            for (j = 1; j < area_count && (i + j) < bit_count; j++) {
                if (bitmap[(i + j) / 8] & (1 << ((i + j) % 8))) {
                    break; // run broken by a set bit
                }
            }
            if (j == area_count)
                return i; // found a contiguous run starting at i
            i += j; // skip past the broken run
        }
    }
    return UINT32_MAX;
}

uint32_t bsfs_alloc_block(bsfs_header_t *header, uint8_t *block_bitmap) {
    uint32_t free_block = bsfs_bitmap_find(block_bitmap, header->total_blocks);
    if (free_block == UINT32_MAX) {
        BSFS_PANIC("bsfs_alloc_block: No free block remain on image!");
        return UINT32_MAX;
    }
    bsfs_bitmap_set(block_bitmap, free_block);
    header->free_blocks--;
    return free_block;
}

uint32_t bsfs_alloc_block_contiguous(bsfs_header_t *header, const uint32_t area_count, uint8_t *block_bitmap) {
    uint32_t free_blocks = bsfs_bitmap_find_contiguous(block_bitmap, area_count, header->total_blocks);
    if (free_blocks != UINT32_MAX) {
        bsfs_bitmap_set_contiguous(block_bitmap, area_count * 8, free_blocks);
        header->free_blocks -= free_blocks;
        return free_blocks;
    }

    return UINT32_MAX;
}

uint32_t bsfs_alloc_inode(bsfs_header_t *header, uint8_t *inode_bitmap) {
    uint32_t free_inode = bsfs_bitmap_find(inode_bitmap, header->inode_count);
    if (free_inode == UINT32_MAX) {
        BSFS_PANIC("bsfs_alloc_inode: No free inode remain on image!");
        return UINT32_MAX;
    }
    bsfs_bitmap_set(inode_bitmap, free_inode);
    header->free_inodes--;
    return free_inode;
}

// TODO: We can add a new "empty dirent slot" to bsfs_inode_t to keep track or next empty dirent in inode to save some time

uint64_t bsfs_alloc_dirent(bsfs_header_t *header, bsfs_inode_t *inode, uint8_t *block_bitmap) {
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
        uint32_t block_idx = bsfs_alloc_block(header, block_bitmap);
        inode->blocks_direct[0] = block_idx;
        inode->blocks++;
    } else if (slot_in_block == 0) {
        // current block is full, allocate a new one
        if (inode->blocks == (sizeof(inode->blocks_direct) / sizeof(inode->blocks_direct[0]))) {
            BSFS_PANIC("bsfs_alloc_dirent: Inode contains more than 10 blocks, unimplemented.");
            return UINT64_MAX;
        }
        uint32_t block_idx = bsfs_alloc_block(header, block_bitmap);
        inode->blocks_direct[current_block] = block_idx;
        inode->blocks++;
    }

    return (header->block_size * inode->blocks_direct[current_block]) + (slot_in_block * sizeof(bsfs_dirent_t));
}
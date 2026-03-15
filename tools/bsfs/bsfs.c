/*
 * BSFS
 * bsfs - reference implementation and toolset support for BSFS filesystem
 * SPDX-License-Identifier: MIT
 */

#include <bsfs.h>
#include <stdint.h>
#include <stdio.h>

inline void bsfs_bitmap_set(uint8_t *bitmap, const uint32_t index) {
#ifdef BSFS_DEBUG
    if (bsfs_bitmap_test(bitmap, index)) {
        BSFS_PANIC("bsfs_bitmap_set: index %u is already set", index);
    }
#endif
    const uint32_t slot = index / 8;
    bitmap[slot] |= 1 << (index % 8);
}

inline void bsfs_bitmap_clear(uint8_t *bitmap, const uint32_t index) {
    #ifdef BSFS_DEBUG
    if (!bsfs_bitmap_test(bitmap, index)) {
        BSFS_PANIC("bsfs_bitmap_clear: index %u is already clear", index);
    }
    #endif
    const uint32_t slot = index / 8;
    bitmap[slot] &= ~(1 << (index % 8));
}

inline int bsfs_bitmap_test(uint8_t *bitmap, const uint32_t index) {
    const uint32_t slot = index / 8;
    return bitmap[slot] & (1 << (index % 8));
}

uint32_t bsfs_bitmap_find(uint8_t *bitmap, const uint32_t bit_count)
{
    const uint64_t *ptr = (const uint64_t*)bitmap;
    uint32_t word_count = bit_count / 64;
    for (uint32_t i = 0; i < word_count; i++) {
        uint64_t word = ptr[i];
        if (word != UINT64_MAX)
            return i * 64 + __builtin_ctzll(~word);
    }
    // Handle remaining bits (partial last word)
    uint32_t remaining = bit_count % 64;
    if (remaining) {
        uint64_t word = ptr[word_count];
        uint64_t mask = (1ULL << remaining) - 1; // only valid bits
        word |= ~mask; // mask out bits beyond bit_count as "used"
        if (word != UINT64_MAX)
            return word_count * 64 + __builtin_ctzll(~word);
    }
    return UINT32_MAX;
}


uint32_t bsfs_alloc_block(BsfsHeader *header, uint8_t *block_bitmap, uint8_t *megablock_bitmap) {
    uint32_t free_megablock = bsfs_bitmap_find(megablock_bitmap, header->megablock_bitmap_size * 8);
    if (free_megablock == UINT32_MAX) {
        // TODO: in this case we might still have some free blocks in the last megablock.
        // Those can be used instead of just throwing an error.
        BSFS_PANIC("bsfs_alloc_block: No free megablocks remain on image!");
    }

    // uint32_t free_block_offset = free_megablock * 8;
    uint32_t base_block = free_megablock * header->block_size * 8;
    uint32_t free_block = bsfs_bitmap_find(
        block_bitmap + free_megablock * header->block_size,
        header->block_size * 8
    );
    if (free_block == UINT32_MAX) {
        BSFS_PANIC("bsfs_alloc_block: No free block remain on image!");
        return UINT32_MAX;
    }
    free_block += base_block; // convert local block index to absolute block index
    #ifdef BSFS_DEBUG
    if (free_block < header->data_start) {
        BSFS_PANIC("bsfs_alloc_block: allocated metadata block %u!", free_block);
    }
    #endif

    bsfs_bitmap_set(block_bitmap, free_block);
    uint8_t *mb_bitmap_slice = block_bitmap + free_megablock * header->block_size;
    int megablock_full = 1;
    for (size_t i = 0; i < header->block_size; i++) {
        if (mb_bitmap_slice[i] != UINT8_MAX) { megablock_full = 0; break; }
    }
    if (megablock_full)
        bsfs_bitmap_set(megablock_bitmap, free_megablock);

    if (header->free_blocks == 0) // This shouldn't happen
        BSFS_PANIC("No more free blocks left while allocating a new one");
    header->free_blocks--;
    return free_block;
}

uint32_t bsfs_alloc_inode(BsfsHeader *header, uint8_t *inode_bitmap) {
    uint32_t free_inode = bsfs_bitmap_find(inode_bitmap, header->inode_count);
    if (free_inode == UINT32_MAX) {
        BSFS_PANIC("bsfs_alloc_inode: No free inode remain on image!");
        return UINT32_MAX;
    }
    bsfs_bitmap_set(inode_bitmap, free_inode);
    header->free_inodes--;
    return free_inode;
}

uint64_t bsfs_alloc_dirent(BsfsHeader *header, BsfsInode *inode, uint8_t *block_bitmap, uint8_t *megablock_bitmap)
{
    #ifdef BSFS_DEBUG
    if (inode->type != BSFS_INODE_TYPE_DIRECTORY) {
        BSFS_PANIC("bsfs_alloc_dirent: called with non-directory inode.");
        return UINT64_MAX;
    }
    #endif

    // search for empty dirent in inode
    uint32_t new_dirent_local_block = inode->size / header->block_size; // Nth direct block
    if (new_dirent_local_block >= BSFS_MAX_DIRECTBLOCKS) {
        BSFS_PANIC("bsfs_alloc_dirent: new dirent exceeds maximum direct blocks of inode.");
        return UINT64_MAX;
    }
    if (inode->blocks_direct[new_dirent_local_block] == 0) {
        inode->blocks_direct[new_dirent_local_block] = bsfs_alloc_block(header, block_bitmap, megablock_bitmap);
        inode->blocks++;
    }
    uint32_t new_dirent_offset = inode->size % header->block_size; // Byte offset in the determined block
    return header->block_size * inode->blocks_direct[new_dirent_local_block] + new_dirent_offset;
}

// TODO: We can add a new "empty dirent slot" to BsfsInode to keep track or next empty dirent in inode to save some time

void bsfs_bitmap_set_contiguous(uint8_t *bitmap, const uint64_t area_bit_count, const uint64_t index) {
    BSFS_PANIC("bsfs_bitmap_set_contiguous: unimplemented");
}

void bsfs_bitmap_clear_contiguous(uint8_t *bitmap, const uint64_t area_bit_count, const uint64_t index) {
    BSFS_PANIC("bsfs_bitmap_clear_contiguous: unimplemented");
}

uint32_t bsfs_bitmap_find_contiguous(uint8_t *bitmap, const uint32_t area_count, const uint32_t bit_count) {
    BSFS_PANIC("bsfs_bitmap_find_contiguous: unimplemented");
    return UINT32_MAX;
}

uint32_t bsfs_alloc_block_contiguous(BsfsHeader *header, const uint32_t area_count, uint8_t *block_bitmap) {
    BSFS_PANIC("bsfs_alloc_block_contiguous: unimplemented");
    return UINT32_MAX;
}

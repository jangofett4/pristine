/*
 * Pristine
 * bitmap: bitmap operations
 * SPDX-License-Identifier MIT
 */

#include <bitmap.h>
#include <common/string.h>

inline void bitmap_clear_all(uint8_t *bitmap, const uint32_t size) {
    memset(bitmap, 0, size);
}

inline void bitmap_set(uint8_t *bitmap, const uint32_t index) {
    const uint32_t slot = index / 8;
    bitmap[slot] |= 1 << (index % 8);
}

inline void bitmap_clear(uint8_t *bitmap, const uint32_t index) {
    const uint32_t slot = index / 8;
    bitmap[slot] &= ~(1 << (index % 8));
}

inline int bitmap_test(uint8_t *bitmap, const uint32_t index) {
    const uint32_t slot = index / 8;
    return bitmap[slot] & (1 << (index % 8));
}

uint32_t bitmap_find(uint8_t *bitmap, const uint32_t bit_count)
{
    const uint64_t *ptr = (const uint64_t*)bitmap;
    uint32_t word_count = bit_count / 64;
    for (uint32_t i = 0; i < word_count; i++) {
        uint64_t word = ptr[i];
        if (word != UINT64_MAX)
            return i * 64 + __builtin_ctzll(~word);
    }
    uint32_t remaining = bit_count % 64;
    if (remaining) {
        uint64_t word = ptr[word_count];
        uint64_t mask = (1ULL << remaining) - 1;
        word |= ~mask;
        if (word != UINT64_MAX)
            return word_count * 64 + __builtin_ctzll(~word);
    }
    return UINT32_MAX;
}
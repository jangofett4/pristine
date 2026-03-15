/*
 * Pristine
 * arena: arena allocator
 * SPDX-License-Identifier: MIT
 */

#include <common/arena.h>

static uint8_t  _arena_buf[ARENA_SIZE];
static uint32_t _arena_offset = 0;

static inline size_t arena_align_up(size_t val, size_t align) {
    return (val + align - 1) & ~(align - 1);
}

void *arena_alloc(size_t size, size_t align) {
    size_t aligned = arena_align_up(_arena_offset, align);
    if (aligned + size > ARENA_SIZE) {
        return 0;
    }
    _arena_offset = aligned + size;
    return (void*)(_arena_buf + aligned);
}

void arena_reset(void) {
    _arena_offset = 0;
}
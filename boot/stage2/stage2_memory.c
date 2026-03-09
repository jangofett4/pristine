/*
 * Pristine
 * stage2_memory - memory functions
 * SPDX-License-Identifier: MIT
 */

// TODO: I should probably use builtin functions instead

#include "stage2_memory.h"

// Very crude memcpy, using uint32_t (or bigger) would be better
void memcpy(void* dst, const void* src, size_t count) {
    uint8_t* dst_ptr = (uint8_t*)dst;
    const uint8_t* src_ptr = (uint8_t*)src;
    for (size_t i = 0; i < count; i++)
        dst_ptr[i] = src_ptr[i];
}

void memcpy_i(void *dst, const void* src, size_t srcIdx, size_t dstIdx, size_t count) {
    uint8_t *dst_ptr = (uint8_t*)dst;
    const uint8_t *src_ptr = (const uint8_t*)src;
    
    size_t endIdx = srcIdx + count;
    for (; srcIdx < endIdx; srcIdx++, dstIdx++) {
        dst_ptr[dstIdx] = src_ptr[srcIdx];
    }
}

void memset(void *dst, uint8_t data, size_t count) {
    uint8_t *dst_ptr = (uint8_t*)dst;
    for (size_t i = 0; i < count; i++)
        dst_ptr[i] = data;
}

void memset_i(void *dst, uint8_t data, size_t dstIdx, size_t count) {
    uint8_t *dst_ptr = (uint8_t*)dst;
    for (size_t i = 0; i < count; i++)
        dst_ptr[dstIdx++] = data;
}

// 16 bit

void memcpy16(void* dst, const void* src, size_t count) {
    uint16_t* dst_ptr = (uint16_t*)dst;
    const uint16_t* src_ptr = (const uint16_t*)src;
    for (size_t i = 0; i < count; i++)
        dst_ptr[i] = src_ptr[i];
}

void memcpy16_i(void *dst, const void* src, size_t srcIdx, size_t dstIdx, size_t count) {
    uint16_t *dst_ptr = (uint16_t*)dst;
    const uint16_t *src_ptr = (const uint16_t*)src;
    
    size_t endIdx = srcIdx + count;
    for (; srcIdx < endIdx; srcIdx++, dstIdx++) {
        dst_ptr[dstIdx] = src_ptr[srcIdx];
    }
}

void memset16(void *dst, uint16_t data, size_t count) {
    uint16_t *dst_ptr = (uint16_t*)dst;
    for (size_t i = 0; i < count; i++)
        dst_ptr[i] = data;
}

void memset16_i(void *dst, uint16_t data, size_t dstIdx, size_t count) {
    uint16_t *dst_ptr = (uint16_t*)dst;
    for (size_t i = 0; i < count; i++)
        dst_ptr[dstIdx++] = data;
}

// 32 bit

void memcpy32(void* dst, const void* src, size_t count) {
    uint32_t* dst_ptr = (uint32_t*)dst;
    const uint32_t* src_ptr = (const uint32_t*)src;
    for (size_t i = 0; i < count; i++)
        dst_ptr[i] = src_ptr[i];
}

void memcpy32_i(void *dst, const void* src, size_t srcIdx, size_t dstIdx, size_t count) {
    uint32_t *dst_ptr = (uint32_t*)dst;
    const uint32_t *src_ptr = (const uint32_t*)src;
    
    size_t endIdx = srcIdx + count;
    for (; srcIdx < endIdx; srcIdx++, dstIdx++) {
        dst_ptr[dstIdx] = src_ptr[srcIdx];
    }
}

void memset32(void *dst, uint32_t data, size_t count) {
    uint32_t *dst_ptr = (uint32_t*)dst;
    for (size_t i = 0; i < count; i++)
        dst_ptr[i] = data;
}

void memset32_i(void *dst, uint32_t data, size_t dstIdx, size_t count) {
    uint32_t *dst_ptr = (uint32_t*)dst;
    for (size_t i = 0; i < count; i++)
        dst_ptr[dstIdx++] = data;
}

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
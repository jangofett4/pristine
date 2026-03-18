/*
 * Pristine
 * SPDX-License-Identifier MIT
 */

#pragma once

#include <stddef.h>
#include <stdint.h>

uint32_t crc32(const uint8_t *data, size_t length);
uint32_t crc32_update(uint32_t crc, const uint8_t *data, size_t length);

static inline uint32_t crc32_finalize(uint32_t crc) {
    return ~crc;
}
/*
 * Pristine
 * SPDX-License-Identifier: MIT
 */

#pragma once

#include <stdint.h>
#include <stddef.h>

#define DISK_READ_MAX_BLOCKS 32

typedef struct {
    uint8_t (*read) (uint32_t lba, size_t block_count, uint8_t *buf);
    uint8_t (*write)(uint32_t lba, size_t block_count, const uint8_t *buf);
} DiskOpsVtable;
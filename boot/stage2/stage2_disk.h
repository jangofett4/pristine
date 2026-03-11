/*
 * Pristine
 * SPDX-License-Identifier: MIT
 */

#pragma once

#include <stdint.h>
#include <stddef.h>

#define DISK_SECTOR_SIZE      512
#define DISK_READ_MAX_SECTORS 8

typedef struct {
    uint8_t (*read) (uint32_t lba, size_t sector_count, uint8_t *buf);
    uint8_t (*write)(uint32_t lba, size_t sector_count, const uint8_t *buf);
} DiskOpsVtable;
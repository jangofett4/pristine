/*
 * Pristine
 * SPDX-License-Identifier: MIT
 */

#pragma once

#include "stage2_kstdint.h"

#define DISK_READ_MAX_SECTORS 4

typedef struct {
    uint8_t (*read) (uint32_t lba, size_t sector_count, uint8_t *buf);
    uint8_t (*write)(uint32_t lba, size_t sector_count, const uint8_t *buf);
} disk_ops_vtable_t;
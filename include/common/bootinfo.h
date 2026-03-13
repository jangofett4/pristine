/*
 * Pristine
 * bootinfo: data expected by kernel upon loading
 * SPDX-License-Identifier MIT
 */

#pragma once

#include <stdint.h>

typedef struct {
    uint32_t memory_map_addr;
} BootInfo;
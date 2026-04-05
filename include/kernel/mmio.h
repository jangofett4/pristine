/*
 * Pristine
 * SPDX-License-Identifier: MIT
 */

#pragma once

#define MMIO_VIRT_START       0xFFFFFFFF00000000

// Each LAPIC register table is 1024 bytes in size, we can map 4096 size pages

#define MMIO_LAPIC_MAX_COUNT  1024
#define MMIO_VIRT_LAPIC_START MMIO_VIRT_START
#define MMIO_VIRT_LAPIC_END   (MMIO_VIRT_LAPIC_START + (MMIO_LAPIC_MAX_COUNT * 0x1000))
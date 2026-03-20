/*
 * Pristine
 * SPDX-License-Identifier MIT
 */

#pragma once

#include <stdint.h>

void     bitmap_clear_all(uint8_t *bitmap, const uint32_t size);
void     bitmap_set(uint8_t *bitmap, const uint32_t index);
void     bitmap_clear(uint8_t *bitmap, const uint32_t index);
int      bitmap_test(uint8_t *bitmap, const uint32_t index);
uint32_t bitmap_find(uint8_t *bitmap, const uint32_t bit_count);
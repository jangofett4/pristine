/*
 * Pristine
 * SPDX-License-Identifier: MIT
 */

#pragma once

#include "stage2_kstdint.h"

typedef struct {
    volatile uint16_t* address;
    uint32_t row, column;
    uint32_t max_rows, max_columns;
} Video;

void video_init(Video*);
void video_putch(Video*, char);
void video_puts(Video*, const char*);
void video_clear(Video*);
void video_set_default(Video*);
void video_scroll(Video *self, uint8_t lines);
Video* video_get_default(void);
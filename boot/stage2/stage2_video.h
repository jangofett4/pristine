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
} video_t;

void video_init(video_t*);
void video_putch(video_t*, char);
void video_puts(video_t*, const char*);
void video_clear(video_t*);
void video_set_default(video_t*);
void video_scroll(video_t *self, uint8_t lines);
video_t* video_get_default(void);
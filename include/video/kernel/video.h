/*
 * Pristine
 * SPDX-License-Identifier: MIT
 */

#include "kstdint.h"

typedef struct {
    uint16_t* address;
    uint32_t row, column;
    uint32_t max_rows, max_columns;
} video_t;

void video_init(video_t*);
void video_putch(video_t*, char);
void video_puts(video_t*, const char*);
void video_clear(video_t*);
void video_default(video_t*);
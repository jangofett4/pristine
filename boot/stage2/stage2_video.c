/*
 * Pristine
 * stage2_video - basic video subroutines for stage 2 bootloader
 * SPDX-License-Identifier: MIT
 */

#include "stage2_video.h"
#include "stage2_memory.h"
#include "stage2_kstdint.h"

Video *_ptr_video_default;

void video_init(Video *self)
{
    self->address = (uint16_t*)0xb8000;
    self->row = 0;
    self->column = 0;
    self->max_columns = 80;
    self->max_rows = 25;
}

void video_putch(Video *self, char ch) {
    if (ch == '\n') {
        self->column = 0;
        self->row++;
    } else {
        if (self->column >= self->max_columns) {
            self->column = 0;
            self->row++;
        }

        if (self->row >= self->max_rows)
            video_scroll(self, 1);

        self->address[(self->row * self->max_columns) + self->column] = (uint16_t)ch | (0x0F << 8);
        self->column++;
    }

    if (self->row >= self->max_rows)
        video_scroll(self, 1);
}

void video_scroll(Video *self, uint8_t lines) {
    size_t line_size = self->max_columns;

    memcpy16_i(
        (void*)self->address,
        (void*)self->address,
        line_size * lines,
        0,
        line_size * (self->max_rows - lines)
    );

    memset16_i(
        (void*)self->address,
        0x0720,
        line_size * (self->max_rows - lines),
        line_size * lines
    );

    if (self->row >= lines)
        self->row -= lines;
    else
        self->row = 0;
    
    self->column = 0;
}

void video_puts(Video *self, const char *str) {
    while (*str) {
        video_putch(self, *str++);
    }
}

void video_set_default(Video *video) {
    _ptr_video_default = video;
}

Video* video_get_default() {
    return _ptr_video_default;
}
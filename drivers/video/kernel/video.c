/*
 * Pristine
 * video - Rudimentary "video" driver
 * SPDX-License-Identifier: MIT
 */

#include "video/kernel/video.h"
#include "stdint.h"

volatile video_t* _ptr_video_default;

void _putchar(char ch) {
    video_putch(_ptr_video_default, ch);
}

void video_init(video_t *self)
{
    self->address = (uint16_t*)0xb8000;
    self->row = 0;
    self->column = 0;
    self->max_columns = 80;
    self->max_rows = 25;
}

void video_putch(video_t *self, char ch) {
    if (self->column >= self->max_columns) {
        self->column = 0;
        self->row++;
    }

    if (ch == '\n') {
        self->column = 0;
        self->row++;
        return;
    }

    if (self->row == self->max_rows) {
        // We need to scroll here
    }

    self->address[(self->row * self->max_columns) + self->column] = ch;
    self->address[(self->row * self->max_columns) + self->column] = (uint16_t)ch | (0x0F << 8);

    self->column++;
}

void video_puts(video_t *self, const char *str) {
    while (*str) {
        video_putch(self, *str++);
    }
}

void video_default(video_t* video) {
    _ptr_video_default = video;
}
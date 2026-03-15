/*
 * Pristine
 * SPDX-License-Identifier: MIT
 */

#pragma once

#include <stdint.h>
#include <stddef.h>
#include <common/vesa.h>

typedef struct {
    uint8_t  *address;
    uint32_t width;
    uint32_t height;
    uint32_t bpp;
    uint32_t bytes_per_scanline;
    uint32_t bytes_per_pixel;
    uint32_t row;
    uint32_t column;
    uint32_t color;
} Video;

void video_init(Video *video, const VesaVbeModeInfo *mode_info);
void video_clear(Video *video);
void video_set_default(Video *video);
void video_scroll(Video *video, uint8_t lines);
Video* video_get_default(void);
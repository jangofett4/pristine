/*
 * Pristine
 * SPDX-License-Identifier: MIT
 */

#pragma once

#include <stdint.h>
#include <stddef.h>
#include <common/vesa.h>

#define VIDEO_VIRT_START 0xFFFF920040000000

typedef struct {
    uintptr_t  phys;
    void      *data;
    uint32_t   width;
    uint32_t   height;
    uint32_t   bpp;
    uint32_t   bytes_per_scanline;
    uint32_t   bytes_per_pixel;
    uint32_t   row;
    uint32_t   column;
    uint32_t   color;
} Video;

void video_init(Video *video, const VesaVbeModeInfo *mode_info);
void video_clear(Video *video);
void video_set_default(Video *video);
void video_scroll(Video *video, uint8_t lines);
Video* video_get_default(void);
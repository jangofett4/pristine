/*
 * Pristine
 * SPDX-License-Identifier: MIT
 */

#pragma once

#include "stage2_kstdint.h"
#include "stage2_vesa.h"

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

#define PSF1_MODE_512     0x01 
#define PSF1_MODE_HASHTAB 0x02
#define PSF1_MODE_MODESEQ 0x04

typedef struct {
    uint8_t magic[2];
    uint8_t mode;
    uint8_t size;
} __attribute__((packed)) Psf1Header;

void video_init(Video *video, const VesaVbeModeInfo *mode_info);
void video_set_color(Video *video, uint8_t r, uint8_t g, uint8_t b);
void video_putch(Video *video, char ch);
void video_puts(Video *video, const char* str);
void video_clear(Video *video);
void video_set_default(Video *video);
void video_scroll(Video *video, uint8_t lines);
Video* video_get_default(void);

#define PSF_MAX_SUPPORTED_SIZE     16
#define PSF_SET_GLYPH_SIZE_TOO_BIG -1

void video_psf_set_font(Psf1Header header);
int video_psf_set_glyph(size_t idx, uint8_t *glyph, const size_t size);
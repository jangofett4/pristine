/*
 * Pristine
 * stage2_video - basic video subroutines for stage 2 bootloader
 * SPDX-License-Identifier: MIT
 */

#include "include/stage2_video.h"
#include "include/stage2_vesa.h"
#include "include/stage2_memory.h"

Video *_ptr_video_default;

static Psf1Header _font;
static uint8_t _font_glyphs[256][PSF_MAX_SUPPORTED_SIZE];

void video_init(Video *video, const VesaVbeModeInfo *mode_info)
{
    video->address = (uint8_t*)mode_info->PhysBasePtr;
    video->width = mode_info->XResolution;
    video->height = mode_info->YResolution;
    video->bpp = mode_info->BitsPerPixel;
    video->bytes_per_scanline = mode_info->BytesPerScanLine;
    video->bytes_per_pixel = video->bpp / 8;
    video->row = 0;
    video->column = 0;
    video->color = 0x00FFFFFF;
}

void video_set_color(Video *video, uint8_t r, uint8_t g, uint8_t b) {
    return;
}

void video_putch(Video *video, char ch) {
    if (ch == '\n') {
        video->column = 0;
        video->row += _font.size;
    } else {
        if (video->column >= video->width) {
            video->column = 0;
            video->row += _font.size;
        }

        if (video->row >= video->height)
            video_scroll(video, 1);

        uint32_t *pixel = (uint32_t*)(video->address + video->row * video->bytes_per_scanline + video->column * video->bytes_per_pixel);
        for (size_t i = 0; i < _font.size; i++) {
            const uint8_t rowdata = _font_glyphs[(int)ch][i];
            uint32_t *row = (uint32_t*)((uint8_t*)pixel + i * video->bytes_per_scanline);
            row[0] = (rowdata >> 7 & 1) ? video->color : 0;
            row[1] = (rowdata >> 6 & 1) ? video->color : 0;
            row[2] = (rowdata >> 5 & 1) ? video->color : 0;
            row[3] = (rowdata >> 4 & 1) ? video->color : 0;
            row[4] = (rowdata >> 3 & 1) ? video->color : 0;
            row[5] = (rowdata >> 2 & 1) ? video->color : 0;
            row[6] = (rowdata >> 1 & 1) ? video->color : 0;
            row[7] = (rowdata >> 0 & 1) ? video->color : 0;
        }
        video->column += 8;
    }

    if (video->row >= video->height)
        video_scroll(video, 8);
}

void video_scroll(Video *video, uint8_t lines) {
    size_t line_bytes = video->bytes_per_scanline * _font.size * lines;
    memcpy(
        video->address,
        video->address + line_bytes,
        video->height * video->bytes_per_scanline - line_bytes
    );
    video->row = ((video->height / _font.size) - lines) * _font.size;
    memset(
        video->address + video->row * video->bytes_per_scanline,
        0,
        video->height * video->bytes_per_scanline - video->row * video->bytes_per_scanline
    );
}

void video_puts(Video *video, const char *str) {
    while (*str) {
        video_putch(video, *str++);
    }
}

void video_set_default(Video *video) {
    _ptr_video_default = video;
}

Video* video_get_default() {
    return _ptr_video_default;
}

void video_psf_set_font(Psf1Header header) {
    _font = header;
}

int video_psf_set_glyph(size_t idx, uint8_t *glyph, const size_t size) {
    if (size > PSF_MAX_SUPPORTED_SIZE) {
        return PSF_SET_GLYPH_SIZE_TOO_BIG;
    }
    
    memcpy(_font_glyphs + idx, glyph, size);

    return 0;
}
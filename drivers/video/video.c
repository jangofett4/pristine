/*
 * Pristine
 * video: basic video driver
 * SPDX-License-Identifier: MIT
 */

#include <common/string.h>
#include <common/vesa.h>
#include <drivers/video/video.h>
#include <stdint.h>

Video *_ptr_video_default;

void video_init(Video *video, const VesaVbeModeInfo *mode_info)
{
    video->address = (uint8_t*)(uintptr_t)mode_info->PhysBasePtr;
    video->width = mode_info->XResolution;
    video->height = mode_info->YResolution;
    video->bpp = mode_info->BitsPerPixel;
    video->bytes_per_scanline = mode_info->BytesPerScanLine;
    video->bytes_per_pixel = video->bpp / 8;
    video->row = 0;
    video->column = 0;
}

void video_clear(Video *video) {
    memset(video->address, 0, video->bytes_per_scanline * video->height);
}

void video_scroll(Video *video, uint8_t lines) {
    size_t line_bytes = video->bytes_per_scanline * lines;
    memcpy(
        video->address,
        video->address + line_bytes,
        video->height * video->bytes_per_scanline - line_bytes
    );
    video->row = video->height - lines;
    memset(
        video->address + video->row * video->bytes_per_scanline,
        0,
        video->height * video->bytes_per_scanline - video->row * video->bytes_per_scanline
    );
}

void video_set_default(Video *video) {
    _ptr_video_default = video;
}

Video* video_get_default() {
    return _ptr_video_default;
}
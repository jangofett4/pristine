/*
 * Pristine
 * stage2_common - common functions used throught the stage 2 bootloader
 * SPDX-License-Identifier: MIT
 */

#include "stage2_video.h"
#include "stage2_serial.h"

void _putchar(char ch) {
    Serial* serial = serial_get_default();
    Video* video = video_get_default();
    if (serial)
        serial_putch(serial, ch);
    if (video)
        video_putch(video, ch);
}
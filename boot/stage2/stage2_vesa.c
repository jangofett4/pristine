/*
 * Pristine
 * stage2_vesa: VESA VBE subroutines
 * SPDX-License-Identifier: MIT
 */

#include "stage2_vesa.h"
#include "stage2_memory.h"

VesaVbeInfo vesa_vbe_get_info(void) {
    VesaVbeInfo *info = (VesaVbeInfo*)VESA_INFO_ADDR;
    return *info;
}

VesaVbeModeInfo vesa_vbe_get_mode_info(void) {
    VesaVbeModeInfo *info = (VesaVbeModeInfo*)VESA_MODE_INFO_ADDR;
    return *info;
}
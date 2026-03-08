/*
 * Pristine
 * stage2_vesa: VESA VBE subroutines
 * SPDX-License-Identifier: MIT
 */

#include "stage2_vesa.h"
#include "stage2_memory.h"

vesa_vbe_info vesa_vbe_get_info(void) {
    vesa_vbe_info *info = (vesa_vbe_info*)VESA_INFO_ADDR;
    return *info;
}

vesa_vbe_mode_info vesa_vbe_get_mode_info(void) {
    vesa_vbe_mode_info *info = (vesa_vbe_mode_info*)VESA_MODE_INFO_ADDR;
    return *info;
}
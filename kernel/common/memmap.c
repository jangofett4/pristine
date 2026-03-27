/*
 * Pristine
 * memmap: memory map related definitions
 * SPDX-License-Identifier MIT
 */

#include <common/memmap.h>
#include <printf.h>

uint64_t memmap_bitmap_init(MemmapEntry *entries, size_t count, uint8_t *bitmap) {
    uint32_t total_usable_memory = 0;
    // Mark the regions we got from memory map
    for (size_t i = 0; i < count; i++) {
        MemmapEntry *entry = entries + i;
        uint64_t entry_base = (uint64_t)entry->BaseAddrHigh << 32 | (uint64_t)entry->BaseAddrLow;
        uint64_t entry_size = (uint64_t)entry->LengthHigh << 32 | (uint64_t)entry->LengthLow;

        uint64_t base = ALIGN_UP(entry_base, VMM_DEFAULT_PAGE_SIZE);
        uint64_t end  = ALIGN_UP(entry_base + entry_size, VMM_DEFAULT_PAGE_SIZE);

        printf_(" 0x%016lx:0x%016lx (%lu KiB), Type: ", base, end, entry_size / 1024);
        if (entry->Type == 1) {
            total_usable_memory += entry_size;
            for (uint64_t m = base; m < end; m += VMM_DEFAULT_PAGE_SIZE) {
                bitmap_clear(bitmap, m / VMM_DEFAULT_PAGE_SIZE);
            }
        } else {
            for (uint64_t m = base; m < end; m += VMM_DEFAULT_PAGE_SIZE) {
                bitmap_set(bitmap, m / VMM_DEFAULT_PAGE_SIZE);
            }
        }
        switch (entry->Type) {
            case 1:
                printf_("Usable");
                break;
            case 2:
                printf_("Reserved");
                break;
            case 3:
                printf_("ACPI");
                break;
            case 4:
                printf_("NVS");
                break;
            case 5:
                printf_("Unusable");
                break;
            case 6:
                printf_("Disabled");
                break;
            default:
                printf_("Unknown");
                break;
        }
        printf_("\n");
    }
    return total_usable_memory;
}
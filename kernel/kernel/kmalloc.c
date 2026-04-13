/*
 * Pristine
 * SPDX-License-Identifier: MIT
 */

#include "kernel/panic.h"
#include <kernel/kmalloc.h>
#include <kernel/slab.h>

void *kmalloc(size_t num_bytes) {
    // TODO: This is very naive, an alternative would be a 1024 element lookup table
    //       Another alternative would be (if the bin sizes were equally spaced) do
    //       some arithmetic to determine the bin
    if (num_bytes == 0)    KPANIC("kmalloc: cannot allocated 0 bytes");
    if (num_bytes <= 8)    return bin_alloc(0);
    if (num_bytes <= 16)   return bin_alloc(1);
    if (num_bytes <= 24)   return bin_alloc(2);
    if (num_bytes <= 32)   return bin_alloc(3);
    if (num_bytes <= 40)   return bin_alloc(4);
    if (num_bytes <= 48)   return bin_alloc(5);
    if (num_bytes <= 56)   return bin_alloc(6);
    if (num_bytes <= 64)   return bin_alloc(7);
    if (num_bytes <= 80)   return bin_alloc(8);
    if (num_bytes <= 96)   return bin_alloc(9);
    if (num_bytes <= 112)  return bin_alloc(10);
    if (num_bytes <= 128)  return bin_alloc(11);
    if (num_bytes <= 160)  return bin_alloc(12);
    if (num_bytes <= 192)  return bin_alloc(13);
    if (num_bytes <= 224)  return bin_alloc(14);
    if (num_bytes <= 256)  return bin_alloc(15);
    if (num_bytes <= 320)  return bin_alloc(16);
    if (num_bytes <= 384)  return bin_alloc(17);
    if (num_bytes <= 448)  return bin_alloc(18);
    if (num_bytes <= 512)  return bin_alloc(19);
    if (num_bytes <= 640)  return bin_alloc(20);
    if (num_bytes <= 768)  return bin_alloc(21);
    if (num_bytes <= 896)  return bin_alloc(22);
    if (num_bytes <= 1024) return bin_alloc(23);

    KPANIC("kmalloc: unsupported num_bytes: %lu", num_bytes);
}

void kfree(void *ptr) {
    bin_free(ptr);
}
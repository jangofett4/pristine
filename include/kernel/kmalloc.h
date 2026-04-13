/*
 * Pristine
 * SPDX-License-Identifier: MIT
 */

#pragma once

#include <stddef.h>

// Allocates 'num_bytes' bytes of memory and returns the address
void *kmalloc(size_t num_bytes);

// Frees given kmalloc'ed memory
void  kfree(void* ptr);
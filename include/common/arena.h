/*
 * Pristine
 * SPDX-License-Identifier: MIT
 */

#pragma once

#include <stdint.h>
#include <stddef.h>

#define ARENA_SIZE (16 * 1024)

void *arena_alloc(size_t size, size_t align);
void arena_reset(void);
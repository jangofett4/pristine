/*
 * Pristine
 * SPDX-License-Identifier: MIT
 */

#pragma once

#include "stage2_kstdint.h"

void memcpy(void* dst, const void* src, size_t count);
void memcpy_i(void *dst, const void* src, size_t srcIdx, size_t dstIdx, size_t count);

void memcpy16(void* dst, const void* src, size_t count);
void memcpy16_i(void *dst, const void* src, size_t srcIdx, size_t dstIdx, size_t count);

void memcpy32(void* dst, const void* src, size_t count);
void memcpy32_i(void *dst, const void* src, size_t srcIdx, size_t dstIdx, size_t count);


void memset(void *dst, uint8_t data, size_t count);
void memset_i(void *dst, uint8_t data, size_t dstIdx, size_t count);

void memset16(void *dst, uint16_t data, size_t count);
void memset16_i(void *dst, uint16_t data, size_t dstIdx, size_t count);

void memset32(void *dst, uint32_t data, size_t count);
void memset32_i(void *dst, uint32_t data, size_t dstIdx, size_t count);
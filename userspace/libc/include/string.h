/*
 * Pristine
 * SPDX-License-Identifier: MIT
 */

#pragma once

#include <stddef.h>
#include <stdint.h>

void *memcpy(void* dst, const void* src, size_t count);
void *memset(void *dst, uint8_t data, size_t count);
void *memmove(void *dst, const void *src, size_t count);

size_t strlen(const char str[]);
int strcmp(const char *lhs, const char *rhs);
char *strcpy(char *dst, const char *src);
char *strncpy(char *dst, const char *src, size_t count);
char *strchr(char *str, char chr);
char *strrchr(char *str, char chr);
/*
 * Pristine
 * string: memory functions
 * SPDX-License-Identifier MIT
 */


#include <stddef.h>
#include <stdint.h>

void *memcpy(void* dst, const void* src, size_t count);
void *memset(void *dst, uint8_t data, size_t count);

size_t strlen(const char str[]);
int strcmp(const char *lhs, const char *rhs);

void strrev(char *str);
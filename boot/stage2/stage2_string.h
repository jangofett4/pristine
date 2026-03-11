/*
 * Pristine
 * SPDX-License-Identifier: MIT
 */

#pragma once

#include <stddef.h>

size_t strlen(const char str[]);
int strcmp(const char *lhs, const char *rhs);
char* strtok(const char *string, const char sep);

void reverse(char str[]);

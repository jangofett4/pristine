/*
 * Pristine
 * stage2_string - string routines
 * SPDX-License-Identifier: MIT
 */

#include "stage2_string.h"

size_t strlen(char str[]) {
    char* ptr = str;
    size_t len = 0;
    while (*(ptr++)) len++;
    return len;
}

void reverse(char str[]) {
    char c;
    size_t len = strlen(str);
    for (size_t i = 0, j = len - 1; i < j; i++, j--) {
        c = str[i];
        str[i] = str[j];
        str[j] = c;
    }
}
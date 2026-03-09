/*
 * Pristine
 * stage2_string - string routines
 * SPDX-License-Identifier: MIT
 */

#include "stage2_string.h"

size_t strlen(const char str[]) {
    size_t len = 0;
    while (str[len] != '\0') len++;
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

int strcmp(const char *s1, const char *s2) {
    while (*s1 && (*s1 == *s2)) {
        s1++;
        s2++;
    }
    return (unsigned char)*s1 - (unsigned char)*s2;
}

char* strtok(const char *string, const char sep) {
    return 0;
    
}

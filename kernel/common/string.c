/*
 * Pristine
 * string: memory functions
 * SPDX-License-Identifier MIT
 */

#include <common/string.h>

void *memcpy(void* dst, const void* src, size_t count) {
    // rep movsb
    // inputs: RDI = destination address, RSI = source address, ECX = count
    // clobbers memory, advances both RDI and RSI and decrements ECX
    __asm__ volatile(
        "rep movsb"
        : "+D"(dst), "+S"(src), "+c"(count)
        :
        : "memory"
    );
    return dst;
}

void *memset(void *dst, uint8_t data, size_t count) {
    // rep stosb
    // inputs: RDI = destination address, ECX = count, AL = data
    // clobbers memory, advances RDI, decrements ECX
    __asm__ volatile(
        "rep stosb"
        : "+D"(dst), "+c"(count)
        : "a"(data)
        : "memory"
    );
    return dst;
}

size_t strlen(const char *str) {
    size_t len = 0;
    while (str[len] != '\0') len++;
    return len;
}

int strcmp(const char *lhs, const char *rhs) {
    while (*lhs && (*lhs == *rhs)) {
        lhs++;
        rhs++;
    }
    return (unsigned char)*lhs - (unsigned char)*rhs;
}

void strrev(char *str) {
    if (str == 0 || *str == 0) return;

    char *start = str;
    char *end   = str + strlen(str) - 1;

    char tmp;
    while (end > start) {
        tmp = *end;
        *end = *start;
        *start = tmp;
        start++; end--;
    }
}
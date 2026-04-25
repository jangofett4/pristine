/*
 * Pristine
 * string: memory operations
 * SPDX-License-Identifier: MIT
 */

#include <string.h>

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

void *memmove(void *dst, const void *src, size_t count) {
    if (dst < src) {
        // forward copy
        __asm__ volatile(
            "cld\n"
            "rep movsb"
            : "+D"(dst), "+S"(src), "+c"(count)
            :
            : "memory"
        );
    } else {
        // backwars copy
        void *last_dst = dst + count - 1;
        const void *last_src = src + count - 1;
        __asm__ volatile(
            "std\n"
            "rep movsb\n"
            "cld"
            : "+D"(last_dst), "+S"(last_src), "+c"(count)
            :
            : "memory"
        );
    }
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

char *strcpy(char *dst, const char *src) {
    size_t len = strlen(src) + 1;
    memcpy(dst, src, len);
    return dst;
}

char *strncpy(char *dst, const char *src, size_t count) {
    const size_t len = strlen(src);
    const size_t copy_len = len < count ? len : count;
    const size_t pad_len = copy_len == count ? 0 : count - len;
    memcpy(dst, src, copy_len);
    memset(dst + copy_len, 0, pad_len);
    return dst;
}

char *strchr(char *str, char chr) {
    const size_t len = strlen(str);
    char *ptr = str;
    for (size_t i = 0; i < len; i++, ptr++) {
        if (*ptr == chr) return ptr;
    }
    return NULL;
}

char *strrchr(char *str, char chr) {
    const size_t len = strlen(str);
    char *ptr = str;
    char *found = NULL;
    for (size_t i = 0; i < len; i++, ptr++) {
        if (*ptr == chr) found = ptr;
    }
    return found;
}
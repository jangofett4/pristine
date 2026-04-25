#include <malloc.h>
#include <unistd.h>
#include <stdint.h>
#include <string.h>
#include <syscall.h>

int main() {
    const char *str = "Hello, World!\n";
    char *ptr = malloc(20);
    memcpy(ptr, str, 15);
    free(ptr);
    return 0;
}
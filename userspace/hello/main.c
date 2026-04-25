#include <unistd.h>
#include <stdint.h>
#include <string.h>
#include <syscall.h>

int main() {
    const char *str = "Hello, World!\n";
    char *ptr = sbrk(0x4000);
    memcpy(ptr, str, 15);
    sbrk(-0x1000);
    sbrk(-0x1000);
    sbrk(-0x1000);
    sbrk(-0x1000);
    return 0;
}
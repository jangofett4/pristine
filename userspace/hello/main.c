#include <stdint.h>
#include <string.h>
#include <syscall.h>

int main() {
    const char *str = "Hello, World!\n";

    while (1) {
        syscall1(SYS_SERIAL_PUTS, (uintptr_t)str);
    }
    return 0;
}
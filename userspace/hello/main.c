#include <syscall.h>

int main() {
    const char *test_str = "Hello, World!\n";
    syscall1(1, (uint64_t)test_str);
    return 0;
}
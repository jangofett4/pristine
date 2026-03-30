ifndef REPO_ROOT
    $(error REPO_ROOT is not set. Run from root Makefile)
endif

CC      := $(CGUEST64)
CFLAGS  := -ffreestanding -nostdlib -O2 -Wall -Wextra -c
TARGET  := $(REPO_ROOT)/bin/userspace/libc/crt0.o

all: $(TARGET)

$(TARGET): crt0.c
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -o $@ $^

clean:
	rm -f $(TARGET)
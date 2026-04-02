ifndef REPO_ROOT
    $(error REPO_ROOT is not set. Run from root Makefile)
endif

ifdef DEBUG
	COPT += -DPRISTINE_DEBUG -O0 -g 
else
	COPT += -O2
endif

CC      = $(CGUEST64)
CFLAGS  = -ffreestanding -nostdlib -O2 -Wall -Wextra -lgcc $(COPT)

CFLAGS  += -I$(REPO_ROOT)/userspace/libc/include -I$(REPO_ROOT)/include/uapi

TARGET  = $(REPO_ROOT)/root/bin/hello.elf
CRT0    = $(REPO_ROOT)/bin/userspace/libc/crt0.o

all: $(TARGET)

$(TARGET): main.c
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -o $@ $(CRT0) main.c

clean:
	rm -f $(TARGET)
ifndef REPO_ROOT
    $(error REPO_ROOT is not set. Run from root Makefile)
endif

ifdef DEBUG
	COPT += -DPRISTINE_DEBUG -O0 -g 
else
	COPT += -O2
endif

CC      = $(CGUEST64)
CFLAGS  = -ffreestanding -nostdlib -O2 -Wall -Wextra -lgcc -ffunction-sections -fdata-sections  $(COPT)
LDFLAGS = -Wl,--gc-sections 

CFLAGS  += -I$(REPO_ROOT)/userspace/libc/include -I$(REPO_ROOT)/include/uapi

TARGET  = $(REPO_ROOT)/root/bin/hello.elf
CRT0    = $(REPO_ROOT)/bin/userspace/libc/crt0.o
LIBC    = $(REPO_ROOT)/bin/userspace/libc/libc.a

all: $(TARGET)

$(TARGET): main.c
	@mkdir -p $(dir $@)
	$(CC) $(LDFLAGS) $(CFLAGS) -o $@ $(CRT0) main.c $(LIBC)

clean:
	rm -f $(TARGET)
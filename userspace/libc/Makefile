ifndef REPO_ROOT
    $(error REPO_ROOT is not set. Run from root Makefile)
endif

ifdef DEBUG
	COPT += -DPRISTINE_DEBUG -O0 -g 
else
	COPT += -O2
endif

AR      = $(ARGUEST64)
ARFLAGS = rcs

CC      = $(CGUEST64)
CFLAGS  = -ffreestanding -nostdlib -O2 -Wall -Wextra -c $(COPT) -I$(REPO_ROOT)/include/uapi -I$(REPO_ROOT)/userspace/libc/include

# TODO: remove DEINVAL and DENOMEM flags here after porting errno
CFLAGS += -DLACKS_UNISTD_H -DLACKS_SYS_MMAN_H -DHAVE_MMAP=0 -DHAVE_MORECORE=1 -DLACKS_SYS_TYPES_H -DNO_MALLOC_STATS -DLACKS_ERRNO_H -DLACKS_TIME_H -DLACKS_SYS_PARAM_H -DLACKS_ERRNO_H -DMALLOC_FAILURE_ACTION= -DEINVAL=-1 -DENOMEM=-2

ALL_SRCS  = $(shell find . -name "*.c")
CRT0_SRC  = ./crt0.c
LIBC_SRCS = $(filter-out $(CRT0_SRC), $(ALL_SRCS))

LIBC_OBJS = $(patsubst %.c, $(REPO_ROOT)/bin/userspace/libc/%.o, $(LIBC_SRCS))

LIBC = $(REPO_ROOT)/bin/userspace/libc/libc.a
CRT0 = $(REPO_ROOT)/bin/userspace/libc/crt0.o

all: $(CRT0) $(LIBC)

$(REPO_ROOT)/bin/userspace/libc/%.o: %.c
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) $< -o $@

$(LIBC): $(LIBC_OBJS)
	$(AR) $(ARFLAGS) $@ $^

clean:
	rm -f $(TARGET)
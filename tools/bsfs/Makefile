C = clang
CFLAGS = -O2 -std=c11

all: mkfs.bsfs

mkfs.bsfs: mkfs.bsfs.c
	$(C) $(CFLAGS) $< -o $@
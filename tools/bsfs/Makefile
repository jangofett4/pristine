C = clang
CFLAGS = -O2 -std=c11 -DBSFS_DEBUG=1

all: mkfs.bsfs bsfs-populate

clean:
	rm -f bsfs-populate mkfs.bsfs

mkfs.bsfs: mkfs.bsfs.c
	$(C) $(CFLAGS) $< -o $@

bsfs-populate: bsfs-populate.c bsfs.c
	$(C) $(CFLAGS) $^ -o $@
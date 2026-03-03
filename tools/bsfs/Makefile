C = clang
CFLAGS = -O2 -std=c11 -DBSFS_DEBUG=1 -g

all: mkfs.bsfs bsfs-populate bsfs-extract

clean:
	rm -rf bin

bin/mkfs.bsfs: mkfs.bsfs.c
	@mkdir -p $(dir $@)
	$(C) $(CFLAGS) $< -o $@

bin/bsfs-populate: bsfs-populate.c bsfs.c
	$(C) $(CFLAGS) $^ -o $@

bin/bsfs-extract: bsfs-extract.c bsfs.c
	$(C) $(CFLAGS) $^ -o $@

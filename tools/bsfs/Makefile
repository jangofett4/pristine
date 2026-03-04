C = clang
CFLAGS = -O2 -std=c11 -DBSFS_DEBUG=1 -g

all: bin/mkfs.bsfs bin/bsfs-populate bin/bsfs-extract

clean:
	rm -rf bin

bin/mkfs.bsfs: mkfs.bsfs.c
	@mkdir -p $(dir $@)
	$(C) $(CFLAGS) $< -o $@

bin/bsfs-populate: bsfs-populate.c bsfs.c
	@mkdir -p $(dir $@)
	$(C) $(CFLAGS) $^ -o $@

bin/bsfs-extract: bsfs-extract.c bsfs.c
	@mkdir -p $(dir $@)
	$(C) $(CFLAGS) $^ -o $@

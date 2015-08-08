bundlefs: bundlefs.c
	$(CC) bundlefs.c -D_FILE_OFFSET_BITS=64 `pkg-config --cflags --libs fuse` -o bundlefs

.PHONY: clean
clean:
	rm -f bundlefs

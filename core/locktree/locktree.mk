SOURCES+=locktree/locktree.c
HEADERS+=locktree/locktree.h
TESTS+=locktree_test

locktree_test: libcore.a locktree/locktree_test.c
	$(CC) locktree/locktree_test.c -static -L. -lcore $(CFLAGS) $(LDFLAGS) -o locktree_test

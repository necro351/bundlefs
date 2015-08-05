SOURCES+=repo/repo.c
HEADERS+=repo/repo.h
TESTS+=repo_test

repo_test: libcore.a repo/repo_test.c
	$(CC) repo/repo_test.c -static -L. -lcore $(CFLAGS) $(LDFLAGS) -o repo_test

SOURCES+=repo/repo.c
SOURCES+=repo/moveobjects.c
SOURCES+=repo/fault.c
SOURCES+=repo/maxid.c
HEADERS+=repo/repo.h
TESTS+=repo_test

repo_test: libcore.a repo/repo_test.c
	$(CC) repo/repo_test.c -static -L. -lcore $(CFLAGS) $(LDFLAGS) -o repo_test

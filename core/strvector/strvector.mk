SOURCES+=strvector/strvector.c
HEADERS+=strvector/strvector.h
TESTS+=strvector_test

strvector_test: libcore.a strvector/strvector_test.c
	$(CC) strvector/strvector_test.c -static -L. -lcore $(CFLAGS) $(LDFLAGS) -o strvector_test

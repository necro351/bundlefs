SOURCES+=strvector.c
HEADERS+=strvector.h
TESTS+=strvector_test

strvector_test: strvector/strvector_test.c
	$(CC) $(CFLAGS) -o strvector_test

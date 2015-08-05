all: libcore.a

CFLAGS=-g -Wall -Werror -I. -DPOSIX_CONTEXT
LDFLAGS=-lpthread
SOURCES=
HEADERS=
TESTS=
MODULES=context/context.mk \
	locktree/locktree.mk \
	strvector/strvector.mk \
	test/test.mk \
	repo/repo.mk

include $(MODULES)

libcore.a: $(SOURCES) $(HEADERS)
	$(CC) -c $(SOURCES) $(CFLAGS)
	$(AR) -rcs libcore.a *.o

# We have to tell valgrind to not look for undefined value errors because it
# gets confused by malloc in glibc and by simple usage like memset(&thing, 0,
# sizeof(thing)), so obviously we cannot use it, but we can still search for
# leaks with valgrind.
check: $(TESTS)
	@for test in $(TESTS); do valgrind --quiet --undef-value-errors=no --error-exitcode=1 ./$$test; done

.PHONY: clean
clean:
	rm -f *.o *.a *_test

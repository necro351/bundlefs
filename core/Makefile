CFLAGS=-g -Wall -Werror -I. -DPOSIX_CONTEXT

SOURCES=
HEADERS=
TESTS=
MODULES=context/context.mk \
	locktree/locktree.mk \
	strvector/strvector.mk \
	test/test.mk

include $(MODULES)

libcore.a: $(SOURCES) $(HEADERS)
	$(CC) -c $(SOURCES) $(CFLAGS)
	$(AR) -rcs libcore.a test.o strvector.o

check: $(TESTS)
	for test in $(TESTS); do $$test; done

.PHONY: clean
clean:
	rm -f *.o *.a *_test

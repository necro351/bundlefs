CFLAGS=-Wall -Werror

SOURCES=
HEADERS=
TESTS=
MODULES=context.mk \
	locktree.mk \
	strvector.mk \
	test.mk

include $(MODULES)

libcore.a: $(SOURCES) $(HEADERS)
	$(CC) -c $(SOURCES) $(CFLAGS) -o core.o
	$(AR) -rcs libcore.a core.o

check: $(TESTS)
	for test in $(TESTS); do $$test; done

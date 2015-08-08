#include "test/test.h"
#include "context/context.h"

int test_exit_code = 0;

void test_run(test_func_t test, const char* description) {
	gen_printf("TEST: %s", description);
	if (test() < 0) {
		gen_printf("...FAILED\n");
		test_exit_code = 1;
	} else {
		gen_printf("\n");
	}
}

void test_exit() {
	exit(test_exit_code);
}

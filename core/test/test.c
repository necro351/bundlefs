#include "test/test.h"
#include "context/context.h"

int test_exit_code = 0;

void test_run(test_func_t test, const char* description) {
	print("TEST: %s", description);
	if (test() < 0) {
		print("...FAILED\n");
		test_exit_code = 1;
	} else {
		print("\n");
	}
}

void test_exit() {
	exit(test_exit_code);
}

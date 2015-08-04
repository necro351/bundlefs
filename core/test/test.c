#include "test.h"
#include "context.h"

void run_test(test_func_t test, const char* description) {
	print("TEST: %s", description);
	if (test() < 0) {
		print("...FAILED\n");
	} else {
		print("\n");
	}
}

#include <stdlib.h>
#include "repo/repo.h"
#include "test/test.h"

int main(void) {
	system("mkdir -p TEST/repo");
	system("mkdir TEST/mnt");
	test_run(test_branch_name, "branch_name works");
	system("rm -rf TEST");
	test_exit();
	return 0;
}

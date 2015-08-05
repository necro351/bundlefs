#include <stdlib.h>
#include "repo/repo.h"
#include "test/test.h"

extern char* branch_name(repo* rep, const char* abspath);

int test_branch_name() {
	char* brname;
	repo rep;
	repo_init(&rep);
	int err = repo_open(&rep, "TEST/mnt", "TEST/repo");
	if (err) {
		return -1;
	}
	brname = branch_name(&rep, "TEST/mnt/branches/a");
	if (brname && strcmp(brname, "a"))
		return -1;
	brname = branch_name(&rep, "TEST/mnt/branches/b/file");
	if (brname && strcmp(brname, "b"))
		return -1;
	brname = branch_name(&rep, "TEST/mnt/b/file");
	if (brname)
		return -1;
	brname = branch_name(&rep, "TEST/other/a");
	if (brname)
		return -1;
	return 0;
}

int main(void) {
	system("mkdir -p TEST/repo");
	system("mkdir TEST/mnt");
	test_run(test_branch_name, "branch_name works");
	system("rm -rf TEST");
	test_exit();
	return 0;
}

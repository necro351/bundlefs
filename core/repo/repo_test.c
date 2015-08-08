#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include "repo/repo.h"
#include "test/test.h"

/*
int test_repo_commit() {
	int err = 0;
	system("mkdir -p TEST/repo");
	system("mkdir TEST/mnt");
	repo rep;
	repo_commit(&rep, 
exit:
	system("rm -rf TEST");
	return err;
}
*/

int test_repo_new() {
	// Make the repository
	repo rep;
	repo_init(&rep);
	int err = 0;
	system("mkdir -p TEST/repo");
	system("mkdir TEST/mnt");
	err = repo_open(&rep, "TEST/mnt", "TEST/repo");
	if (err)
		goto exit;

	// Create some new branches
	err = repo_new(&rep, "rick");
	if (err)
		goto exit;
	err = repo_new(&rep, "sean");
	if (err)
		goto exit;
	err = repo_new(&rep, "sandra");
	if (err)
		goto exit;

	// Make sure they are there
	struct stat buf;
	err = stat("TEST/repo/rick", &buf);
	if (err)
		goto exit;
	err = stat("TEST/repo/rick/", &buf);
	if (err)
		goto exit;
	err = stat("TEST/repo/sean", &buf);
	if (err)
		goto exit;
	err = stat("TEST/repo/sandra", &buf);
	if (err)
		goto exit;
exit:
	system("rm -rf TEST");
	repo_destroy(&rep);
	return err;
}

extern int moveobjects(const char* dst, const char* src);
int test_moveobjects() {
	int err = 0;
	system("mkdir -p TEST/staged/objs");
	system("mkdir -p TEST/expected/objs");
	char mkcommand[256] = "mkdir TEST/staged/objs/x";
	int pos = strlen(mkcommand) - 1;
	char a;
	for (a = '0'; a <= '9'; a++) {
		mkcommand[pos] = a;
		err = system(mkcommand);
		if (err)
			goto exit;
	}
	err = moveobjects("TEST/expected/objs", "TEST/staged/objs");
	if (err)
		goto exit;
	char checkpath[256] = "TEST/expected/objs/x";
	pos = strlen(checkpath) - 1;
	for (a = '0'; a <= '9'; a++) {
		checkpath[pos] = a;
		struct stat buf;
		err = stat(checkpath, &buf);
		if (err)
			goto exit;
	}
exit:
	system("rm -rf TEST");
	return err;
}

int main(void) {
	test_run(test_moveobjects, "moveobjects");
	//test_run(test_repo_commit, "repo_commit");
	test_exit();
	return 0;
}

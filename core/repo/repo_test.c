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

extern int getvalue(char* objectstore, char* obj);
int test_getvalue() {
	int err = 0;
	system("mkdir -p TEST/objs");
	system("echo i1 > TEST/objs/i0");
	system("echo i3 > TEST/objs/i2");
	char obj[MAX_OBJID_LEN];
	err = getvalue("TEST/objs/i0", obj);
	if (err)
		goto exit;
	if (strcmp(obj, "i1")) {
		gen_printf("expected 'i1' got '%s'\n", obj);
		err = -1;
		goto exit;
	}
	err = getvalue("TEST/objs/i2", obj);
	if (err)
		goto exit;
	if (strcmp(obj, "i3")) {
		gen_printf("expected 'i3' got '%s'\n", obj);
		err = -1;
		goto exit;
	}
exit:
	system("rm -rf TEST");
	return err;
}

int test_object_type() {
	int err = 0;
	system("mkdir -p TEST/objs/i0");
	system("echo f > TEST/objs/i0/.bundlefs_type");
	object obj;
	object_init(&obj, "TEST/objs", "i0");
	int type;

	err = object_type(&obj, &type);
	if (err)
		goto exit;
	if (type != OBJECT_TYPE_FILE) {
		err = -1;
		goto exit;
	}

	system("echo d > TEST/objs/i0/.bundlefs_type");
	err = object_type(&obj, &type);
	if (err)
		goto exit;
	if (type != OBJECT_TYPE_DIR) {
		err = -1;
		goto exit;
	}

	system("rm TEST/objs/i0/.bundlefs_type");
	system("touch TEST/objs/i0/.bundlefs_type");
	err = object_type(&obj, &type);
	if (err)
		goto exit;
	if (type == OBJECT_TYPE_FILE || type == OBJECT_TYPE_DIR) {
		err = -1;
		goto exit;
	}
exit:
	system("rm -rf TEST");
	return err;
}

int test_pathlookup() {
	int err = 0;
	system("mkdir -p TEST/objs");
	system("echo i0 > TEST/root");
	system("mkdir -p  TEST/objs/i0/");
	system("echo i1 > TEST/objs/i0/home");
	system("echo d >  TEST/objs/i0/" OBJECT_HIDDEN_FILE);
	system("mkdir -p  TEST/objs/i1/");
	system("echo i3 > TEST/objs/i1/rick");
	system("echo d >  TEST/objs/i1/" OBJECT_HIDDEN_FILE);
	system("mkdir -p  TEST/objs/i3/");
	system("echo i4 > TEST/objs/i3/hello.txt");
	system("echo d >  TEST/objs/i3/" OBJECT_HIDDEN_FILE);
	system("mkdir -p  TEST/objs/i4/");
	system("echo i5 > TEST/objs/i4/0");
	system("echo f >  TEST/objs/i4/" OBJECT_HIDDEN_FILE);
	system("mkdir -p  TEST/objs/i5/");
	system("echo hello world > TEST/objs/i5/data");

	path pth;
	path_init(&pth);
	err = path_lookup(&pth, "TEST", "home/rick/hello.txt");
	if (err)
		goto exit;
	if (pth.components.size != 4 ||
			strcmp("/", pth.components.strings[0]) ||
			strcmp("home", pth.components.strings[1]) ||
			strcmp("rick", pth.components.strings[2]) ||
			strcmp("hello.txt", pth.components.strings[3]) ||
			strcmp("i0", pth.objids[0]) ||
			strcmp("i1", pth.objids[1]) ||
			strcmp("i3", pth.objids[2]) ||
			strcmp("i4", pth.objids[3])) {
		err = -1;
		goto exit;
	}
exit:
	path_destroy(&pth);
	system("rm -rf TEST");
	return err;
}

int main(void) {
	test_run(test_moveobjects, "moveobjects");
	test_run(test_getvalue, "getvalue");
	test_run(test_pathlookup, "path lookup");
	test_run(test_object_type, "getobjtype");
	//test_run(test_repo_commit, "repo_commit");
	test_exit();
	return 0;
}

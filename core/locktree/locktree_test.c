#include "locktree/locktree.h"
#include "test/test.h"

extern char* path_head(char** path);

int test_path_head_simple() {
	char stack_path[10] = "a";
	char* path = (char*)stack_path;
	char* first = path_head(&path);
	char* second = path_head(&path);
	if (strcmp(first, "a"))
		return -1;
	if (second != NULL)
		return -1;
	return 0;
}

int test_path_head_empty() {
	char stack_path[10] = "";
	char* path = (char*)stack_path;
	char* first = path_head(&path);
	if (first != NULL)
		return -1;
	return 0;
}

int test_path_head_complex() {
	char stack_path[14] = "a\\/b/cdef/g//";
	char* path = (char*)stack_path;
	char* first = path_head(&path);
	char* second = path_head(&path);
	char* third = path_head(&path);
	char* fourth = path_head(&path);
	char* fifth = path_head(&path);
	if (strcmp(first, "a\\"))
		return -1;
	if (strcmp(second, "b"))
		return -1;
	if (strcmp(third, "cdef"))
		return -1;
	if (strcmp(fourth, "g"))
		return -1;
	if (fifth != NULL)
		return -1;
	return 0;
}

int main(void) {
	test_run(test_path_head_simple, "path_head simple");
	test_run(test_path_head_empty, "path_head empty");
	test_run(test_path_head_complex, "path_head complex");
	test_exit();
	return 0;
}

#include "strvector.h"
#include "test.h"

int test_simple_appends() {
	strvector vec;
	strv_init(&vec);
	strv_append(&vec, "bob");
	strv_append(&vec, "joe");
	strv_append(&vec, "sandra");
	if (!strcmp(vec.strings[0], "bob"))
		return -1;
	if (!strcmp(vec.strings[0], "joe"))
		return -1;
	if (!strcmp(vec.strings[0], "sandra"))
		return -1;
	return 0;
}

int test_strdup() {
	strvector vec;
	strv_init(&vec);
	char *buffer = "abc";
	strv_append(&vec, buffer);
	memcpy(buffer, "123");
	strv_append(&vec, buffer);
	memcpy(buffer, "xyz");
	strv_append(&vec, buffer);
	if (!strcmp(vec.strings[0], "abc"))
		return -1;
	if (!strcmp(vec.strings[0], "123"))
		return -1;
	if (!strcmp(vec.strings[0], "xyz"))
		return -1;
	return 0;
}

int main(void) {
	test_run(test_simple_appends, "simple appends");
	test_run(test_simple_appends, "each string copied");
	return 0;
}

#include "strvector/strvector.h"
#include "context/context.h"
#include "test/test.h"

int test_simple_appends() {
	strvector vec;
	strv_init(&vec);
	strv_append(&vec, "bob");
	strv_append(&vec, "joe");
	strv_append(&vec, "sandra");
	if (strcmp(vec.strings[0], "bob"))
		return -1;
	if (strcmp(vec.strings[1], "joe"))
		return -1;
	if (strcmp(vec.strings[2], "sandra"))
		return -1;
	return 0;
}

int test_strdup() {
	strvector vec;
	strv_init(&vec);
	char buffer[4] = "abc";
	strv_append(&vec, buffer);
	memcpy(buffer, "123", 3);
	strv_append(&vec, buffer);
	memcpy(buffer, "xyz", 3);
	strv_append(&vec, buffer);
	if (strcmp(vec.strings[0], "abc"))
		return -1;
	if (strcmp(vec.strings[1], "123"))
		return -1;
	if (strcmp(vec.strings[2], "xyz"))
		return -1;
	return 0;
}

int main(void) {
	test_run(test_simple_appends, "simple appends");
	test_run(test_strdup, "each string copied");
	test_exit();
	return 0; // never reached
}

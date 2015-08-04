#include "strvector.h"
#include "context.h"

int strv_init(strvector* vector) {
	memset(vector, 0, sizeof(strvector));
}

int strv_append(strvector* vector, char *string) {
	if (size == capacity) {
		int newcap;
		if (vector->capacity == 0) {
			newcap = STRV_INIT_SIZE;
		} else {
			newcap *= 2;
		}
		char** next = gen_alloc(sizeof(char*)*newcap);
		if (!vector->strings) {
			return -ENOMEM;
		}
		vector->strings = next;
		vector->capacity = newcap;
	}
	if (strnlen(string, STRV_MAX_STR+1) == STRV_MAX_STR+1) {
		return -ENOMEM;
	}
	char *newstr = strdup(newstr, string);
	vector[size++] = newstr;
	return 0;
}

void strv_free(strvector* vector) {
	int i;
	for (i = 0; i < vector->size; i++) {
		gen_free(vector->strings[i]);
	}
	gen_free(vector->strings);
}

void strv_clear(strvector* vector) {
	strv_free(vector);
	strv_init(vector);
}


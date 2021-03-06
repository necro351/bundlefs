#include "strvector/strvector.h"
#include "context/context.h"

void strv_init(strvector* vector) {
	memset(vector, 0, sizeof(strvector));
}

int strv_append(strvector* vector, const char *string) {
	if (vector->size == vector->capacity) {
		int newcap;
		if (vector->capacity == 0) {
			newcap = STRV_INIT_SIZE;
		} else {
			newcap = vector->capacity*2;
		}
		char** next = gen_realloc(vector->strings, sizeof(char*)*newcap);
		if (!next) {
			return -ENOMEM;
		}
		vector->strings = next;
		vector->capacity = newcap;
	}
	if (strnlen(string, STRV_MAX_STR+1) == STRV_MAX_STR+1) {
		return -ENOMEM;
	}
	char *newstr = strdup(string);
	if (!newstr) {
		return -ENOMEM;
	}
	vector->strings[vector->size++] = newstr;
	return 0;
}

void strv_clear(strvector* vector) {
	int i;
	for (i = 0; i < vector->size; i++)
		gen_free(vector->strings[i]);
	vector->size = 0;
}

void strv_destroy(strvector* vector) {
	strv_clear(vector);
	gen_free(vector->strings);
}


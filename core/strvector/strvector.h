#ifndef STRVECTOR_H
#define STRVECTOR_H

typedef struct {
	char** strings;
	int capacity;
	int size;
} strvector;

void strv_init(strvector* vector);
int strv_append(strvector* vector, const char *string);
void strv_clear(strvector* vector);
void strv_free(strvector* vector);

#define STRV_INIT_SIZE 8
#define STRV_MAX_STR 1024

#endif

#ifndef TEST_H
#define TEST_H

typedef int (*test_func_t)();

void test_run(test_func_t test, const char* description);
void test_exit();

#endif

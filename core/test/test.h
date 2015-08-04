#ifndef TEST_H
#define TEST_H

typedef int (*test_func_t)();

void run_test(test_func_t test, const char* description);

#endif

#include "stdio.h"
#include "stdlib.h"

void init_test(void) __attribute__((constructor));
void fini_test(void) __attribute((destructor));
int g_param = 10;
static int param = 2;

void init_first(void)
{
    g_param = 100;
}

void init_test(void)
{
    g_param += param;
}

void fini_end(void)
{
    g_param = 0;
}

void fini_test(void)
{
    param = 0;
}

int callee(int a, int b)
{
    return g_param + a + b;
}

int test_api(int a, int b)
{
    int c = callee(a, b);
    return c;
}

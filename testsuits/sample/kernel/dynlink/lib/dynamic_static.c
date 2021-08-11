static int staticParam = 9;
int g_param = 10;

int add_test(int a, int b)
{
    int c;
    c = a + b + g_param - staticParam;
    return c;
}

static int static_sub_test(int a, int b)
{
    int c;
    c = a - b + g_param - staticParam;
    return c;
}

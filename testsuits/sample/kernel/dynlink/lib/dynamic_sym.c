int g_param = 10;

int callee(int a, int b)
{
    return a + b + g_param;
}

int caller(int a, int b)
{
    return 2021 + callee(a, b);
}

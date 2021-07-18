extern int g_param;

int caller(int a, int b)
{
    return a + b + g_param;
}

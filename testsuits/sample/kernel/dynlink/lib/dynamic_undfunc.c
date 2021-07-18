extern int undef_func(int a, int b);

int caller(int a, int b)
{
    return a + undef_func(a, b);
}

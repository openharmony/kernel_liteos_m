int test_array[100];

int dyn_bss_func(void)
{
    test_array[0] += 100;
    return 2017 + test_array[0];
}

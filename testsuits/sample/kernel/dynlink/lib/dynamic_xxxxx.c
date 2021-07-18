int g_value100 = 100;
int *g_pValue100 = &g_value100;
static int value200 = 200;
int *g_pValue200 = &value200;

int get_value100(void)
{
    return *g_pValue100;
}

int get_value200(void)
{
    return *g_pValue200;
}

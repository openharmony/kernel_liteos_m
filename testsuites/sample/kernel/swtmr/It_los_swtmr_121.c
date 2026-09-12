#include "It_los_swtmr.h"

static volatile int g_swtmr121Fired = 0;

static void SwtmrHandler121(UINT32 arg)
{
    g_swtmr121Fired = 1;
}

static UINT32 Testcase(void)
{
    UINT32 ret;
    UINT32 swtmrId;

    g_swtmr121Fired = 0;

    ret = LOS_SwtmrCreate(2, LOS_SWTMR_MODE_ONCE, (SWTMR_PROC_FUNC)SwtmrHandler121, &swtmrId, 0xffff
#if (LOSCFG_BASE_CORE_SWTMR_ALIGN == 1)
        , OS_SWTMR_ROUSES_IGNORE, OS_SWTMR_ALIGN_INSENSITIVE
#endif
    );
    ICUNIT_GOTO_EQUAL(ret, LOS_OK, ret, EXIT);

    ret = LOS_SwtmrDelete(swtmrId);
    ICUNIT_GOTO_EQUAL(ret, LOS_OK, ret, EXIT);

    ret = LOS_SwtmrStart(swtmrId);
    ICUNIT_GOTO_EQUAL(ret, LOS_ERRNO_SWTMR_NOT_CREATED, ret, EXIT);

EXIT:
    return LOS_OK;
}

VOID ItLosSwtmr121(void)
{
    TEST_ADD_CASE("ItLosSwtmr121", Testcase, TEST_LOS, TEST_SWTMR, TEST_LEVEL2, TEST_FUNCTION);
}

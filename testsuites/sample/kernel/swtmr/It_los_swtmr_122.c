#include "It_los_swtmr.h"

static volatile int g_swtmr122CbCount = 0;

static void SwtmrHandler122(UINT32 arg)
{
    g_swtmr122CbCount++;
}

static UINT32 Testcase(void)
{
    UINT32 ret;
    UINT32 swtmrId;
    int countBefore;

    g_swtmr122CbCount = 0;

    ret = LOS_SwtmrCreate(2, LOS_SWTMR_MODE_PERIOD, (SWTMR_PROC_FUNC)SwtmrHandler122, &swtmrId, 0xffff
#if (LOSCFG_BASE_CORE_SWTMR_ALIGN == 1)
        , OS_SWTMR_ROUSES_IGNORE, OS_SWTMR_ALIGN_INSENSITIVE
#endif
    );
    ICUNIT_GOTO_EQUAL(ret, LOS_OK, ret, EXIT);

    ret = LOS_SwtmrStart(swtmrId);
    ICUNIT_GOTO_EQUAL(ret, LOS_OK, ret, EXIT_STOP);

    LOS_TaskDelay(5);

    countBefore = g_swtmr122CbCount;
    ICUNIT_GOTO_NOT_EQUAL(countBefore, 0, countBefore, EXIT_STOP);

    ret = LOS_SwtmrStop(swtmrId);
    ICUNIT_GOTO_EQUAL(ret, LOS_OK, ret, EXIT_STOP);

    ret = LOS_SwtmrDelete(swtmrId);
    ICUNIT_GOTO_EQUAL(ret, LOS_OK, ret, EXIT_STOP);

    LOS_TaskDelay(5);

    ICUNIT_ASSERT_EQUAL(g_swtmr122CbCount, countBefore, g_swtmr122CbCount);

    return LOS_OK;

EXIT_STOP:
    LOS_SwtmrStop(swtmrId);
    LOS_SwtmrDelete(swtmrId);
EXIT:
    return LOS_NOK;
}

VOID ItLosSwtmr122(void)
{
    TEST_ADD_CASE("ItLosSwtmr122", Testcase, TEST_LOS, TEST_SWTMR, TEST_LEVEL2, TEST_FUNCTION);
}

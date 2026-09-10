/*
 * Copyright (c) 2013-2019 Huawei Technologies Co., Ltd. All rights reserved.
 * Copyright (c) 2020-2021 Huawei Device Co., Ltd. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without modification,
 * are permitted under the conditions listed in the accompanying LICENSE file.
 *
 * Implementation of busy-wait calibration (OsTestCalibrateBusyDelay)
 * and the g_itersPerTick global used by TEST_BUSY_DELAY.
 */

#include "test_common.h"
#include "los_debug.h"
#include "los_swtmr.h"

volatile UINT32 g_itersPerTick = 0;

/* ===========================================================================
 * Busy-wait calibration
 *
 * Two implementations selected at compile time:
 *   - LOSCFG_BASE_CORE_SWTMR == 1  → software-timer approach (preferred)
 *   - else                          → LOS_SysCycleGet fallback
 *
 * The swtmr approach measures actual CPU instruction throughput by counting
 * loop iterations between two swtmr triggers (exactly 10 ticks apart).
 * This works on QEMU where CPU speed != SysTick virtual clock frequency.
 *
 * The fallback uses LOS_SysCycleGet() across LOS_TaskDelay(10) and assumes
 * CPU frequency == g_sysClock (true on real hardware, not on QEMU).
 * ======================================================================== */
#if (LOSCFG_BASE_CORE_SWTMR == 1)

/* --- swtmr-based calibration --- */
/*
 * State machine: swtmr (period=10 ticks) drives the calibration.
 *   IDLE     → 1st trigger → COUNTING  (task starts counting loop iterations)
 *   COUNTING → 2nd trigger → DONE      (task stops, computes iters_per_tick)
 *   DONE     → subsequent triggers ignored
 */
#define CALIB_IDLE      0
#define CALIB_COUNTING   1
#define CALIB_DONE       2

static volatile UINT32 g_calibState = CALIB_IDLE;
static volatile UINT32 g_calibCount = 0;

static VOID OsTestCalibSwtmrHandler(VOID)
{
    if (g_calibState == CALIB_IDLE) {
        g_calibState = CALIB_COUNTING;          /* 1st trigger: start */
    } else if (g_calibState == CALIB_COUNTING) {
        g_calibState = CALIB_DONE;              /* 2nd trigger: stop */
    }
    /* DONE: further triggers are no-ops */
}

VOID OsTestCalibrateBusyDelay(VOID)
{
    UINT32 swTmrID;
    UINT32 ret;

    g_calibState = CALIB_IDLE;
    g_calibCount = 0;
    g_itersPerTick = 0;

#if (LOSCFG_BASE_CORE_SWTMR_ALIGN == 1)
    ret = LOS_SwtmrCreate(10, LOS_SWTMR_MODE_PERIOD,
                          (SWTMR_PROC_FUNC)OsTestCalibSwtmrHandler,
                          &swTmrID, 0,
                          OS_SWTMR_ROUSES_ALLOW, OS_SWTMR_ALIGN_INSENSITIVE);
#else
    ret = LOS_SwtmrCreate(10, LOS_SWTMR_MODE_PERIOD,
                          (SWTMR_PROC_FUNC)OsTestCalibSwtmrHandler,
                          &swTmrID, 0);
#endif
    if (ret != LOS_OK) {
        PRINTK("[calibrate] swtmr create failed, fallback to g_sysClock\n");
        return;
    }

    ret = LOS_SwtmrStart(swTmrID);
    if (ret != LOS_OK) {
        LOS_SwtmrDelete(swTmrID);
        PRINTK("[calibrate] swtmr start failed, fallback to g_sysClock\n");
        return;
    }

    /* Phase 1: spin-wait for 1st trigger (IDLE → COUNTING) */
    while (g_calibState == CALIB_IDLE) {
    }

    /* Phase 2: count iterations until 2nd trigger (COUNTING → DONE) */
    while (g_calibState == CALIB_COUNTING) {
        g_calibCount++;
    }

    /* Exactly 10 ticks elapsed between 1st and 2nd trigger */
    g_itersPerTick = g_calibCount / 10;

    LOS_SwtmrStop(swTmrID);
    LOS_SwtmrDelete(swTmrID);

    PRINTK("[calibrate] count=%u iters_per_tick=%u\n",
            g_calibCount, g_itersPerTick);
}

#else /* LOSCFG_BASE_CORE_SWTMR == 0 */

/* --- fallback: LOS_SysCycleGet across LOS_TaskDelay(10) --- */
VOID OsTestCalibrateBusyDelay(VOID)
{
    UINT64 before, after;
    UINT32 cyclesPerTick;

    before = LOS_SysCycleGet();
    LOS_TaskDelay(10); /* 10, delay 10 ticks for measurement */
    after = LOS_SysCycleGet();

    cyclesPerTick = (UINT32)((after - before) / 10); /* 10, ticks delayed */
    g_itersPerTick = cyclesPerTick / TEST_BUSY_DELAY_CYCLES_PER_ITER;

    PRINTK("[calibrate] cycles_per_tick=%u iters_per_tick=%u\n",
            cyclesPerTick, g_itersPerTick);
}

#endif /* LOSCFG_BASE_CORE_SWTMR */

/*
 * Copyright (c) 2013-2019 Huawei Technologies Co., Ltd. All rights reserved.
 * Copyright (c) 2020-2021 Huawei Device Co., Ltd. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without modification,
 * are permitted under the conditions listed in the accompanying LICENSE file.
 *
 * This file provides common test utilities shared across all test suites:
 *   - TEST_DELAY          : tick-based wait (LOS_TaskDelay loop)
 *   - TEST_BUSY_DELAY     : cycle-based busy-wait with runtime calibration
 *   - OsTestCalibrateBusyDelay : calibrates TEST_BUSY_DELAY at startup
 */

#ifndef _TEST_COMMON_H
#define _TEST_COMMON_H

#include "los_config.h"
#include "los_task.h"
#include "los_tick.h"

/* Runtime-calibrated busy-wait iterations per tick.
 * 0 = not yet calibrated; TEST_BUSY_DELAY falls back to g_sysClock estimate.
 * Set by OsTestCalibrateBusyDelay() before any test case runs.
 */
extern volatile UINT32 g_itersPerTick;
VOID OsTestCalibrateBusyDelay(VOID);

/* Tick-based delay: yields CPU via LOS_TaskDelay(1) in a loop.
 * Exits early when (count) >= (target) or timeout ticks elapse.
 * CPU-speed independent — works on both real hardware and QEMU.
 */
#define TEST_DELAY(count, target, timeout) \
    do { \
        UINT32 _wait = (timeout); \
        while ((count) < (target) && _wait > 0) { \
            LOS_TaskDelay(1); \
            _wait--; \
        } \
    } while (0)

/* Cycle-based busy-wait: spins without yielding.
 * Uses g_itersPerTick (calibrated at startup) to compute the iteration
 * limit so that the wait lasts approximately (timeout) ticks.
 * Falls back to g_sysClock-based estimate if calibration was not run.
 */
#define TEST_BUSY_DELAY_CYCLES_PER_ITER 10
#define TEST_BUSY_DELAY(count, target, timeout) \
    do { \
        UINT32 _busLimit = (g_itersPerTick != 0) ? (g_itersPerTick * (UINT32)(timeout)) : \
            (UINT32)(((UINT64)g_sysClock * (UINT64)(timeout)) / \
                     ((UINT64)LOSCFG_BASE_CORE_TICK_PER_SECOND * TEST_BUSY_DELAY_CYCLES_PER_ITER)); \
        volatile UINT32 _busI = 0; \
        while ((count) < (target) && _busI < _busLimit) { \
            _busI++; \
        } \
    } while (0)

#endif /* _TEST_COMMON_H */

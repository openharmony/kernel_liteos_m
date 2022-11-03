/*
 * Copyright (c) 2022 Huawei Device Co., Ltd. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without modification,
 * are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice, this list of
 * conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form must reproduce the above copyright notice, this list
 * of conditions and the following disclaimer in the documentation and/or other materials
 * provided with the distribution.
 *
 * 3. Neither the name of the copyright holder nor the names of its contributors may be used
 * to endorse or promote products derived from this software without specific prior written
 * permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
 * THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS;
 * OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
 * OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF
 * ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include "It_posix_mutex.h"

UINT32 PosixPthreadDestroy(pthread_attr_t *attr, pthread_t thread)
{
    UINT32 uwRet = 0;

    uwRet = pthread_join(thread, NULL);
    ICUNIT_GOTO_EQUAL(uwRet, 0, uwRet, NOK);

    uwRet = pthread_attr_destroy(attr);
    ICUNIT_GOTO_EQUAL(uwRet, 0, uwRet, NOK);

    return LOS_OK;
NOK:
    return LOS_NOK;
}

UINT32 PosixPthreadInit(pthread_attr_t *attr, int pri)
{
    UINT32 uwRet = 0;
    struct sched_param sp;

    uwRet = pthread_attr_init(attr);
    ICUNIT_GOTO_EQUAL(uwRet, 0, uwRet, NOK);

    uwRet = pthread_attr_setinheritsched(attr, PTHREAD_EXPLICIT_SCHED);
    ICUNIT_GOTO_EQUAL(uwRet, 0, uwRet, NOK);

    sp.sched_priority = pri;
    uwRet = pthread_attr_setschedparam(attr, &sp);
    ICUNIT_GOTO_EQUAL(uwRet, 0, uwRet, NOK);

    return LOS_OK;
NOK:
    return LOS_NOK;
}

VOID TestExtraTaskDelay(UINT32 uwTick)
{
#ifdef LOSCFG_KERNEL_SMP
    // trigger task schedule may occor on another core
    // needs adding delay and checking status later
    LosTaskDelay(uwTick);
#else
    // do nothing
#endif
}

VOID ItSuitePosixMutex(void)
{
    PRINTF("*********** Begin sample posix mutex test ************\n");
    ItPosixMux001();
    ItPosixMux002();
    ItPosixMux003();
    ItPosixMux004();
    ItPosixMux005();
    ItPosixMux006();
    ItPosixMux007();
    ItPosixMux008();
    ItPosixMux009();
    ItPosixMux010();
    ItPosixMux011();
    ItPosixMux012();
    ItPosixMux013();
    ItPosixMux014();
    ItPosixMux015();
    ItPosixMux016();
    ItPosixMux017();
    ItPosixMux018();
    ItPosixMux019();
    ItPosixMux020();
    ItPosixMux021();
    ItPosixMux022();
    ItPosixMux023();
    ItPosixMux024();
    ItPosixMux025();
    ItPosixMux026();
    ItPosixMux027();
    ItPosixMux028();
    ItPosixMux029();
    ItPosixMux030();
    ItPosixMux031();
    ItPosixMux032();
    ItPosixMux033();
    ItPosixMux034();
    ItPosixMux035();
    ItPosixMux036();
    ItPosixMux037();
    ItPosixMux038();
    ItPosixMux039();
    ItPosixMux040();
    ItPosixMux041();
    ItPosixMux042();
    ItPosixMux043();
    ItPosixMux044();
    ItPosixMux045();
    ItPosixMux046();
    ItPosixMux047();
    ItPosixMux048();
#ifndef LOSCFG_KERNEL_SMP
    ItPosixMux049();
#endif
}

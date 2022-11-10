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

#include "It_posix_pthread.h"

/*
 * return value of pthread_self() is 0 when
 * pthread create from LOS_TaskCreate()
 */
pthread_t TestPthreadSelf(void)
{
    pthread_t tid = pthread_self();
    if (tid == 0) {
        tid = ((LosTaskCB *)(OsCurrTaskGet()))->taskID;
    }
    return tid;
}

void ItSuitePosixPthread(void)
{
    printf("************** begin SAMPLE POSIX pthread test *************\n");
    ItPosixPthread001();
    ItPosixPthread002();
    ItPosixPthread003();
    ItPosixPthread004();
    ItPosixPthread005();
    ItPosixPthread006();
    ItPosixPthread007();
    ItPosixPthread008();
    ItPosixPthread009();
    ItPosixPthread010();
    ItPosixPthread011();
    ItPosixPthread012();
    ItPosixPthread013();
    ItPosixPthread014();
    ItPosixPthread015();
    ItPosixPthread016();
    ItPosixPthread017();
    ItPosixPthread018();
    ItPosixPthread019();
    ItPosixPthread020();
    ItPosixPthread021();
    ItPosixPthread022();
    ItPosixPthread023();
    ItPosixPthread024();
    ItPosixPthread025();
}

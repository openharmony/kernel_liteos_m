/*
 * Copyright (c) 2022 Huawei Device Co., Ltd. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without modification,
 * are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice, this list of
 *    conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form must reproduce the above copyright notice, this list
 *    of conditions and the following disclaimer in the documentation and/or other materials
 *    provided with the distribution.
 *
 * 3. Neither the name of the copyright holder nor the names of its contributors may be used
 *    to endorse or promote products derived from this software without specific prior written
 *    permission.
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

#include "It_posix_rwlock.h"

VOID ItSuitePosixRwlock(void)
{
    dprintf("*********** Begin sample posix rwlock test ************\n");
    ItPosixRwlock001();
    ItPosixRwlock003();
    ItPosixRwlock004();
    ItPosixRwlock006();
    ItPosixRwlock007();
#ifndef LOSCFG_PLATFORM_HI3322
    ItPosixRwlock008();
    ItPosixRwlock009();
#endif
    ItPosixRwlock059();
    ItPosixRwlock060();
    ItPosixRwlock061();
    ItPosixRwlock062();
    ItPosixRwlock063();
    ItPosixRwlock064();
    ItPosixRwlock065();
    //ItPosixRwlock066();
    ItPosixRwlock067();
    ItPosixRwlock068();
    //ItPosixRwlock069();
    //ItPosixRwlock070();
    //ItPosixRwlock071();
    ItPosixRwlock072();
    ItPosixRwlock073();
    ItPosixRwlock074();
    //ItPosixRwlock075();
}

/*
 * Copyright (c) 2013-2019 Huawei Technologies Co., Ltd. All rights reserved.
 * Copyright (c) 2020-2021 Huawei Device Co., Ltd. All rights reserved.
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

#include "los_arch_regs.h"

.text
.align    4
.literal_position
.global   SpillWindow
SpillWindow:

    rsr     a2, WINDOWBASE    /* read windowbase reg */

    movi    a3, WINDOWSTARTBITS
    sub     a3, a3, a2        /* get the num of high bits */
    addi    a3, a3, -1
    
    ssl     a3                /* set left shift bit */
    rsr     a3, WINDOWSTART   /* read windowstart reg */
    sll     a3, a3
    movi    a2, WINDOWSTARTMASK
    and     a3, a3, a2

    beqi    a3, 0, .Linvalidwindowstart /* if a3 == 0 goto Linvalidwindowstart */

    rsr     a2, WINDOWBASE    /* read windowbase reg */

    addi    a2, a2, 1
    ssr     a2
    rsr     a2, WINDOWSTART
    srl     a2, a2
    or      a2, a2, a3
    movi    a3, 0

.bitclear0:
    bbsi.l  a2, 0, .bitcleardone
    srli    a2, a2, 1
    addi    a3, a3, 1
    j       .bitclear0

.bitcleardone:
    srli    a2, a2, 1         /* drop the first bit */

    wsr     a2, WINDOWSTART
    rsr     a2, WINDOWBASE

    add     a2, a2, a3
    wsr     a2, WINDOWBASE
    rsync

    rsr     a2, WINDOWSTART

.Lspillstart:
    beqi    a2, 0, .Ldone
    bbsi.l  a2, 0, .Lspill4
    bbsi.l  a2, 1, .Lspill8
    bbsi.l  a2, 2, .Lspill12
    j       .Linvalidwindow

.Lspill4:
    s32e    a4, a9, -16
    s32e    a5, a9, -12
    s32e    a6, a9, -8
    s32e    a7, a9, -4

    srli    a6, a2, 1
    rotw    1
    j       .Lspillstart

.Lspill8:
    s32e    a4, a13, -16
    s32e    a5, a13, -12
    s32e    a6, a13, -8
    s32e    a7, a13, -4

    l32i    a3, a5, -12       /* call[i - 1]'s sp */
    addi    a3, a3, -16       /* base area */

    s32e    a8, a3, -16
    s32e    a9, a3, -12
    s32e    a10, a3, -8
    s32e    a11, a3, -4

    srli    a10, a2, 2
    rotw    2
    j       .Lspillstart

.Lspill12:
    rotw    1
    s32e    a0, a13, -16
    s32e    a1, a13, -12
    s32e    a2, a13, -8
    s32e    a3, a13, -4
   
    l32i    a3, a1, -12   /* call[i - 1]'s sp */
    addi    a3, a3, -16   /* base area */
   
    s32e    a4, a3, -32
    s32e    a5, a3, -28
    s32e    a6, a3, -24
    s32e    a7, a3, -20
    s32e    a8, a3, -16
    s32e    a9, a3, -12
    s32e    a10, a3, -8
    s32e    a11, a3, -4

    rotw    -1
    srli    a14, a2, 3
    rotw    3
    j       .Lspillstart

.Ldone:
    rotw    1
    rsr     a2, WINDOWBASE
    ssl     a2
    movi    a2, 1
    sll     a2, a2
    wsr     a2, WINDOWSTART
    rsync
    movi    a2, 0
    ret

.Linvalidwindowstart:
    movi    a2, 1
    ret    

.Linvalidwindow:
    movi    a3, 1
    slli    a2, a2, 1
    or      a2, a2, a3
    rsr     a3, WINDOWBASE

.RestoreWindowBase:
    bbsi.l  a2, WINDOWSTARTBITS - 1, .RestoreWindow
    slli    a2, a2, 1
    addi    a3, a3, -1
    j       .RestoreWindowBase

.RestoreWindow:
    extui   a3, a3, 0, WINDOWSTARTBITS    /* the original bit */

    addi    a3, a3, 1
    ssl     a3
    sll     a3, a2
    slli    a2, a2, 32 - WINDOWSTARTBITS
    srl     a2, a2

    or      a2, a2, a3
    extui   a2, a2, 0, WINDOWSTARTBITS
    wsr     a2, WINDOWSTART

    movi    a3, 31
    rsr     a2, SAR
    sub     a2, a3, a2
    wsr     a2, WINDOWBASE
    rsync

    movi    a2, 2
    ret

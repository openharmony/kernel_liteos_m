/*
 * Copyright (c) 2013-2020, Huawei Technologies Co., Ltd. All rights reserved.
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
#include <csi_config.h>

#define VIC_TSPDR 0XE000EC08
#define OS_TASK_STATUS_RUNNING    0x0010
#define VIC_TSPDR                 0XE000EC08

.section .text
.align  2
/********************************************************************
 * Functions: HalTaskContextSwitch
 *
 ********************************************************************/
.global HalTaskContextSwitch
.type   HalTaskContextSwitch, %function
HalTaskContextSwitch:
    lrw    r0, VIC_TSPDR
    bgeni  r1, 0
    stw    r1, (r0)
	nop
	nop
	nop
    rts
    
.type HalStartToRun, %function
.global HalStartToRun
HalStartToRun:
    psrclr   ie
    lrw      r4, g_losTask
    ld.w     r4, (r4)
    ld.w     sp, (r4)

    ldw      r0, (sp, 192)
    mtcr     r0, epc
    ldw      r0, (sp, 188)
    mtcr     r0, epsr

    ldm      r0-r13, (sp)
    ldw      r15, (sp, 56)
    addi     sp, 60
    ldm      r16-r31, (sp)
    addi     sp, 64
    fldms    vr0-vr15, (sp)
    addi     sp, 72

    rte

.align  2
.type tspend_handler, %function
.global tspend_handler
tspend_handler:

    subi     sp, 196
    stm      r0-r13, (sp)
    stw      r15, (sp, 56)
    addi     r0, sp, 60
    stm      r16-r31, (r0)
    addi     r0, 64
    fstms    vr0-vr15, (r0)
    mfcr     r1, epsr
    stw      r1, (r0, 64)
    mfcr     r1, epc
    stw      r1, (r0, 68)

    jbsr    OsSchedTaskSwitch
    bez     r0, ret_con
    
    lrw    r2, g_losTask
    ldw    r0, (r2)
    stw    sp, (r0)

    lrw    r3, g_losTask + 4
    ldw    r0, (r3)
    stw    r0, (r2)
    
#if 0
    lrw     r4, g_sysSchedCount
    ldw     r3, (r4, 0)
    subi    r3, 1
    stw     r3, (r4, 0)    
#endif

    ldw    sp, (r0)
    
 ret_con:   
    ldw      r0, (sp, 192)
    mtcr     r0, epc
    ldw      r0, (sp, 188)
    mtcr     r0, epsr

    ldm      r0-r13, (sp)
    ldw      r15, (sp, 56)
    addi     sp, 60
    ldm      r16-r31, (sp)
    addi     sp, 64
    fldms    vr0-vr15, (sp)
    addi     sp, 72

    rte


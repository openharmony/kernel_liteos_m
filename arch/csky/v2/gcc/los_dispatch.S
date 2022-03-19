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

#define OS_TASK_STATUS_RUNNING    0x0010
#define VIC_TSPDR                 0XE000EC08

#ifdef CPU_CK804
.section .text
.align  2
.type HalStartToRun, %function
.global HalStartToRun
HalStartToRun:
    lrw    r1, g_losTask
    lrw    r2, g_losTask + 4

    ldw    r0, (r2)

    st.w   r0, (r1)
    st.w   r0, (r2)

    ldw    sp, (r0)

    ldw    r0, (sp, 128)
    mtcr   r0, epc
    ldw    r0, (sp, 124)
    mtcr   r0, epsr
    ldw    r15, (sp, 56)
    ldm    r0-r13, (sp)
    addi     sp, 60
    ldm      r16-r31, (sp)
    addi     sp, 72

    rte

.align  2
.type HalTaskContextSwitch, %function
.global HalTaskContextSwitch
HalTaskContextSwitch:
    lrw    r0, VIC_TSPDR
    bgeni  r1, 0
    stw    r1, (r0)
    nop
    nop
    nop
    rts
    
.align  2
.type tspend_handler, %function
.global tspend_handler
tspend_handler:

    subi   sp, 132
    stm    r0-r13, (sp)
    stw    r15, (sp, 56)
    addi     r0, sp, 60
    stm      r16-r31, (r0)
    mfcr     r1, epsr
    stw    r1, (sp, 124)
    mfcr     r1, epc
    stw    r1, (sp, 128)

    jbsr    OsSchedTaskSwitch
    bez     r0, ret_con
    
    lrw    r2, g_losTask
    ldw    r0, (r2)
    stw    sp, (r0)

    lrw    r3, g_losTask + 4
    ldw    r0, (r3)
    stw    r0, (r2)

    ldw    sp, (r0)
    
 ret_con:   
    ldw    r0, (sp, 128)
    mtcr   r0, epc
    ldw    r0, (sp, 124)
    mtcr   r0, epsr
    ldw    r15, (sp, 56)
    ldm    r0-r13, (sp)
    addi     sp, 60
    ldm      r16-r31, (sp)

    addi   sp, 72

    rte
    
#else
.section .text
.align  2
.type HalStartToRun, %function
.global HalStartToRun
HalStartToRun:
    lrw    r1, g_losTask
    lrw    r2, g_losTask + 4

    ldw    r0, (r2)

    st.w   r0, (r1)
    st.w   r0, (r2)

    ldw    sp, (r0)

    ldw    r0, (sp, 64)
    mtcr   r0, epc
    ldw    r0, (sp, 60)
    mtcr   r0, epsr
    ldw    r15, (sp, 56)
    ldm    r0-r13, (sp)

    rte

.align  2
.type HalTaskContextSwitch, %function
.global HalTaskContextSwitch
HalTaskContextSwitch:

    subi   sp, 68
    stm    r0-r13, (sp)
    stw    r15, (sp, 56)
    mfcr   r1, psr
    stw    r1, (sp, 60)
    stw    r15, (sp, 64)
    lrw    r2, g_losTask
    ldw    r0, (r2)
    stw    sp, (r0)

    lrw    r3, g_losTask + 4
    ldw    r0, (r3)
    stw    r0, (r2)

    ldw    sp, (r0)
    ldw    r0, (sp, 64)
    mtcr   r0, epc
    ldw    r0, (sp, 60)
    mtcr   r0, epsr
    ldw    r15, (sp, 56)
    ldm    r0-r13, (sp)

    addi   sp, 68

    rte

#endif

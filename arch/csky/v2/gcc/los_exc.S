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

.import HalExcHandleEntry
.extern g_trapStackBase

#ifdef CPU_CK804
.section .text
.align  2
.global HandleEntry
HandleEntry:
    mov    r10, sp
    lrw    r14, g_trapStackBase

    stm    r0-r15, (sp)
    stw    r10, (sp, 56)
    addi     r0, sp, 64
    stm      r16-r31, (r0)
    mfcr   r0, epsr
    stw    r0, (sp, 128)
    mfcr   r0, epc
    stw    r0, (sp, 132)
    mov    r0, sp

    mfcr   r1, epc

    mov    sp, r10
    lrw    r2, HalExcHandleEntry
    jmp    r2

.section .text
.align  2
.global IrqEntry
IrqEntry:
    psrset  ee
    subi   sp, 136
    stm    r0-r15, (sp)
    addi     r0, sp, 64
    stm      r16-r31, (r0)
    mfcr   r0, epsr
    stw    r0, (sp, 128)
    mfcr   r0, epc
    stw    r0, (sp, 132)

    jbsr   HalInterrupt

    ldw    r0, (sp, 132)
    mtcr   r0, epc
    ldw    r0, (sp, 128)
    bseti  r0, r0, 6
    mtcr   r0, epsr
    ldm    r0-r15, (sp)
    addi     sp, 64
    ldm      r16-r31, (sp)
    addi   sp, 72
    rte
#else
.section .text
.align  2
.global HandleEntry
HandleEntry:
    mov    r10, sp
    lrw    r14, g_trapStackBase

    stm    r0-r15, (sp)
    stw    r10, (sp, 56)
    mfcr   r0, epsr
    stw    r0, (sp, 64)
    mfcr   r0, epc
    stw    r0, (sp, 68)
    mov    r0, sp

    mfcr   r1, epc

    mov    sp, r10
    lrw    r2, HalExcHandleEntry
    jmp    r2

.section .text
.align  2
.global IrqEntry
IrqEntry:
    psrset  ee
    subi   sp, 72
    stm    r0-r15, (sp)
    mfcr   r0, epsr
    stw    r0, (sp, 64)
    mfcr   r0, epc
    stw    r0, (sp, 68)

    jbsr   HalInterrupt

    ldw    r0, (sp, 68)
    mtcr   r0, epc
    ldw    r0, (sp, 64)
    bseti  r0, r0, 6
    mtcr   r0, epsr
    ldm    r0-r15, (sp)
    addi   sp, 72
    rte
#endif
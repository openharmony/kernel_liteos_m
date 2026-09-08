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

    PRESERVE8

    EXPORT HalSecureContextInitAsm
    EXPORT HalSecureContextLoadAsm
    EXPORT HalSecureContextSaveAsm

OS_SECURE_SCB_AIRCR     EQU     0xE000ED0C

    SECTION     .text:CODE(2)
    THUMB
    REQUIRE8

HalSecureContextInitAsm
    LDR     R0, =OS_SECURE_SCB_AIRCR
    LDR     R1, [R0]

    MOV     R2, #0xFFFF
    LSL     R2, R2, #16
    BIC     R1, R1, R2
    MOV     R2, #0x05FA
    LSL     R2, R2, #16
    ORR     R1, R1, R2

    BIC     R1, R1, #0x4000
    MOV     R2, #0x4000
    ORR     R1, R1, R2
    STR     R1, [R0]

    MOV     R0, #0
    MSR     PSPLIM, R0
    MSR     PSP, R0
    MOV     R0, #2
    MSR     CONTROL, R0 
    BX      LR

HalSecureContextLoadAsm
    MRS     R1, IPSR
    CBZ     R1, __ThreadMode

    LDMIA   R0!, {R1, R2}               /* R1 = g_secureContext->curStackPointer, R2 = g_secureContext->stackLimit. */
    MSR     PSPLIM, R2                  /* Restore PSPLIM. */
    MSR     PSP, R1                     /* Restore PSP. */
    BX      LR

HalSecureContextSaveAsm
    MRS     R0, IPSR
    CBZ     R0, __ThreadMode

    MRS     R0, PSP
    STR     R0, [R1]                    /* g_secureContext->curStackPointer = R0. */
    MOV     R0, #0
    MSR     PSPLIM, R0                  /* No PSPLIM for the current task. */
    MSR     PSP, R0                     /* No secure stack for the current task. */

__ThreadMode
    BX      LR

    END
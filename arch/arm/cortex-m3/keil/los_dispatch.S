;
; Copyright (c) 2013-2019 Huawei Technologies Co., Ltd. All rights reserved.
; Copyright (c) 2020-2021 Huawei Device Co., Ltd. All rights reserved.
;
; Redistribution and use in source and binary forms, with or without modification,
; are permitted provided that the following conditions are met:
;
; 1. Redistributions of source code must retain the above copyright notice, this list of
;    conditions and the following disclaimer.
;
; 2. Redistributions in binary form must reproduce the above copyright notice, this list
;    of conditions and the following disclaimer in the documentation and/or other materials
;    provided with the distribution.
;
; 3. Neither the name of the copyright holder nor the names of its contributors may be used
;    to endorse or promote products derived from this software without specific prior written
;    permission.
;
; THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
; "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
; THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
; PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR
; CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
; EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
; PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS;
; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
; WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
; OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF
; ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
;

        PRESERVE8

        EXPORT  HalIntLock
        EXPORT  HalIntUnLock
        EXPORT  HalIntRestore
        EXPORT  HalStartToRun
        EXPORT  HalTaskSchedule
        EXPORT  HalPendSV
        IMPORT  OsSchedTaskSwitch
        IMPORT  g_losTask

OS_NVIC_INT_CTRL            EQU     0xE000ED04
OS_NVIC_SYSPRI2             EQU     0xE000ED20
OS_NVIC_PENDSV_PRI          EQU     0xF0F00000
OS_NVIC_PENDSVSET           EQU     0x10000000
OS_TASK_STATUS_RUNNING      EQU     0x0010

    AREA  |.text|, CODE, READONLY
    THUMB
    REQUIRE8

HalStartToRun
    LDR     R4, =OS_NVIC_SYSPRI2
    LDR     R5, =OS_NVIC_PENDSV_PRI
    STR     R5, [R4]

    MOV     R0, #2
    MSR     CONTROL, R0

    LDR     R1, =g_losTask
    LDR     R0, [R1, #4]
    LDR     R12, [R0]
    ;ADD     R12, R12, #100
    ADD     R12, R12, #36

    LDMFD   R12!, {R0-R7}
    ;ADD     R12, R12, #72
    MSR     PSP, R12
    ;VPUSH   S0;
    ;VPOP    S0;

    MOV     LR, R5
   ;MSR     xPSR, R7

    CPSIE   I
    BX      R6


HalIntLock
    MRS     R0, PRIMASK
    CPSID   I
    BX      LR

HalIntUnLock
    MRS     R0, PRIMASK
    CPSIE   I
    BX      LR

HalIntRestore
    MSR     PRIMASK, R0
    BX      LR

HalTaskSchedule
    LDR     R0, =OS_NVIC_INT_CTRL
    LDR     R1, =OS_NVIC_PENDSVSET
    STR     R1, [R0]
    DSB
    ISB
    BX      LR

HalPendSV
    MRS     R12, PRIMASK
    CPSID   I

HalTaskSwitch
    PUSH    {R12, LR}
    BL      OsSchedTaskSwitch
    POP     {R12, LR}
    CMP     R0, #0
    MOV     R0, LR
    BNE     TaskContextSwitch
    MSR     PRIMASK, R12
    BX      LR

TaskContextSwitch
    MOV     LR, R0
    MRS     R0, PSP

    STMFD   R0!, {R4-R12}
    ;VSTMDB  R0!, {D8-D15}

    LDR     R5, =g_losTask
    LDR     R6, [R5]
    STR     R0, [R6]

    LDR     R0, [R5, #4]
    STR     R0, [R5]

    LDR     R1,   [R0]
    ;VLDMIA  R1!, {D8-D15}
    LDMFD   R1!, {R4-R12}
    MSR     PSP,  R1

    MSR     PRIMASK, R12
    BX      LR

    END

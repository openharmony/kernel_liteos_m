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

#include "los_backtrace.h"
#include "los_task.h"
#include "los_debug.h"
#include "los_arch.h"
#if (LOSCFG_BACKTRACE_TYPE == 4)
#include "los_arch_regs.h"
#endif

#if (LOSCFG_BACKTRACE_TYPE != 0)
/* This function is used to judge whether the data in the stack is a code section address.
   The default code section is only one, but there may be more than one. Modify the
   judgment condition to support multiple code sections. */
WEAK BOOL OsStackDataIsCodeAddr(UINTPTR value)
{
    if ((value >= CODE_START_ADDR) && (value < CODE_END_ADDR)) {
        return TRUE;
    }
    return FALSE;
}

#if (LOSCFG_BACKTRACE_TYPE == 1)
#define OS_BACKTRACE_START     2
/* Thumb instruction, so the pc must be an odd number */
#define OS_IS_THUMB_INSTRUCTION(pc)    ((pc & 0x1) == 1)

/* BL or BLX instruction flag. */
#define OS_BL_INS_MASK     0xF800
#define OS_BL_INS_HIGH     0xF800
#define OS_BL_INS_LOW      0xF000
#define OS_BLX_INX_MASK    0xFF00
#define OS_BLX_INX         0x4700

STATIC INLINE BOOL OsInsIsBlOrBlx(UINTPTR addr)
{
    UINT16 ins1 = *((UINT16 *)addr);
    UINT16 ins2 = *((UINT16 *)(addr + 2)); /* 2: Thumb instruction is two bytes. */

    if (((ins2 & OS_BL_INS_MASK) == OS_BL_INS_HIGH) &&
        ((ins1 & OS_BL_INS_MASK) == OS_BL_INS_LOW)) {
        return TRUE;
    } else if ((ins2 & OS_BLX_INX_MASK) == OS_BLX_INX) {
        return TRUE;
    } else {
        return FALSE;
    }
}

STATIC INLINE UINT32 OsStackAddrGet(UINTPTR *stackStart, UINTPTR *stackEnd, UINTPTR SP)
{
    if (SP != 0) {
        *stackStart = SP;
        if ((SP >= CSTACK_START_ADDR) && (SP < CSTACK_END_ADDR)) {
            *stackEnd = CSTACK_END_ADDR;
        } else {
            UINT32 taskID = LOS_CurTaskIDGet();
            LosTaskCB *taskCB = OS_TCB_FROM_TID(taskID);
            *stackEnd = (UINTPTR)taskCB->topOfStack + taskCB->stackSize;
            if ((SP < (UINTPTR)taskCB->topOfStack) || (SP >= *stackEnd)) {
                PRINT_ERR("msp statck [0x%x, 0x%x], cur task stack [0x%x, 0x%x], cur sp(0x%x) is overflow!\n",
                          CSTACK_START_ADDR, CSTACK_END_ADDR, (UINTPTR)taskCB->topOfStack, *stackEnd, SP);
                return LOS_NOK;
            }
        }
    } else {
        if (ArchSpGet() != ArchPspGet()) {
            *stackStart = ArchMspGet();
            *stackEnd = CSTACK_END_ADDR;
            if ((*stackStart < CSTACK_START_ADDR) || (*stackStart >= CSTACK_END_ADDR)) {
                PRINT_ERR("msp stack [0x%x, 0x%x], cur sp(0x%x) is overflow!\n",
                          CSTACK_START_ADDR, CSTACK_END_ADDR, *stackStart);
                return LOS_NOK;
            }
            PRINTK("msp, start = %x, end = %x\n", *stackStart, *stackEnd);
        } else {
            *stackStart = ArchPspGet();
            UINT32 taskID = LOS_CurTaskIDGet();
            LosTaskCB *taskCB = OS_TCB_FROM_TID(taskID);
            *stackEnd = (UINTPTR)taskCB->topOfStack + taskCB->stackSize;
            if ((*stackStart < (UINTPTR)taskCB->topOfStack) || (*stackStart >= *stackEnd)) {
                PRINT_ERR("psp stack [0x%x, 0x%x], cur sp(0x%x) is overflow, cur task id is %d!\n",
                          taskCB->topOfStack, *stackEnd, *stackStart, taskID);
                return LOS_NOK;
            }
            PRINTK("psp, start = %x, end = %x\n", *stackStart, *stackEnd);
        }
    }

    return LOS_OK;
}

STATIC INLINE UINTPTR OsAddrIsValid(UINTPTR sp)
{
    UINTPTR pc;
    BOOL ret;

    /* The stack space pointed to by the current SP may store the LR,
       so need decrease a word to PC. */
    pc = *((UINTPTR *)sp) - sizeof(UINTPTR);

    if (!OS_IS_THUMB_INSTRUCTION(pc)) {
        return 0;
    }

    /* PC in thumb mode is an odd number, fix the PC address by decreasing one byte. */
    pc = *((UINTPTR *)sp) - 1;

    ret = OsStackDataIsCodeAddr(pc);
    if (ret == FALSE) {
        return 0;
    }

    ret = OsInsIsBlOrBlx(pc - sizeof(UINTPTR));
    if (ret == FALSE) {
        return 0;
    }

    return pc;
}
#elif (LOSCFG_BACKTRACE_TYPE == 2)
#define OS_BACKTRACE_START     1
#define OS_RA_OFFSET           4
#define OS_FP_OFFSET           8
#define OS_FP_ALIGN(value)     (((UINT32)(value) & (UINT32)(LOSCFG_STACK_POINT_ALIGN_SIZE - 1)) == 0)

STATIC INLINE UINTPTR OsFpGet(VOID)
{
    UINTPTR fp = 0;
    __asm volatile("mv %0, s0" : "=r"(fp));
    dsb();
    return fp;
}

WEAK BOOL IsValidFP(UINTPTR fp)
{
    LosTaskCB *taskCB = NULL;
    UINTPTR stackTop, stackBottom;
    UINTPTR irqStackTop, irqStackBottom;

    if ((fp == FP_INIT_VALUE) || !OS_FP_ALIGN(fp)) {
        return FALSE;
    }

    if (LOS_TaskIsRunning()) {
        taskCB = OS_TCB_FROM_TID(LOS_CurTaskIDGet());
        stackTop = taskCB->topOfStack;
        stackBottom = taskCB->topOfStack + taskCB->stackSize;
        irqStackTop = (UINTPTR)CSTACK_START_ADDR;
        irqStackBottom = (UINTPTR)CSTACK_SECTION_END;
    } else {
        stackTop = 0;
        stackBottom = 0;
        irqStackTop = (UINTPTR)CSTACK_START_ADDR;
        irqStackBottom = (UINTPTR)CSTACK_SECTION_END;
    }

    if (((fp > stackTop) && (fp <= stackBottom)) || ((fp > irqStackTop) && (fp <= irqStackBottom))) {
        return TRUE;
    }

    return FALSE;
}

VOID LOS_RecordLR(UINTPTR *LR, UINT32 LRSize, UINT32 jumpCount, UINTPTR SP)
{
    UINTPTR backFp;
    UINTPTR tmpFp;
    UINTPTR backRa;
    UINT32 count = 0;
    UINT32 index = 0;

    if (LR == NULL) {
        return;
    }

    if (SP != 0) {
        backFp = SP;
    } else {
        backFp = OsFpGet();
    }

    if (!IsValidFP(backFp)) {
        PRINT_ERR("BackTrace failed! Invalid fp 0x%x\n", backFp);
        return;
    }

    do {
        tmpFp = backFp;
        backRa = *((UINTPTR *)(UINTPTR)(tmpFp - OS_RA_OFFSET));
        backFp = *((UINTPTR *)(UINTPTR)(tmpFp - OS_FP_OFFSET));
        if (index++ < jumpCount) {
            continue;
        }

        LR[count] = backRa;
        count++;
        if ((count == LRSize) || (backFp == tmpFp)) {
            break;
        }
    } while (IsValidFP(backFp));

    if (count < LRSize) {
        LR[count] = 0;
    }
}
#elif (LOSCFG_BACKTRACE_TYPE == 3)
#define OS_BACKTRACE_START  1
#define OS_JALX_INS_MASK    0x7F
#define OS_JAL_INS_LOW      0x6F
#define OS_JAL_16_INS_MASK  0x2001
#define OS_JALR_INS_LOW     0x67
#define OS_JALR_16_INS_MASK 0x9002
#define OS_JR_16_INS_MASK   0x8002
#define OS_J_16_INS_MASK    0xA001

STATIC INLINE BOOL OsInsIsJump(UINTPTR addr)
{
    UINT16 ins1 = *((UINT16 *)addr);
    UINT16 ins2 = *((UINT16 *)(addr + 2)); // 2, for the mask

    /* Jal ins */
    if (((ins1 & OS_JALX_INS_MASK) == OS_JAL_INS_LOW) ||
        ((ins1 & OS_JAL_16_INS_MASK) == OS_JAL_16_INS_MASK) ||
        ((ins2 & OS_JAL_16_INS_MASK) == OS_JAL_16_INS_MASK)) {
        return TRUE;
    }

    /* Jalr ins */
    if (((ins1 & OS_JALX_INS_MASK) == OS_JALR_INS_LOW) ||
        ((ins1 & OS_JALR_16_INS_MASK) == OS_JALR_16_INS_MASK) ||
        ((ins2 & OS_JALR_16_INS_MASK) == OS_JALR_16_INS_MASK)) {
        return TRUE;
    }

    /* Jr ins */
    if (((ins1 & OS_JR_16_INS_MASK) == OS_JR_16_INS_MASK) ||
        ((ins2 & OS_JR_16_INS_MASK) == OS_JR_16_INS_MASK)) {
        return TRUE;
    }

    /* J ins */
    if (((ins1 & OS_J_16_INS_MASK) == OS_J_16_INS_MASK) ||
        ((ins2 & OS_J_16_INS_MASK) == OS_J_16_INS_MASK)) {
        return TRUE;
    }

    return FALSE;
}

STATIC INLINE UINTPTR OsSpGet(VOID)
{
    UINTPTR sp = 0;
    __asm volatile("mv %0, sp" : "=r"(sp));
    dsb();
    return sp;
}

STATIC INLINE UINT32 OsStackAddrGet(UINTPTR *stackStart, UINTPTR *stackEnd, UINTPTR SP)
{
    if (SP != 0) {
        *stackStart = SP;
        if ((SP >= CSTACK_START_ADDR) && (SP < CSTACK_END_ADDR)) {
            *stackEnd = CSTACK_END_ADDR;
        } else {
            UINT32 taskID = LOS_CurTaskIDGet();
            LosTaskCB *taskCB = OS_TCB_FROM_TID(taskID);
            *stackEnd = (UINTPTR)taskCB->topOfStack + taskCB->stackSize;
            if ((SP < (UINTPTR)taskCB->topOfStack) || (SP >= *stackEnd)) {
                PRINT_ERR("msp statck [0x%x, 0x%x], cur task stack [0x%x, 0x%x], cur sp(0x%x) is overflow!\n",
                          CSTACK_START_ADDR, CSTACK_END_ADDR, (UINTPTR)taskCB->topOfStack, *stackEnd, SP);
                return LOS_NOK;
            }
        }
    } else {
        if (!LOS_TaskIsRunning()) {
            *stackStart = OsSpGet();
            *stackEnd = CSTACK_END_ADDR;
            if ((*stackStart < CSTACK_START_ADDR) || (*stackStart >= CSTACK_END_ADDR)) {
                PRINT_ERR("msp stack [0x%x, 0x%x], cur sp(0x%x) is overflow!\n",
                          CSTACK_START_ADDR, CSTACK_END_ADDR, *stackStart);
                return LOS_NOK;
            }
        } else {
            *stackStart = OsSpGet();
            UINT32 taskID = LOS_CurTaskIDGet();
            LosTaskCB *taskCB = OS_TCB_FROM_TID(taskID);
            *stackEnd = (UINTPTR)taskCB->topOfStack + taskCB->stackSize;
            if ((*stackStart < (UINTPTR)taskCB->topOfStack) || (*stackStart >= *stackEnd)) {
                PRINT_ERR("psp stack [0x%x, 0x%x], cur sp(0x%x) is overflow, cur task id is %d!\n",
                          taskCB->topOfStack, *stackEnd, *stackStart, taskID);
                return LOS_NOK;
            }
        }
    }

    return LOS_OK;
}

STATIC INLINE UINTPTR OsAddrIsValid(UINTPTR sp)
{
    UINTPTR pc;
    BOOL ret;

    pc = *((UINTPTR *)sp);

    ret = OsStackDataIsCodeAddr(pc);
    if (ret == FALSE) {
        return 0;
    }

    ret = OsInsIsJump(pc - sizeof(UINTPTR));
    if (ret == FALSE) {
        return 0;
    }

    return pc;
}

#elif (LOSCFG_BACKTRACE_TYPE == 4)
#define OS_BACKTRACE_START     0
#define ALIGN_MASK             (4 - 1)
#define OS_REG_LR_OFFSET       (CONTEXT_SIZE - 8)

UINT32 IsSpAligned(UINT32 value)
{
    return (value & (UINT32)(ALIGN_MASK)) == 0;
}

STATIC INLINE UINTPTR HalGetLr(VOID)
{
    UINTPTR regLr;

    __asm__ __volatile__("mov %0, a0" : "=r"(regLr));

    return regLr;
}

/* This function is used to check sp address. */
BOOL IsValidSP(UINTPTR regSP, UINTPTR start, UINTPTR end)
{
    return (regSP >= start) && (regSP <= end) && IsSpAligned(regSP);
}

/* This function is used to check return address. */
BOOL IsValidRa(UINTPTR regRA)
{
    regRA &= ~VIR_TEXT_ADDR_MASK;
    regRA |= TEXT_ADDR_MASK;

    return OsStackDataIsCodeAddr(regRA);
}

BOOL FindSuitableStack(UINTPTR regSP, UINTPTR *start, UINTPTR *end)
{
    UINT32 stackStart;
    UINT32 stackEnd;
    BOOL found = FALSE;

    if (LOS_TaskIsRunning()) {
        UINT32 taskID = LOS_CurTaskIDGet();
        LosTaskCB *taskCB = OS_TCB_FROM_TID(taskID);
        stackStart = taskCB->topOfStack;
        stackEnd = taskCB->topOfStack + taskCB->stackSize;
        if (IsValidSP(regSP, stackStart, stackEnd)) {
            found = TRUE;
            goto FOUND;
        }
    }

    if (IsValidSP(regSP, CSTACK_START_ADDR, CSTACK_END_ADDR)) {
        stackStart = CSTACK_START_ADDR;
        stackEnd = CSTACK_END_ADDR;
        found = TRUE;
        goto FOUND;
    }

FOUND:
    if (found == TRUE) {
        *start = stackStart;
        *end = stackEnd;
    }

    return found;
}

UINT32 HalBackTraceGet(UINTPTR sp, UINT32 retAddr, UINTPTR *callChain, UINT32 maxDepth, UINT32 jumpCount)
{
    UINTPTR tmpSp;
    UINT32 tmpRa;
    UINTPTR backRa = retAddr;
    UINTPTR backSp = sp;
    UINTPTR stackStart;
    UINT32 stackEnd;
    UINT32 count = 0;
    UINT32 index = 0;

    if (FindSuitableStack(sp, &stackStart, &stackEnd) == FALSE) {
        PRINTK("sp:0x%x error, backtrace failed!\n", sp);
        return 0;
    }

    while (IsValidSP(backSp, stackStart, stackEnd)) {
        if (callChain == NULL) {
            PRINTK("trace%u  ra:0x%x  sp:0x%x\n", count, (backRa << WINDOW_INCREMENT_SHIFT) >>
                    WINDOW_INCREMENT_SHIFT, backSp);
        } else {
            if (index++ < jumpCount) {
                continue;
            }
            backRa &= ~VIR_TEXT_ADDR_MASK;
            backRa |= TEXT_ADDR_MASK;
            callChain[count++] = backRa;
        }

        tmpRa = backRa;
        tmpSp = backSp;
        backRa = *((UINT32 *)(UINTPTR)(tmpSp - RA_OFFSET));
        backSp = *((UINT32 *)(UINTPTR)(tmpSp - SP_OFFSET));

        if ((tmpRa == backRa) || (backSp == tmpSp) || (count == maxDepth) || !IsValidRa(backRa)) {
            break;
        }
    }

    return count;
}

VOID LOS_RecordLR(UINTPTR *LR, UINT32 LRSize, UINT32 jumpCount, UINTPTR SP)
{
    UINTPTR reglr;
    if (LR == NULL) {
        return;
    }

    if (SP == 0) {
        __asm__ __volatile__("mov %0, sp" : "=a"(SP) : :);
        __asm__ __volatile__("mov %0, a0" : "=a"(reglr) : :);
    } else {
        reglr = *(UINT32 *)(SP - OS_REG_LR_OFFSET);
    }
    HakSpillWindow();
    HalBackTraceGet(SP, reglr, LR, LRSize, jumpCount);
}
#elif (LOSCFG_BACKTRACE_TYPE == 5)
#define OS_BACKTRACE_START     0

UINT32 IsAligned(UINT32 val, UINT32 align)
{
    return ((val & (align - 1)) == 0);
}

STATIC INLINE UINTPTR OsSpGet(VOID)
{
    UINTPTR regSp;

    __asm__ __volatile__("mov %0, sp" : "=r"(regSp));

    return regSp;
}

/* This function is used to check sp. */
BOOL IsValidSP(UINTPTR regSP, UINTPTR start, UINTPTR end)
{
    return (regSP >= start) && (regSP <= end);
}

BOOL FindSuitableStack(UINTPTR regSP, UINTPTR *start, UINTPTR *end)
{
    UINT32 stackStart;
    UINT32 stackEnd;
    BOOL found = FALSE;

    if (LOS_TaskIsRunning()) {
        UINT32 taskID = LOS_CurTaskIDGet();
        LosTaskCB *taskCB = OS_TCB_FROM_TID(taskID);
        stackStart = taskCB->topOfStack;
        stackEnd = taskCB->topOfStack + taskCB->stackSize;
        if (IsValidSP(regSP, stackStart, stackEnd)) {
            found = TRUE;
            goto FOUND;
        }
    }

    if (IsValidSP(regSP, CSTACK_START_ADDR, CSTACK_END_ADDR)) {
        stackStart = CSTACK_START_ADDR;
        stackEnd = CSTACK_END_ADDR;
        found = TRUE;
        goto FOUND;
    }

FOUND:
    if (found == TRUE) {
        *start = stackStart;
        *end = stackEnd;
    }

    return found;
}

VOID LOS_RecordLR(UINTPTR *LR, UINT32 LRSize, UINT32 jumpCount, UINTPTR SP)
{
    UINTPTR stackPointer;
    UINTPTR topOfStack;
    UINTPTR tmpStack = 0;
    UINTPTR stackBottom;
    UINTPTR checkBL;
    UINT32 count = 0;
    UINT32 index = 0;

    if (LR == NULL) {
        return;
    }

    if (SP == 0) {
        SP = OsSpGet();
    }

    stackPointer = SP;

    if (FindSuitableStack(stackPointer, &topOfStack, &stackBottom) == FALSE) {
        return;
    }

    while ((stackPointer < stackBottom) && (count < LRSize)) {
        if (IsValidSP(*(UINT32 *)stackPointer, topOfStack, stackBottom)
            && OsStackDataIsCodeAddr(*(UINT32 *)(stackPointer + STACK_OFFSET))
            && IsAligned(*(UINT32 *)stackPointer, ALGIN_CODE)) {
            if (tmpStack == *(UINT32 *)stackPointer) {
                break;
            }
            tmpStack = *(UINT32 *)stackPointer;
            checkBL = *(UINT32 *)(stackPointer + STACK_OFFSET);
            if (count++ < jumpCount) {
                continue;
            }
            stackPointer = tmpStack;
            LR[index++] =  checkBL;
            continue;
        }
        stackPointer += STACK_OFFSET;
    }

    if (index < LRSize) {
        LR[index] = 0;
    }
}
#elif (LOSCFG_BACKTRACE_TYPE == 6)
#define OS_BACKTRACE_START               1
#define STACK_OFFSET                     4
#define THUMB_OFFSET                     2
#define THUMB_BIT                        16
#define ARM_ALIGN_CODE                   4
#define THUMB_ALIGN_CODE                 2
#define BL_CMD_OFFSET                    4
#define ARM_BL_MASK                      0xEB000000
#define THUMB_BL_MASK                    0xF000F000
#define CLEAR_LOW_BIT_MASK               0xFFFFFFFE

STATIC INLINE BOOL IsAligned(UINT32 val, UINT32 align)
{
    return ((val & (align - 1)) == 0);
}

STATIC INLINE UINTPTR OsSpGet(VOID)
{
    UINTPTR SP;
    __asm volatile("mov %0, sp" : "=r"(SP));
    return SP;
}

STATIC INLINE BOOL IsArmValidLr(UINTPTR lr)
{
    return ((*(UINT32 *)(lr - BL_CMD_OFFSET) & ARM_BL_MASK) == ARM_BL_MASK);
}

STATIC INLINE BOOL IsThumbValidLr(UINTPTR lr)
{
    lr = (*(UINT16 *)(lr - BL_CMD_OFFSET) << THUMB_BIT) + *(UINT16 *)(lr - THUMB_OFFSET);
    return ((lr & THUMB_BL_MASK) == THUMB_BL_MASK);
}

VOID LOS_RecordLR(UINTPTR *LR, UINT32 LRSize, UINT32 jumpCount, UINTPTR SP)
{
    UINT32 count = 0;
    UINT32 index = 0;
    LosTaskCB *taskCB = NULL;
    UINT32 taskID;
    UINT32 stackStart, stackEnd;
    UINTPTR framePtr, tmpFramePtr, linkReg;

    if (LR == NULL) {
        return;
    }

    if (SP == 0) {
        SP = OsSpGet();
    }

    if (LOS_TaskIsRunning()) {
        taskID = LOS_CurTaskIDGet();
        taskCB = OS_TCB_FROM_TID(taskID);
        stackStart = taskCB->topOfStack;
        stackEnd = stackStart + taskCB->stackSize;
    } else {
        stackStart = CSTACK_START_ADDR;
        stackEnd = CSTACK_END_ADDR;
    }

    while ((SP > stackStart) && (SP < stackEnd)) {
        linkReg = *(UINTPTR *)SP;
        if (!OsStackDataIsCodeAddr(linkReg)) {
            SP += STACK_OFFSET;
            continue;
        }
        if (((!IsAligned(linkReg, ARM_ALIGN_CODE)) || !IsArmValidLr(linkReg)) &&
            ((!IsAligned(linkReg - 1, THUMB_ALIGN_CODE)) || !IsThumbValidLr(linkReg - 1))) {
            SP += STACK_OFFSET;
            continue;
        }
        if (index >= jumpCount) {
            LR[count++] = linkReg & CLEAR_LOW_BIT_MASK;
            if (count == LRSize) {
                break;
            }
        }
        ++index;
        SP += STACK_OFFSET;
    }

    /* if linkReg is not enough,clean up the last of the effective LR as the end. */
    if (count < LRSize) {
        LR[count] = 0;
    }
}
#else
#error Unknown backtrace type.
#endif

#if (LOSCFG_BACKTRACE_TYPE == 1) || (LOSCFG_BACKTRACE_TYPE == 3)
VOID LOS_RecordLR(UINTPTR *LR, UINT32 LRSize, UINT32 jumpCount, UINTPTR SP)
{
    if (LR == NULL) {
        return;
    }

    UINTPTR stackStart;
    UINTPTR stackEnd;
    UINT32 count = 0;
    UINT32 index = 0;
    UINTPTR sp;
    UINTPTR pc;
    UINT32 ret;

    ret = OsStackAddrGet(&stackStart, &stackEnd, SP);
    if (ret != LOS_OK) {
        return;
    }

    /* Traverse the stack space and find the LR address. */
    for (sp = stackStart; sp < stackEnd; sp += sizeof(UINTPTR)) {
        pc = OsAddrIsValid(sp);
        if ((pc != 0) && (count < LRSize)) {
            if (index++ < jumpCount) {
                continue;
            }
            LR[count] = pc;
            count++;
            if (count == LRSize) {
                break;
            }
        }
    }

    if (count < LRSize) {
        LR[count] = 0;
    }
}
#endif

VOID LOS_BackTrace(VOID)
{
    UINTPTR LR[BACKTRACE_MAX_DEPTH] = {0};
    UINT32 index;

    LOS_RecordLR(LR, BACKTRACE_MAX_DEPTH, OS_BACKTRACE_START, 0);

    if (LOS_TaskIsRunning()) {
        PRINTK("taskName = %s\n", g_losTask.runTask->taskName);
        PRINTK("taskID   = %u\n", g_losTask.runTask->taskID);
    }

    PRINTK("----- traceback start -----\r\n");
    for (index = 0; index < BACKTRACE_MAX_DEPTH; index++) {
        if (LR[index] == 0) {
            break;
        }
        PRINTK("traceback %d -- lr = 0x%x\r\n", index, LR[index]);
    }
    PRINTK("----- traceback end -----\r\n");
}

VOID OsBackTraceInit(VOID)
{
    OsBackTraceHookSet(LOS_RecordLR);
}
#endif


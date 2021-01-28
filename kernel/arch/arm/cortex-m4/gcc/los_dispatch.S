.syntax unified
.arch armv7e-m
.thumb

.equ    OS_NVIC_INT_CTRL,            0xE000ED04
.equ    OS_NVIC_SYSPRI2,             0xE000ED20
.equ    OS_NVIC_PENDSV_PRI,          0xF0F00000
.equ    OS_NVIC_PENDSVSET,           0x10000000
.equ    OS_TASK_STATUS_RUNNING,      0x0010

    .section .text
    .thumb

    .type HalStartToRun, %function
    .global HalStartToRun
HalStartToRun:
    .fnstart
    .cantunwind

    ldr     r4, =OS_NVIC_SYSPRI2
    ldr     r5, =OS_NVIC_PENDSV_PRI
    str     r5, [r4]

    ldr     r0, =g_taskScheduled
    mov     r1, #1
    str     r1, [r0]

    mov     r0, #2
    msr     CONTROL, r0


    ldr     r0, =g_losTask
    ldr     r2, [r0, #4]
    ldr     r0, =g_losTask
    str     r2, [r0]

    ldr     r3, =g_losTask
    ldr     r0, [r3]
    ldrh    r7, [r0 , #4]
    mov     r8,  #OS_TASK_STATUS_RUNNING
    orr     r7,  r7,  r8
    strh    r7,  [r0 , #4]

    ldr     r12, [r0]
    add     r12, r12, #36

    ldmfd   r12!, {r0-r7}
    msr     psp, r12
    mov     lr, r5

    cpsie   I
    bx      r6

    .fnend


    .type HalIntLock, %function
    .global HalIntLock
HalIntLock:
    .fnstart
    .cantunwind

    MRS R0, PRIMASK
    CPSID I
    BX LR
    .fnend

    .type HalIntUnLock, %function
    .global HalIntUnLock
HalIntUnLock:
    .fnstart
    .cantunwind

    MRS R0, PRIMASK
    CPSIE I
    BX LR
    .fnend

    .type HalIntRestore, %function
    .global HalIntRestore
HalIntRestore:
    .fnstart
    .cantunwind

    MSR PRIMASK, R0
    BX LR
    .fnend

    .type HalTaskSchedule, %function
    .global HalTaskSchedule
HalTaskSchedule:
    .fnstart
    .cantunwind

    ldr     r0, =OS_NVIC_INT_CTRL
    ldr     r1, =OS_NVIC_PENDSVSET
    str     r1, [r0]
    bx      lr
   .fnend




    .type HalPendSV, %function
    .global HalPendSV
HalPendSV:
    .fnstart
    .cantunwind

    mrs     r12, PRIMASK
    cpsid   I

HalTaskSwitch:

    mrs     r0, psp

    stmfd   r0!, {r4-r12}

    ldr     r5, =g_losTask
    ldr     r6, [r5]
    str     r0, [r6]


    ldrh    r7, [r6 , #4]
    mov     r8,#OS_TASK_STATUS_RUNNING
    bic     r7, r7, r8
    strh    r7, [r6 , #4]


    ldr     r0, =g_losTask
    ldr     r0, [r0, #4]
    str     r0, [r5]


    ldrh    r7, [r0 , #4]
    mov     r8,  #OS_TASK_STATUS_RUNNING
    orr     r7, r7, r8
    strh    r7,  [r0 , #4]

    ldr     r1,   [r0]

    ldmfd   r1!, {r4-r12}
    msr     psp,  r1

    msr     PRIMASK, r12


    bx      lr
    .fnend

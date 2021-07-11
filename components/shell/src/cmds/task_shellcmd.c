#include "stdlib.h"
#include "los_config.h"
#include "los_exc.h"
#include "los_task.h"
#include "los_sem.h"
#include "shcmd.h"
#include "shell.h"

#define OS_INVALID_SEM_ID  0xFFFFFFFF

LITE_OS_SEC_TEXT_MINOR UINT8 *OsShellCmdConvertTskStatus(UINT16 taskStatus)
{
    if (taskStatus & OS_TASK_STATUS_RUNNING) {
        return (UINT8 *)"Running";
    } else if (taskStatus & OS_TASK_STATUS_READY) {
        return (UINT8 *)"Ready";
    } else {
        if (taskStatus & OS_TASK_STATUS_DELAY) {
            return (UINT8 *)"Delay";
        } else if (taskStatus & OS_TASK_STATUS_PEND_QUEUE) {
            return (UINT8 *)"Pend";
        } else if (taskStatus & OS_TASK_STATUS_PEND) {
            return (UINT8 *)"Pend";
        } else if (taskStatus & OS_TASK_STATUS_SUSPEND) {
            return (UINT8 *)"Suspend";
        }
    }

    return (UINT8 *)"Invalid";
}

LITE_OS_SEC_TEXT_MINOR STATIC VOID OsShellCmdTskInfoTitle(VOID)
{
    PRINTK("\r\nName                   TaskEntryAddr       TID    ");

    PRINTK("Priority   Status       "
           "StackSize      StackPoint   TopOfStack");

    PRINTK("\n");
    PRINTK("----                   -------------       ---    ");
    PRINTK("--------   --------     "
           "---------    ----------     ----------");
    PRINTK("\n");
}

LITE_OS_SEC_TEXT_MINOR STATIC INLINE UINT32 OsGetSemID(const LosTaskCB *taskCB)
{
    UINT32 semId = OS_INVALID_SEM_ID;

    if (taskCB->taskSem != NULL) {
        semId = ((LosSemCB *)taskCB->taskSem)->semID;
    }

    return semId;
}

LITE_OS_SEC_TEXT_MINOR STATIC VOID OsShellCmdTskInfoData(const LosTaskCB *allTaskArray)
{
    const LosTaskCB *taskCB = NULL;
    UINT32 loop;
    UINT32 semId;

    for (loop = 0; loop < g_taskMaxNum; ++loop) {
        taskCB = allTaskArray + loop;
        if (taskCB->taskStatus & OS_TASK_STATUS_UNUSED) {
            continue;
        }

        semId = OsGetSemID(taskCB);

        PRINTK("%-23s%-20p0x%-5x", taskCB->taskName, taskCB->taskEntry, taskCB->taskID);
        PRINTK("%-11u%-13s0x%-11x   0x%-8x   0x%-10x   ", taskCB->priority,
               OsShellCmdConvertTskStatus(taskCB->taskStatus), taskCB->stackSize,
               taskCB->stackPointer, taskCB->topOfStack, semId);
        PRINTK("\n");
    }
}

LITE_OS_SEC_TEXT_MINOR UINT32 OsShellCmdTskInfoGet(UINT32 taskId)
{
    BOOL backupFlag = TRUE;
    UINT32 size;
    LosTaskCB *tcbArray = NULL;
    INT32 ret;

    if (taskId == OS_ALL_TASK_MASK) {
        size = g_taskMaxNum * sizeof(LosTaskCB);
        tcbArray = (LosTaskCB *)LOS_MemAlloc(m_aucSysMem0, size);
        if (tcbArray == NULL) {
            PRINTK("Memory is not enough to save task info!\n");
            tcbArray = g_taskCBArray;
            backupFlag = FALSE;
        }
        if (backupFlag == TRUE) {
            ret = memcpy_s(tcbArray, size, g_taskCBArray, size);
            if (ret != 0) {
                return LOS_NOK;
            }
        }

        OsShellCmdTskInfoTitle();
        OsShellCmdTskInfoData(tcbArray);

        if (backupFlag == TRUE) {
            (VOID)LOS_MemFree(m_aucSysMem0, tcbArray);
        }
    }

    return LOS_OK;
}

INT32 OsShellCmdDumpTask(INT32 argc, const CHAR **argv)
{
    size_t taskId = OS_ALL_TASK_MASK;

    if (argc < 1) {
        return OsShellCmdTskInfoGet((UINT32)taskId);
    } else {
        PRINTK("\nUsage: task\n");
        return OS_ERROR;
    }
}


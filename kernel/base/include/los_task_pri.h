#ifndef _LOS_TASK_PRI_H
#define _LOS_TASK_PRI_H

#include "los_task_base.h"
#include "los_task.h"
#include "los_arch_context.h"

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#define OS_TCB_FROM_PENDLIST(ptr)                       LOS_DL_LIST_ENTRY(ptr, LosTaskCB, pendList)
#define OS_TCB_FROM_TID(taskId)                         (((LosTaskCB *)g_taskCBArray) + (taskId))

STATIC INLINE BOOL OsTaskIsExit(const LosTaskCB *taskCB)
{
    return ((taskCB->taskStatus & OS_TASK_STATUS_EXIT) != 0);
}

/**
 * @ingroup los_task
 * Time slice structure.
 */
typedef struct TaskTimeSlice {
    LosTaskCB               *task;                        /**< Current running task */
    UINT16                  time;                         /**< Expiration time point */
    UINT16                  tout;                         /**< Expiration duration */
} OsTaskRobin;
extern LosTaskCB            *g_taskCBArray;
extern UINT32               g_taskMaxNum;
extern VOID                 OsIdleHandler(VOID);

STATIC INLINE LosTaskCB *OsCurrTaskGet(VOID)
{
    return (LosTaskCB *)ArchCurrTaskGet();
}

STATIC INLINE VOID OsCurrTaskSet(LosTaskCB *task)
{
    ArchCurrTaskSet((VOID *)task);
}

#ifdef __cplusplus
}
#endif /* __cplusplus */

#ifdef LOSCFG_KERNEL_SMP
STATIC INLINE UINT32 OsSpinLockTaskRq(LosTaskCB *taskCB)
{
    (VOID)taskCB;
    UINT32 intSave;
    LOS_SpinLockSave(&g_taskSpin, &intSave);
    return intSave;
}

STATIC INLINE VOID OsSpinUnlockTaskRq(LosTaskCB *taskCB, UINT32 intSave)
{
    (VOID)taskCB;
    LOS_SpinUnlockRestore(&g_taskSpin, intSave);
}
#else
STATIC INLINE UINT32 OsSpinLockTaskRq(LosTaskCB *taskCB)
{
    (VOID)taskCB;
    return 0;
}

STATIC INLINE VOID OsSpinUnlockTaskRq(LosTaskCB *taskCB, UINT32 intSave)
{
    (VOID)taskCB;
    (VOID)intSave;
}
#endif

#endif /* _LOS_TASK_PRI_H */

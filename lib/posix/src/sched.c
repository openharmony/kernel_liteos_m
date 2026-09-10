#define _GNU_SOURCE
#include <sched.h>
#include <errno.h>
#include <unistd.h>
#include "los_task_pri.h"

#ifdef LOSCFG_KERNEL_SMP
STATIC INT32 MapErrno(UINT32 err)
{
    INT32 posixRet;

    if (err == LOS_OK) {
        return ENOERR;
    }
    switch (err) {
        case LOS_ERRNO_TSK_ID_INVALID:
        case LOS_ERRNO_TSK_PTR_NULL:
        case LOS_ERRNO_TSK_NAME_EMPTY:
        case LOS_ERRNO_TSK_ENTRY_NULL:
        case LOS_ERRNO_TSK_PRIOR_ERROR:
        case LOS_ERRNO_TSK_STKSZ_TOO_LARGE:
        case LOS_ERRNO_TSK_STKSZ_TOO_SMALL:
        case LOS_ERRNO_TSK_NOT_CREATED:
        case LOS_ERRNO_TSK_CPU_AFFINITY_MASK_ERR:
            posixRet = EINVAL;
            break;
        case LOS_ERRNO_TSK_TCB_UNAVAILABLE:
        case LOS_ERRNO_TSK_MP_SYNC_RESOURCE:
            posixRet = ENOSPC;
            break;
        case LOS_ERRNO_TSK_NO_MEMORY:
            posixRet = ENOMEM;
            break;
        default:
            posixRet = EINVAL;
            break;
    }
    errno = posixRet;

    return posixRet;
}
#endif

int sched_yield(void)
{
    if (LOS_TaskYield() != LOS_OK) {
        return -1;
    }

    return 0;
}

int sched_get_priority_min(int policy)
{
    if (policy != SCHED_RR) {
        errno = EINVAL;
        return -1;
    }

    return LOS_TASK_PRIORITY_LOWEST;
}

int sched_get_priority_max(int policy)
{
    if (policy != SCHED_RR) {
        errno = EINVAL;
        return -1;
    }

    return LOS_TASK_PRIORITY_HIGHEST;
}

int sched_setaffinity(pid_t pid, size_t set_size, const cpu_set_t *set)
{
#ifdef LOSCFG_KERNEL_SMP
    UINT32 taskId = (UINT32)pid;
    UINT32 ret;

    if ((set == NULL) || (set_size != sizeof(cpu_set_t)) || (set->__bits[0] > LOSCFG_KERNEL_CPU_MASK)) {
        errno = EINVAL;
        return -1;
    }

    if (taskId == 0) {
        taskId = LOS_CurTaskIDGet();
        if (taskId == LOS_ERRNO_TSK_ID_INVALID) {
            errno = EINVAL;
            return -1;
        }
    }

    ret = LOS_TaskCpuAffiSet(taskId, (UINT16)set->__bits[0]);
    if (ret != LOS_OK) {
        errno = MapErrno(ret);
        return -1;
    }
#else
    (void)pid;
    (void)set_size;
    (void)set;
#endif

    return 0;
}

int sched_getaffinity(pid_t pid, size_t set_size, cpu_set_t *set)
{
#ifdef LOSCFG_KERNEL_SMP
    UINT32 taskId = (UINT32)pid;
    UINT16 cpuAffiMask;

    if ((set == NULL) || (set_size != sizeof(cpu_set_t))) {
        errno = EINVAL;
        return -1;
    }

    if (taskId == 0) {
        taskId = LOS_CurTaskIDGet();
        if (taskId == LOS_ERRNO_TSK_ID_INVALID) {
            errno = EINVAL;
            return -1;
        }
    }

    cpuAffiMask = LOS_TaskCpuAffiGet(taskId);
    if (cpuAffiMask == 0) {
        errno = EINVAL;
        return -1;
    }

    set->__bits[0] = cpuAffiMask;
#else
    (void)pid;
    (void)set_size;
    (void)set;
#endif

    return 0;
}

int __sched_cpucount(size_t set_size, const cpu_set_t *set)
{
    INT32 count = 0;
    UINT32 i;

    if ((set_size != sizeof(cpu_set_t)) || (set == NULL)) {
        return 0;
    }

    for (i = 0; i < (set_size / sizeof(unsigned long)); i++) {
        count += __builtin_popcountl(set->__bits[i]);
    }

    return count;
}

/*
 * Copyright (c) 2013-2019, Huawei Technologies Co., Ltd. All rights reserved.
 * Copyright (c) 2020, Huawei Device Co., Ltd. All rights reserved.
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

#include <errno.h>
#include <pthread.h>
#include <unistd.h>
#include <securec.h>
#include "los_task.h"
#include "los_task_pri.h"

#define PTHREAD_NAMELEN 16

/* this is just an assertion: LOS_TASK_ARG_NUM >= 4 */
typedef char NULNAM[-!((LOS_TASK_ARG_NUM * 4) >= PTHREAD_NAMELEN)];

static void *_pthread_entry(UINT32 param1, UINT32 param2, UINT32 param3, UINT32 param4)
{
    void *(*startRoutine)(void *) = (void *)(UINTPTR)param1;
    void *param = (void *)(UINTPTR)param2;
    (void)param3;
    (void)param4;
    int ret;

    LosTaskCB *tcb = OS_TCB_FROM_TID(LOS_CurTaskIDGet());
    char *tmp = tcb->taskName;
    tcb->taskName = (char *)tcb->args; /* args are reused as task name */
    ret = strcpy_s(tcb->taskName, PTHREAD_NAMELEN, tmp);
    if (ret != 0) {
        free(tmp);
        return NULL;
    }
    free(tmp);

    return startRoutine(param);
}

int pthread_create(pthread_t *thread, const pthread_attr_t *attr,
                   void *(*startRoutine)(void *), void *arg)
{
    TSK_INIT_PARAM_S taskInitParam = {0};
    UINT32 taskID;

    if ((thread == NULL) || (startRoutine == NULL)) {
        return EINVAL;
    }

    taskInitParam.usTaskPrio = LOSCFG_BASE_CORE_TSK_DEFAULT_PRIO;
    taskInitParam.uwStackSize = LOSCFG_BASE_CORE_TSK_DEFAULT_STACK_SIZE;
    if (attr) {
        if (attr->detachstate == PTHREAD_CREATE_DETACHED) {
            return ENOTSUP;
        }
        if (attr->stackaddr_set) {
            return ENOTSUP;
        }
        if (attr->stacksize_set) {
            taskInitParam.uwStackSize = attr->stacksize;
        }
        taskInitParam.usTaskPrio = (UINT16)attr->schedparam.sched_priority;
    }

    taskInitParam.pcName       = malloc(PTHREAD_NAMELEN);
    if (taskInitParam.pcName == NULL) {
        return ENOMEM;
    }

    taskInitParam.pfnTaskEntry = _pthread_entry;
    taskInitParam.auwArgs[0]   = (UINT32)(UINTPTR)startRoutine;
    taskInitParam.auwArgs[1]   = (UINT32)(UINTPTR)arg;

    if (LOS_TaskCreate(&taskID, &taskInitParam) != LOS_OK) {
        free(taskInitParam.pcName);
        return EINVAL;
    }

    /* set pthread default name */
    (void)sprintf_s(taskInitParam.pcName, PTHREAD_NAMELEN, "pthread%u", taskID);

    *thread = (pthread_t)taskID;
    return ENOERR;
}

int pthread_setschedparam(pthread_t thread, int policy, const struct sched_param *param)
{
    if ((param == NULL) || (param->sched_priority > OS_TASK_PRIORITY_LOWEST)) {
        return EINVAL;
    }

    /* Only support SCHED_RR policy now */
    if (policy != SCHED_RR) {
        return ENOTSUP;
    }

    if (LOS_TaskPriSet((UINT32)thread, (UINT16)param->sched_priority) != LOS_OK) {
        return EINVAL;
    }

    return ENOERR;
}

int pthread_getschedparam(pthread_t thread, int *policy, struct sched_param *param)
{
    UINT32 prio;

    if ((policy == NULL) || (param == NULL)) {
        return EINVAL;
    }

    prio = LOS_TaskPriGet((UINT32)thread);
    if (prio == OS_INVALID) {
        return EINVAL;
    }

    *policy = SCHED_RR;
    param->sched_priority = prio;
    return ENOERR;
}

pthread_t pthread_self(void)
{
    return (pthread_t)LOS_CurTaskIDGet();
}

int pthread_cancel(pthread_t thread)
{
    (void)thread;
    return ENOSYS;
}

static void *void_task(UINT32 param1, UINT32 param2, UINT32 param3, UINT32 param4)
{
    (void)param1;
    (void)param2;
    (void)param3;
    (void)param4;
    return 0;
}

static void cleanup_task_resource(void)
{
    TSK_INIT_PARAM_S taskInitParam = {0};
    UINT32 taskID;

    taskInitParam.pcName       = "void";
    taskInitParam.pfnTaskEntry = void_task;
    taskInitParam.usTaskPrio = LOSCFG_BASE_CORE_TSK_DEFAULT_PRIO;
    taskInitParam.uwStackSize = LOSCFG_BASE_CORE_TSK_MIN_STACK_SIZE;

    (void)LOS_TaskCreate(&taskID, &taskInitParam);
}

int pthread_join(pthread_t thread, void **retval)
{
    UINT32 taskStatus;

    if (retval) {
        /* retrieve thread exit code is not supported currently */
        return ENOTSUP;
    }

    if (thread == pthread_self()) {
        return EDEADLK;
    }

    while (LOS_TaskStatusGet((UINT32)thread, &taskStatus) == LOS_OK) {
        usleep(10000);
    }

    cleanup_task_resource();
    return 0;
}

int pthread_detach(pthread_t thread)
{
    (void)thread;
    return ENOSYS;
}

void pthread_exit(void *retVal)
{
    (void)retVal;
    (void)LOS_TaskDelete(LOS_CurTaskIDGet());
}

int pthread_setname_np(pthread_t thread, const char *name)
{
    char *taskName = LOS_TaskNameGet((UINT32)thread);
    if (taskName == NULL) {
        return EINVAL;
    }
    if (strnlen(name, PTHREAD_NAMELEN) >= PTHREAD_NAMELEN) {
        return ERANGE;
    }
    (void)strcpy_s(taskName, PTHREAD_NAMELEN, name);
    return 0;
}

int pthread_getname_np(pthread_t thread, char *buf, size_t buflen)
{
    int ret;

    const char *name = LOS_TaskNameGet((UINT32)thread);
    if (name == NULL) {
        return EINVAL;
    }
    if (buflen > strlen(name)) {
        ret = strcpy_s(buf, buflen, name);
        if (ret == 0) {
            return 0;
        }
    }
    return ERANGE;
}

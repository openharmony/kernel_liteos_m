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

#include "pthread.h"
#include <errno.h>
#include <stdlib.h>
#include <unistd.h>
#include <securec.h>
#include <limits.h>
#include "los_config.h"
#include "los_task.h"
#include "los_debug.h"

#define PTHREAD_DEFAULT_NAME     "pthread"
#define PTHREAD_DEFAULT_NAME_LEN 8
#define PTHREAD_NAMELEN 16
#define PTHREAD_KEY_UNUSED 0
#define PTHREAD_KEY_USED   1
#define PTHREAD_TASK_INVAILD 0

typedef void (*PthreadKeyDtor)(void *);
typedef struct {
    int flag;
    PthreadKeyDtor destructor;
} PthreadKey;
static unsigned int g_pthreadkeyCount = 0;
static PthreadKey   g_pthreadKeyData[PTHREAD_KEYS_MAX];
static LOS_DL_LIST  g_pthreadListHead;

typedef struct {
    void *(*startRoutine)(void *);
    void *param;
    char name[PTHREAD_NAMELEN];
    uintptr_t *key;
    LOS_DL_LIST threadList;
    unsigned char cancelstate;
    unsigned char canceltype;
    unsigned char canceled;
} PthreadData;

static void PthreadExitKeyDtor(PthreadData *pthreadData);

static void *PthreadEntry(UINT32 param)
{
    PthreadData *pthreadData = (PthreadData *)(UINTPTR)param;
    void *(*startRoutine)(void *) = pthreadData->startRoutine;
    void *ret = startRoutine(pthreadData->param);
    pthread_exit(ret);

    return ret;
}

static inline int IsPthread(pthread_t thread)
{
    return ((UINT32)thread <= LOSCFG_BASE_CORE_TSK_LIMIT);
}

static int PthreadCreateAttrInit(const pthread_attr_t *attr, void *(*startRoutine)(void *), void *arg,
    TSK_INIT_PARAM_S *taskInitParam)
{
    const pthread_attr_t *threadAttr = attr;
    struct sched_param schedParam = { 0 };
    INT32 policy = 0;
    pthread_attr_t attrTmp;
    INT32 ret;

    if (!attr) {
        (VOID)pthread_attr_init(&attrTmp);
        threadAttr = &attrTmp;
    }

    if (threadAttr->stackaddr_set != 0) {
        return ENOTSUP;
    }
    if (threadAttr->stacksize < PTHREAD_STACK_MIN) {
        return EINVAL;
    }
    taskInitParam->uwStackSize = threadAttr->stacksize;
    if (threadAttr->inheritsched == PTHREAD_EXPLICIT_SCHED) {
        taskInitParam->usTaskPrio = (UINT16)threadAttr->schedparam.sched_priority;
    } else if (IsPthread(pthread_self())) {
        ret = pthread_getschedparam(pthread_self(), &policy, &schedParam);
        if (ret != 0) {
            return ret;
        }
        taskInitParam->usTaskPrio = (UINT16)schedParam.sched_priority;
    } else {
        taskInitParam->usTaskPrio = (UINT16)threadAttr->schedparam.sched_priority;
    }

    PthreadData *pthreadData = (PthreadData *)malloc(sizeof(PthreadData));
    if (pthreadData == NULL) {
        return ENOMEM;
    }

    errno_t error = memcpy_s(pthreadData->name, PTHREAD_NAMELEN, PTHREAD_DEFAULT_NAME, PTHREAD_DEFAULT_NAME_LEN);
    if (error != EOK) {
        free(pthreadData);
        return error;
    }

    pthreadData->cancelstate    = PTHREAD_CANCEL_ENABLE;
    pthreadData->canceltype     = PTHREAD_CANCEL_DEFERRED;
    pthreadData->canceled       = 0;
    pthreadData->startRoutine   = startRoutine;
    pthreadData->param          = arg;
    pthreadData->key            = NULL;
    taskInitParam->pcName       = pthreadData->name;
    taskInitParam->pfnTaskEntry = PthreadEntry;
    taskInitParam->uwArg        = (UINT32)(UINTPTR)pthreadData;
    if (threadAttr->detachstate != PTHREAD_CREATE_DETACHED) {
        taskInitParam->uwResved = LOS_TASK_ATTR_JOINABLE;
    }

    return 0;
}

static int CheckForCancel(void)
{
    UINT32 intSave;
    LosTaskCB *tcb = NULL;

    intSave = LOS_IntLock();
    tcb = OS_TCB_FROM_TID(LOS_CurTaskIDGet());
    PthreadData *pthreadData = (PthreadData *)(UINTPTR)tcb->arg;
    if ((pthreadData->canceled) && (pthreadData->cancelstate == PTHREAD_CANCEL_ENABLE)) {
        LOS_IntRestore(intSave);
        return 1;
    }
    LOS_IntRestore(intSave);
    return 0;
}

int pthread_create(pthread_t *thread, const pthread_attr_t *attr,
    void *(*startRoutine)(void *), void *arg)
{
    TSK_INIT_PARAM_S taskInitParam = { 0 };
    UINT32 taskID;
    UINT32 ret;
    UINT32 intSave;

    if ((thread == NULL) || (startRoutine == NULL)) {
        return EINVAL;
    }

    ret = PthreadCreateAttrInit(attr, startRoutine, arg, &taskInitParam);
    if (ret != 0) {
        return ret;
    }

    if (LOS_TaskCreateOnly(&taskID, &taskInitParam) != LOS_OK) {
        free((VOID *)(UINTPTR)taskInitParam.uwArg);
        return EINVAL;
    }

    PthreadData *pthreadData = (PthreadData *)taskInitParam.uwArg;
    intSave = LOS_IntLock();
    if (g_pthreadListHead.pstNext == NULL) {
        LOS_ListInit(&g_pthreadListHead);
    }

    LOS_ListAdd(&g_pthreadListHead, &pthreadData->threadList);
    LOS_IntRestore(intSave);

    (void)LOS_TaskResume(taskID);

    *thread = (pthread_t)taskID;

    return 0;
}

int pthread_setschedparam(pthread_t thread, int policy, const struct sched_param *param)
{
    if ((param == NULL) || (param->sched_priority < OS_TASK_PRIORITY_HIGHEST) ||
        (param->sched_priority >= OS_TASK_PRIORITY_LOWEST) || !IsPthread(thread)) {
        return EINVAL;
    }

    /* Only support SCHED_RR policy now */
    if (policy != SCHED_RR) {
        return ENOTSUP;
    }

    if (LOS_TaskPriSet((UINT32)thread, (UINT16)param->sched_priority) != LOS_OK) {
        return EINVAL;
    }

    return 0;
}

int pthread_setschedprio(pthread_t thread, int prio)
{
    if (LOS_TaskPriSet((UINT32)thread, (UINT16)prio) != LOS_OK) {
        return EINVAL;
    }

    return 0;
}

int pthread_once(pthread_once_t *onceControl, void (*initRoutine)(void))
{
    UINT32 intSave;
    pthread_once_t old;

    if ((onceControl == NULL) || (initRoutine == NULL)) {
        return EINVAL;
    }
    intSave = LOS_IntLock();
    old = *onceControl;
    *onceControl = 1;
    LOS_IntRestore(intSave);

    if (!old) {
        initRoutine();
    }

    return 0;
}

int pthread_equal(pthread_t thread1, pthread_t thread2)
{
    return (int)(thread1 == thread2);
}

int pthread_setcancelstate(int state, int *oldState)
{
    UINT32 intSave;
    LosTaskCB *tcb = NULL;
    PthreadData *pthreadData = NULL;
    if ((state != PTHREAD_CANCEL_ENABLE) && (state != PTHREAD_CANCEL_DISABLE)) {
        return EINVAL;
    }

    intSave = LOS_IntLock();
    tcb = OS_TCB_FROM_TID(LOS_CurTaskIDGet());
    pthreadData = (PthreadData *)(UINTPTR)tcb->arg;
    if (pthreadData == NULL) {
        LOS_IntRestore(intSave);
        return EINVAL;
    }

    if (oldState != NULL) {
        *oldState = pthreadData->cancelstate;
    }
    pthreadData->cancelstate = (UINT8)state;
    LOS_IntRestore(intSave);

    return 0;
}

int pthread_setcanceltype(int type, int *oldType)
{
    UINT32 intSave;
    LosTaskCB *tcb = NULL;
    PthreadData *pthreadData = NULL;

    if ((type != PTHREAD_CANCEL_ASYNCHRONOUS) && (type != PTHREAD_CANCEL_DEFERRED)) {
        return EINVAL;
    }

    intSave = LOS_IntLock();
    tcb = OS_TCB_FROM_TID(LOS_CurTaskIDGet());
    pthreadData = (PthreadData *)(UINTPTR)tcb->arg;
    if (pthreadData == NULL) {
        LOS_IntRestore(intSave);
        return EINVAL;
    }

    if (oldType != NULL) {
        *oldType = pthreadData->canceltype;
    }

    pthreadData->canceltype = (UINT8)type;
    LOS_IntRestore(intSave);

    return 0;
}

int pthread_getschedparam(pthread_t thread, int *policy, struct sched_param *param)
{
    UINT32 prio;

    if ((policy == NULL) || (param == NULL) || !IsPthread(thread)) {
        return EINVAL;
    }

    prio = LOS_TaskPriGet((UINT32)thread);
    if (prio == OS_INVALID) {
        return EINVAL;
    }

    *policy = SCHED_RR;
    param->sched_priority = prio;

    return 0;
}

pthread_t pthread_self(void)
{
    return (pthread_t)LOS_CurTaskIDGet();
}

STATIC UINT32 DoPthreadCancel(LosTaskCB *task)
{
    UINT32 ret = LOS_OK;
    PthreadData *pthreadData = NULL;

    LOS_TaskLock();
    pthreadData = (PthreadData *)(UINTPTR)task->arg;
    pthreadData->canceled = 0;
    if ((task->taskStatus == PTHREAD_TASK_INVAILD) || (LOS_TaskSuspend(task->taskID) != LOS_OK)) {
        ret = LOS_NOK;
        goto OUT;
    }
    free((VOID *)(UINTPTR)task->arg);
    task->arg = (UINT32)(UINTPTR)NULL;
    (void)LOS_TaskDelete(task->taskID);

OUT:
    LOS_TaskUnlock();
    return ret;
}

int pthread_cancel(pthread_t thread)
{
    UINT32 intSave;
    LosTaskCB *tcb = NULL;
    PthreadData *pthreadData = NULL;
    if (!IsPthread(thread)) {
        return EINVAL;
    }
    intSave = LOS_IntLock();
    tcb = OS_TCB_FROM_TID((UINT32)thread);
    pthreadData = (PthreadData *)(UINTPTR)tcb->arg;
    pthreadData->canceled = 1;
    if ((pthreadData->cancelstate == PTHREAD_CANCEL_ENABLE) &&
        (pthreadData->canceltype == PTHREAD_CANCEL_ASYNCHRONOUS)) {
        /*
         * If the thread has cancellation enabled, and it is in
         * asynchronous mode, suspend it and set corresponding thread's status.
         * We also release the thread out of any current wait to make it wake up.
         */
        if (DoPthreadCancel(tcb) == LOS_NOK) {
            LOS_IntRestore(intSave);
            return ESRCH;
        }
    }
    LOS_IntRestore(intSave);

    return 0;
}

void pthread_testcancel(void)
{
    if (CheckForCancel()) {
        /*
         * If we have cancellation enabled, and there is a cancellation
         * pending, then go ahead and do the deed.
         * Exit now with special retVal. pthread_exit() calls the
         * cancellation handlers implicitly.
         */
        pthread_exit((void *)PTHREAD_CANCELED);
    }
}

int pthread_join(pthread_t thread, void **retval)
{
    UINTPTR result;

    UINT32 ret = LOS_TaskJoin((UINT32)thread, &result);
    if (ret == LOS_ERRNO_TSK_NOT_JOIN_SELF) {
        return EDEADLK;
    } else if ((ret == LOS_ERRNO_TSK_NOT_CREATED) ||
               (ret == LOS_ERRNO_TSK_OPERATE_IDLE) ||
               (ret == LOS_ERRNO_TSK_ID_INVALID) ||
               (ret == LOS_ERRNO_TSK_SUSPEND_SWTMR_NOT_ALLOWED)) {
        return ESRCH;
    } else if (ret != LOS_OK) {
        return EINVAL;
    }

    if (retval != NULL) {
        *retval = (VOID *)result;
    }

    return 0;
}

int pthread_detach(pthread_t thread)
{
    UINT32 ret = LOS_TaskDetach((UINT32)thread);
    if (ret == LOS_ERRNO_TSK_NOT_JOIN) {
        return ESRCH;
    } else if (ret != LOS_OK) {
        return EINVAL;
    }

    return 0;
}

void pthread_exit(void *retVal)
{
    UINT32 intSave;

    LosTaskCB *tcb = OS_TCB_FROM_TID(LOS_CurTaskIDGet());
    tcb->joinRetval = (UINTPTR)retVal;
    PthreadData *pthreadData = (PthreadData *)(UINTPTR)tcb->arg;
    if (pthread_setcancelstate(PTHREAD_CANCEL_DISABLE, NULL) != 0) {
        PRINT_ERR("%s: %d failed\n", __FUNCTION__, __LINE__);
    }
	
    if (pthreadData->key != NULL) {
        PthreadExitKeyDtor(pthreadData);
    }

    intSave = LOS_IntLock();
    LOS_ListDelete(&pthreadData->threadList);
    tcb->taskName = PTHREAD_DEFAULT_NAME;
    LOS_IntRestore(intSave);
    free(pthreadData);
    (void)LOS_TaskDelete(tcb->taskID);
    while (1) {
    }
}

int pthread_setname_np(pthread_t thread, const char *name)
{
    UINT32 intSave;
    LosTaskCB *taskCB = NULL;
    char *taskName = LOS_TaskNameGet((UINT32)thread);
    if (taskName == NULL || !IsPthread(thread)) {
        return EINVAL;
    }

    if (strnlen(name, PTHREAD_NAMELEN) >= PTHREAD_NAMELEN) {
        return ERANGE;
    }

    taskCB = OS_TCB_FROM_TID((UINT32)thread);
    intSave = LOS_IntLock();
    if (taskCB->taskStatus & OS_TASK_STATUS_EXIT) {
        LOS_IntRestore(intSave);
        return EINVAL;
    }

    if (taskCB->taskEntry == PthreadEntry) {
        (void)strcpy_s(taskName, PTHREAD_NAMELEN, name);
    } else {
        LOS_IntRestore(intSave);
        return EINVAL;
    }
    LOS_IntRestore(intSave);

    return 0;
}

int pthread_getname_np(pthread_t thread, char *buf, size_t buflen)
{
    int ret;

    const char *name = LOS_TaskNameGet((UINT32)thread);
    if (name == NULL || !IsPthread(thread)) {
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

static void PthreadExitKeyDtor(PthreadData *pthreadData)
{
    PthreadKey *keys = NULL;
    unsigned int intSave;

    intSave = LOS_IntLock();
    for (unsigned int count = 0; count < PTHREAD_KEYS_MAX; count++) {
        keys = &g_pthreadKeyData[count];
        if (keys->flag == PTHREAD_KEY_UNUSED) {
            continue;
        }
        PthreadKeyDtor dtor = keys->destructor;
        LOS_IntRestore(intSave);

        if ((dtor != NULL) && (pthreadData->key[count] != 0)) {
            dtor((void *)pthreadData->key[count]);
        }

        intSave = LOS_IntLock();
    }
    LOS_IntRestore(intSave);

    free((void *)pthreadData->key);
}

int pthread_key_create(pthread_key_t *k, void (*dtor)(void *))
{
    unsigned int intSave;
    unsigned int count = 0;
    PthreadKey *keys = NULL;

    if (k == NULL) {
        return EINVAL;
    }

    intSave = LOS_IntLock();
    if (g_pthreadkeyCount >= PTHREAD_KEYS_MAX) {
        LOS_IntRestore(intSave);
        return EAGAIN;
    }

    do {
        keys = &g_pthreadKeyData[count];
        if (keys->flag == PTHREAD_KEY_UNUSED) {
            break;
        }
        count++;
    } while (count < PTHREAD_KEYS_MAX);

    keys->destructor = dtor;
    keys->flag = PTHREAD_KEY_USED;
    g_pthreadkeyCount++;
    LOS_IntRestore(intSave);

    *k = count;

    return 0;
}

int pthread_key_delete(pthread_key_t k)
{
    unsigned int intSave;

    if (k >= PTHREAD_KEYS_MAX) {
        return EINVAL;
    }

    intSave = LOS_IntLock();
    if ((g_pthreadkeyCount == 0) || (g_pthreadKeyData[k].flag == PTHREAD_KEY_UNUSED)) {
        LOS_IntRestore(intSave);
        return EAGAIN;
    }

    LOS_DL_LIST *list = g_pthreadListHead.pstNext;
    while (list != &g_pthreadListHead) {
        PthreadData *pthreadData = (PthreadData *)LOS_DL_LIST_ENTRY(list, PthreadData, threadList);
        if (pthreadData->key != NULL) {
            if ((g_pthreadKeyData[k].destructor != NULL) && (pthreadData->key[k] != 0)) {
                g_pthreadKeyData[k].destructor((void *)pthreadData->key[k]);
            }
            pthreadData->key[k] = 0;
        }
        list = list->pstNext;
    }

    g_pthreadKeyData[k].destructor = NULL;
    g_pthreadKeyData[k].flag = PTHREAD_KEY_UNUSED;
    g_pthreadkeyCount--;
    LOS_IntRestore(intSave);

    return 0;
}

int pthread_setspecific(pthread_key_t k, const void *x)
{
    pthread_t self = pthread_self();
    unsigned int intSave;
    uintptr_t *key = NULL;

    if (k >= PTHREAD_KEYS_MAX) {
        return EINVAL;
    }

    if (!IsPthread(self)) {
        return EINVAL;
    }

    LosTaskCB *taskCB = OS_TCB_FROM_TID((UINT32)self);
    PthreadData *pthreadData = (PthreadData *)taskCB->arg;
    if (pthreadData->key == NULL) {
        key = (uintptr_t *)malloc(sizeof(uintptr_t) * PTHREAD_KEYS_MAX);
        if (key == NULL) {
            return ENOMEM;
        }
        (void)memset_s(key, sizeof(uintptr_t) * PTHREAD_KEYS_MAX, 0, sizeof(uintptr_t) * PTHREAD_KEYS_MAX);
    }

    intSave = LOS_IntLock();
    if (g_pthreadKeyData[k].flag == PTHREAD_KEY_UNUSED) {
        LOS_IntRestore(intSave);
        free(key);
        return EAGAIN;
    }

    if (pthreadData->key == NULL) {
        pthreadData->key = key;
    }

    pthreadData->key[k] = (uintptr_t)x;
    LOS_IntRestore(intSave);

    return 0;
}

void *pthread_getspecific(pthread_key_t k)
{
    unsigned int intSave;
    void *key = NULL;
    pthread_t self = pthread_self();

    if (k >= PTHREAD_KEYS_MAX) {
        return NULL;
    }

    if (!IsPthread(self)) {
        return NULL;
    }

    LosTaskCB *taskCB = OS_TCB_FROM_TID((UINT32)self);
    PthreadData *pthreadData = (PthreadData *)taskCB->arg;
    intSave = LOS_IntLock();
    if ((g_pthreadKeyData[k].flag == PTHREAD_KEY_UNUSED) || (pthreadData->key == NULL)) {
        LOS_IntRestore(intSave);
        return NULL;
    }

    key = (void *)pthreadData->key[k];
    LOS_IntRestore(intSave);

    return key;
}


/*
 * Copyright (c) 2022-2022 Huawei Device Co., Ltd. All rights reserved.
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

#include "poll.h"
#include <sys/time.h>
#include <time.h>
#include "securec.h"
#include "poll_impl.h"
#include "los_interrupt.h"
#include "los_memory.h"

VOID PollWaitQueueInit(struct PollWaitQueue *waitQueue)
{
    if (waitQueue == NULL) {
        return;
    }
    LOS_ListInit(&waitQueue->queue);
}

STATIC INLINE VOID SetAddPollWaitFlag(struct PollTable *table, BOOL addQueueFlag)
{
    table->addQueueFlag = addQueueFlag;
}

STATIC VOID DestroyPollWait(struct PollTable *table)
{
    UINT32 intSave;
    struct PollWaitNode *waitNode = table->node;

    intSave = LOS_IntLock();
    LOS_ListDelete(&waitNode->node);
    LOS_IntRestore(intSave);

    (VOID)LOS_MemFree(OS_SYS_MEM_ADDR, waitNode);
    if (LOS_SemDelete(table->sem) != LOS_OK) {
        PRINT_ERR("destroy poll sem failed!\n");
    }
}

STATIC VOID AddPollWaitQueue(struct PollWaitQueue *waitQueue, struct PollTable *table)
{
    UINT32 intSave;
    struct PollWaitNode *waitNode = LOS_MemAlloc(OS_SYS_MEM_ADDR, sizeof(struct PollWaitNode));
    if (waitNode == NULL) {
        return;
    }

    intSave = LOS_IntLock();
    waitNode->table = table;
    LOS_ListAdd(&waitQueue->queue, &waitNode->node);
    table->node = waitNode;
    LOS_IntRestore(intSave);
}

STATIC INT32 WaitSemTime(struct PollTable *table, UINT32 timeout)
{
    if (timeout != 0) {
        return LOS_SemPend(table->sem, LOS_MS2Tick(timeout));
    } else {
        return LOS_SemPend(table->sem, LOS_WAIT_FOREVER);
    }
}

STATIC INT32 QueryFds(struct pollfd *fds, nfds_t nfds, struct PollTable *table)
{
    UINT32 i;
    INT32 ret;
    INT32 count = 0;

    if (((nfds != 0) && (fds == NULL)) || (table == NULL)) {
        errno = EINVAL;
        return -1;
    }

    for (i = 0; i < nfds; i++) {
        struct pollfd *tmpFds = &fds[i];
        if (tmpFds->fd < 0) {
            errno = EBADF;
            return -1;
        }
        table->event = tmpFds->events | POLLERR | POLLHUP;

        ret = PollQueryFd(tmpFds->fd, table);
        if (ret < 0) {
            errno = -ret;
            return -1;
        }

        tmpFds->revents = (tmpFds->events | POLLERR | POLLHUP) & (PollEvent)ret;
        if (tmpFds->revents != 0) {
            count++;
            SetAddPollWaitFlag(table, FALSE);
        }
    }

    return count;
}

VOID PollNotify(struct PollWaitQueue *waitQueue, PollEvent event)
{
    UINT32 intSave;
    struct PollWaitNode *waitNode = NULL;

    if (waitQueue == NULL) {
        return;
    }

    intSave = LOS_IntLock();
    LOS_DL_LIST_FOR_EACH_ENTRY(waitNode, &waitQueue->queue, struct PollWaitNode, node) {
        if (!event || (event & waitNode->table->event)) {
            if (LOS_SemPost(waitNode->table->sem) != LOS_OK) {
                PRINT_ERR("poll notify sem post failed!\n");
            }
        }
    }
    LOS_IntRestore(intSave);
}

VOID PollWait(struct PollWaitQueue *waitQueue, struct PollTable *table)
{
    if ((waitQueue == NULL) || (table == NULL)) {
        return;
    }

    if (table->addQueueFlag == TRUE) {
        AddPollWaitQueue(waitQueue, table);
    }
}

STATIC INLINE INT32 PollTimedWait(struct pollfd *fds, nfds_t nfds, struct PollTable *table, INT32 timeout)
{
    struct timespec startTime = {0};
    struct timespec curTime = {0};
    INT32 left, last;
    INT32 ret;
    INT32 count = 0;

    if (timeout > 0) {
        clock_gettime(CLOCK_REALTIME, &startTime);
    }

    left = timeout;
    while (count == 0) {
        if (timeout < 0) {
            ret = WaitSemTime(table, 0);
            if (ret != 0) {
                break;
            }
        } else if (left <= 0) {
            break;
        } else if (left > 0) {
            clock_gettime(CLOCK_REALTIME, &curTime);
            last = (INT32)((curTime.tv_sec - startTime.tv_sec) * OS_SYS_MS_PER_SECOND +
                   (curTime.tv_nsec - startTime.tv_nsec) / (OS_SYS_NS_PER_SECOND / OS_SYS_MS_PER_SECOND));
            if (last >= timeout) {
                break;
            } else {
                left = timeout - last;
            }

            ret = WaitSemTime(table, left);
            if (ret == LOS_ERRNO_SEM_TIMEOUT) {
                errno = ETIMEDOUT;
                break;
            }
        }
        count = QueryFds(fds, nfds, table);
    }

    return count;
}

int poll(struct pollfd *fds, nfds_t nfds, int timeout)
{
    struct PollTable table = {0};
    INT32 count;

    if (LOS_SemCreate(0, &table.sem) != LOS_OK) {
        errno = EINVAL;
        return -1;
    }

    SetAddPollWaitFlag(&table, ((timeout == 0) ? FALSE : TRUE));

    count = QueryFds(fds, nfds, &table);
    if (count != 0) {
        goto DONE;
    }

    if (timeout == 0) {
        goto DONE;
    }

    SetAddPollWaitFlag(&table, FALSE);

    count = PollTimedWait(fds, nfds, &table, timeout);

DONE:
    DestroyPollWait(&table);
    return count;
}

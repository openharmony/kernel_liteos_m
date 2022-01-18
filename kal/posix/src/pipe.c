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

#include "securec.h"
#include "los_fs.h"
#include "los_list.h"
#include "los_mux.h"
#include "los_sem.h"
#include "los_debug.h"
#include "los_memory.h"
#include "pipe_impl.h"

#if (LOSCFG_POSIX_PIPE_API == 1)

#define PIPE_DEV_NUM            32
#define PIPE_FD_NUM             (PIPE_DEV_NUM << 1)
#define PIPE_DEV_NAME_MAX       32
#define PIPE_DEV_FD_BITMAP_LEN  ((PIPE_FD_NUM >> 5) + 1)
#define PIPE_DEV_BUF_SIZE       1024
#define PIPE_READ               1
#define PIPE_WRITE              2

#define PIPE_DEV_LOCK(mutex)    do {                            \
    UINT32 __ret = LOS_MuxPend(mutex, LOS_WAIT_FOREVER);        \
    if (__ret != LOS_OK) {                                      \
        PRINT_ERR("pipe device lock error, ret = %x\n", __ret); \
    }                                                           \
} while (0)

#define PIPE_DEV_UNLOCK(mutex)    do {                            \
    UINT32 __ret = LOS_MuxPost(mutex);                            \
    if (__ret != LOS_OK) {                                        \
        PRINT_ERR("pipe device unlock error, ret = %x\n", __ret); \
    }                                                             \
} while (0)

#define PIPE_RW_WAIT(sem)    do {                                           \
    LosSemCB *__sem = GET_SEM(sem);                                         \
    while (__sem->semCount != 0) {                                          \
        UINT32 __ret = LOS_SemPend(__sem->semID, LOS_WAIT_FOREVER);         \
        if (__ret != LOS_OK) {                                              \
            PRINT_ERR("pipe device R/W sem wait error, ret = %x\n", __ret); \
        }                                                                   \
    }                                                                       \
} while (0)

#define PIPE_RW_POST(sem)    do {                                           \
    LosSemCB *__sem = GET_SEM(sem);                                         \
    while (__sem->semCount == 0) {                                          \
        UINT32 __ret = LOS_SemPost(__sem->semID);                           \
        if (__ret != LOS_OK) {                                              \
            PRINT_ERR("pipe device R/W sem post error, ret = %x\n", __ret); \
        }                                                                   \
    }                                                                       \
} while (0)

struct PipeDev {
    CHAR devName[PIPE_DEV_NAME_MAX];
    UINT32 num;
    UINT32 mutex;
    LOS_DL_LIST list;
    UINT32 readSem;
    UINT32 writeSem;
    UINT8 *ringBuffer;
    size_t bufferSize;
    size_t readIndex;
    size_t writeIndex;
    BOOL roll;
    UINT32 readerCnt;
    UINT32 writerCnt;
    UINT32 ref;
    struct PollWaitQueue wq;
};

struct PipeFdDev {
    struct PipeDev *dev;
    BOOL openFlag;
};

STATIC LOS_DL_LIST g_devList = {&g_devList, &g_devList};
STATIC UINT32 g_devListMutex = LOSCFG_BASE_IPC_MUX_LIMIT;

STATIC UINT32 g_devNumBitmap = 0;
STATIC UINT32 g_devFdBitmap[PIPE_DEV_FD_BITMAP_LEN] = {0};
STATIC struct PipeFdDev g_devFd[PIPE_FD_NUM] = {0};
STATIC UINT32 g_devFdMutex = LOSCFG_BASE_IPC_MUX_LIMIT;
STATIC INT32 g_devStartFd = 0;

STATIC INT32 PipeDevNumAlloc(VOID)
{
    UINT32 temp = g_devNumBitmap;
    INT32 devNum = 0;

    while (temp & 0x1) {
        devNum++;
        temp = temp >> 1;
    }

    if (devNum >= PIPE_DEV_NUM) {
        return -1;
    }
    g_devNumBitmap |= 1U << devNum;

    return devNum;
}

STATIC VOID PipeDevNumFree(INT32 devNum)
{
    if ((devNum < 0) || (devNum >= PIPE_DEV_NUM)) {
        return;
    }
    g_devNumBitmap &= ~(1U << devNum);
}

STATIC struct PipeDev *PipeDevFind(const CHAR *path)
{
    struct PipeDev *dev = NULL;

    (VOID)LOS_MuxPend(g_devListMutex, LOS_WAIT_FOREVER);
    if (!LOS_ListEmpty(&g_devList)) {
        LOS_DL_LIST_FOR_EACH_ENTRY(dev, &g_devList, struct PipeDev, list) {
            if (!strncmp(dev->devName, path, PIPE_DEV_NAME_MAX)) {
                (VOID)LOS_MuxPost(g_devListMutex);
                return dev;
            }
        }
    }
    (VOID)LOS_MuxPost(g_devListMutex);
    return NULL;
}

STATIC size_t PipeRingbufferRead(struct PipeDev *dev, VOID *buf, size_t len)
{
    size_t nbytes;

    if (dev->readIndex < dev->writeIndex) {
        nbytes = dev->writeIndex - dev->readIndex;
    } else if (dev->readIndex > dev->writeIndex) {
        nbytes = dev->bufferSize - dev->readIndex;
    } else {
        if (dev->roll == FALSE) {
            return 0;
        } else {
            nbytes = dev->bufferSize - dev->readIndex;
        }
    }
    nbytes = (nbytes > len) ? len : nbytes;
    (VOID)memcpy_s((char *)buf, len, dev->ringBuffer + dev->readIndex, nbytes);
    dev->readIndex += nbytes;
    if (dev->readIndex >= dev->bufferSize) {
        dev->readIndex = 0;
        dev->roll = FALSE;
    }

    return nbytes;
}

STATIC size_t PipeRingbufferWrite(struct PipeDev *dev, VOID *buf, size_t len)
{
    size_t nbytes;

    if (dev->readIndex < dev->writeIndex) {
        nbytes = dev->bufferSize - dev->writeIndex;
    } else if (dev->readIndex > dev->writeIndex) {
        nbytes = dev->readIndex - dev->writeIndex;
    } else {
        if (dev->roll == TRUE) {
            return 0;
        } else {
            nbytes = dev->bufferSize - dev->writeIndex;
        }
    }

    nbytes = (nbytes > len) ? len : nbytes;
    (VOID)memcpy_s(dev->ringBuffer + dev->writeIndex, dev->bufferSize
                   - dev->writeIndex, buf, nbytes);
    dev->writeIndex += nbytes;
    if (dev->writeIndex >= dev->bufferSize) {
        dev->roll = TRUE;
        dev->writeIndex = 0;
    }

    return nbytes;
}

STATIC INT32 PipeDevRegister(CHAR *devName, UINT32 len)
{
    INT32 ret;
    INT32 num = PipeDevNumAlloc();
    if (num < 0) {
        return -ENODEV;
    }

    struct PipeDev *devTemp = NULL;
    struct PipeDev *dev = LOS_MemAlloc(OS_SYS_MEM_ADDR, sizeof(struct PipeDev));
    if (dev == NULL) {
        ret = -ENOMEM;
        goto ERROR;
    }
    (VOID)memset_s(dev, sizeof(struct PipeDev), 0, sizeof(struct PipeDev));
    (VOID)snprintf_s(dev->devName, PIPE_DEV_NAME_MAX, PIPE_DEV_NAME_MAX - 1, "%s%d", PIPE_DEV_PATH, num);
    (VOID)memcpy_s(devName, len, dev->devName, strlen(dev->devName));

    devTemp = PipeDevFind(dev->devName);
    if (devTemp != NULL) {
        ret = -EEXIST;
        goto ERROR;
    }

    ret = LOS_MuxCreate(&dev->mutex);
    if (ret != LOS_OK) {
        ret = -ENOSPC;
        goto ERROR;
    }

    ret = LOS_SemCreate(0, &dev->readSem);
    if (ret != LOS_OK) {
        (VOID)LOS_MuxDelete(dev->mutex);
        ret = -ENOSPC;
        goto ERROR;
    }

    ret = LOS_SemCreate(0, &dev->writeSem);
    if (ret != LOS_OK) {
        (VOID)LOS_MuxDelete(dev->mutex);
        (VOID)LOS_SemDelete(dev->readSem);
        ret = -ENOSPC;
        goto ERROR;
    }

    dev->num = num;
    PollWaitQueueInit(&dev->wq);

    (VOID)LOS_MuxPend(g_devListMutex, LOS_WAIT_FOREVER);
    LOS_ListAdd(&g_devList, &dev->list);
    (VOID)LOS_MuxPost(g_devListMutex);

    return ENOERR;
ERROR:
    if (dev != NULL) {
        (VOID)LOS_MemFree(OS_SYS_MEM_ADDR, dev);
    }
    PipeDevNumFree(num);
    return ret;
}

STATIC INT32 PipeDevUnregister(struct PipeDev *dev)
{
    BOOL findFlag = FALSE;

    (VOID)LOS_MuxPend(g_devListMutex, LOS_WAIT_FOREVER);
    if (LOS_ListEmpty(&g_devList)) {
        (VOID)LOS_MuxPost(g_devListMutex);
        return -ENODEV;
    }

    struct PipeDev *tmpDev = NULL;
    LOS_DL_LIST_FOR_EACH_ENTRY(tmpDev, &g_devList, struct PipeDev, list) {
        if (tmpDev == dev) {
            LOS_ListDelete(&dev->list);
            findFlag = TRUE;
            break;
        }
    }
    (VOID)LOS_MuxPost(g_devListMutex);

    if (findFlag != TRUE) {
        return -ENODEV;
    }

    PipeDevNumFree(dev->num);
    (VOID)LOS_MuxDelete(dev->mutex);
    (VOID)LOS_SemDelete(dev->readSem);
    (VOID)LOS_SemDelete(dev->writeSem);
    (VOID)LOS_MemFree(OS_SYS_MEM_ADDR, dev->ringBuffer);
    dev->ringBuffer = NULL;
    (VOID)LOS_MemFree(OS_SYS_MEM_ADDR, dev);

    return ENOERR;
}

STATIC INT32 PipeDevFdAlloc(VOID)
{
    UINT32 i = 0;
    INT32 fd = 0;

    while (g_devFdBitmap[i] & (1U << fd)) {
        fd++;
        if (fd == 32) { /* 32: bit index max is 32. */
            i++;
            fd = 0;
        }

        if (i == PIPE_DEV_FD_BITMAP_LEN) {
            return -1;
        }
    }
    g_devFdBitmap[i] |= (1U << fd);
    return (fd + (i << 5)); /* 5: i is the multiple of 32. */
}

STATIC VOID PipeDevFdFree(INT32 fd)
{
    g_devFdBitmap[fd >> 5] &= ~(1U << (fd & 0x1F)); /* 5: fd is the multiple of 32. */
}

STATIC VOID PipePollNotify(struct PipeDev *dev, PollEvent event)
{
    struct PollWaitQueue *waitQueue = &dev->wq;

    if (event & POLLERR) {
        event &= ~(POLLIN | POLLOUT);
    }

    PollNotify(waitQueue, event);
}

INT32 PipeOpen(const CHAR *path, INT32 openFlag, INT32 minFd)
{
    struct PipeDev *dev = NULL;
    INT32 fd = -1;

    if (path == NULL) {
        errno = EINVAL;
        return -1;
    }

    if ((openFlag != O_RDONLY) && (openFlag != O_WRONLY)) {
        errno = EINVAL;
        return -1;
    }

    dev = PipeDevFind(path);
    if (dev == NULL) {
        errno = ENODEV;
        return -1;
    }

    fd = PipeDevFdAlloc();
    if (fd < 0) {
        errno = EBUSY;
        return -1;
    }

    (VOID)LOS_MuxPend(g_devFdMutex, LOS_WAIT_FOREVER);
    g_devFd[fd].dev = dev;
    g_devFd[fd].openFlag = openFlag;
    g_devStartFd = minFd;
    (VOID)LOS_MuxPost(g_devFdMutex);

    PIPE_DEV_LOCK(dev->mutex);
    if (openFlag == O_RDONLY) {
        dev->readerCnt++;
    } else if (openFlag == O_WRONLY) {
        dev->writerCnt++;
    }
    dev->ref++;

    if (dev->ringBuffer == NULL) {
        dev->ringBuffer = LOS_MemAlloc(OS_SYS_MEM_ADDR, PIPE_DEV_BUF_SIZE);
        if (dev->ringBuffer == NULL) {
            PIPE_DEV_UNLOCK(dev->mutex);
            errno = ENOMEM;
            goto ERROR;
        }
        dev->bufferSize = PIPE_DEV_BUF_SIZE;
    }
    PIPE_DEV_UNLOCK(dev->mutex);

    return (fd + minFd);
ERROR:
    if (dev->ringBuffer != NULL) {
        (VOID)LOS_MemFree(OS_SYS_MEM_ADDR, dev->ringBuffer);
        dev->ringBuffer = NULL;
    }

    PipeDevFdFree(fd);
    return -1;
}

STATIC INLINE struct PipeFdDev *PipeFdDevGet(INT32 fd)
{
    if (fd < g_devStartFd) {
        return NULL;
    }

    fd -= g_devStartFd;
    if (fd >= PIPE_FD_NUM) {
        return NULL;
    }
    return &g_devFd[fd];
}

STATIC struct PipeDev *PipeFd2Dev(INT32 fd)
{
    struct PipeFdDev *devFd = PipeFdDevGet(fd);

    return (devFd != NULL) ? devFd->dev : NULL;
}

INT32 PipeClose(INT32 fd)
{
    struct PipeDev *dev = NULL;
    UINT32 openFlag;

    (VOID)LOS_MuxPend(g_devFdMutex, LOS_WAIT_FOREVER);
    dev = PipeFd2Dev(fd);
    if (dev == NULL) {
        errno = ENODEV;
        goto ERROR;
    }
    fd -= g_devStartFd;
    openFlag = g_devFd[fd].openFlag;
    g_devFd[fd].dev = NULL;
    g_devFd[fd].openFlag = FALSE;
    PipeDevFdFree(fd);
    (VOID)LOS_MuxPost(g_devFdMutex);

    PIPE_DEV_LOCK(dev->mutex);
    if (openFlag == O_RDONLY) {
        dev->readerCnt--;
    }

    if (openFlag == O_WRONLY) {
        dev->writerCnt--;
    }

    if (dev->readerCnt == 0) {
        PIPE_RW_POST(dev->writeSem);
        PipePollNotify(dev, POLLOUT);
    }

    if (dev->writerCnt == 0) {
        PIPE_RW_POST(dev->readSem);
        PipePollNotify(dev, POLLIN);
    }

    if (--dev->ref == 0) {
        PIPE_DEV_UNLOCK(dev->mutex);
        (VOID)PipeDevUnregister(dev);
    } else {
        PIPE_DEV_UNLOCK(dev->mutex);
    }

    return ENOERR;
ERROR:
    (VOID)LOS_MuxPost(g_devFdMutex);
    return -1;
}

STATIC INLINE BOOL PipeWriterIsWaiting(UINT32 sem)
{
    LosSemCB *semCB = GET_SEM(sem);
    UINT32 num = 0;

    while (semCB->semCount == 0) {
        UINT32 ret = LOS_SemPost(semCB->semID);
        if (ret != LOS_OK) {
            PRINT_ERR("pipe device write sem post error, ret = %x\n", ret);
        }
        num++;
    }
    return (num <= 1) ? FALSE : TRUE;
}

STATIC struct PipeDev *PipeDevGet(INT32 fd)
{
    struct PipeDev *dev = NULL;

    (VOID)LOS_MuxPend(g_devFdMutex, LOS_WAIT_FOREVER);
    dev = PipeFd2Dev(fd);
    (VOID)LOS_MuxPost(g_devFdMutex);

    return dev;
}

INT32 PipeRead(INT32 fd, VOID *buf, size_t len)
{
    struct PipeDev *dev = NULL;
    INT32 ret;
    size_t nread = 0;
    size_t tmpLen;

    if ((buf == NULL) || (len == 0)) {
        errno = EINVAL;
        return -1;
    }

    dev = PipeDevGet(fd);
    if (dev == NULL) {
        errno = ENODEV;
        return -1;
    }

    PIPE_DEV_LOCK(dev->mutex);
    if ((dev->readIndex == dev->writeIndex) &&
        (dev->roll == FALSE)) {
        PIPE_DEV_UNLOCK(dev->mutex);

        ret = LOS_SemPend(dev->readSem, LOS_WAIT_FOREVER);
        if (ret != LOS_OK) {
            errno = EINVAL;
            goto ERROR;
        }
        PIPE_DEV_LOCK(dev->mutex);
    }

    while (nread < len) {
        tmpLen = PipeRingbufferRead(dev, (CHAR *)buf + nread, len - nread);
        if (tmpLen == 0) {
            PIPE_RW_WAIT(dev->readSem);
            /* No writer operates at present, which indicates that the write operation may have ended */
            if (!PipeWriterIsWaiting(dev->writeSem)) {
                PipePollNotify(dev, POLLOUT);
                PIPE_DEV_UNLOCK(dev->mutex);
                return nread;
            }
            PipePollNotify(dev, POLLOUT);

            PIPE_DEV_UNLOCK(dev->mutex);
            ret = LOS_SemPend(dev->readSem, LOS_WAIT_FOREVER);
            if (ret != LOS_OK) {
                errno = EINVAL;
                goto ERROR;
            }
            PIPE_DEV_LOCK(dev->mutex);
        }
        nread += tmpLen;
    }
    PIPE_RW_POST(dev->writeSem);
    PIPE_DEV_UNLOCK(dev->mutex);
    PipePollNotify(dev, POLLOUT);

    return nread;
ERROR:
    return -1;
}

INT32 PipeWrite(INT32 fd, const VOID *buf, size_t len)
{
    struct PipeDev *dev = NULL;
    INT32 ret;
    size_t nwrite = 0;
    size_t tmpLen;

    if ((buf == NULL) || (len == 0)) {
        errno = EINVAL;
        return -1;
    }

    dev = PipeDevGet(fd);
    if (dev == NULL) {
        errno = ENODEV;
        return -1;
    }

    PIPE_DEV_LOCK(dev->mutex);
    while (nwrite < len) {
        tmpLen = PipeRingbufferWrite(dev, (char *)buf + nwrite, len - nwrite);
        if (tmpLen == 0) {
            PIPE_RW_POST(dev->readSem);
            PipePollNotify(dev, POLLIN);
            PIPE_RW_WAIT(dev->writeSem);

            PIPE_DEV_UNLOCK(dev->mutex);
            ret = LOS_SemPend(dev->writeSem, LOS_WAIT_FOREVER);
            if (ret != LOS_OK) {
                errno = EINVAL;
                goto ERROR;
            }
            PIPE_DEV_LOCK(dev->mutex);
        }
        nwrite += tmpLen;
    }
    PIPE_RW_POST(dev->readSem);
    PipePollNotify(dev, POLLIN);
    PIPE_DEV_UNLOCK(dev->mutex);

    return nwrite;
ERROR:
    return -1;
}

INT32 PipePoll(INT32 fd, struct PollTable *table)
{
    struct PipeDev *dev = NULL;
    struct PipeFdDev *devFd = NULL;
    UINT32 openFlag;
    INT32 mask;
    size_t nbytes;
    PollEvent event = 0;

    if (table == NULL) {
        errno = EINVAL;
        return -1;
    }

    (VOID)LOS_MuxPend(g_devFdMutex, LOS_WAIT_FOREVER);
    devFd = PipeFdDevGet(fd);
    if (devFd == NULL) {
        (VOID)LOS_MuxPost(g_devFdMutex);
        errno = ENODEV;
        return -1;
    }
    openFlag = devFd->openFlag;
    dev = devFd->dev;
    (VOID)LOS_MuxPost(g_devFdMutex);

    PIPE_DEV_LOCK(dev->mutex);
    if (dev->readIndex == dev->writeIndex) {
        if (dev->roll == TRUE) {
            nbytes = dev->bufferSize;
        } else {
            nbytes = 0;
        }
    } else if (dev->writeIndex > dev->readIndex) {
        nbytes = dev->writeIndex - dev->readIndex;
    } else {
        nbytes = dev->bufferSize - dev->readIndex + dev->writeIndex;
    }

    if (((openFlag & O_WRONLY) != 0) && (nbytes < (dev->bufferSize - 1))) {
        event |= POLLOUT;
    }

    if (((openFlag & O_WRONLY) == 0) && (nbytes > 0)) {
        event |= POLLIN;
    }

    mask = event & table->event;
    if (mask == 0) {
        PollWait(&dev->wq, table);
    }
    PIPE_DEV_UNLOCK(dev->mutex);

    return mask;
}

int pipe(int filedes[2])
{
    INT32 ret;
    CHAR devName[PIPE_DEV_NAME_MAX] = {0};

    ret = PipeDevRegister(devName, PIPE_DEV_NAME_MAX);
    if (ret < 0) {
        errno = -ret;
        return -1;
    }

    filedes[0] = open(devName, O_RDONLY);
    filedes[1] = open(devName, O_WRONLY);

    return ENOERR;
}

UINT32 OsPipeInit(VOID)
{
    UINT32 ret;

    ret = LOS_MuxCreate(&g_devListMutex);
    if (ret != LOS_OK) {
        return ret;
    }

    ret = LOS_MuxCreate(&g_devFdMutex);
    if (ret != LOS_OK) {
        LOS_MuxDelete(g_devListMutex);
        return ret;
    }

    return LOS_OK;
}
#endif

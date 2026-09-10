#include <pthread.h>
#include <errno.h>
#include <string.h>
#include <stdint.h>
#include <stdlib.h>
#include <securec.h>
#include "los_compiler.h"
#include "los_interrupt.h"
#include "los_task_pri.h"

#define _MUX_MAGIC 0xEBCFDEA0

struct RwList {
    void *ptr;
    void *next;
};

#define OS_RWLOCK_IS_LIST_EMPTY(head) ((head)->next == (void *)(head))

static inline int OsMutexIsInit(const pthread_mutex_t *mutex)
{
    return (mutex != NULL && mutex->magic == _MUX_MAGIC);
}

#define OsIntLock LOS_IntLock
#define OsIntRestore LOS_IntRestore
#define RUNNING_TASK OsCurrTaskGet()

static inline int OsRwlockMutexLock(pthread_mutex_t *mutex, const struct timespec *tp)
{
    if (tp == NULL) {
        return pthread_mutex_lock(mutex);
    }

    return pthread_mutex_timedlock(mutex, tp);
}

static inline int OsRwlockIsOwner(pthread_rwlock_t *rwl, LosTaskCB *runTsk)
{
    rwlock_node_t *head = &rwl->owner;
    rwlock_node_t *node;

    for (node = head->next; node != head; node = node->next) {
        if (node->ptr == (void *)runTsk) {
            return TRUE;
        }
    }

    return FALSE;
}

/*
 * Caller must guarantee interrupts are disabled.
 */
static int OsRwlockAddNode(pthread_rwlock_t *rwl, LosTaskCB *runTsk)
{
    struct RwList *rwListPtr;
    rwlock_node_t *ownerPtr;
    UINT32 spinIntSave;

    rwListPtr = (struct RwList *)malloc(sizeof(struct RwList) + sizeof(rwlock_node_t));
    if (rwListPtr == NULL) {
        return ENOMEM;
    }
    spinIntSave = OsSpinLockTaskRq(runTsk);
    rwListPtr->next = ((struct RwList *)&runTsk->rwList)->next;
    ((struct RwList *)&runTsk->rwList)->next = (void *)rwListPtr;
    OsSpinUnlockTaskRq(runTsk, spinIntSave);
    rwListPtr->ptr = (void *)rwl;

    ownerPtr = (rwlock_node_t *)((uintptr_t)rwListPtr + sizeof(struct RwList));
    ownerPtr->next = rwl->owner.next;
    rwl->owner.next = (void *)ownerPtr;
    ownerPtr->ptr = (void *)runTsk;
    return ENOERR;
}

static void OsRwlockRmvNode(pthread_rwlock_t *rwl, LosTaskCB *runTsk, void *rwListArg)
{
    struct RwList *rwList = (struct RwList *)rwListArg;
    struct RwList *prev = rwList;
    struct RwList *node = rwList->next;
    rwlock_node_t *ownerHead = &rwl->owner;
    rwlock_node_t *ownerPrev = ownerHead;
    rwlock_node_t *ownerNode = ownerHead->next;

    for (; node != rwList; prev = node, node = prev->next) {
        if (node->ptr == (void *)rwl) {
            break;
        }
    }
    prev->next = node->next;

    for (; ownerNode != ownerHead; ownerPrev = ownerNode, ownerNode = ownerPrev->next) {
        if (ownerNode->ptr == (void *)runTsk) {
            break;
        }
    }
    ownerPrev->next = ownerNode->next;

    free(node);
}

int pthread_rwlockattr_init(pthread_rwlockattr_t *attr)
{
    if (attr == NULL) {
        return EINVAL;
    }

    attr->__attr[0] = PTHREAD_PROCESS_PRIVATE;
    attr->__attr[1] = 0;

    return ENOERR;
}

int pthread_rwlockattr_destroy(pthread_rwlockattr_t *attr)
{
    (void)attr;

    return ENOERR;
}

int pthread_rwlockattr_setpshared(pthread_rwlockattr_t *attr, int pshared)
{
    if (attr == NULL) {
        return EINVAL;
    }
    if (pshared != PTHREAD_PROCESS_PRIVATE) {
        return ENOSYS;
    }

    attr->__attr[0] = PTHREAD_PROCESS_PRIVATE;

    return ENOERR;
}

int pthread_rwlockattr_getpshared(const pthread_rwlockattr_t *attr, int *pshared)
{
    if (attr == NULL || pshared == NULL) {
        return EINVAL;
    }

    *pshared = *(const int *)attr;
    return ENOERR;
}

int pthread_rwlock_init(pthread_rwlock_t *restrict rwl, const pthread_rwlockattr_t *restrict attr)
{
    (void)attr;
    int ret;

    if (rwl == NULL) {
        return EINVAL;
    }

    if (OsMutexIsInit(&rwl->mutex)) {
        return EBUSY;
    }

    ret = pthread_mutex_init(&rwl->mutex, NULL);
    if (ret != ENOERR) {
        return ret;
    }

    ret = pthread_cond_init(&rwl->read_cond, NULL);
    if (ret != ENOERR) {
        pthread_mutex_destroy(&rwl->mutex);
        return ret;
    }

    ret = pthread_cond_init(&rwl->write_cond, NULL);
    if (ret != ENOERR) {
        pthread_mutex_destroy(&rwl->mutex);
        pthread_cond_destroy(&rwl->read_cond);
        return ret;
    }

    rwl->pending_writers = 0;
    rwl->readers = 0;
    rwl->writer = 0;
    rwl->owner.ptr = (void *)rwl;
    rwl->owner.next = (rwlock_node_t *)&rwl->owner;

    return ENOERR;
}

int pthread_rwlock_destroy(pthread_rwlock_t *rwl)
{
    int ret;

    if (rwl == NULL || !OsMutexIsInit(&rwl->mutex)) {
        return EINVAL;
    }

    if (rwl->readers != 0 || rwl->writer != 0 || rwl->pending_writers != 0 ||
        !OS_RWLOCK_IS_LIST_EMPTY(&rwl->owner)) {
        return EBUSY;
    }

    ret = pthread_mutex_destroy(&rwl->mutex);
    if (ret != ENOERR) {
        return ret;
    }

    ret = pthread_cond_destroy(&rwl->read_cond);
    if (ret != ENOERR) {
        return ret;
    }

    return pthread_cond_destroy(&rwl->write_cond);
}

int pthread_rwlock_timedwrlock(pthread_rwlock_t *restrict rwl, const struct timespec *restrict time)
{
    int ret;
    LosTaskCB *runTsk;
    uintptr_t intSave;

    if (rwl == NULL || !OsMutexIsInit(&rwl->mutex)) {
        return EINVAL;
    }

    intSave = OsIntLock();
    runTsk = RUNNING_TASK;
    if (OsRwlockIsOwner(rwl, runTsk)) {
        OsIntRestore(intSave);
        return EDEADLK;
    }

    ret = OsRwlockMutexLock(&rwl->mutex, time);
    if (ret != ENOERR) {
        OsIntRestore(intSave);
        return ret;
    }

    rwl->pending_writers++;
    while (rwl->readers > 0 || rwl->writer != 0) {
        if (time != NULL) {
            ret = pthread_cond_timedwait(&rwl->write_cond, &rwl->mutex, time);
        } else {
            ret = pthread_cond_wait(&rwl->write_cond, &rwl->mutex);
        }
        if (ret != ENOERR) {
            rwl->pending_writers--;
            pthread_mutex_unlock(&rwl->mutex);
            OsIntRestore(intSave);
            return ret;
        }
    }

    rwl->pending_writers--;

    ret = OsRwlockAddNode(rwl, runTsk);
    if (ret != ENOERR) {
        pthread_mutex_unlock(&rwl->mutex);
        OsIntRestore(intSave);
        return ret;
    }
    rwl->writer = 1;

    pthread_mutex_unlock(&rwl->mutex);
    OsIntRestore(intSave);

    return ENOERR;
}

int pthread_rwlock_wrlock(pthread_rwlock_t *rwl)
{
    return pthread_rwlock_timedwrlock(rwl, NULL);
}

int pthread_rwlock_trywrlock(pthread_rwlock_t *rwl)
{
    int ret;
    uintptr_t intSave;
    LosTaskCB *runTsk;

    if (rwl == NULL || !OsMutexIsInit(&rwl->mutex)) {
        return EINVAL;
    }

    intSave = OsIntLock();
    runTsk = RUNNING_TASK;
    if (OsRwlockIsOwner(rwl, runTsk)) {
        OsIntRestore(intSave);
        return EDEADLK;
    }

    ret = pthread_mutex_trylock(&rwl->mutex);
    if (ret != ENOERR) {
        OsIntRestore(intSave);
        return ret;
    }

    if (rwl->readers == 0 && rwl->writer == 0) {
        ret = OsRwlockAddNode(rwl, runTsk);
        if (ret != ENOERR) {
            pthread_mutex_unlock(&rwl->mutex);
            OsIntRestore(intSave);
            return ret;
        }
        rwl->writer = 1;
        ret = ENOERR;
    } else {
        ret = EBUSY;
    }

    pthread_mutex_unlock(&rwl->mutex);
    OsIntRestore(intSave);

    return ret;
}

int pthread_rwlock_timedrdlock(pthread_rwlock_t *restrict rwl, const struct timespec *restrict time)
{
    int ret;
    uintptr_t intSave;
    LosTaskCB *runTsk;

    if (rwl == NULL || !OsMutexIsInit(&rwl->mutex)) {
        return EINVAL;
    }

    intSave = OsIntLock();
    runTsk = RUNNING_TASK;
    if (OsRwlockIsOwner(rwl, runTsk)) {
        OsIntRestore(intSave);
        return EDEADLK;
    }

    ret = OsRwlockMutexLock(&rwl->mutex, time);
    if (ret != ENOERR) {
        OsIntRestore(intSave);
        return ret;
    }

    while (rwl->writer != 0 || rwl->pending_writers > 0) {
        if (time != NULL) {
            ret = pthread_cond_timedwait(&rwl->read_cond, &rwl->mutex, time);
        } else {
            ret = pthread_cond_wait(&rwl->read_cond, &rwl->mutex);
        }
        if (ret != ENOERR) {
            pthread_mutex_unlock(&rwl->mutex);
            OsIntRestore(intSave);
            return ret;
        }
    }

    ret = OsRwlockAddNode(rwl, runTsk);
    if (ret != ENOERR) {
        pthread_mutex_unlock(&rwl->mutex);
        OsIntRestore(intSave);
        return ret;
    }
    rwl->readers++;

    pthread_mutex_unlock(&rwl->mutex);
    OsIntRestore(intSave);

    return ENOERR;
}

int pthread_rwlock_rdlock(pthread_rwlock_t *rwl)
{
    return pthread_rwlock_timedrdlock(rwl, NULL);
}

int pthread_rwlock_tryrdlock(pthread_rwlock_t *rwl)
{
    int ret;
    uintptr_t intSave;
    LosTaskCB *runTsk;

    if (rwl == NULL || !OsMutexIsInit(&rwl->mutex)) {
        return EINVAL;
    }

    intSave = OsIntLock();
    runTsk = RUNNING_TASK;
    if (OsRwlockIsOwner(rwl, runTsk)) {
        OsIntRestore(intSave);
        return EDEADLK;
    }

    ret = pthread_mutex_trylock(&rwl->mutex);
    if (ret != ENOERR) {
        OsIntRestore(intSave);
        return ret;
    }

    if (rwl->writer == 0 && rwl->pending_writers == 0) {
        ret = OsRwlockAddNode(rwl, runTsk);
        if (ret != ENOERR) {
            pthread_mutex_unlock(&rwl->mutex);
            OsIntRestore(intSave);
            return ret;
        }
        rwl->readers++;
        ret = ENOERR;
    } else {
        ret = EBUSY;
    }

    pthread_mutex_unlock(&rwl->mutex);
    OsIntRestore(intSave);

    return ret;
}

int pthread_rwlock_unlock(pthread_rwlock_t *rwl)
{
    int ret;
    uintptr_t intSave;
    LosTaskCB *runTsk;

    if (rwl == NULL || !OsMutexIsInit(&rwl->mutex)) {
        return EINVAL;
    }

    intSave = OsIntLock();
    runTsk = RUNNING_TASK;
    if (!OsRwlockIsOwner(rwl, runTsk)) {
        OsIntRestore(intSave);
        return EPERM;
    }

    ret = pthread_mutex_lock(&rwl->mutex);
    if (ret != ENOERR) {
        OsIntRestore(intSave);
        return ret;
    }

    if (rwl->writer != 0) {
        rwl->writer = 0;
        UINT32 spinIntSave = OsSpinLockTaskRq(runTsk);
        OsRwlockRmvNode(rwl, runTsk, &runTsk->rwList);
        OsSpinUnlockTaskRq(runTsk, spinIntSave);
    } else {
        rwl->readers--;
        UINT32 spinIntSave = OsSpinLockTaskRq(runTsk);
        OsRwlockRmvNode(rwl, runTsk, &runTsk->rwList);
        OsSpinUnlockTaskRq(runTsk, spinIntSave);

        if (rwl->readers > 0) {
            pthread_mutex_unlock(&rwl->mutex);
            OsIntRestore(intSave);
            return ENOERR;
        }
    }

    if (rwl->pending_writers > 0) {
        ret = pthread_cond_signal(&rwl->write_cond);
    } else {
        ret = pthread_cond_broadcast(&rwl->read_cond);
    }

    pthread_mutex_unlock(&rwl->mutex);
    OsIntRestore(intSave);

    return ret;
}

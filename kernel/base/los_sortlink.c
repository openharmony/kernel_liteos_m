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

#include "los_sortlink_pri.h"
#include "los_debug.h"
#include "los_percpu_pri.h"
#include "los_tick.h"

#ifdef LOSCFG_BASE_CORE_USE_MULTI_LIST

LITE_OS_SEC_TEXT STATIC INLINE UINT32 OsCalcExpireTime(UINT32 rollNum, UINT32 sortIndex, UINT16 curSortIndex)
{
    UINT32 expireTime;

    if (sortIndex > curSortIndex) {
        sortIndex = sortIndex - curSortIndex;
    } else {
        sortIndex = OS_TSK_SORTLINK_LEN - curSortIndex + sortIndex;
    }
    expireTime = ((rollNum - 1) << OS_TSK_SORTLINK_LOGLEN) + sortIndex;
    return expireTime;
}

UINT32 OsSortLinkInit(SortLinkAttribute *sortLinkHeader, LOS_DL_LIST *list)
{
    UINT32 index;
    LOS_DL_LIST *listObject = list;

    sortLinkHeader->sortLink = listObject;
    sortLinkHeader->cursor = 0;
#ifdef LOSCFG_KERNEL_TICKLESS_GLOBAL
    sortLinkHeader->baseTime = 0;
#endif
    for (index = 0; index < OS_TSK_SORTLINK_LEN; index++, listObject++) {
        LOS_ListInit(listObject);
    }
    return LOS_OK;
}

VOID OsSortLinkAdvanceHead(SortLinkAttribute *sortLinkHeader, UINT64 delta)
{
    if (delta == 0) {
        return;
    }
    LOS_DL_LIST *listObject = NULL;
    SortLinkList *sortList = NULL;
    UINT32 i;
    UINT32 sortIndex = (UINT32)delta & OS_TSK_SORTLINK_MASK;
    UINT32 rollNum = (UINT32)(delta >> OS_TSK_SORTLINK_LOGLEN);

    for (i = 0; i < OS_TSK_SORTLINK_LEN; i++) {
        listObject = sortLinkHeader->sortLink + ((sortLinkHeader->cursor + i) & OS_TSK_SORTLINK_MASK);
        if (listObject->pstNext != listObject) {
            sortList = LOS_DL_LIST_ENTRY(listObject->pstNext, SortLinkList, sortLinkNode);
            ROLLNUM_SUB(sortList->idxRollNum, rollNum);
            if ((i > 0) && (i <= sortIndex)) {
                ROLLNUM_DEC(sortList->idxRollNum);
            }
        }
    }
    sortLinkHeader->cursor = (sortLinkHeader->cursor + (UINT32)delta) & OS_TSK_SORTLINK_MASK;
}

VOID OsAdd2SortLink(const SortLinkAttribute *sortLinkHeader, SortLinkList *sortList)
{
    SortLinkList *listSorted = NULL;
    LOS_DL_LIST *listObject = NULL;
    UINT32 sortIndex;
    UINT32 rollNum;
    UINT32 timeout;

    /*
     * huge rollnum could cause carry to invalid high bit
     * and eventually affect the calculation of sort index.
     */
    if (sortList->idxRollNum > OS_TSK_MAX_ROLLNUM) {
        SET_SORTLIST_VALUE(sortList, OS_TSK_MAX_ROLLNUM);
    }
    timeout = (UINT32)sortList->idxRollNum;
    sortIndex = timeout & OS_TSK_SORTLINK_MASK;
    rollNum = (timeout >> OS_TSK_SORTLINK_LOGLEN) + 1;
    if (sortIndex == 0) {
        rollNum--;
    }
    EVALUATE_L(sortList->idxRollNum, rollNum);
    sortIndex = sortIndex + sortLinkHeader->cursor;
    sortIndex = sortIndex & OS_TSK_SORTLINK_MASK;
    EVALUATE_H(sortList->idxRollNum, sortIndex);

    listObject = sortLinkHeader->sortLink + sortIndex;
    if (listObject->pstNext == listObject) {
        LOS_ListTailInsert(listObject, &sortList->sortLinkNode);
    } else {
        listSorted = LOS_DL_LIST_ENTRY(listObject->pstNext, SortLinkList, sortLinkNode);
        do {
            if (ROLLNUM(listSorted->idxRollNum) <= ROLLNUM(sortList->idxRollNum)) {
                ROLLNUM_SUB(sortList->idxRollNum, listSorted->idxRollNum);
            } else {
                ROLLNUM_SUB(listSorted->idxRollNum, sortList->idxRollNum);
                break;
            }

            listSorted = LOS_DL_LIST_ENTRY(listSorted->sortLinkNode.pstNext, SortLinkList, sortLinkNode);
        } while (&listSorted->sortLinkNode != listObject);

        LOS_ListTailInsert(&listSorted->sortLinkNode, &sortList->sortLinkNode);
    }
}

VOID OsDeleteSortLink(const SortLinkAttribute *sortLinkHeader, SortLinkList *sortList)
{
    if (sortList->idxRollNum == OS_SORT_LINK_INVALID_TIME) {
        return;
    }
    LOS_DL_LIST *listObject = sortLinkHeader->sortLink + SORT_INDEX(sortList->idxRollNum);
    SortLinkList *nextSortList = NULL;

    if (listObject != sortList->sortLinkNode.pstNext) {
        nextSortList = LOS_DL_LIST_ENTRY(sortList->sortLinkNode.pstNext, SortLinkList, sortLinkNode);
        ROLLNUM_ADD(nextSortList->idxRollNum, sortList->idxRollNum);
    }
    LOS_ListDelete(&sortList->sortLinkNode);
    SET_SORTLIST_VALUE(sortList, OS_SORT_LINK_INVALID_TIME);
}

UINT64 OsSortLinkGetTargetExpireTime(const SortLinkAttribute *sortLinkHeader, const SortLinkList *targetSortList)
{
    SortLinkList *listSorted = NULL;
    LOS_DL_LIST *listObject = NULL;
    UINT32 sortIndex = SORT_INDEX(targetSortList->idxRollNum);
    UINT64 rollNum = ROLLNUM(targetSortList->idxRollNum);

    listObject = sortLinkHeader->sortLink + sortIndex;

    listSorted = LOS_DL_LIST_ENTRY(listObject->pstNext, SortLinkList, sortLinkNode);
    while (listSorted != targetSortList) {
        rollNum += ROLLNUM(listSorted->idxRollNum);
        listSorted = LOS_DL_LIST_ENTRY(listSorted->sortLinkNode.pstNext, SortLinkList, sortLinkNode);
    }
    return (UINT64)OsCalcExpireTime((UINT32)rollNum, sortIndex, sortLinkHeader->cursor);
}

STATIC UINT64 OsSortLinkGetNextExpireSortLinkList(const SortLinkAttribute *sortLinkHeader,
                                                   SortLinkList **outlistSorted)
{
    UINT16 cursor;
    UINT32 i;
    UINT64 minRollNum = OS_TSK_LOW_BITS_MASK;
    LOS_DL_LIST *listObject = NULL;
    SortLinkList *listSorted = NULL;
    UINT32 minSortIndex = 0;

    cursor = (sortLinkHeader->cursor + 1) & OS_TSK_SORTLINK_MASK;
    for (i = 0; i < OS_TSK_SORTLINK_LEN; i++) {
        listObject = sortLinkHeader->sortLink + ((cursor + i) & OS_TSK_SORTLINK_MASK);
        if (!LOS_ListEmpty(listObject)) {
            listSorted = LOS_DL_LIST_ENTRY(listObject->pstNext, SortLinkList, sortLinkNode);
            if (minRollNum > ROLLNUM(listSorted->idxRollNum)) {
                minRollNum = ROLLNUM(listSorted->idxRollNum);
                minSortIndex = (cursor + i) & OS_TSK_SORTLINK_MASK;
                *outlistSorted = listSorted;
            }
        }
    }
    if (minRollNum == OS_TSK_LOW_BITS_MASK) {
        return OS_SORT_LINK_UINT64_MAX;
    }
    return (UINT64)OsCalcExpireTime((UINT32)minRollNum, minSortIndex, sortLinkHeader->cursor);
}

UINT64 OsSortLinkGetNextExpireTime(const SortLinkAttribute *sortLinkHeader)
{
    SortLinkList *expirelistSorted = NULL;
    return OsSortLinkGetNextExpireSortLinkList(sortLinkHeader, &expirelistSorted);
}

#else /* LOSCFG_BASE_CORE_USE_SINGLE_LIST */

UINT32 OsSortLinkInit(SortLinkAttribute *sortLinkHeader, LOS_DL_LIST *list)
{
    LOS_DL_LIST *listObject = list;

    sortLinkHeader->sortLink = listObject;
    sortLinkHeader->cursor = 0;
#ifdef LOSCFG_KERNEL_TICKLESS_GLOBAL
    sortLinkHeader->baseTime = 0;
#endif
    LOS_ListInit(listObject);
    return LOS_OK;
}

VOID OsSortLinkAdvanceHead(SortLinkAttribute *sortLinkHeader, UINT64 delta)
{
    if (delta == 0) {
        return;
    }
    LOS_DL_LIST *listObject = sortLinkHeader->sortLink;
    if (LOS_ListEmpty(listObject)) {
        return;
    }

    LOS_DL_LIST *curNode = listObject->pstNext;
    while (delta > 0 && curNode != listObject) {
        SortLinkList *cur = LOS_DL_LIST_ENTRY(curNode, SortLinkList, sortLinkNode);
        if (cur->idxRollNum > delta) {
            cur->idxRollNum -= delta;
            delta = 0;
        } else {
            delta -= cur->idxRollNum;
            cur->idxRollNum = 0;
            curNode = curNode->pstNext;
        }
    }
}

VOID OsAdd2SortLink(const SortLinkAttribute *sortLinkHeader, SortLinkList *sortList)
{
    SortLinkList *listSorted = NULL;
    LOS_DL_LIST *listObject = NULL;

    if (sortList->idxRollNum > OS_TSK_MAX_ROLLNUM) {
        SET_SORTLIST_VALUE(sortList, OS_TSK_MAX_ROLLNUM);
    }

    listObject = sortLinkHeader->sortLink;
    if (listObject->pstNext == listObject) {
        LOS_ListTailInsert(listObject, &sortList->sortLinkNode);
    } else {
        listSorted = LOS_DL_LIST_ENTRY(listObject->pstNext, SortLinkList, sortLinkNode);
        do {
            if (ROLLNUM(listSorted->idxRollNum) <= ROLLNUM(sortList->idxRollNum)) {
                ROLLNUM_SUB(sortList->idxRollNum, listSorted->idxRollNum);
            } else {
                ROLLNUM_SUB(listSorted->idxRollNum, sortList->idxRollNum);
                break;
            }
            listSorted = LOS_DL_LIST_ENTRY(listSorted->sortLinkNode.pstNext, SortLinkList, sortLinkNode);
        } while (&listSorted->sortLinkNode != listObject);
        LOS_ListTailInsert(&listSorted->sortLinkNode, &sortList->sortLinkNode);
    }
}

VOID OsDeleteSortLink(const SortLinkAttribute *sortLinkHeader, SortLinkList *sortList)
{
    if (sortList->idxRollNum == OS_SORT_LINK_INVALID_TIME) {
        return;
    }
    LOS_DL_LIST *listObject = sortLinkHeader->sortLink;
    SortLinkList *nextSortList = NULL;

    if (listObject != sortList->sortLinkNode.pstNext) {
        nextSortList = LOS_DL_LIST_ENTRY(sortList->sortLinkNode.pstNext, SortLinkList, sortLinkNode);
        ROLLNUM_ADD(nextSortList->idxRollNum, sortList->idxRollNum);
    }
    LOS_ListDelete(&sortList->sortLinkNode);
    SET_SORTLIST_VALUE(sortList, OS_SORT_LINK_INVALID_TIME);
}

UINT64 OsSortLinkGetTargetExpireTime(const SortLinkAttribute *sortLinkHeader, const SortLinkList *targetSortList)
{
    SortLinkList *listSorted = NULL;
    LOS_DL_LIST *listObject = NULL;
    UINT64 rollNum = targetSortList->idxRollNum;

    listObject = sortLinkHeader->sortLink;
    listSorted = LOS_DL_LIST_ENTRY(listObject->pstNext, SortLinkList, sortLinkNode);

    while (listSorted != targetSortList) {
        rollNum += listSorted->idxRollNum;
        listSorted = LOS_DL_LIST_ENTRY(listSorted->sortLinkNode.pstNext, SortLinkList, sortLinkNode);
    }

    return rollNum;
}

STATIC SortLinkList *OsSortLinkGetNextExpireSortLinkList(const SortLinkAttribute *sortLinkHeader)
{
    LOS_DL_LIST *listObject = sortLinkHeader->sortLink;
    if (LOS_ListEmpty(listObject)) {
        return NULL;
    }
    return LOS_DL_LIST_ENTRY(listObject->pstNext, SortLinkList, sortLinkNode);
}

UINT64 OsSortLinkGetNextExpireTime(const SortLinkAttribute *sortLinkHeader)
{
    SortLinkList *listSorted = OsSortLinkGetNextExpireSortLinkList(sortLinkHeader);
    if (listSorted == NULL) {
        return OS_SORT_LINK_UINT64_MAX;
    }
    return listSorted->idxRollNum;
}

#endif /* LOSCFG_BASE_CORE_USE_MULTI_LIST */

#ifdef LOSCFG_KERNEL_TICKLESS_GLOBAL
STATIC INLINE VOID SortLinkNodeTimeUpdate(SortLinkAttribute *sortLinkHeader, UINT32 oldFreq)
{
    LOS_DL_LIST *listObject = sortLinkHeader->sortLink;

    if (LOS_ListEmpty(listObject)) {
        return;
    }

    LOS_DL_LIST *nextNode = listObject->pstNext;
    do {
        SortLinkList *listSorted = LOS_DL_LIST_ENTRY(nextNode, SortLinkList, sortLinkNode);
        listSorted->idxRollNum = OsTimeConvertFreq(listSorted->idxRollNum, oldFreq, g_sysClock);
        nextNode = nextNode->pstNext;
    } while (nextNode != listObject);
}
#else
#define SortLinkNodeTimeUpdate(head, freq) ((void)0)
#endif

VOID OsSortLinkResponseTimeConvertFreq(UINT32 oldFreq)
{
#ifdef LOSCFG_KERNEL_TICKLESS_GLOBAL
    Percpu *percpu = OsPercpuGet();
    SortLinkNodeTimeUpdate(&percpu->taskSortLink, oldFreq);

#if (LOSCFG_BASE_CORE_SWTMR == 1)
    SortLinkNodeTimeUpdate(&percpu->swtmrSortLink, oldFreq);
#endif
#else
    (void)oldFreq;
#endif
}

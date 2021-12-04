/*
 * Copyright (c) 2021-2021 Huawei Device Co., Ltd. All rights reserved.
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

#ifndef _ADAPT_MQUEUE_H
#define _ADAPT_MQUEUE_H

#include <sys/features.h>
#include <sys/types.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef int mqd_t;

struct mq_attr {
    long mq_flags, mq_maxmsg, mq_msgsize, mq_curmsgs, _unused[4];
};

struct sigevent;

int mq_close(mqd_t personal);
int mq_getattr(mqd_t personal, struct mq_attr *attr);
mqd_t mq_open(const char *mqName, int openFlag, ...);
ssize_t mq_receive(mqd_t personal, char *msg_ptr, size_t msg_len, unsigned *msg_prio);
int mq_send(mqd_t personal, const char *msg_ptr, size_t msg_len, unsigned msg_prio);
int mq_setattr(mqd_t personal, const struct mq_attr *__restrict new, struct mq_attr *__restrict old);
ssize_t mq_timedreceive(mqd_t personal, char *__restrict msg, size_t msg_len, \
                        unsigned *__restrict msg_prio, const struct timespec *__restrict absTimeout);
int mq_timedsend(mqd_t personal, const char *msg, size_t msg_len, unsigned msg_prio, const struct timespec *absTimeout);
int mq_unlink(const char *name);

#ifdef __cplusplus
}
#endif

#endif /* !_ADAPT_MQUEUE_H */

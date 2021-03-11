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

#ifndef _SYS_CAPABILITY_H
#define _SYS_CAPABILITY_H

#ifdef __cplusplus
extern "C" {
#endif
#define __NEED_pid_t
#include <bits/alltypes.h>
#include <linux/capability.h>

// posix capabilities
#define OHOS_CAP_CHOWN                       0
#define OHOS_CAP_DAC_EXECUTE                 1
#define OHOS_CAP_DAC_WRITE                   2
#define OHOS_CAP_DAC_READ_SEARCH             3
#define OHOS_CAP_FOWNER                      4
#define OHOS_CAP_KILL                        5
#define OHOS_CAP_SETGID                      6
#define OHOS_CAP_SETUID                      7

// socket capabilities
#define OHOS_CAP_NET_BIND_SERVICE            8
#define OHOS_CAP_NET_BROADCAST               9
#define OHOS_CAP_NET_ADMIN                   10
#define OHOS_CAP_NET_RAW                     11

// fs capabilities
#define OHOS_CAP_FS_MOUNT                    12
#define OHOS_CAP_FS_FORMAT                   13

// process capabilities
#define OHOS_CAP_SCHED_SETPRIORITY           14

// time capabilities
#define OHOS_CAP_SET_TIMEOFDAY               15
#define OHOS_CAP_CLOCK_SETTIME               16

// process capabilities
#define OHOS_CAP_CAPSET                      17

// reboot capability
#define OHOS_CAP_REBOOT                      18
// self deined privileged syscalls
#define OHOS_CAP_SHELL_EXEC                  19

int capget(cap_user_header_t hdr_ptr, cap_user_data_t data_ptr);
int capset(cap_user_header_t hdr_ptr, const cap_user_data_t data_ptr);
int ohos_capget(pid_t pid, unsigned int *caps);
int ohos_capset(unsigned int caps);

#ifdef __cplusplus
}
#endif

#endif /* _SYS_CAPABILITY_H */

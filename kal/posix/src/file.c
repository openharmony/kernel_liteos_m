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
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include "lwip/sockets.h"
#include "hks_client.h"

#define RANDOM_DEV_FD  LWIP_CONFIG_NUM_SOCKETS
#define RANDOM_DEV_PATH  "/dev/random"

int open(const char *file, int oflag, ...)
{
    if (strcmp(file, RANDOM_DEV_PATH) == 0) {
        if (oflag != O_RDONLY) {
            errno = EINVAL;
            return -1;
        }
        return RANDOM_DEV_FD;
    }
    errno = ENOENT;
    return -1;
}

int close(int fd)
{
    if (fd == RANDOM_DEV_FD) {
        return 0;
    }
    return closesocket(fd);
}

ssize_t read(int fd, void *buf, size_t nbytes)
{
    if (fd == RANDOM_DEV_FD) {
        if (nbytes == 0) {
            return 0;
        }
        if (buf == NULL) {
            errno = EINVAL;
            return -1;
        }
        if (nbytes > 1024) {
            nbytes = 1024; /* hks_generate_random: random_size must <= 1024 */
        }
        struct hks_blob key = {HKS_BLOB_TYPE_RAW, (uint8_t *)buf, nbytes};
        if (hks_generate_random(&key) != 0) {
            errno = EIO;
            return -1;
        }
        return (ssize_t)nbytes;
    }
    return recv(fd, buf, nbytes, 0);
}

ssize_t write(int fd, const void *buf, size_t nbytes)
{
    if (fd == RANDOM_DEV_FD) {
        errno = EBADF; /* "/dev/random" is readonly */
        return -1;
    }
    return send(fd, buf, nbytes, 0);
}

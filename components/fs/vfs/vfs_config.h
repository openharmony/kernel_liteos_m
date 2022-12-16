/*
 * Copyright (c) 2013-2019 Huawei Technologies Co., Ltd. All rights reserved.
 * Copyright (c) 2020-2022 Huawei Device Co., Ltd. All rights reserved.
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

#ifndef _VFS_CONFIG_H_
#define _VFS_CONFIG_H_

/* file descriptors configure */

#define MIN_START_FD 3 // 0,1,2 are used for stdin,stdout,stderr respectively

/* net configure */

#ifdef LOSCFG_NET_LWIP_SACK             // enable socket and net function
#include "lwip/lwipopts.h"
#define CONFIG_NSOCKET_DESCRIPTORS  LWIP_CONFIG_NUM_SOCKETS  // max numbers of socket descriptor
#else
#define CONFIG_NSOCKET_DESCRIPTORS  0
#endif

/* max numbers of other descriptors except socket descriptors */

#ifdef LOSCFG_FS_FAT
#include "fatfs_conf.h"
#endif

#ifdef LOSCFG_FS_LITTLEFS
#include "lfs_conf.h"
#endif

#ifndef CONFIG_NFILE_DESCRIPTORS
#define CONFIG_NFILE_DESCRIPTORS 256
#endif

#define NR_OPEN_DEFAULT (CONFIG_NFILE_DESCRIPTORS - MIN_START_FD)

/* time configure */

#define CONFIG_NTIME_DESCRIPTORS     0

/* mqueue configure */

#define CONFIG_NQUEUE_DESCRIPTORS    256

#define TIMER_FD_OFFSET                 (CONFIG_NFILE_DESCRIPTORS + CONFIG_NSOCKET_DESCRIPTORS)
#define CONFIG_NEXPANED_DESCRIPTORS     (CONFIG_NTIME_DESCRIPTORS + CONFIG_NQUEUE_DESCRIPTORS)
#define FD_SET_TOTAL_SIZE               (TIMER_FD_OFFSET + CONFIG_NEXPANED_DESCRIPTORS)
#define MQUEUE_FD_OFFSET                (TIMER_FD_OFFSET + CONFIG_NTIME_DESCRIPTORS)

/* directory configure */

#define VFS_USING_WORKDIR               // enable current working directory

/* permission configure */
#define DEFAULT_DIR_MODE        0777
#define DEFAULT_FILE_MODE       0666

#define MAX_DIRENT_NUM 14 // 14 means 4096 length buffer can store 14 dirent, see struct DIR

/* max number of open directories */
#ifndef LOSCFG_MAX_OPEN_DIRS
#define LOSCFG_MAX_OPEN_DIRS 10
#endif

#endif

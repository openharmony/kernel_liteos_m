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

#ifndef _ADAPT_NET_IF_ARP_H
#define _ADAPT_NET_IF_ARP_H

#include <inttypes.h>
#include <sys/types.h>
#include <sys/socket.h>

#ifdef __cplusplus
extern "C" {
#endif

#define MAX_ADDR_LEN        7

#define ARPOP_REQUEST       1
#define ARPOP_REPLY         2
#define ARPOP_RREQUEST      3
#define ARPOP_RREPLY        4
#define ARPOP_InREQUEST     8
#define ARPOP_InREPLY       9
#define ARPOP_NAK           10

#define ATF_INUSE           0x01
#define ATF_COM             0x02
#define ATF_PERM            0x04
#define ATF_PUBL            0x08
#define ATF_USETRAILERS     0x10

#define ARPD_UPDATE         0x01
#define ARPD_LOOKUP         0x02
#define ARPD_FLUSH          0x03

struct arphdr {
    uint16_t ar_hrd;
    uint16_t ar_pro;
    uint8_t ar_hln;
    uint8_t ar_pln;
    uint16_t ar_op;
};

#define ARP_DEV_LEN         16
struct arpreq {
    struct sockaddr arp_pa;
    struct sockaddr arp_ha;
    int arp_flags;
    struct sockaddr arp_netmask;
    char arp_dev[ARP_DEV_LEN];
};

struct arpreq_old {
    struct sockaddr arp_pa;
    struct sockaddr arp_ha;
    int arp_flags;
    struct sockaddr arp_netmask;
};

struct arpd_request {
    unsigned short req;
    uint32_t ip;
    unsigned long dev;
    unsigned long stamp;
    unsigned long updated;
    unsigned char ha[MAX_ADDR_LEN];
};

#ifdef __cplusplus
}
#endif

#endif /* !_ADAPT_NET_IF_ARP_H */

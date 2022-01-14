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

#ifndef _ADAPT_NETINET_IF_ETHER_H
#define _ADAPT_NETINET_IF_ETHER_H

#include <stdint.h>

#define ETH_ALEN        6
#define ETH_TLEN        2
#define ETH_HLEN        14
#define ETH_ZLEN        60
#define ETH_DATA_LEN    1500
#define ETH_FRAME_LEN   1514
#define ETH_FCS_LEN     4
#define ETH_MIN_MTU     68
#define ETH_MAX_MTU     0xFFFFU

struct ethhdr {
    uint8_t h_dest[ETH_ALEN];
    uint8_t h_source[ETH_ALEN];
    uint16_t h_proto;
};

#include <net/ethernet.h>
#include <net/if_arp.h>

struct ether_arp {
    struct arphdr ea_hdr;
    uint8_t arp_sha[ETH_ALEN];
    uint8_t arp_spa[4];
    uint8_t arp_tha[ETH_ALEN];
    uint8_t arp_tpa[4];
};

#define arp_hrd ea_hdr.ar_hrd
#define arp_pro ea_hdr.ar_pro
#define arp_hln ea_hdr.ar_hln
#define arp_pln ea_hdr.ar_pln
#define arp_op  ea_hdr.ar_op

#define ETHER_MAP_IP_MULTICAST(ipaddr, enaddr)          \
    do {                                                \
        (enaddr)[0] = 0x01;                             \
        (enaddr)[1] = 0x00;                             \
        (enaddr)[2] = 0x5e;                             \
        (enaddr)[3] = ((uint8_t *)(ipaddr))[1] & 0x7f;    \
        (enaddr)[4] = ((uint8_t *)(ipaddr))[2];           \
        (enaddr)[5] = ((uint8_t *)(ipaddr))[3];           \
    } while (0)

#endif /* !_ADAPT_NETINET_IF_ETHER_H */
